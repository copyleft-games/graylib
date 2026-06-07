/* grl-path.c
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Retained 2D vector path implementation.
 */

#include "config.h"
#include "grl-path.h"
#include "grl-image.h"
#include "../math/grl-matrix.h"
#include <math.h>
#include <string.h>
#include <stdlib.h>

/* =============================================================================
 * Internal data structures
 * =============================================================================
 */

/* Path verb codes */
typedef enum
{
    GRL_PATH_VERB_MOVE  = 0,
    GRL_PATH_VERB_LINE  = 1,
    GRL_PATH_VERB_CUBIC = 2,
    GRL_PATH_VERB_QUAD  = 3,
    GRL_PATH_VERB_CLOSE = 4
} GrlPathVerb;

/* A single subpath: verb list, corresponding points, and closed flag. */
typedef struct
{
    GArray   *verbs;    /* GArray<GrlPathVerb> */
    GArray   *points;   /* GArray<GrlVector2> — same ordering as SVG */
    gboolean  closed;
    gfloat    start_x;
    gfloat    start_y;
    gfloat    cur_x;
    gfloat    cur_y;
} GrlSubpath;

/* Flat cache per path. Rebuilt lazily. */
typedef struct
{
    GrlVector2 *pts;           /* concatenated points for all subpaths */
    guint      *lengths;       /* point count per subpath */
    guint       n_subpaths;
    guint       total_points;
    gfloat      tolerance;
    gboolean    valid;
} GrlPathFlat;

struct _GrlPath
{
    GObject     parent_instance;
    GArray     *subpaths;   /* GArray<GrlSubpath> */
    GrlPathFlat flat;       /* lazily rebuilt flat cache */
};

G_DEFINE_TYPE (GrlPath, grl_path, G_TYPE_OBJECT)

/* =============================================================================
 * Helpers
 * =============================================================================
 */

static void
grl_subpath_clear (GrlSubpath *sp)
{
    g_clear_pointer (&sp->verbs,  g_array_unref);
    g_clear_pointer (&sp->points, g_array_unref);
}

static void
grl_subpath_init (GrlSubpath *sp, gfloat x, gfloat y)
{
    sp->verbs  = g_array_new (FALSE, FALSE, sizeof (GrlPathVerb));
    sp->points = g_array_new (FALSE, FALSE, sizeof (GrlVector2));
    sp->closed = FALSE;
    sp->start_x = x;
    sp->start_y = y;
    sp->cur_x = x;
    sp->cur_y = y;

    {
        GrlPathVerb v = GRL_PATH_VERB_MOVE;
        GrlVector2 pt;
        pt.x = x;
        pt.y = y;
        g_array_append_val (sp->verbs,  v);
        g_array_append_val (sp->points, pt);
    }
}

static void
grl_subpath_copy_into (const GrlSubpath *src, GrlSubpath *dst)
{
    guint i;
    GrlPathVerb v;
    GrlVector2 pt;

    dst->verbs  = g_array_new (FALSE, FALSE, sizeof (GrlPathVerb));
    dst->points = g_array_new (FALSE, FALSE, sizeof (GrlVector2));
    dst->closed = src->closed;
    dst->start_x = src->start_x;
    dst->start_y = src->start_y;
    dst->cur_x = src->cur_x;
    dst->cur_y = src->cur_y;

    for (i = 0; i < src->verbs->len; i++)
    {
        v = g_array_index (src->verbs, GrlPathVerb, i);
        g_array_append_val (dst->verbs, v);
    }

    for (i = 0; i < src->points->len; i++)
    {
        pt = g_array_index (src->points, GrlVector2, i);
        g_array_append_val (dst->points, pt);
    }
}

static void
grl_path_flat_free (GrlPathFlat *f)
{
    g_clear_pointer (&f->pts,     g_free);
    g_clear_pointer (&f->lengths, g_free);
    f->n_subpaths   = 0;
    f->total_points = 0;
    f->valid        = FALSE;
}

static void
grl_path_invalidate (GrlPath *self)
{
    grl_path_flat_free (&self->flat);
}

/* Ensure a current open subpath; if there are none or last is closed, open a
 * new one at (0,0). */
static GrlSubpath *
grl_path_ensure_subpath (GrlPath *self)
{
    GrlSubpath sp;
    GrlSubpath *last;

    if (self->subpaths->len > 0)
    {
        last = &g_array_index (self->subpaths, GrlSubpath, self->subpaths->len - 1);
        if (!last->closed)
            return last;
    }

    grl_subpath_init (&sp, 0.0f, 0.0f);
    g_array_append_val (self->subpaths, sp);
    return &g_array_index (self->subpaths, GrlSubpath, self->subpaths->len - 1);
}

/* Cubic bezier subdivision flattening (de Casteljau). */
static void
grl_flatten_cubic (GArray      *out,
                   gfloat       x0,
                   gfloat       y0,
                   gfloat       x1,
                   gfloat       y1,
                   gfloat       x2,
                   gfloat       y2,
                   gfloat       x3,
                   gfloat       y3,
                   gfloat       tol2) /* tolerance squared */
{
    gfloat dx, dy, d;
    gfloat mx, my;
    gfloat ax1, ay1, ax2, ay2;
    GrlVector2 pt;

    /* Chord test: distance from midpoint of bezier to midpoint of chord */
    mx = (x0 + x3) * 0.5f;
    my = (y0 + y3) * 0.5f;
    ax1 = (x0 + x1) * 0.5f;
    ay1 = (y0 + y1) * 0.5f;
    ax2 = (x2 + x3) * 0.5f;
    ay2 = (y2 + y3) * 0.5f;
    dx = (ax1 + ax2) * 0.5f - mx;
    dy = (ay1 + ay2) * 0.5f - my;
    d  = dx * dx + dy * dy;

    if (d <= tol2)
    {
        pt.x = x3;
        pt.y = y3;
        g_array_append_val (out, pt);
        return;
    }

    /* Subdivide at t=0.5 */
    {
        gfloat p01x = (x0 + x1) * 0.5f, p01y = (y0 + y1) * 0.5f;
        gfloat p12x = (x1 + x2) * 0.5f, p12y = (y1 + y2) * 0.5f;
        gfloat p23x = (x2 + x3) * 0.5f, p23y = (y2 + y3) * 0.5f;
        gfloat p012x = (p01x + p12x) * 0.5f, p012y = (p01y + p12y) * 0.5f;
        gfloat p123x = (p12x + p23x) * 0.5f, p123y = (p12y + p23y) * 0.5f;
        gfloat pmidx = (p012x + p123x) * 0.5f, pmidy = (p012y + p123y) * 0.5f;

        grl_flatten_cubic (out, x0, y0, p01x, p01y, p012x, p012y, pmidx, pmidy, tol2);
        grl_flatten_cubic (out, pmidx, pmidy, p123x, p123y, p23x, p23y, x3, y3, tol2);
    }
}

/* Quadratic bezier flattening (elevate to cubic and delegate). */
static void
grl_flatten_quad (GArray *out,
                  gfloat  x0,
                  gfloat  y0,
                  gfloat  cx,
                  gfloat  cy,
                  gfloat  x3,
                  gfloat  y3,
                  gfloat  tol2)
{
    gfloat c1x = x0 + (2.0f / 3.0f) * (cx - x0);
    gfloat c1y = y0 + (2.0f / 3.0f) * (cy - y0);
    gfloat c2x = x3 + (2.0f / 3.0f) * (cx - x3);
    gfloat c2y = y3 + (2.0f / 3.0f) * (cy - y3);
    grl_flatten_cubic (out, x0, y0, c1x, c1y, c2x, c2y, x3, y3, tol2);
}

/* Flatten one subpath into @pts (not including the start point, which the
 * caller must have already added). Returns number of points appended. */
static guint
grl_subpath_flatten (const GrlSubpath *sp, GArray *pts, gfloat tol2)
{
    guint i;
    guint verb_idx = 0;   /* skip MOVE verb */
    guint pt_idx   = 1;   /* skip MOVE point */
    guint count    = 0;
    GrlVector2 start, cur, p;

    start = g_array_index (sp->points, GrlVector2, 0);
    cur   = start;

    /* Add start point */
    g_array_append_val (pts, start);
    count++;

    for (i = 1; i < sp->verbs->len; i++)
    {
        GrlPathVerb v = g_array_index (sp->verbs, GrlPathVerb, i);

        switch (v)
        {
        case GRL_PATH_VERB_MOVE:
            /* Should not happen mid-subpath, but handle gracefully */
            cur = g_array_index (sp->points, GrlVector2, pt_idx);
            g_array_append_val (pts, cur);
            count++;
            pt_idx++;
            break;

        case GRL_PATH_VERB_LINE:
            cur = g_array_index (sp->points, GrlVector2, pt_idx);
            g_array_append_val (pts, cur);
            count++;
            pt_idx++;
            break;

        case GRL_PATH_VERB_CUBIC:
            {
                GrlVector2 c1, c2, ep;
                gfloat     px0, py0;
                guint      before;

                c1 = g_array_index (sp->points, GrlVector2, pt_idx);
                c2 = g_array_index (sp->points, GrlVector2, pt_idx + 1);
                ep = g_array_index (sp->points, GrlVector2, pt_idx + 2);
                pt_idx += 3;

                px0 = cur.x;
                py0 = cur.y;
                before = pts->len;

                grl_flatten_cubic (pts,
                                   px0, py0,
                                   c1.x, c1.y,
                                   c2.x, c2.y,
                                   ep.x, ep.y,
                                   tol2);
                count += pts->len - before;
                cur = ep;
            }
            break;

        case GRL_PATH_VERB_QUAD:
            {
                GrlVector2 cp, ep;
                gfloat     px0, py0;
                guint      before;

                cp = g_array_index (sp->points, GrlVector2, pt_idx);
                ep = g_array_index (sp->points, GrlVector2, pt_idx + 1);
                pt_idx += 2;

                px0 = cur.x;
                py0 = cur.y;
                before = pts->len;

                grl_flatten_quad (pts,
                                  px0, py0,
                                  cp.x, cp.y,
                                  ep.x, ep.y,
                                  tol2);
                count += pts->len - before;
                cur = ep;
            }
            break;

        case GRL_PATH_VERB_CLOSE:
            /* Add closing point == start only if different from cur */
            p = start;
            if (sp->closed &&
                (fabsf (cur.x - p.x) > 1e-6f || fabsf (cur.y - p.y) > 1e-6f))
            {
                g_array_append_val (pts, p);
                count++;
            }
            cur = p;
            break;

        default:
            break;
        }

        (void)verb_idx;
        verb_idx++;
    }

    return count;
}

/* =============================================================================
 * GObject boilerplate
 * =============================================================================
 */

static void
grl_path_finalize (GObject *object)
{
    GrlPath *self = GRL_PATH (object);
    guint i;

    for (i = 0; i < self->subpaths->len; i++)
    {
        GrlSubpath *sp = &g_array_index (self->subpaths, GrlSubpath, i);
        grl_subpath_clear (sp);
    }

    g_clear_pointer (&self->subpaths, g_array_unref);
    grl_path_flat_free (&self->flat);

    G_OBJECT_CLASS (grl_path_parent_class)->finalize (object);
}

static void
grl_path_class_init (GrlPathClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->finalize = grl_path_finalize;
}

static void
grl_path_init (GrlPath *self)
{
    self->subpaths = g_array_new (FALSE, TRUE, sizeof (GrlSubpath));
    self->flat.pts          = NULL;
    self->flat.lengths      = NULL;
    self->flat.n_subpaths   = 0;
    self->flat.total_points = 0;
    self->flat.tolerance    = 0.0f;
    self->flat.valid        = FALSE;
}

/* =============================================================================
 * Public API — Construction
 * =============================================================================
 */

/**
 * grl_path_new:
 *
 * Creates a new, empty #GrlPath with no subpaths.
 *
 * Returns: (transfer full): A new empty #GrlPath.
 */
GrlPath *
grl_path_new (void)
{
    return g_object_new (GRL_TYPE_PATH, NULL);
}

/**
 * grl_path_copy:
 * @self: A #GrlPath.
 *
 * Creates a deep copy of @self. Mutating the copy does not affect the original.
 *
 * Returns: (transfer full): A new #GrlPath that is an independent copy.
 */
GrlPath *
grl_path_copy (GrlPath *self)
{
    GrlPath *copy;
    guint i;

    g_return_val_if_fail (GRL_IS_PATH (self), NULL);

    copy = grl_path_new ();

    for (i = 0; i < self->subpaths->len; i++)
    {
        GrlSubpath src = g_array_index (self->subpaths, GrlSubpath, i);
        GrlSubpath dst;

        grl_subpath_copy_into (&src, &dst);
        g_array_append_val (copy->subpaths, dst);
    }

    return copy;
}

/* =============================================================================
 * Public API — Builder
 * =============================================================================
 */

/**
 * grl_path_move_to:
 * @self: A #GrlPath.
 * @x: X coordinate.
 * @y: Y coordinate.
 *
 * Starts a new subpath at (@x, @y).
 */
void
grl_path_move_to (GrlPath *self,
                  gfloat   x,
                  gfloat   y)
{
    GrlSubpath sp;

    g_return_if_fail (GRL_IS_PATH (self));

    grl_path_invalidate (self);
    grl_subpath_init (&sp, x, y);
    g_array_append_val (self->subpaths, sp);
}

/**
 * grl_path_line_to:
 * @self: A #GrlPath.
 * @x: X coordinate of the line endpoint.
 * @y: Y coordinate of the line endpoint.
 *
 * Appends a straight line from the current point to (@x, @y).
 */
void
grl_path_line_to (GrlPath *self,
                  gfloat   x,
                  gfloat   y)
{
    GrlSubpath *sp;
    GrlPathVerb v;
    GrlVector2 pt;

    g_return_if_fail (GRL_IS_PATH (self));

    grl_path_invalidate (self);
    sp = grl_path_ensure_subpath (self);

    v = GRL_PATH_VERB_LINE;
    pt.x = x;
    pt.y = y;
    g_array_append_val (sp->verbs, v);
    g_array_append_val (sp->points, pt);
    sp->cur_x = x;
    sp->cur_y = y;
}

/**
 * grl_path_cubic_to:
 * @self: A #GrlPath.
 * @c1x: X coordinate of the first control point.
 * @c1y: Y coordinate of the first control point.
 * @c2x: X coordinate of the second control point.
 * @c2y: Y coordinate of the second control point.
 * @x: X coordinate of the endpoint.
 * @y: Y coordinate of the endpoint.
 *
 * Appends a cubic bezier curve.
 */
void
grl_path_cubic_to (GrlPath *self,
                   gfloat   c1x,
                   gfloat   c1y,
                   gfloat   c2x,
                   gfloat   c2y,
                   gfloat   x,
                   gfloat   y)
{
    GrlSubpath *sp;
    GrlPathVerb v;
    GrlVector2 pt;

    g_return_if_fail (GRL_IS_PATH (self));

    grl_path_invalidate (self);
    sp = grl_path_ensure_subpath (self);

    v = GRL_PATH_VERB_CUBIC;
    g_array_append_val (sp->verbs, v);

    pt.x = c1x; pt.y = c1y;
    g_array_append_val (sp->points, pt);
    pt.x = c2x; pt.y = c2y;
    g_array_append_val (sp->points, pt);
    pt.x = x;   pt.y = y;
    g_array_append_val (sp->points, pt);

    sp->cur_x = x;
    sp->cur_y = y;
}

/**
 * grl_path_quad_to:
 * @self: A #GrlPath.
 * @cx: X coordinate of the control point.
 * @cy: Y coordinate of the control point.
 * @x: X coordinate of the endpoint.
 * @y: Y coordinate of the endpoint.
 *
 * Appends a quadratic bezier curve.
 */
void
grl_path_quad_to (GrlPath *self,
                  gfloat   cx,
                  gfloat   cy,
                  gfloat   x,
                  gfloat   y)
{
    GrlSubpath *sp;
    GrlPathVerb v;
    GrlVector2 pt;

    g_return_if_fail (GRL_IS_PATH (self));

    grl_path_invalidate (self);
    sp = grl_path_ensure_subpath (self);

    v = GRL_PATH_VERB_QUAD;
    g_array_append_val (sp->verbs, v);

    pt.x = cx; pt.y = cy;
    g_array_append_val (sp->points, pt);
    pt.x = x;  pt.y = y;
    g_array_append_val (sp->points, pt);

    sp->cur_x = x;
    sp->cur_y = y;
}

/**
 * grl_path_close:
 * @self: A #GrlPath.
 *
 * Closes the current subpath.
 */
void
grl_path_close (GrlPath *self)
{
    GrlSubpath *sp;
    GrlPathVerb v;

    g_return_if_fail (GRL_IS_PATH (self));

    if (self->subpaths->len == 0)
        return;

    sp = &g_array_index (self->subpaths, GrlSubpath, self->subpaths->len - 1);
    if (sp->closed)
        return;

    grl_path_invalidate (self);
    sp->closed = TRUE;
    v = GRL_PATH_VERB_CLOSE;
    g_array_append_val (sp->verbs, v);
}

/* =============================================================================
 * Public API — Shape helpers
 * =============================================================================
 */

/* Kappa constant for circle approximation with cubics */
#define GRL_PATH_KAPPA 0.5522847498f

/**
 * grl_path_add_rect:
 * @self: A #GrlPath.
 * @rect: The rectangle to add.
 *
 * Appends a closed rectangular subpath.
 */
void
grl_path_add_rect (GrlPath            *self,
                   const GrlRectangle *rect)
{
    gfloat x, y, w, h;

    g_return_if_fail (GRL_IS_PATH (self));
    g_return_if_fail (rect != NULL);

    x = rect->x;
    y = rect->y;
    w = rect->width;
    h = rect->height;

    grl_path_move_to (self, x, y);
    grl_path_line_to (self, x + w, y);
    grl_path_line_to (self, x + w, y + h);
    grl_path_line_to (self, x, y + h);
    grl_path_close (self);
}

/**
 * grl_path_add_circle:
 * @self: A #GrlPath.
 * @cx: X coordinate of the centre.
 * @cy: Y coordinate of the centre.
 * @r: Radius.
 *
 * Appends a closed circular subpath.
 */
void
grl_path_add_circle (GrlPath *self,
                     gfloat   cx,
                     gfloat   cy,
                     gfloat   r)
{
    grl_path_add_ellipse (self, cx, cy, r, r);
}

/**
 * grl_path_add_ellipse:
 * @self: A #GrlPath.
 * @cx: X coordinate of the centre.
 * @cy: Y coordinate of the centre.
 * @rx: Horizontal radius.
 * @ry: Vertical radius.
 *
 * Appends a closed elliptical subpath.
 */
void
grl_path_add_ellipse (GrlPath *self,
                      gfloat   cx,
                      gfloat   cy,
                      gfloat   rx,
                      gfloat   ry)
{
    gfloat kx;
    gfloat ky;

    g_return_if_fail (GRL_IS_PATH (self));

    kx = rx * GRL_PATH_KAPPA;
    ky = ry * GRL_PATH_KAPPA;

    /* Start at top-center, go clockwise */
    grl_path_move_to  (self, cx, cy - ry);
    grl_path_cubic_to (self, cx + kx, cy - ry, cx + rx, cy - ky, cx + rx, cy);
    grl_path_cubic_to (self, cx + rx, cy + ky, cx + kx, cy + ry, cx, cy + ry);
    grl_path_cubic_to (self, cx - kx, cy + ry, cx - rx, cy + ky, cx - rx, cy);
    grl_path_cubic_to (self, cx - rx, cy - ky, cx - kx, cy - ry, cx, cy - ry);
    grl_path_close (self);
}

/* =============================================================================
 * Public API — Introspection
 * =============================================================================
 */

/**
 * grl_path_is_empty:
 * @self: A #GrlPath.
 *
 * Returns: %TRUE if the path contains no subpaths.
 */
gboolean
grl_path_is_empty (GrlPath *self)
{
    g_return_val_if_fail (GRL_IS_PATH (self), TRUE);

    return self->subpaths->len == 0;
}

/**
 * grl_path_get_subpath_count:
 * @self: A #GrlPath.
 *
 * Returns: The number of subpaths.
 */
guint
grl_path_get_subpath_count (GrlPath *self)
{
    g_return_val_if_fail (GRL_IS_PATH (self), 0);

    return self->subpaths->len;
}

/**
 * grl_path_get_bounds:
 * @self: A #GrlPath.
 * @out: (out): Return location for the bounding rectangle.
 *
 * Computes the axis-aligned bounding box of all control points.
 *
 * Returns: %TRUE if the path is non-empty.
 */
gboolean
grl_path_get_bounds (GrlPath      *self,
                     GrlRectangle *out)
{
    gfloat minx, miny, maxx, maxy;
    guint i, j;
    gboolean found;

    g_return_val_if_fail (GRL_IS_PATH (self), FALSE);
    g_return_val_if_fail (out != NULL, FALSE);

    if (self->subpaths->len == 0)
        return FALSE;

    minx = miny = 1e30f;
    maxx = maxy = -1e30f;
    found = FALSE;

    for (i = 0; i < self->subpaths->len; i++)
    {
        GrlSubpath *sp = &g_array_index (self->subpaths, GrlSubpath, i);

        for (j = 0; j < sp->points->len; j++)
        {
            GrlVector2 p = g_array_index (sp->points, GrlVector2, j);

            if (p.x < minx) minx = p.x;
            if (p.y < miny) miny = p.y;
            if (p.x > maxx) maxx = p.x;
            if (p.y > maxy) maxy = p.y;
            found = TRUE;
        }
    }

    if (!found)
        return FALSE;

    out->x = minx;
    out->y = miny;
    out->width  = maxx - minx;
    out->height = maxy - miny;

    return TRUE;
}

/* =============================================================================
 * Public API — Flatten
 * =============================================================================
 */

static void
grl_path_rebuild_flat (GrlPath *self, gfloat tolerance)
{
    GArray *pts;
    guint   i;
    gfloat  tol2;

    if (tolerance <= 0.0f)
        tolerance = 0.25f;
    tol2 = tolerance * tolerance;

    grl_path_flat_free (&self->flat);

    if (self->subpaths->len == 0)
    {
        self->flat.valid = TRUE;
        return;
    }

    self->flat.lengths    = g_new0 (guint, self->subpaths->len);
    self->flat.n_subpaths = self->subpaths->len;
    pts = g_array_new (FALSE, FALSE, sizeof (GrlVector2));

    for (i = 0; i < self->subpaths->len; i++)
    {
        GrlSubpath *sp;
        guint before;

        sp = &g_array_index (self->subpaths, GrlSubpath, i);
        if (sp->points->len == 0)
        {
            self->flat.lengths[i] = 0;
            continue;
        }

        before = pts->len;
        grl_subpath_flatten (sp, pts, tol2);
        self->flat.lengths[i] = pts->len - before;
    }

    self->flat.total_points = pts->len;
    if (pts->len > 0)
    {
        self->flat.pts = (GrlVector2 *)g_array_free (pts, FALSE);
    }
    else
    {
        g_array_free (pts, TRUE);
        self->flat.pts = NULL;
    }

    self->flat.tolerance = tolerance;
    self->flat.valid     = TRUE;
}

/**
 * grl_path_get_flattened:
 * @self: A #GrlPath.
 * @tolerance: Maximum chord deviation in pixels.
 * @subpath_lengths: (out) (array length=n_subpaths) (transfer full): Per-subpath
 *   point counts.
 * @n_subpaths: (out): Number of subpaths.
 * @total_points: (out): Total point count.
 *
 * Returns: (transfer full) (array length=total_points) (nullable): Flattened points.
 */
GrlVector2 *
grl_path_get_flattened (GrlPath *self,
                         gfloat   tolerance,
                         guint  **subpath_lengths,
                         guint   *n_subpaths,
                         guint   *total_points)
{
    GrlVector2 *result;
    guint       i;

    g_return_val_if_fail (GRL_IS_PATH (self), NULL);
    g_return_val_if_fail (subpath_lengths != NULL, NULL);
    g_return_val_if_fail (n_subpaths != NULL, NULL);
    g_return_val_if_fail (total_points != NULL, NULL);

    if (!self->flat.valid || fabsf (self->flat.tolerance - tolerance) > 1e-5f)
        grl_path_rebuild_flat (self, tolerance);

    *n_subpaths = self->flat.n_subpaths;
    *total_points = self->flat.total_points;

    if (self->flat.total_points == 0)
    {
        *subpath_lengths = g_new0 (guint, MAX (1u, self->flat.n_subpaths));
        return NULL;
    }

    /* Return a copy so the caller owns it */
    result = g_new (GrlVector2, self->flat.total_points);
    memcpy (result, self->flat.pts,
            self->flat.total_points * sizeof (GrlVector2));

    *subpath_lengths = g_new (guint, self->flat.n_subpaths);
    for (i = 0; i < self->flat.n_subpaths; i++)
        (*subpath_lengths)[i] = self->flat.lengths[i];

    return result;
}

/* =============================================================================
 * Public API — Transforms
 * =============================================================================
 */

/**
 * grl_path_transform:
 * @self: A #GrlPath.
 * @matrix: The 4x4 matrix to apply (only the 2D affine part is used).
 *
 * Applies @matrix to every control point in the path.
 */
void
grl_path_transform (GrlPath          *self,
                    const GrlMatrix  *matrix)
{
    guint i, j;

    g_return_if_fail (GRL_IS_PATH (self));
    g_return_if_fail (matrix != NULL);

    grl_path_invalidate (self);

    for (i = 0; i < self->subpaths->len; i++)
    {
        GrlSubpath *sp = &g_array_index (self->subpaths, GrlSubpath, i);

        for (j = 0; j < sp->points->len; j++)
        {
            GrlVector2 *p = &g_array_index (sp->points, GrlVector2, j);
            gfloat      x = p->x;
            gfloat      y = p->y;

            p->x = matrix->m0 * x + matrix->m4 * y + matrix->m12;
            p->y = matrix->m1 * x + matrix->m5 * y + matrix->m13;
        }

        /* Update start/cur from transformed point arrays */
        if (sp->points->len > 0)
        {
            GrlVector2 first = g_array_index (sp->points, GrlVector2, 0);
            GrlVector2 last  = g_array_index (sp->points, GrlVector2, sp->points->len - 1);
            sp->start_x = first.x;
            sp->start_y = first.y;
            sp->cur_x   = last.x;
            sp->cur_y   = last.y;
        }
    }
}

/**
 * grl_path_translate:
 * @self: A #GrlPath.
 * @dx: X translation.
 * @dy: Y translation.
 *
 * Translates all control points by (@dx, @dy).
 */
void
grl_path_translate (GrlPath *self,
                    gfloat   dx,
                    gfloat   dy)
{
    guint i, j;

    g_return_if_fail (GRL_IS_PATH (self));

    grl_path_invalidate (self);

    for (i = 0; i < self->subpaths->len; i++)
    {
        GrlSubpath *sp = &g_array_index (self->subpaths, GrlSubpath, i);

        for (j = 0; j < sp->points->len; j++)
        {
            GrlVector2 *p = &g_array_index (sp->points, GrlVector2, j);
            p->x += dx;
            p->y += dy;
        }

        sp->start_x += dx;
        sp->start_y += dy;
        sp->cur_x   += dx;
        sp->cur_y   += dy;
    }
}

/**
 * grl_path_scale:
 * @self: A #GrlPath.
 * @sx: X scale factor.
 * @sy: Y scale factor.
 *
 * Scales all control points by (@sx, @sy) about the origin.
 */
void
grl_path_scale (GrlPath *self,
                gfloat   sx,
                gfloat   sy)
{
    guint i, j;

    g_return_if_fail (GRL_IS_PATH (self));

    grl_path_invalidate (self);

    for (i = 0; i < self->subpaths->len; i++)
    {
        GrlSubpath *sp = &g_array_index (self->subpaths, GrlSubpath, i);

        for (j = 0; j < sp->points->len; j++)
        {
            GrlVector2 *p = &g_array_index (sp->points, GrlVector2, j);
            p->x *= sx;
            p->y *= sy;
        }

        sp->start_x *= sx;
        sp->start_y *= sy;
        sp->cur_x   *= sx;
        sp->cur_y   *= sy;
    }
}

/**
 * grl_path_rotate:
 * @self: A #GrlPath.
 * @degrees: Rotation angle in degrees, clockwise.
 *
 * Rotates all control points about the origin.
 */
void
grl_path_rotate (GrlPath *self,
                 gfloat   degrees)
{
    gfloat rad, c, s;
    guint i, j;

    g_return_if_fail (GRL_IS_PATH (self));

    rad = degrees * (3.14159265358979323846f / 180.0f);
    c = cosf (rad);
    s = sinf (rad);

    grl_path_invalidate (self);

    for (i = 0; i < self->subpaths->len; i++)
    {
        GrlSubpath *sp = &g_array_index (self->subpaths, GrlSubpath, i);

        for (j = 0; j < sp->points->len; j++)
        {
            GrlVector2 *p = &g_array_index (sp->points, GrlVector2, j);
            gfloat x = p->x;
            gfloat y = p->y;
            p->x = x * c - y * s;
            p->y = x * s + y * c;
        }

        {
            gfloat x = sp->start_x, y = sp->start_y;
            sp->start_x = x * c - y * s;
            sp->start_y = x * s + y * c;
            x = sp->cur_x; y = sp->cur_y;
            sp->cur_x = x * c - y * s;
            sp->cur_y = x * s + y * c;
        }
    }
}

/* =============================================================================
 * Boolean operations (raster-approximate)
 *
 * Working resolution: we pick a pixel density of 8 pixels per (control-point)
 * unit, clamped to a max of 2048 in either dimension, with a 4-pixel margin.
 * The two paths are filled into 8-bit grayscale coverage masks, combined, and
 * the result is traced back to a GrlPath via marching-squares.
 * =============================================================================
 */

#define GRL_BOOL_RESOLUTION 4.0f
#define GRL_BOOL_MAX_DIM    2048
#define GRL_BOOL_MARGIN     4

typedef enum
{
    GRL_BOOL_OP_UNION     = 0,
    GRL_BOOL_OP_INTERSECT = 1,
    GRL_BOOL_OP_SUBTRACT  = 2,
    GRL_BOOL_OP_XOR       = 3
} GrlBoolOp;

/* Get union bounds of two paths with margin. */
static gboolean
grl_bool_get_union_bounds (GrlPath *a,
                            GrlPath *b,
                            gfloat  *ox,
                            gfloat  *oy,
                            gint    *w,
                            gint    *h)
{
    GrlRectangle ra, rb;
    gboolean     ha, hb;
    gfloat       minx, miny, maxx, maxy;
    gfloat       margin;

    ha = grl_path_get_bounds (a, &ra);
    hb = grl_path_get_bounds (b, &rb);

    if (!ha && !hb)
        return FALSE;

    if (ha && hb)
    {
        minx = MIN (ra.x, rb.x);
        miny = MIN (ra.y, rb.y);
        maxx = MAX (ra.x + ra.width,  rb.x + rb.width);
        maxy = MAX (ra.y + ra.height, rb.y + rb.height);
    }
    else if (ha)
    {
        minx = ra.x; miny = ra.y;
        maxx = ra.x + ra.width; maxy = ra.y + ra.height;
    }
    else
    {
        minx = rb.x; miny = rb.y;
        maxx = rb.x + rb.width; maxy = rb.y + rb.height;
    }

    margin = (gfloat)GRL_BOOL_MARGIN / GRL_BOOL_RESOLUTION;
    minx -= margin; miny -= margin;
    maxx += margin; maxy += margin;

    *ox = minx;
    *oy = miny;
    *w = (gint)((maxx - minx) * GRL_BOOL_RESOLUTION) + 1;
    *h = (gint)((maxy - miny) * GRL_BOOL_RESOLUTION) + 1;
    if (*w > GRL_BOOL_MAX_DIM) *w = GRL_BOOL_MAX_DIM;
    if (*h > GRL_BOOL_MAX_DIM) *h = GRL_BOOL_MAX_DIM;
    if (*w < 1) *w = 1;
    if (*h < 1) *h = 1;

    return TRUE;
}

/* Rasterize a path into an 8-bit coverage buffer (caller allocates buf). */
static void
grl_bool_rasterize (GrlPath *path,
                    gfloat   ox,
                    gfloat   oy,
                    gint     width,
                    gint     height,
                    guint8  *buf)
{
    GrlImage  *mask;
    GrlColor   white;
    gint       i;

    /* Use grl_image_new_mask (grayscale) + grl_image_fill_path */
    mask = grl_image_new_mask (width, height);
    if (mask == NULL)
        return;

    /* Set transform to map path coords -> pixel coords */
    grl_image_push_matrix (mask);

    {
        GrlMatrix m;

        memset (&m, 0, sizeof (m));
        m.m0  = GRL_BOOL_RESOLUTION;
        m.m5  = GRL_BOOL_RESOLUTION;
        m.m10 = 1.0f;
        m.m15 = 1.0f;
        m.m12 = -ox * GRL_BOOL_RESOLUTION;
        m.m13 = -oy * GRL_BOOL_RESOLUTION;

        grl_image_set_matrix (mask, &m);
    }

    white.r = 255; white.g = 255; white.b = 255; white.a = 255;
    grl_image_fill_path (mask, path, GRL_FILL_RULE_NONZERO, &white);

    grl_image_pop_matrix (mask);

    /* Copy grayscale data to buf.
     * grl_image_get_handle returns a pointer to the internal raylib Image struct.
     * The mask is PIXELFORMAT_UNCOMPRESSED_GRAYSCALE (1 byte/pixel). */
    {
        typedef struct { void *data; int width; int height; int mipmaps; int format; } RlImg;
        RlImg *rli = (RlImg *)grl_image_get_handle (mask);
        if (rli && rli->data)
        {
            gint n = width * height;
            for (i = 0; i < n; i++)
                buf[i] = ((guint8 *)rli->data)[i];
        }
    }

    g_object_unref (mask);
}

/* Scanline run extractor.
 * Converts the 8-bit coverage buffer to a path by emitting one thin rectangle
 * per horizontal run of filled pixels.  Each rectangle spans exactly one pixel
 * row in mask coordinates (1/GRL_BOOL_RESOLUTION world-space units tall) and
 * merges adjacent filled pixels into a single rect to keep subpath count low.
 * This is simpler and more reliable than marching-squares for the raster-
 * approximate boolean workflow. */
static GrlPath *
grl_bool_trace (const guint8 *buf,
                gint          width,
                gint          height,
                gfloat        ox,
                gfloat        oy)
{
    GrlPath *result;
    gint     y;
    gfloat   inv_res = 1.0f / GRL_BOOL_RESOLUTION;

    result = grl_path_new ();

    for (y = 0; y < height; y++)
    {
        gint x = 0;

        while (x < width)
        {
            gint   run_start, run_end;
            gfloat wx0, wx1, wy0, wy1;

            /* Skip empty pixels */
            if (buf[y * width + x] <= 127)
            {
                x++;
                continue;
            }

            /* Accumulate a horizontal run of filled pixels */
            run_start = x;
            while (x < width && buf[y * width + x] > 127)
                x++;
            run_end = x;   /* exclusive */

            /* Convert to world coordinates */
            wx0 = ox + (gfloat)run_start * inv_res;
            wx1 = ox + (gfloat)run_end   * inv_res;
            wy0 = oy + (gfloat)y         * inv_res;
            wy1 = wy0 + inv_res;

            /* Emit as a closed rectangle (CW winding) */
            grl_path_move_to (result, wx0, wy0);
            grl_path_line_to (result, wx1, wy0);
            grl_path_line_to (result, wx1, wy1);
            grl_path_line_to (result, wx0, wy1);
            grl_path_close (result);
        }
    }

    return result;
}

static GrlPath *
grl_path_bool_op (GrlPath    *a,
                  GrlPath    *b,
                  GrlBoolOp   op)
{
    gfloat  ox, oy;
    gint    width, height;
    guint8 *buf_a;
    guint8 *buf_b;
    guint8 *buf_c;
    gint    i, n;
    GrlPath *result;

    if (!grl_bool_get_union_bounds (a, b, &ox, &oy, &width, &height))
    {
        return grl_path_new ();
    }

    n = width * height;
    buf_a = g_new0 (guint8, n);
    buf_b = g_new0 (guint8, n);
    buf_c = g_new0 (guint8, n);

    if (!grl_path_is_empty (a))
        grl_bool_rasterize (a, ox, oy, width, height, buf_a);
    if (!grl_path_is_empty (b))
        grl_bool_rasterize (b, ox, oy, width, height, buf_b);

    for (i = 0; i < n; i++)
    {
        guint va = buf_a[i];
        guint vb = buf_b[i];
        guint vc;

        switch (op)
        {
        case GRL_BOOL_OP_UNION:
            vc = va > vb ? va : vb;
            break;
        case GRL_BOOL_OP_INTERSECT:
            vc = va < vb ? va : vb;
            break;
        case GRL_BOOL_OP_SUBTRACT:
            vc = va * (255u - vb) / 255u;
            break;
        case GRL_BOOL_OP_XOR:
            vc = (va > vb) ? (va - vb) : (vb - va);
            break;
        default:
            vc = 0;
            break;
        }

        buf_c[i] = (guint8)(vc > 255u ? 255u : vc);
    }

    result = grl_bool_trace (buf_c, width, height, ox, oy);

    g_free (buf_a);
    g_free (buf_b);
    g_free (buf_c);

    return result;
}

/**
 * grl_path_union:
 * @a: First operand path.
 * @b: Second operand path.
 *
 * Returns: (transfer full): The raster-approximate union of @a and @b.
 */
GrlPath *
grl_path_union (GrlPath *a,
                GrlPath *b)
{
    g_return_val_if_fail (GRL_IS_PATH (a), NULL);
    g_return_val_if_fail (GRL_IS_PATH (b), NULL);

    return grl_path_bool_op (a, b, GRL_BOOL_OP_UNION);
}

/**
 * grl_path_intersect:
 * @a: First operand path.
 * @b: Second operand path.
 *
 * Returns: (transfer full): The raster-approximate intersection of @a and @b.
 */
GrlPath *
grl_path_intersect (GrlPath *a,
                    GrlPath *b)
{
    g_return_val_if_fail (GRL_IS_PATH (a), NULL);
    g_return_val_if_fail (GRL_IS_PATH (b), NULL);

    return grl_path_bool_op (a, b, GRL_BOOL_OP_INTERSECT);
}

/**
 * grl_path_subtract:
 * @a: First operand path (base).
 * @b: Second operand path (to subtract).
 *
 * Returns: (transfer full): The raster-approximate difference @a minus @b.
 */
GrlPath *
grl_path_subtract (GrlPath *a,
                   GrlPath *b)
{
    g_return_val_if_fail (GRL_IS_PATH (a), NULL);
    g_return_val_if_fail (GRL_IS_PATH (b), NULL);

    return grl_path_bool_op (a, b, GRL_BOOL_OP_SUBTRACT);
}

/**
 * grl_path_xor:
 * @a: First operand path.
 * @b: Second operand path.
 *
 * Returns: (transfer full): The raster-approximate symmetric difference.
 */
GrlPath *
grl_path_xor (GrlPath *a,
              GrlPath *b)
{
    g_return_val_if_fail (GRL_IS_PATH (a), NULL);
    g_return_val_if_fail (GRL_IS_PATH (b), NULL);

    return grl_path_bool_op (a, b, GRL_BOOL_OP_XOR);
}
