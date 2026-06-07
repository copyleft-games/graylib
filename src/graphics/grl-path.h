/* grl-path.h
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Retained 2D vector path type.
 *
 * GrlPath represents a collection of subpaths built from move-to, line-to,
 * cubic/quadratic bezier, and close commands. Paths can be transformed in
 * place, filled or stroked onto a GrlImage, and combined with raster-
 * approximate boolean operations.
 *
 * Boolean operations (union, intersect, subtract, xor) are implemented as
 * raster-approximate: each operand is rasterized to a coverage mask at a
 * working resolution derived from the union of both paths' bounding boxes,
 * the masks are combined per-pixel, and the result is traced back into
 * GrlPath subpaths via marching-squares contour following. This approach is
 * resolution-dependent and may introduce small errors at coincident edges;
 * an exact vector clipper may replace the backend in a future release without
 * changing the public API.
 */

#pragma once

#if !defined(GRAYLIB_INSIDE) && !defined(GRAYLIB_COMPILATION)
#error "Only <graylib.h> can be included directly."
#endif

#include <glib-object.h>
#include "../grl-version.h"
#include "../grl-types.h"
#include "../grl-enums.h"
#include "../math/grl-vector2.h"
#include "../math/grl-rectangle.h"
#include "../math/grl-matrix.h"

G_BEGIN_DECLS

#define GRL_TYPE_PATH (grl_path_get_type ())

GRL_AVAILABLE_IN_ALL
G_DECLARE_FINAL_TYPE (GrlPath, grl_path, GRL, PATH, GObject)

/*
 * Construction
 */

/**
 * grl_path_new:
 *
 * Creates a new, empty #GrlPath with no subpaths.
 *
 * Returns: (transfer full): A new empty #GrlPath.
 */
GRL_AVAILABLE_IN_ALL
GrlPath *   grl_path_new        (void);

/**
 * grl_path_copy:
 * @self: A #GrlPath.
 *
 * Creates a deep copy of @self. Mutating the copy does not affect the original.
 *
 * Returns: (transfer full): A new #GrlPath that is an independent copy.
 */
GRL_AVAILABLE_IN_ALL
GrlPath *   grl_path_copy       (GrlPath *self);

/*
 * Builder verbs
 */

/**
 * grl_path_move_to:
 * @self: A #GrlPath.
 * @x: X coordinate.
 * @y: Y coordinate.
 *
 * Starts a new subpath at (@x, @y). Calling this implicitly finishes any
 * currently open subpath (without closing it) and begins a new one.
 */
GRL_AVAILABLE_IN_ALL
void        grl_path_move_to    (GrlPath *self,
                                 gfloat   x,
                                 gfloat   y);

/**
 * grl_path_line_to:
 * @self: A #GrlPath.
 * @x: X coordinate of the line endpoint.
 * @y: Y coordinate of the line endpoint.
 *
 * Appends a straight line from the current point to (@x, @y).
 * If there is no current subpath, an implicit move_to (0, 0) is performed
 * first.
 */
GRL_AVAILABLE_IN_ALL
void        grl_path_line_to    (GrlPath *self,
                                 gfloat   x,
                                 gfloat   y);

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
 * Appends a cubic bezier curve from the current point to (@x, @y) with
 * control points (@c1x, @c1y) and (@c2x, @c2y).
 */
GRL_AVAILABLE_IN_ALL
void        grl_path_cubic_to   (GrlPath *self,
                                 gfloat   c1x,
                                 gfloat   c1y,
                                 gfloat   c2x,
                                 gfloat   c2y,
                                 gfloat   x,
                                 gfloat   y);

/**
 * grl_path_quad_to:
 * @self: A #GrlPath.
 * @cx: X coordinate of the control point.
 * @cy: Y coordinate of the control point.
 * @x: X coordinate of the endpoint.
 * @y: Y coordinate of the endpoint.
 *
 * Appends a quadratic bezier curve from the current point to (@x, @y) with
 * control point (@cx, @cy).
 */
GRL_AVAILABLE_IN_ALL
void        grl_path_quad_to    (GrlPath *self,
                                 gfloat   cx,
                                 gfloat   cy,
                                 gfloat   x,
                                 gfloat   y);

/**
 * grl_path_close:
 * @self: A #GrlPath.
 *
 * Closes the current subpath by adding a straight line from the current point
 * back to the starting point of the subpath, then marks the subpath as closed.
 * A subsequent drawing command starts a new subpath. Calling close on an
 * already-closed or empty path is a no-op.
 */
GRL_AVAILABLE_IN_ALL
void        grl_path_close      (GrlPath *self);

/*
 * Shape convenience helpers
 */

/**
 * grl_path_add_rect:
 * @self: A #GrlPath.
 * @rect: The rectangle to add.
 *
 * Appends a closed rectangular subpath (four line segments, clockwise).
 */
GRL_AVAILABLE_IN_ALL
void        grl_path_add_rect   (GrlPath              *self,
                                 const GrlRectangle   *rect);

/**
 * grl_path_add_circle:
 * @self: A #GrlPath.
 * @cx: X coordinate of the centre.
 * @cy: Y coordinate of the centre.
 * @r: Radius.
 *
 * Appends a closed circular subpath approximated with cubic bezier segments.
 */
GRL_AVAILABLE_IN_ALL
void        grl_path_add_circle (GrlPath *self,
                                 gfloat   cx,
                                 gfloat   cy,
                                 gfloat   r);

/**
 * grl_path_add_ellipse:
 * @self: A #GrlPath.
 * @cx: X coordinate of the centre.
 * @cy: Y coordinate of the centre.
 * @rx: Horizontal radius.
 * @ry: Vertical radius.
 *
 * Appends a closed elliptical subpath approximated with cubic bezier segments.
 */
GRL_AVAILABLE_IN_ALL
void        grl_path_add_ellipse (GrlPath *self,
                                  gfloat   cx,
                                  gfloat   cy,
                                  gfloat   rx,
                                  gfloat   ry);

/*
 * Introspection
 */

/**
 * grl_path_is_empty:
 * @self: A #GrlPath.
 *
 * Returns: %TRUE if the path contains no subpaths (or only empty subpaths).
 */
GRL_AVAILABLE_IN_ALL
gboolean    grl_path_is_empty           (GrlPath *self);

/**
 * grl_path_get_subpath_count:
 * @self: A #GrlPath.
 *
 * Returns the number of subpaths (including empty or single-point ones).
 *
 * Returns: The number of subpaths.
 */
GRL_AVAILABLE_IN_ALL
guint       grl_path_get_subpath_count  (GrlPath *self);

/**
 * grl_path_get_bounds:
 * @self: A #GrlPath.
 * @out: (out): Return location for the bounding rectangle.
 *
 * Computes the axis-aligned bounding box that encloses all control points
 * of the path. This is a conservative estimate — the true bounds of a bezier
 * curve may be tighter.
 *
 * Returns: %TRUE if the path is non-empty and @out was set; %FALSE otherwise.
 */
GRL_AVAILABLE_IN_ALL
gboolean    grl_path_get_bounds         (GrlPath      *self,
                                         GrlRectangle *out);

/*
 * Flatten
 */

/**
 * grl_path_get_flattened:
 * @self: A #GrlPath.
 * @tolerance: Maximum chord deviation in pixels. Values <= 0 use the default
 *   tolerance of ~0.25 px, which is adequate for screen-resolution rendering.
 * @subpath_lengths: (out) (array length=n_subpaths) (transfer full): Return
 *   location for a newly-allocated array giving the number of points in each
 *   subpath, in order. The caller must free this with g_free().
 * @n_subpaths: (out): Return location for the number of subpaths.
 * @total_points: (out): Return location for the total point count across all
 *   subpaths (equals the sum of @subpath_lengths).
 *
 * Flattens all curves in the path to polylines and returns the resulting
 * points as a flat array of #GrlVector2. All subpaths are concatenated in
 * order; use @subpath_lengths to find the boundaries between them.
 *
 * The returned array and @subpath_lengths array are owned by the caller and
 * must be freed with g_free().
 *
 * If the path is empty, returns %NULL and sets @n_subpaths and @total_points
 * to zero.
 *
 * Returns: (transfer full) (array length=total_points) (nullable): Flat array
 *   of flattened points, or %NULL if the path is empty.
 */
GRL_AVAILABLE_IN_ALL
GrlVector2 *grl_path_get_flattened     (GrlPath *self,
                                         gfloat   tolerance,
                                         guint  **subpath_lengths,
                                         guint   *n_subpaths,
                                         guint   *total_points);

/*
 * In-place transforms
 */

/**
 * grl_path_transform:
 * @self: A #GrlPath.
 * @matrix: The 4x4 matrix to apply (only the 2D affine part is used).
 *
 * Applies @matrix to every control point in the path. The flat cache is
 * invalidated.
 */
GRL_AVAILABLE_IN_ALL
void        grl_path_transform  (GrlPath          *self,
                                 const GrlMatrix  *matrix);

/**
 * grl_path_translate:
 * @self: A #GrlPath.
 * @dx: X translation.
 * @dy: Y translation.
 *
 * Translates all control points by (@dx, @dy).
 */
GRL_AVAILABLE_IN_ALL
void        grl_path_translate  (GrlPath *self,
                                 gfloat   dx,
                                 gfloat   dy);

/**
 * grl_path_scale:
 * @self: A #GrlPath.
 * @sx: X scale factor.
 * @sy: Y scale factor.
 *
 * Scales all control points by (@sx, @sy) about the origin.
 */
GRL_AVAILABLE_IN_ALL
void        grl_path_scale      (GrlPath *self,
                                 gfloat   sx,
                                 gfloat   sy);

/**
 * grl_path_rotate:
 * @self: A #GrlPath.
 * @degrees: Rotation angle in degrees, clockwise.
 *
 * Rotates all control points about the origin by @degrees.
 */
GRL_AVAILABLE_IN_ALL
void        grl_path_rotate     (GrlPath *self,
                                 gfloat   degrees);

/*
 * Boolean operations (raster-approximate)
 *
 * The following functions combine two paths using pixel-level coverage masks.
 * Each operand is rasterized to an 8-bit mask at a working resolution chosen
 * to cover the union of both bounding boxes with a small margin. Pixels are
 * combined per-channel (union=max, intersect=min, subtract=a*(255-b)/255,
 * xor=|a-b|). The resulting combined mask is then traced back into a new
 * GrlPath via marching-squares contour following with collinear-run
 * collapsing.
 *
 * LIMITATIONS — resolution-dependent approximation:
 *   - Subpixel features smaller than the working pixel are lost.
 *   - Coincident edges shared by both operands may produce thin artefact
 *     strips; widen strokes or add a small blur before combining if this
 *     occurs.
 *   - The marching-squares tracer follows cell boundaries at half-integer
 *     coordinates, so the output path has vertices snapped to that grid.
 *   - An exact vector clipper may replace this backend in a future release
 *     without changing the public API.
 *
 * All operations return a new path (transfer full). Either operand may be
 * empty; the result is the identity for that operation (e.g. union(empty, B)
 * returns a copy of B).
 */

/**
 * grl_path_union:
 * @a: First operand path.
 * @b: Second operand path.
 *
 * Returns the raster-approximate union of @a and @b: the region covered by
 * @a, by @b, or by both.
 *
 * Returns: (transfer full): A new #GrlPath.
 */
GRL_AVAILABLE_IN_ALL
GrlPath *   grl_path_union      (GrlPath *a,
                                 GrlPath *b);

/**
 * grl_path_intersect:
 * @a: First operand path.
 * @b: Second operand path.
 *
 * Returns the raster-approximate intersection of @a and @b: the region
 * covered by both.
 *
 * Returns: (transfer full): A new #GrlPath.
 */
GRL_AVAILABLE_IN_ALL
GrlPath *   grl_path_intersect  (GrlPath *a,
                                 GrlPath *b);

/**
 * grl_path_subtract:
 * @a: First operand path (base).
 * @b: Second operand path (to subtract).
 *
 * Returns the raster-approximate difference @a minus @b: the region covered
 * by @a but not @b.
 *
 * Returns: (transfer full): A new #GrlPath.
 */
GRL_AVAILABLE_IN_ALL
GrlPath *   grl_path_subtract   (GrlPath *a,
                                 GrlPath *b);

/**
 * grl_path_xor:
 * @a: First operand path.
 * @b: Second operand path.
 *
 * Returns the raster-approximate symmetric difference of @a and @b: the
 * region covered by exactly one of the two paths.
 *
 * Returns: (transfer full): A new #GrlPath.
 */
GRL_AVAILABLE_IN_ALL
GrlPath *   grl_path_xor        (GrlPath *a,
                                 GrlPath *b);

G_END_DECLS
