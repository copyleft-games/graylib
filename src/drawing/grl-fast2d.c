/* grl-fast2d.c
 *
 * Copyright 2026 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Fast 2D vertex path: quads written straight into rlgl's active render
 * batch through rlgl's public API. The design, and why output stays
 * identical to raylib's own functions, is in grl-fast2d-private.h.
 */

#include "config.h"
#include "grl-draw.h"
#include "grl-fast2d-private.h"
#include <stdlib.h>
#include <string.h>
#include <raylib.h>
#include <rlgl.h>

/* A mode value rlgl never uses. It sits in the last draws[] slot, which
 * rlgl never draws from (reaching it flushes the batch at once) but which
 * every flush resets to RL_QUADS: while it is there, no flush happened
 * since the vertex count below was cached. */
#define GRL_FAST2D_SENTINEL   (0x47524C46)
#define GRL_FAST2D_LAST_DRAW  (RL_DEFAULT_BATCH_DRAWCALLS - 1)

/* rshapes.c's circle error rate (its default; raylib's config.h does
 * not override it), for the automatic segment count */
#define GRL_FAST2D_SMOOTH_CIRCLE_ERROR_RATE (0.5f)

/* Largest segment count whose rounded-rectangle corner table is kept */
#define GRL_FAST2D_MAX_TABLE_SEGMENTS (64)

typedef struct
{
    gboolean      enabled;        /* grl_draw_set_fast_path_enabled() */
    gboolean      env_checked;    /* GRL_FAST2D read */
    gboolean      installed;      /* batch loaded and made active */
    rlRenderBatch batch;

    /* rlgl's vertex count = prefix + draws[known_draws - 1].vertexCount,
     * prefix being the slots of draws[0 .. known_draws - 2] */
    gint          known_draws;
    gint          prefix;

    guint64       quads_written;  /* statistics for tests */
} GrlFast2dState;

static GrlFast2dState fast = { TRUE, FALSE, FALSE, { 0 }, 1, 0, 0 };

static const Matrix identity_matrix = {
    1.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 1.0f
};

/* ==========================================================================
 * Batch installation
 * ========================================================================== */

/*
 * fast2d_forget:
 *
 * Drops a batch that is no longer rlgl's active one (another window was
 * opened, or someone else activated a batch) without touching GL: its
 * buffer ids may belong to a context that is gone. The CPU arrays came
 * from rlLoadRenderBatch()'s RL_CALLOC (calloc) and are freed here.
 */
static void
fast2d_forget (void)
{
    gint i;

    if (fast.batch.vertexBuffer != NULL)
    {
        for (i = 0; i < fast.batch.bufferCount; i++)
        {
            free (fast.batch.vertexBuffer[i].vertices);
            free (fast.batch.vertexBuffer[i].texcoords);
            free (fast.batch.vertexBuffer[i].normals);
            free (fast.batch.vertexBuffer[i].colors);
            free (fast.batch.vertexBuffer[i].indices);
        }
        free (fast.batch.vertexBuffer);
    }
    free (fast.batch.draws);
    memset (&fast.batch, 0, sizeof fast.batch);
    fast.installed = FALSE;
}

/*
 * fast2d_bind_default_attributes:
 *
 * rlLoadRenderBatch() binds the vertex attributes of each buffer's VAO
 * at the locations of whatever shader is current. raylib's default batch
 * is loaded while the default shader is current; set our VAOs up the
 * same way whichever shader is current now (position, texcoord and
 * colour at the default shader's locations, the normal array left
 * unbound as in the default batch).
 */
static void
fast2d_bind_default_attributes (void)
{
    const int *locs;
    gint       i;

    locs = rlGetShaderLocsDefault ();
    if (locs == NULL)
        return;

    for (i = 0; i < fast.batch.bufferCount; i++)
    {
        rlVertexBuffer *vb = &fast.batch.vertexBuffer[i];
        unsigned char   has_vao;

        has_vao = rlEnableVertexArray (vb->vaoId);
        if (has_vao == 0)
            return;     /* attributes are bound per draw without VAOs */

        if (locs[RL_SHADER_LOC_VERTEX_POSITION] >= 0)
        {
            rlEnableVertexBuffer (vb->vboId[0]);
            rlSetVertexAttribute ((unsigned int) locs[RL_SHADER_LOC_VERTEX_POSITION],
                                  3, RL_FLOAT, false, 0, 0);
            rlEnableVertexAttribute ((unsigned int) locs[RL_SHADER_LOC_VERTEX_POSITION]);
        }
        if (locs[RL_SHADER_LOC_VERTEX_TEXCOORD01] >= 0)
        {
            rlEnableVertexBuffer (vb->vboId[1]);
            rlSetVertexAttribute ((unsigned int) locs[RL_SHADER_LOC_VERTEX_TEXCOORD01],
                                  2, RL_FLOAT, false, 0, 0);
            rlEnableVertexAttribute ((unsigned int) locs[RL_SHADER_LOC_VERTEX_TEXCOORD01]);
        }
        if (locs[RL_SHADER_LOC_VERTEX_NORMAL] >= 0)
        {
            rlEnableVertexBuffer (vb->vboId[2]);
            rlSetVertexAttribute ((unsigned int) locs[RL_SHADER_LOC_VERTEX_NORMAL],
                                  3, RL_FLOAT, false, 0, 0);
            rlEnableVertexAttribute ((unsigned int) locs[RL_SHADER_LOC_VERTEX_NORMAL]);
        }
        else
        {
            rlDisableVertexAttribute (RL_DEFAULT_SHADER_ATTRIB_LOCATION_NORMAL);
        }
        if (locs[RL_SHADER_LOC_VERTEX_COLOR] >= 0)
        {
            rlEnableVertexBuffer (vb->vboId[3]);
            rlSetVertexAttribute ((unsigned int) locs[RL_SHADER_LOC_VERTEX_COLOR],
                                  4, RL_UNSIGNED_BYTE, true, 0, 0);
            rlEnableVertexAttribute ((unsigned int) locs[RL_SHADER_LOC_VERTEX_COLOR]);
        }
        rlDisableVertexArray ();
        rlDisableVertexBuffer ();
    }
}

/*
 * fast2d_install:
 *
 * Loads graylib's batch (raylib's default size) and makes it rlgl's
 * active batch. The current batch is drawn first, and rlLoadRenderBatch()
 * zeroes rlgl's vertex count, so that must come after the flush.
 */
static gboolean
fast2d_install (void)
{
    unsigned char ready;
    int           version;

    ready = IsWindowReady ();
    if (ready == 0)
        return FALSE;

    version = rlGetVersion ();
    if (version == RL_OPENGL_SOFTWARE || version == RL_OPENGL_11)
        return FALSE;

    rlDrawRenderBatchActive ();
    fast.batch = rlLoadRenderBatch (RL_DEFAULT_BATCH_BUFFERS,
                                    RL_DEFAULT_BATCH_BUFFER_ELEMENTS);
    if (fast.batch.vertexBuffer == NULL || fast.batch.draws == NULL)
    {
        memset (&fast.batch, 0, sizeof fast.batch);
        return FALSE;
    }
    fast2d_bind_default_attributes ();
    rlSetRenderBatchActive (&fast.batch);

    fast.installed = TRUE;
    fast.known_draws = 1;
    fast.prefix = 0;
    fast.batch.draws[GRL_FAST2D_LAST_DRAW].mode = GRL_FAST2D_SENTINEL;
    return TRUE;
}

void
grl_fast2d_shutdown (void)
{
    if (!fast.installed)
        return;

    rlSetRenderBatchActive (NULL);
    rlUnloadRenderBatch (fast.batch);
    memset (&fast.batch, 0, sizeof fast.batch);
    fast.installed = FALSE;
}

/* ==========================================================================
 * Core
 * ========================================================================== */

gboolean
grl_fast2d_begin (void)
{
    Matrix transform;

    if (!fast.env_checked)
    {
        const gchar *env = g_getenv ("GRL_FAST2D");

        fast.env_checked = TRUE;
        if (env != NULL && g_strcmp0 (env, "0") == 0)
            fast.enabled = FALSE;
    }
    if (!fast.enabled)
        return FALSE;
    if (!fast.installed && !fast2d_install ())
        return FALSE;

    /* rlVertex3f() multiplies by the transform while rlPushMatrix() is in
     * effect. With an identity transform that changes no value, so only a
     * real transform sends the caller to raylib's path. */
    transform = rlGetMatrixTransform ();
    return memcmp (&transform, &identity_matrix, sizeof transform) == 0;
}

/*
 * fast2d_vertex_count:
 *
 * rlgl's private vertex count, from the draw calls: finished draws use
 * vertexCount + vertexAlignment slots, the open one vertexCount. The sum
 * over finished draws is cached and extended when draws were added;
 * after a flush (sentinel gone) it starts again from the first draw.
 */
static gint
fast2d_vertex_count (rlRenderBatch *b)
{
    gint draws = b->drawCounter;

    if (b->draws[GRL_FAST2D_LAST_DRAW].mode != GRL_FAST2D_SENTINEL ||
        draws < fast.known_draws)
    {
        fast.known_draws = 1;
        fast.prefix = 0;
        b->draws[GRL_FAST2D_LAST_DRAW].mode = GRL_FAST2D_SENTINEL;
    }
    while (fast.known_draws < draws)
    {
        const rlDrawCall *done = &b->draws[fast.known_draws - 1];

        fast.prefix += done->vertexCount + done->vertexAlignment;
        fast.known_draws++;
    }
    return fast.prefix + b->draws[draws - 1].vertexCount;
}

/*
 * fast2d_reserve_quad:
 *
 * Advances rlgl's vertex count by 4 through rlBegin(): with the last
 * draw call shown as a 4-vertex RL_TRIANGLES draw, rlBegin(RL_QUADS)
 * pads it with 4 alignment vertices and opens a new draw. The new draw is
 * closed again and the last one restored. The caller made sure no flush
 * can happen (room for the 4 vertices, a free draw slot below the
 * sentinel). If the batch is not rlgl's active one, rlBegin() acted on
 * the real one (a harmless rlBegin(RL_QUADS) without vertices) and our
 * draw counter did not move: returns FALSE.
 */
static gboolean
fast2d_reserve_quad (rlRenderBatch *b,
                     gint           draws)
{
    rlDrawCall *last = &b->draws[draws - 1];
    rlDrawCall  saved = *last;

    last->mode = RL_TRIANGLES;
    last->vertexCount = 4;
    rlBegin (RL_QUADS);

    if (b->drawCounter != draws + 1)
    {
        *last = saved;
        return FALSE;
    }

    b->drawCounter = draws;
    *last = saved;
    return TRUE;
}

gboolean
grl_fast2d_quads (guint         texture_id,
                  gint          n_quads,
                  const gfloat *xy,
                  const gfloat *uv,
                  Color         color)
{
    rlRenderBatch  *b = &fast.batch;
    rlDrawCall     *last;
    rlVertexBuffer *vb;
    gint            draws;
    gint            count;
    gint            capacity;
    gint            q;
    gint            k;
    gfloat          z;
    gfloat         *pos;
    gfloat         *tex;
    gfloat         *nrm;
    unsigned char  *col;

    if (!fast.installed || n_quads <= 0)
        return FALSE;

    /* Only a continuation of the current RL_QUADS draw with this texture;
     * raylib opens new draws (alignment, draw limit) itself. */
    draws = b->drawCounter;
    last = &b->draws[draws - 1];
    if (last->mode != RL_QUADS || last->textureId != texture_id)
        return FALSE;
    if (draws >= GRL_FAST2D_LAST_DRAW)
        return FALSE;

    /* Room without a flush: raylib would flush before (or right after)
     * the vertex that reaches the end; leave those quads to it so the
     * flush lands where it always did. */
    vb = &b->vertexBuffer[b->currentBuffer];
    capacity = vb->elementCount * 4;
    count = fast2d_vertex_count (b);
    if (count + 4 * n_quads >= capacity)
        return FALSE;

    /* Reserve: the first reservation also proves the batch is active */
    if (!fast2d_reserve_quad (b, draws))
    {
        fast2d_forget ();
        return FALSE;
    }
    for (q = 1; q < n_quads; q++)
        fast2d_reserve_quad (b, draws);
    last->vertexCount += 4 * n_quads;
    fast.quads_written += (guint64) n_quads;

    /* Write what rlVertex2f() would: position at the batch depth, the
     * texture coordinate, the (0, 0, 1) normal, the colour */
    z = b->currentDepth;
    pos = vb->vertices + 3 * count;
    tex = vb->texcoords + 2 * count;
    nrm = vb->normals + 3 * count;
    col = vb->colors + 4 * count;
    for (k = 0; k < 4 * n_quads; k++)
    {
        pos[0] = xy[0];
        pos[1] = xy[1];
        pos[2] = z;
        tex[0] = uv[0];
        tex[1] = uv[1];
        nrm[0] = 0.0f;
        nrm[1] = 0.0f;
        nrm[2] = 1.0f;
        col[0] = color.r;
        col[1] = color.g;
        col[2] = color.b;
        col[3] = color.a;
        pos += 3;
        tex += 2;
        nrm += 3;
        col += 4;
        xy += 2;
        uv += 2;
    }
    return TRUE;
}

void
grl_fast2d_fallback_done (guint texture_id)
{
    rlRenderBatch    *b = &fast.batch;
    const rlDrawCall *last;

    if (!fast.installed)
        return;

    /* Expected: raylib's draw ended in our batch as RL_QUADS/@texture_id */
    last = &b->draws[b->drawCounter - 1];
    if (last->mode == RL_QUADS && last->textureId == texture_id)
        return;

    /* Anything else with content: raylib drew into another batch */
    if (b->drawCounter != 1 || last->vertexCount != 0)
    {
        fast2d_forget ();
        return;
    }

    /* Empty: either raylib's draw filled our batch and flushed it (its
     * rlSetTexture (0) does that at the limit), or ours was flushed when
     * another batch was activated and never touched since. A flush of the
     * active batch resets the sentinel slot: arm it and make rlgl flush
     * its active batch (an overflow check that cannot pass). Ours being
     * empty, that flush draws nothing if it is ours. */
    b->draws[GRL_FAST2D_LAST_DRAW].mode = GRL_FAST2D_SENTINEL;
    rlCheckRenderBatchLimit (G_MAXINT / 2);
    if (b->draws[GRL_FAST2D_LAST_DRAW].mode == GRL_FAST2D_SENTINEL)
    {
        fast2d_forget ();
        return;
    }
    fast.known_draws = 1;
    fast.prefix = 0;
    b->draws[GRL_FAST2D_LAST_DRAW].mode = GRL_FAST2D_SENTINEL;
}

guint64
grl_fast2d_get_quads_written (void)
{
    return fast.quads_written;
}

void
grl_fast2d_advance_depth (void)
{
    /* rlEnd()'s increment */
    fast.batch.currentDepth += (1.0f / 20000.0f);
}

void
grl_fast2d_end_draw (Color    color,
                     gfloat   u,
                     gfloat   v,
                     gboolean normal)
{
    grl_fast2d_advance_depth ();
    rlColor4ub (color.r, color.g, color.b, color.a);
    if (normal)
        rlNormal3f (0.0f, 0.0f, 1.0f);
    rlTexCoord2f (u, v);
}

/* ==========================================================================
 * Drop-in draws
 * ========================================================================== */

void
grl_fast2d_draw_texture_pro (Texture2D texture,
                             Rectangle source,
                             Rectangle dest,
                             Vector2   origin,
                             gfloat    rotation,
                             Color     tint)
{
    gfloat xy[8];
    gfloat uv[8];

    if (texture.id > 0 && grl_fast2d_begin ())
    {
        grl_fast2d_texture_quad (&texture, source, dest, origin, rotation, xy, uv);
        if (grl_fast2d_quads (texture.id, 1, xy, uv, tint))
        {
            grl_fast2d_end_draw (tint, uv[6], uv[7], TRUE);
            return;
        }
        DrawTexturePro (texture, source, dest, origin, rotation, tint);
        grl_fast2d_fallback_done (texture.id);
        return;
    }
    DrawTexturePro (texture, source, dest, origin, rotation, tint);
}

/*
 * fast2d_shape_uv:
 *
 * The four texture coordinates raylib's shapes take from the shapes
 * texture rectangle: left, top, right, bottom.
 */
static void
fast2d_shape_uv (const Texture2D *tex,
                 const Rectangle *rect,
                 gfloat          *u0,
                 gfloat          *v0,
                 gfloat          *u1,
                 gfloat          *v1)
{
    *u0 = rect->x / tex->width;
    *v0 = rect->y / tex->height;
    *u1 = (rect->x + rect->width) / tex->width;
    *v1 = (rect->y + rect->height) / tex->height;
}

void
grl_fast2d_draw_rectangle_pro (Rectangle rec,
                               Vector2   origin,
                               gfloat    rotation,
                               Color     color)
{
    Texture2D shapes;
    Rectangle shape_rect;
    gfloat    xy[8];
    gfloat    uv[8];
    gfloat    u0, v0, u1, v1;

    if (grl_fast2d_begin ())
    {
        /* DrawRectanglePro()'s corners */
        if (rotation == 0.0f)
        {
            gfloat x = rec.x - origin.x;
            gfloat y = rec.y - origin.y;

            xy[0] = x;               xy[1] = y;
            xy[2] = x;               xy[3] = y + rec.height;
            xy[4] = x + rec.width;   xy[5] = y + rec.height;
            xy[6] = x + rec.width;   xy[7] = y;
        }
        else
        {
            gfloat sin_rotation = sinf (rotation * DEG2RAD);
            gfloat cos_rotation = cosf (rotation * DEG2RAD);
            gfloat x = rec.x;
            gfloat y = rec.y;
            gfloat dx = -origin.x;
            gfloat dy = -origin.y;

            xy[0] = x + dx * cos_rotation - dy * sin_rotation;
            xy[1] = y + dx * sin_rotation + dy * cos_rotation;
            xy[2] = x + dx * cos_rotation - (dy + rec.height) * sin_rotation;
            xy[3] = y + dx * sin_rotation + (dy + rec.height) * cos_rotation;
            xy[4] = x + (dx + rec.width) * cos_rotation - (dy + rec.height) * sin_rotation;
            xy[5] = y + (dx + rec.width) * sin_rotation + (dy + rec.height) * cos_rotation;
            xy[6] = x + (dx + rec.width) * cos_rotation - dy * sin_rotation;
            xy[7] = y + (dx + rec.width) * sin_rotation + dy * cos_rotation;
        }

        shapes = GetShapesTexture ();
        shape_rect = GetShapesTextureRectangle ();
        fast2d_shape_uv (&shapes, &shape_rect, &u0, &v0, &u1, &v1);
        uv[0] = u0; uv[1] = v0;
        uv[2] = u0; uv[3] = v1;
        uv[4] = u1; uv[5] = v1;
        uv[6] = u1; uv[7] = v0;

        if (grl_fast2d_quads (shapes.id, 1, xy, uv, color))
        {
            grl_fast2d_end_draw (color, u1, v0, TRUE);
            return;
        }
        DrawRectanglePro (rec, origin, rotation, color);
        grl_fast2d_fallback_done (shapes.id);
        return;
    }
    DrawRectanglePro (rec, origin, rotation, color);
}

void
grl_fast2d_draw_rectangle_rec (Rectangle rec,
                               Color     color)
{
    Vector2 origin = { 0.0f, 0.0f };

    grl_fast2d_draw_rectangle_pro (rec, origin, 0.0f, color);
}

void
grl_fast2d_draw_rectangle_lines_ex (Rectangle rec,
                                    gfloat    line_thick,
                                    Color     color)
{
    Rectangle top;
    Rectangle bottom;
    Rectangle left;
    Rectangle right;

    /* DrawRectangleLinesEx(): four rectangles */
    if ((line_thick > rec.width) || (line_thick > rec.height))
    {
        if (rec.width >= rec.height) line_thick = rec.height / 2;
        else if (rec.width <= rec.height) line_thick = rec.width / 2;
    }

    top.x = rec.x;
    top.y = rec.y;
    top.width = rec.width;
    top.height = line_thick;

    bottom.x = rec.x;
    bottom.y = rec.y - line_thick + rec.height;
    bottom.width = rec.width;
    bottom.height = line_thick;

    left.x = rec.x;
    left.y = rec.y + line_thick;
    left.width = line_thick;
    left.height = rec.height - line_thick * 2.0f;

    right.x = rec.x - line_thick + rec.width;
    right.y = rec.y + line_thick;
    right.width = line_thick;
    right.height = rec.height - line_thick * 2.0f;

    grl_fast2d_draw_rectangle_rec (top, color);
    grl_fast2d_draw_rectangle_rec (bottom, color);
    grl_fast2d_draw_rectangle_rec (left, color);
    grl_fast2d_draw_rectangle_rec (right, color);
}

/* ==========================================================================
 * Arc tables
 *
 * DrawRectangleRounded() and DrawCircleSector() call cosf()/sinf() three
 * times per quad for angles that depend only on the segment count. The
 * tables hold the same cosf()/sinf() results of the same float angle
 * expressions, in the order the quads use them, so vertices are bit for
 * bit raylib's.
 * ========================================================================== */

typedef struct
{
    gint    segments;
    gint    per_corner;     /* (cos, sin) pairs per corner */
    gfloat *pairs;          /* 4 corners * per_corner * 2 */
} GrlFast2dArcTable;

static GrlFast2dArcTable *rounded_tables[GRL_FAST2D_MAX_TABLE_SEGMENTS + 1];
static GrlFast2dArcTable *circle_table;

/*
 * fast2d_arc_fill:
 *
 * Fills the pairs one arc of @segments steps from @start uses, in the
 * order of raylib's quad loop: per pair of segments the angles
 * angle + 2 * step, angle + step, angle, then angle += 2 * step; an odd
 * segment adds angle + step and angle.
 */
static void
fast2d_arc_fill (gfloat *out,
                 gfloat  start,
                 gfloat  step,
                 gint    segments)
{
    gfloat angle = start;
    gint   i;

    for (i = 0; i < segments / 2; i++)
    {
        *out++ = cosf (DEG2RAD * (angle + step * 2));
        *out++ = sinf (DEG2RAD * (angle + step * 2));
        *out++ = cosf (DEG2RAD * (angle + step));
        *out++ = sinf (DEG2RAD * (angle + step));
        *out++ = cosf (DEG2RAD * angle);
        *out++ = sinf (DEG2RAD * angle);
        angle += (step * 2);
    }
    if (segments % 2)
    {
        *out++ = cosf (DEG2RAD * (angle + step));
        *out++ = sinf (DEG2RAD * (angle + step));
        *out++ = cosf (DEG2RAD * angle);
        *out++ = sinf (DEG2RAD * angle);
    }
}

static GrlFast2dArcTable *
fast2d_rounded_table (gint segments)
{
    static const gfloat angles[4] = { 180.0f, 270.0f, 0.0f, 90.0f };
    GrlFast2dArcTable  *table;
    gfloat              step;
    gint                k;

    if (segments > GRL_FAST2D_MAX_TABLE_SEGMENTS)
        return NULL;
    if (rounded_tables[segments] != NULL)
        return rounded_tables[segments];

    table = g_new0 (GrlFast2dArcTable, 1);
    table->segments = segments;
    table->per_corner = (segments / 2) * 3 + (segments % 2) * 2;
    table->pairs = g_new (gfloat, 4 * table->per_corner * 2);
    step = 90.0f / (float) segments;
    for (k = 0; k < 4; k++)
        fast2d_arc_fill (table->pairs + k * table->per_corner * 2,
                         angles[k], step, segments);
    rounded_tables[segments] = table;
    return table;
}

/*
 * fast2d_arc_quads:
 *
 * Writes the quads of one arc around @center (DrawRectangleRounded()'s
 * corner, DrawCircleSector()'s fan) into @xy/@uv from table pairs.
 * Returns the number of quads.
 */
static gint
fast2d_arc_quads (const gfloat *pairs,
                  gint          segments,
                  Vector2       center,
                  gfloat        radius,
                  gfloat        u0,
                  gfloat        v0,
                  gfloat        u1,
                  gfloat        v1,
                  gfloat       *xy,
                  gfloat       *uv)
{
    gint i;
    gint quads = 0;

    for (i = 0; i < segments / 2; i++)
    {
        xy[0] = center.x;                         xy[1] = center.y;
        xy[2] = center.x + pairs[0] * radius;     xy[3] = center.y + pairs[1] * radius;
        xy[4] = center.x + pairs[2] * radius;     xy[5] = center.y + pairs[3] * radius;
        xy[6] = center.x + pairs[4] * radius;     xy[7] = center.y + pairs[5] * radius;
        uv[0] = u0; uv[1] = v0;
        uv[2] = u1; uv[3] = v0;
        uv[4] = u1; uv[5] = v1;
        uv[6] = u0; uv[7] = v1;
        pairs += 6;
        xy += 8;
        uv += 8;
        quads++;
    }
    if (segments % 2)
    {
        xy[0] = center.x;                         xy[1] = center.y;
        xy[2] = center.x + pairs[0] * radius;     xy[3] = center.y + pairs[1] * radius;
        xy[4] = center.x + pairs[2] * radius;     xy[5] = center.y + pairs[3] * radius;
        xy[6] = center.x;                         xy[7] = center.y;
        uv[0] = u0; uv[1] = v0;
        uv[2] = u1; uv[3] = v1;
        uv[4] = u0; uv[5] = v1;
        uv[6] = u1; uv[7] = v0;
        quads++;
    }
    return quads;
}

/* fast2d_rect_quad: one axis-aligned quad through four given points */
static void
fast2d_rect_quad (Vector2  a,
                  Vector2  b,
                  Vector2  c,
                  Vector2  d,
                  gfloat   u0,
                  gfloat   v0,
                  gfloat   u1,
                  gfloat   v1,
                  gfloat  *xy,
                  gfloat  *uv)
{
    xy[0] = a.x; xy[1] = a.y;
    xy[2] = b.x; xy[3] = b.y;
    xy[4] = c.x; xy[5] = c.y;
    xy[6] = d.x; xy[7] = d.y;
    uv[0] = u0; uv[1] = v0;
    uv[2] = u0; uv[3] = v1;
    uv[4] = u1; uv[5] = v1;
    uv[6] = u1; uv[7] = v0;
}

void
grl_fast2d_draw_rectangle_rounded (Rectangle rec,
                                   gfloat    roundness,
                                   gint      segments,
                                   Color     color)
{
    GrlFast2dArcTable  *table;
    Texture2D           shapes;
    Rectangle           shape_rect;
    Vector2             point[12];
    Vector2             centers[4];
    gfloat              radius;
    gfloat              u0, v0, u1, v1;
    gfloat              xy[8 * (4 * (GRL_FAST2D_MAX_TABLE_SEGMENTS / 2 + 1) + 5)];
    gfloat              uv[8 * (4 * (GRL_FAST2D_MAX_TABLE_SEGMENTS / 2 + 1) + 5)];
    gint                quads;
    gint                k;

    /* DrawRectangleRounded()'s early outs and segment count */
    if (roundness <= 0.0f)
    {
        grl_fast2d_draw_rectangle_rec (rec, color);
        return;
    }
    if (roundness >= 1.0f) roundness = 1.0f;

    radius = (rec.width > rec.height) ? (rec.height * roundness) / 2 : (rec.width * roundness) / 2;
    if (radius <= 0.0f)
        return;

    if (segments < 4)
    {
        gfloat th = acosf (2 * powf (1 - GRL_FAST2D_SMOOTH_CIRCLE_ERROR_RATE / radius, 2) - 1);

        segments = (gint) (ceilf (2 * PI / th) / 4.0f);
        if (segments <= 0) segments = 4;
    }

    table = NULL;
    if (grl_fast2d_begin ())
        table = fast2d_rounded_table (segments);
    if (table == NULL)
    {
        DrawRectangleRounded (rec, roundness, segments, color);
        return;
    }

    /* The 12 points: P0..P7 on the outline, P8..P11 the corner centres */
    point[0].x = (float) rec.x + radius;               point[0].y = rec.y;
    point[1].x = (float) (rec.x + rec.width) - radius; point[1].y = rec.y;
    point[2].x = rec.x + rec.width;                    point[2].y = (float) rec.y + radius;
    point[3].x = rec.x + rec.width;                    point[3].y = (float) (rec.y + rec.height) - radius;
    point[4].x = (float) (rec.x + rec.width) - radius; point[4].y = rec.y + rec.height;
    point[5].x = (float) rec.x + radius;               point[5].y = rec.y + rec.height;
    point[6].x = rec.x;                                point[6].y = (float) (rec.y + rec.height) - radius;
    point[7].x = rec.x;                                point[7].y = (float) rec.y + radius;
    point[8].x = (float) rec.x + radius;               point[8].y = (float) rec.y + radius;
    point[9].x = (float) (rec.x + rec.width) - radius; point[9].y = (float) rec.y + radius;
    point[10].x = (float) (rec.x + rec.width) - radius; point[10].y = (float) (rec.y + rec.height) - radius;
    point[11].x = (float) rec.x + radius;              point[11].y = (float) (rec.y + rec.height) - radius;
    centers[0] = point[8];
    centers[1] = point[9];
    centers[2] = point[10];
    centers[3] = point[11];

    shapes = GetShapesTexture ();
    shape_rect = GetShapesTextureRectangle ();
    fast2d_shape_uv (&shapes, &shape_rect, &u0, &v0, &u1, &v1);

    /* Corners [1] [3] [5] [7], then rectangles [2] [4] [6] [8] [9] */
    quads = 0;
    for (k = 0; k < 4; k++)
        quads += fast2d_arc_quads (table->pairs + k * table->per_corner * 2,
                                   segments, centers[k], radius, u0, v0, u1, v1,
                                   xy + 8 * quads, uv + 8 * quads);
    fast2d_rect_quad (point[0], point[8], point[9], point[1], u0, v0, u1, v1,
                      xy + 8 * quads, uv + 8 * quads);
    quads++;
    fast2d_rect_quad (point[2], point[9], point[10], point[3], u0, v0, u1, v1,
                      xy + 8 * quads, uv + 8 * quads);
    quads++;
    fast2d_rect_quad (point[11], point[5], point[4], point[10], u0, v0, u1, v1,
                      xy + 8 * quads, uv + 8 * quads);
    quads++;
    fast2d_rect_quad (point[7], point[6], point[11], point[8], u0, v0, u1, v1,
                      xy + 8 * quads, uv + 8 * quads);
    quads++;
    fast2d_rect_quad (point[8], point[11], point[10], point[9], u0, v0, u1, v1,
                      xy + 8 * quads, uv + 8 * quads);
    quads++;

    /* raylib sets no normal here; the colour and the middle rectangle's
     * last texture coordinate are what it leaves behind */
    if (grl_fast2d_quads (shapes.id, quads, xy, uv, color))
    {
        grl_fast2d_end_draw (color, u1, v0, FALSE);
        return;
    }
    DrawRectangleRounded (rec, roundness, segments, color);
    grl_fast2d_fallback_done (shapes.id);
}

void
grl_fast2d_draw_circle_v (Vector2 center,
                          gfloat  radius,
                          Color   color)
{
    Texture2D shapes;
    Rectangle shape_rect;
    gfloat    xy[8 * 18];
    gfloat    uv[8 * 18];
    gfloat    u0, v0, u1, v1;
    gint      quads;

    /* DrawCircleV() is DrawCircleSector (center, radius, 0, 360, 36):
     * 36 segments (above the 4 minimum), 10 degree steps, 18 quads */
    if (!grl_fast2d_begin ())
    {
        DrawCircleV (center, radius, color);
        return;
    }
    if (radius <= 0.0f) radius = 0.1f;

    if (circle_table == NULL)
    {
        circle_table = g_new0 (GrlFast2dArcTable, 1);
        circle_table->segments = 36;
        circle_table->per_corner = 18 * 3;
        circle_table->pairs = g_new (gfloat, circle_table->per_corner * 2);
        fast2d_arc_fill (circle_table->pairs, 0.0f,
                         (360.0f - 0.0f) / (float) 36, 36);
    }

    shapes = GetShapesTexture ();
    shape_rect = GetShapesTextureRectangle ();
    fast2d_shape_uv (&shapes, &shape_rect, &u0, &v0, &u1, &v1);
    quads = fast2d_arc_quads (circle_table->pairs, 36, center, radius,
                              u0, v0, u1, v1, xy, uv);

    /* No normal is set; the last texture coordinate is the last fan
     * vertex's (left, bottom) */
    if (grl_fast2d_quads (shapes.id, quads, xy, uv, color))
    {
        grl_fast2d_end_draw (color, u0, v1, FALSE);
        return;
    }
    DrawCircleV (center, radius, color);
    grl_fast2d_fallback_done (shapes.id);
}

/* ==========================================================================
 * Public switch
 * ========================================================================== */

/**
 * grl_draw_set_fast_path_enabled:
 * @enabled: whether 2D draws may use the fast vertex path
 *
 * Turns graylib's fast 2D vertex path on or off. With it on (the
 * default) texture, rectangle, rounded-rectangle, circle and text draws
 * write their quads directly into rlgl's render batch when they continue
 * the current draw call under an identity rlgl transform; output is the
 * same as raylib's own functions. Turning it off sends every draw
 * through raylib's immediate-mode functions, for comparisons or code that
 * manipulates rlgl's batch itself. The environment variable
 * `GRL_FAST2D=0` turns it off at startup.
 */
void
grl_draw_set_fast_path_enabled (gboolean enabled)
{
    fast.env_checked = TRUE;
    fast.enabled = enabled ? TRUE : FALSE;
}

/**
 * grl_draw_get_fast_path_enabled:
 *
 * Whether graylib's fast 2D vertex path is enabled, see
 * grl_draw_set_fast_path_enabled().
 *
 * Returns: %TRUE when 2D draws may use the fast path
 */
gboolean
grl_draw_get_fast_path_enabled (void)
{
    if (!fast.env_checked)
    {
        const gchar *env = g_getenv ("GRL_FAST2D");

        fast.env_checked = TRUE;
        if (env != NULL && g_strcmp0 (env, "0") == 0)
            fast.enabled = FALSE;
    }
    return fast.enabled;
}
