/* grl-svg.h
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * SVG import / export with GrlPath as the geometry carrier.
 *
 * Supported subset (import and export):
 *
 *   Elements:
 *     <svg>, <g>, <path>, <rect>, <circle>, <ellipse>,
 *     <line>, <polyline>, <polygon>
 *
 *   Path mini-language:
 *     M/m  L/l  H/h  V/v  C/c  S/s  Q/q  T/t  Z/z
 *     A/a  — approximated with up to 8 cubic-bezier segments per arc,
 *             using the standard SVG arc-to-bezier conversion.
 *
 *   Presentation attributes:
 *     fill, stroke          — #rgb, #rrggbb, rgb(r,g,b), named colours
 *                             (black, white, red, green, blue, yellow,
 *                              cyan, magenta, gray, grey, lime, navy,
 *                              maroon, purple, teal, silver, orange,
 *                              transparent, none)
 *     fill-opacity, stroke-opacity, opacity
 *     stroke-width
 *     fill-rule             — nonzero / evenodd
 *     transform             — translate(), scale(), rotate(), matrix()
 *                             (composed and baked into path coordinates)
 *
 *   Groups (<g>):
 *     transform and paint attributes are inherited and composed onto
 *     children.  The output is a flat list of shapes with baked
 *     coordinates (no <g> elements on export).
 *
 * Out of scope (silently skipped, never an error):
 *   <linearGradient>, <radialGradient>, <pattern>, <filter>,
 *   <mask>, <clipPath>, <text>, <tspan>, <use>, <defs>, <symbol>,
 *   <animate*>, raster <image>, CSS <style> blocks, inline style=,
 *   marker, viewBox transforms (treated as 1:1 user-unit mapping),
 *   stroke-linecap/-linejoin/-dasharray and similar advanced stroke attrs.
 */

#pragma once

#if !defined(GRAYLIB_INSIDE) && !defined(GRAYLIB_COMPILATION)
#error "Only <graylib.h> can be included directly."
#endif

#include <glib-object.h>
#include "../grl-version.h"
#include "../grl-types.h"
#include "../grl-enums.h"
#include "../math/grl-color.h"
#include "grl-path.h"
#include "grl-image.h"

G_BEGIN_DECLS

/* -------------------------------------------------------------------------
 * Error domain
 * ------------------------------------------------------------------------- */

/**
 * GrlSvgError:
 * @GRL_SVG_ERROR_PARSE: The SVG data could not be parsed (malformed XML or
 *   an unsupported structural feature that prevents further parsing).
 * @GRL_SVG_ERROR_OPEN: The file could not be opened for reading or writing.
 *
 * Errors that can occur during SVG import or export.
 */
typedef enum
{
    GRL_SVG_ERROR_PARSE,
    GRL_SVG_ERROR_OPEN
} GrlSvgError;

#define GRL_SVG_ERROR (grl_svg_error_quark ())

GRL_AVAILABLE_IN_ALL
GQuark grl_svg_error_quark (void);

/* -------------------------------------------------------------------------
 * GrlVectorShape — a GrlPath bundled with paint
 * ------------------------------------------------------------------------- */

/**
 * GrlVectorShape:
 * @path: The geometry, owned by this shape.  Free with grl_path_unref().
 * @has_fill: %TRUE if the shape should be filled.
 * @fill: Fill color (valid when @has_fill is %TRUE).
 * @has_stroke: %TRUE if the shape should be stroked.
 * @stroke: Stroke color (valid when @has_stroke is %TRUE).
 * @stroke_width: Stroke width in user units.
 * @fill_rule: Winding rule for filling.
 *
 * A vector shape: one #GrlPath plus paint properties.
 *
 * #GrlColor is embedded by value (four bytes); the @path field holds an
 * owning GObject reference that is released when the shape is freed via
 * grl_vector_shape_free().
 *
 * This is a #GBoxed type.
 */
typedef struct _GrlVectorShape GrlVectorShape;

struct _GrlVectorShape
{
    GrlPath    *path;           /* owned ref */

    gboolean    has_fill;
    GrlColor    fill;

    gboolean    has_stroke;
    GrlColor    stroke;
    gfloat      stroke_width;

    GrlFillRule fill_rule;
};

#define GRL_TYPE_VECTOR_SHAPE (grl_vector_shape_get_type ())

GRL_AVAILABLE_IN_ALL
GType             grl_vector_shape_get_type (void) G_GNUC_CONST;

/**
 * grl_vector_shape_new:
 *
 * Allocates and zero-initialises a new #GrlVectorShape backed by an empty
 * #GrlPath.  The caller owns the returned value; free with
 * grl_vector_shape_free().
 *
 * Returns: (transfer full): A newly allocated #GrlVectorShape.
 */
GRL_AVAILABLE_IN_ALL
GrlVectorShape *  grl_vector_shape_new      (void);

/**
 * grl_vector_shape_copy:
 * @self: (nullable): A #GrlVectorShape.
 *
 * Creates a deep copy of @self, including a copy of its #GrlPath.
 *
 * Returns: (transfer full) (nullable): A new independent #GrlVectorShape,
 *   or %NULL if @self was %NULL.
 */
GRL_AVAILABLE_IN_ALL
GrlVectorShape *  grl_vector_shape_copy     (const GrlVectorShape *self);

/**
 * grl_vector_shape_free:
 * @self: (nullable): A #GrlVectorShape.
 *
 * Unrefs the embedded #GrlPath and frees @self.  %NULL is silently ignored.
 */
GRL_AVAILABLE_IN_ALL
void              grl_vector_shape_free     (GrlVectorShape *self);

/**
 * grl_vector_shape_get_path:
 * @self: A #GrlVectorShape.
 *
 * Returns: (transfer none): The #GrlPath owned by this shape.
 */
GRL_AVAILABLE_IN_ALL
GrlPath *         grl_vector_shape_get_path (GrlVectorShape *self);

G_DEFINE_AUTOPTR_CLEANUP_FUNC (GrlVectorShape, grl_vector_shape_free)

/* -------------------------------------------------------------------------
 * Import
 * ------------------------------------------------------------------------- */

/**
 * grl_svg_load_from_file:
 * @filename: (type filename): Path to an SVG file.
 * @dpi: Device pixels per inch used to resolve physical units (px, pt, mm …).
 *   Pass 0 to use the default of 96 dpi.
 * @n_shapes: (out): Set to the number of shapes in the returned array.
 * @error: (nullable): Return location for error, or %NULL.
 *
 * Parses an SVG file and returns a %NULL-terminated array of #GrlVectorShape
 * objects representing the geometry and paint of each rendered element.
 * Groups are flattened; transforms are baked into path coordinates.
 *
 * The returned array is owned by the caller.  Free each element with
 * grl_vector_shape_free(), then free the array itself with g_free():
 *
 * |[<!-- language="C" -->
 * guint n = 0;
 * GrlVectorShape **shapes = grl_svg_load_from_file ("icon.svg", 96, &n, NULL);
 * for (guint i = 0; i < n; i++)
 *     grl_vector_shape_free (shapes[i]);
 * g_free (shapes);
 * ]|
 *
 * Returns: (transfer full) (array length=n_shapes) (nullable):
 *   An array of #GrlVectorShape pointers, or %NULL on error.
 */
GRL_AVAILABLE_IN_ALL
GrlVectorShape ** grl_svg_load_from_file   (const gchar  *filename,
                                            gfloat        dpi,
                                            guint        *n_shapes,
                                            GError      **error);

/**
 * grl_svg_load_from_memory:
 * @data: Pointer to SVG source bytes.
 * @len: Length of @data in bytes.
 * @dpi: Device pixels per inch (0 → 96).
 * @n_shapes: (out): Number of shapes in the returned array.
 * @error: (nullable): Return location for error, or %NULL.
 *
 * Parses SVG from an in-memory buffer.  See grl_svg_load_from_file() for
 * full semantics and the required free pattern.
 *
 * Returns: (transfer full) (array length=n_shapes) (nullable):
 *   An array of #GrlVectorShape pointers, or %NULL on error.
 */
GRL_AVAILABLE_IN_ALL
GrlVectorShape ** grl_svg_load_from_memory (const gchar  *data,
                                            gsize         len,
                                            gfloat        dpi,
                                            guint        *n_shapes,
                                            GError      **error);

/**
 * grl_path_new_from_svg_file:
 * @filename: (type filename): Path to an SVG file.
 * @dpi: Device pixels per inch (0 → 96).
 * @error: (nullable): Return location for error, or %NULL.
 *
 * Convenience wrapper: parses the SVG file, builds a single merged #GrlPath
 * from all shape geometries (paint is discarded), and returns it.  Useful
 * when only the combined outline is needed.
 *
 * Returns: (transfer full) (nullable): A new #GrlPath, or %NULL on error.
 */
GRL_AVAILABLE_IN_ALL
GrlPath *         grl_path_new_from_svg_file (const gchar  *filename,
                                              gfloat        dpi,
                                              GError      **error);

/* -------------------------------------------------------------------------
 * Export
 * ------------------------------------------------------------------------- */

/**
 * grl_svg_save_to_file:
 * @shapes: (array length=n_shapes): Array of shapes to serialise.
 * @n_shapes: Number of shapes.
 * @width: SVG canvas width in user units.
 * @height: SVG canvas height in user units.
 * @filename: (type filename): Destination file path.
 * @error: (nullable): Return location for error, or %NULL.
 *
 * Serialises @shapes to an SVG 1.1 file.  Each shape is emitted as a
 * <path> element with baked coordinates (no <g> nesting, no transform
 * attributes).  Path data uses only M, L, C, Q, Z commands so that re-
 * import is geometry-lossless over the documented subset.
 *
 * Returns: %TRUE on success, %FALSE on error (with @error set).
 */
GRL_AVAILABLE_IN_ALL
gboolean          grl_svg_save_to_file      (GrlVectorShape * const *shapes,
                                             guint                   n_shapes,
                                             gint                    width,
                                             gint                    height,
                                             const gchar            *filename,
                                             GError                **error);

/**
 * grl_svg_to_string:
 * @shapes: (array length=n_shapes): Array of shapes to serialise.
 * @n_shapes: Number of shapes.
 * @width: SVG canvas width.
 * @height: SVG canvas height.
 *
 * Serialises @shapes to an SVG string.  See grl_svg_save_to_file() for the
 * format contract.
 *
 * Returns: (transfer full): A newly allocated NUL-terminated SVG string.
 *   Free with g_free().
 */
GRL_AVAILABLE_IN_ALL
gchar *           grl_svg_to_string         (GrlVectorShape * const *shapes,
                                             guint                   n_shapes,
                                             gint                    width,
                                             gint                    height);

/**
 * grl_path_to_svg_string:
 * @self: A #GrlPath.
 *
 * Serialises @self to a bare SVG path data string (the value of the `d`
 * attribute).  Uses only M, L, C, Q, Z commands.
 *
 * Returns: (transfer full): A newly allocated NUL-terminated string.
 *   Free with g_free().
 */
GRL_AVAILABLE_IN_ALL
gchar *           grl_path_to_svg_string    (GrlPath *self);

/* -------------------------------------------------------------------------
 * Rendering
 * ------------------------------------------------------------------------- */

/**
 * grl_image_draw_svg_shapes:
 * @self: A #GrlImage to draw onto.
 * @shapes: (array length=n_shapes): Array of shapes to render.
 * @n_shapes: Number of shapes.
 *
 * Renders each shape from @shapes onto @self in order: fill first (if
 * @has_fill), then stroke (if @has_stroke), using grl_image_fill_path()
 * and grl_image_stroke_path() respectively.
 */
GRL_AVAILABLE_IN_ALL
void              grl_image_draw_svg_shapes (GrlImage              *self,
                                             GrlVectorShape * const *shapes,
                                             guint                   n_shapes);

G_END_DECLS
