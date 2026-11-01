/* grl-fast2d-private.h
 *
 * Copyright 2026 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Fast 2D vertex path: quads written straight into rlgl's active render
 * batch. Not installed, not introspected; shared by graylib's drawing
 * code, its text layout header and libregnum's raylib GUI backend (which
 * embeds graylib statically).
 *
 * Every raylib 2D draw goes through rlgl's immediate mode: rlSetTexture(),
 * rlBegin(), rlColor4ub(), rlNormal3f(), and per vertex rlTexCoord2f()
 * plus rlVertex3f(), which checks the transform matrix and the batch
 * limit and copies the current texcoord, normal and colour into the
 * batch for every vertex. For sprite-, text- and UI-heavy frames that is
 * most of the CPU cost of drawing.
 *
 * rlgl keeps the vertex count of its active batch in private state
 * (RLGL.State.vertexCounter, used to place vertices and to size the
 * upload). raylib's sources are not modified, so the fast path works on
 * rlgl's public API and types only:
 *
 *  - graylib installs its own rlRenderBatch (rlLoadRenderBatch(), same
 *    size as raylib's default) as the active batch with
 *    rlSetRenderBatchActive(), so it knows the arrays rlgl writes to.
 *    Everything raylib draws goes into that batch as before.
 *  - rlgl's private vertex count equals the vertex slots used by the
 *    batch's draw calls (the sum of vertexCount + vertexAlignment of the
 *    finished draws plus the vertexCount of the last); rlgl itself relies
 *    on that to find each draw's vertices. graylib reads it from the
 *    public draws[] array (cached between calls).
 *  - To advance rlgl's count past the four vertices of a quad, graylib
 *    presents the last draw call to rlBegin(RL_QUADS) as a 4-vertex
 *    triangle draw: rlBegin() pads such a draw with exactly 4 alignment
 *    vertices (counter += 4) and opens a new draw call. graylib then
 *    restores the last draw call, closes the new one again and writes the
 *    quad into the four reserved slots. One rlBegin() call per quad
 *    replaces four rlVertex3f() calls and the per-attribute calls.
 *
 * A quad goes the fast way only when it can join the current draw call
 * exactly as raylib's own function would have continued it: same texture,
 * RL_QUADS mode, room in the batch without a flush, and an identity
 * rlgl transform (no rlPushMatrix() translation/rotation/scale). In every
 * other case the caller draws it with raylib's function, which handles
 * the texture switch, the flush or the transform. Draw-call splits and
 * batch flushes therefore happen at the same vertices as before, the
 * vertex values are computed with raylib's own formulas, z comes from the
 * batch depth as rlVertex2f() takes it, and the depth advances once per
 * raylib-equivalent draw like rlEnd() advances it: output is pixel
 * identical (tests/test-fast2d.c compares it with raylib's path).
 *
 * The camera of BeginMode2D() is in the modelview matrix, not the
 * transform, so drawing under a 2D camera takes the fast path.
 */

#pragma once

#include <math.h>
#include <glib.h>
#include <raylib.h>

G_BEGIN_DECLS

/*
 * grl_fast2d_begin:
 *
 * Whether quads may be written directly right now: the fast path is
 * enabled (grl_draw_set_fast_path_enabled(), GRL_FAST2D=0 in the
 * environment turns it off), a GL 2.1+/ES2+ context exists (graylib's
 * batch is installed on first use) and rlgl's transform matrix is the
 * identity. Valid until the caller changes rlgl matrix state; check
 * once per raylib-equivalent draw or text string.
 */
gboolean grl_fast2d_begin           (void);

/*
 * grl_fast2d_quads:
 * @texture_id: GL texture the draw call must use
 * @n_quads: number of quads
 * @xy: 8 floats per quad: x, y of its four corners in rlgl order
 * @uv: 8 floats per quad: texture coordinates of the same corners
 * @color: vertex colour
 *
 * Appends @n_quads quads to the current RL_QUADS draw call when it uses
 * @texture_id and all of them fit without a flush; z is the batch's
 * current depth and normals are (0, 0, 1). Returns %FALSE without
 * writing anything otherwise (the caller then draws them with raylib).
 * Only after grl_fast2d_begin() returned %TRUE.
 */
gboolean grl_fast2d_quads           (guint         texture_id,
                                     gint          n_quads,
                                     const gfloat *xy,
                                     const gfloat *uv,
                                     Color         color);

/*
 * grl_fast2d_fallback_done:
 * @texture_id: texture the raylib draw just used
 *
 * Call after drawing with raylib's function in place of quads that
 * grl_fast2d_quads() refused (grl_fast2d_begin() having returned %TRUE).
 * raylib's draw leaves the active batch's last draw call as RL_QUADS with
 * @texture_id (or flushed, at the batch limit); if graylib's batch does
 * not show that, it is no longer the active batch (another window, or
 * another batch was activated) and is dropped, to be installed again on
 * the next draw.
 */
void     grl_fast2d_fallback_done   (guint         texture_id);

/*
 * grl_fast2d_end_draw:
 * @color: colour raylib's function set last
 * @u: last texture coordinate it set (x)
 * @v: last texture coordinate it set (y)
 * @normal: whether it set the (0, 0, 1) normal
 *
 * Finishes one raylib-equivalent draw written with grl_fast2d_quads():
 * advances the batch depth as rlEnd() does and leaves rlgl's current
 * colour, texture coordinate and (when @normal) normal as raylib's
 * function leaves them, for later immediate-mode vertices that do not
 * set their own.
 */
void     grl_fast2d_end_draw        (Color         color,
                                     gfloat        u,
                                     gfloat        v,
                                     gboolean      normal);

/*
 * grl_fast2d_advance_depth:
 *
 * What rlEnd() does: advances the batch depth by 1/20000. For runs of
 * raylib-equivalent draws (text glyphs) whose attribute state is set
 * once at the end with grl_fast2d_end_draw().
 */
void     grl_fast2d_advance_depth   (void);

/*
 * grl_fast2d_get_quads_written:
 *
 * Quads written by the fast path since startup (for tests).
 */
guint64  grl_fast2d_get_quads_written (void);

/*
 * grl_fast2d_shutdown:
 *
 * Draws and unloads graylib's batch and makes raylib's default batch
 * active again. Called before CloseWindow() (GrlWindow does it); safe
 * to call when nothing is installed.
 */
void     grl_fast2d_shutdown        (void);

/*
 * Drop-in forms of raylib's functions: same arguments, same output,
 * the fast path when it applies, raylib's function otherwise.
 */
void     grl_fast2d_draw_texture_pro       (Texture2D texture,
                                            Rectangle source,
                                            Rectangle dest,
                                            Vector2   origin,
                                            gfloat    rotation,
                                            Color     tint);
void     grl_fast2d_draw_rectangle_pro     (Rectangle rec,
                                            Vector2   origin,
                                            gfloat    rotation,
                                            Color     color);
void     grl_fast2d_draw_rectangle_rec     (Rectangle rec,
                                            Color     color);
void     grl_fast2d_draw_rectangle_lines_ex (Rectangle rec,
                                            gfloat    line_thick,
                                            Color     color);
void     grl_fast2d_draw_rectangle_rounded (Rectangle rec,
                                            gfloat    roundness,
                                            gint      segments,
                                            Color     color);
void     grl_fast2d_draw_circle_v          (Vector2   center,
                                            gfloat    radius,
                                            Color     color);

/*
 * grl_fast2d_texture_quad:
 *
 * DrawTexturePro()'s corner and texture-coordinate math, in rlgl vertex
 * order (top-left, bottom-left, bottom-right, top-right). @texture must
 * be valid (id > 0).
 */
static inline void
grl_fast2d_texture_quad (const Texture2D *texture,
                         Rectangle        source,
                         Rectangle        dest,
                         Vector2          origin,
                         gfloat           rotation,
                         gfloat          *xy,
                         gfloat          *uv)
{
    gfloat   width;
    gfloat   height;
    gboolean flip_x;
    gfloat   u0;
    gfloat   u1;
    gfloat   v0;
    gfloat   v1;

    width = (gfloat) texture->width;
    height = (gfloat) texture->height;
    flip_x = FALSE;

    if (source.width < 0) { flip_x = TRUE; source.width *= -1; }
    if (source.height < 0) source.y -= source.height;

    if (dest.width < 0) dest.width *= -1;
    if (dest.height < 0) dest.height *= -1;

    if (rotation == 0.0f)
    {
        /* Not rotated: no trigonometry, as raylib */
        gfloat x = dest.x - origin.x;
        gfloat y = dest.y - origin.y;

        xy[0] = x;                xy[1] = y;
        xy[2] = x;                xy[3] = y + dest.height;
        xy[4] = x + dest.width;   xy[5] = y + dest.height;
        xy[6] = x + dest.width;   xy[7] = y;
    }
    else
    {
        gfloat sin_rotation = sinf (rotation * DEG2RAD);
        gfloat cos_rotation = cosf (rotation * DEG2RAD);
        gfloat x = dest.x;
        gfloat y = dest.y;
        gfloat dx = -origin.x;
        gfloat dy = -origin.y;

        /* top-left */
        xy[0] = x + dx * cos_rotation - dy * sin_rotation;
        xy[1] = y + dx * sin_rotation + dy * cos_rotation;
        /* bottom-left */
        xy[2] = x + dx * cos_rotation - (dy + dest.height) * sin_rotation;
        xy[3] = y + dx * sin_rotation + (dy + dest.height) * cos_rotation;
        /* bottom-right */
        xy[4] = x + (dx + dest.width) * cos_rotation - (dy + dest.height) * sin_rotation;
        xy[5] = y + (dx + dest.width) * sin_rotation + (dy + dest.height) * cos_rotation;
        /* top-right */
        xy[6] = x + (dx + dest.width) * cos_rotation - dy * sin_rotation;
        xy[7] = y + (dx + dest.width) * sin_rotation + dy * cos_rotation;
    }

    u0 = source.x / width;
    u1 = (source.x + source.width) / width;
    v0 = source.y / height;
    v1 = (source.y + source.height) / height;

    if (flip_x)
    {
        uv[0] = u1; uv[1] = v0;
        uv[2] = u1; uv[3] = v1;
        uv[4] = u0; uv[5] = v1;
        uv[6] = u0; uv[7] = v0;
    }
    else
    {
        uv[0] = u0; uv[1] = v0;
        uv[2] = u0; uv[3] = v1;
        uv[4] = u1; uv[5] = v1;
        uv[6] = u1; uv[7] = v0;
    }
}

G_END_DECLS
