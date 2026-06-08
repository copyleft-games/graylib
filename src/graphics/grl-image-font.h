/* grl-image-font.h
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Headless TTF/OTF font rasterizer for CPU-side image drawing.
 *
 * GrlImageFont wraps stb_truetype and provides glyph rasterisation that works
 * without a GL context.  Use grl_image_draw_text_ttf() to render a UTF-8
 * string onto any #GrlImage.  The result is composited with
 * %GRL_PORTER_DUFF_SRC_OVER so it honours the destination's clip rectangle
 * and colour space; the per-image blend-mode flag has no effect on this
 * composite step (SRC_OVER is always used).
 *
 * Limitations (v1)
 * ================
 * - stb_truetype uses sub-pixel hinting only; small sizes may look slightly
 *   soft.  Prefer px_size >= 14 for screen text; >= 24 for display headings.
 * - Layout is left-to-right, one-codepoint-to-one-glyph, advance-width only.
 *   RTL/bidi, combining marks, ligatures, and vertical text are not supported.
 *   HarfBuzz-class shaping is future work.
 * - Newlines (\n) advance the pen to the next line; \r is silently ignored.
 * - The 8x8 bitmap API (grl_image_draw_text_bitmap()) remains the zero-
 *   dependency fallback for pixel-art and tiny sizes.
 */

#pragma once

#if !defined(GRAYLIB_INSIDE) && !defined(GRAYLIB_COMPILATION)
#error "Only <graylib.h> can be included directly."
#endif

#include <glib-object.h>
#include "../grl-version.h"
#include "../grl-types.h"

G_BEGIN_DECLS

#define GRL_TYPE_IMAGE_FONT (grl_image_font_get_type ())

GRL_AVAILABLE_IN_ALL
G_DECLARE_FINAL_TYPE (GrlImageFont, grl_image_font, GRL, IMAGE_FONT, GObject)

/* -------------------------------------------------------------------------
 * Error domain
 * -------------------------------------------------------------------------
 */

/**
 * GrlImageFontError:
 * @GRL_IMAGE_FONT_ERROR_OPEN:  The font file could not be opened or read.
 * @GRL_IMAGE_FONT_ERROR_PARSE: The font data could not be parsed (not a valid
 *                              TTF/OTF, or the requested font index is absent).
 *
 * Errors that can occur while loading a font.
 */
typedef enum
{
    GRL_IMAGE_FONT_ERROR_OPEN,
    GRL_IMAGE_FONT_ERROR_PARSE
} GrlImageFontError;

#define GRL_IMAGE_FONT_ERROR (grl_image_font_error_quark ())

GRL_AVAILABLE_IN_ALL
GQuark grl_image_font_error_quark (void);

/* -------------------------------------------------------------------------
 * Constructors
 * -------------------------------------------------------------------------
 */

/**
 * grl_image_font_new_from_file:
 * @filename: (type filename): Path to a TTF or OTF font file.
 * @error: (nullable): Return location for error, or %NULL.
 *
 * Loads a TrueType or OpenType font from @filename.
 *
 * On failure sets @error and returns %NULL.  The returned font is fully
 * self-contained; the on-disk file is not needed after this call.
 *
 * Returns: (transfer full) (nullable): A new #GrlImageFont, or %NULL on error.
 */
GRL_AVAILABLE_IN_ALL
GrlImageFont *  grl_image_font_new_from_file   (const gchar  *filename,
                                                GError      **error);

/**
 * grl_image_font_new_from_memory:
 * @data: (array length=len) (element-type guint8): Font file bytes.
 * @len: Number of bytes in @data.
 * @error: (nullable): Return location for error, or %NULL.
 *
 * Loads a font from an in-memory buffer.  @data is copied internally; the
 * caller may free it immediately after this call.
 *
 * Returns: (transfer full) (nullable): A new #GrlImageFont, or %NULL on error.
 */
GRL_AVAILABLE_IN_ALL
GrlImageFont *  grl_image_font_new_from_memory (const guint8 *data,
                                                gsize         len,
                                                GError      **error);

/* -------------------------------------------------------------------------
 * Metrics
 * -------------------------------------------------------------------------
 */

/**
 * grl_image_font_get_v_metrics:
 * @self: A #GrlImageFont.
 * @px_size: Desired pixel height (em height, > 0).
 * @ascent: (out) (optional): Return location for the ascent in pixels, or %NULL.
 * @descent: (out) (optional): Return location for the descent in pixels (≤ 0),
 *   or %NULL.
 * @line_gap: (out) (optional): Return location for the recommended extra inter-
 *   line gap in pixels, or %NULL.
 *
 * Queries the vertical metrics of the font scaled to @px_size.
 *
 * The signed sum (@ascent - @descent + @line_gap) gives the recommended
 * line height.  Callers may omit any of the out-parameters by passing %NULL.
 */
GRL_AVAILABLE_IN_ALL
void            grl_image_font_get_v_metrics   (GrlImageFont *self,
                                                gfloat        px_size,
                                                gfloat       *ascent,
                                                gfloat       *descent,
                                                gfloat       *line_gap);

/**
 * grl_image_font_has_glyph:
 * @self: A #GrlImageFont.
 * @codepoint: A Unicode codepoint.
 *
 * Returns %TRUE if the font contains a glyph for @codepoint.
 *
 * Returns: %TRUE if the glyph is present.
 */
GRL_AVAILABLE_IN_ALL
gboolean        grl_image_font_has_glyph       (GrlImageFont *self,
                                                gunichar      codepoint);

/* -------------------------------------------------------------------------
 * Drawing and measurement (operate on GrlImage)
 * -------------------------------------------------------------------------
 */

/**
 * grl_image_draw_text_ttf:
 * @self: A #GrlImage to draw onto.
 * @font: The #GrlImageFont to use.
 * @text: (nullable): UTF-8 text to render.
 * @x: Left edge of the first character's em box, in destination-image pixels.
 * @y: Top edge of the first line's em box, in destination-image pixels.
 * @px_size: Desired em height in pixels (must be > 0; a ≤ 0 value is a no-op).
 * @color: Foreground colour.  The alpha channel is modulated by each glyph's
 *   anti-aliasing coverage to produce smooth edges.
 *
 * Renders @text onto @self at (@x, @y) using @font at @px_size pixels.
 *
 * Each glyph is rasterised to a temporary RGBA layer with per-pixel coverage
 * from stb_truetype, then composited onto @self with
 * %GRL_PORTER_DUFF_SRC_OVER.  This composite step honours @self's clip
 * rectangle and colour space; the per-image blend-mode property is not
 * consulted for this operation (SRC_OVER is always used).
 *
 * Kerning between adjacent codepoints is applied via the font's kern table.
 * Newlines (\n) advance the pen to the next line.
 *
 * If @text is %NULL or empty, or if @px_size is ≤ 0, the function returns
 * without modifying @self.
 *
 * This function works fully headless (no GL context needed).
 */
GRL_AVAILABLE_IN_ALL
void            grl_image_draw_text_ttf        (GrlImage           *self,
                                                GrlImageFont       *font,
                                                const gchar        *text,
                                                gint                x,
                                                gint                y,
                                                gfloat              px_size,
                                                const GrlColor     *color);

/**
 * grl_image_measure_text_ttf:
 * @font: A #GrlImageFont.
 * @text: (nullable): UTF-8 text to measure.
 * @px_size: Desired em height in pixels.
 *
 * Measures @text without rasterising it.
 *
 * The returned vector's @x component holds the maximum advance width across
 * all lines; @y holds the total rendered height (line height × number of
 * lines, where line height = ascent − descent + line_gap).
 *
 * If @text is %NULL, empty, or @px_size is ≤ 0, returns a (0, 0) vector.
 *
 * Returns: (transfer full): Bounding dimensions as a #GrlVector2.
 *   Free with grl_vector2_free().
 */
GRL_AVAILABLE_IN_ALL
GrlVector2 *    grl_image_measure_text_ttf     (GrlImageFont       *font,
                                                const gchar        *text,
                                                gfloat              px_size);

G_END_DECLS
