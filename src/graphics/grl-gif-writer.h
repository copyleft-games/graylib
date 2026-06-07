/* grl-gif-writer.h
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Streaming animated GIF writer for multi-frame export.
 */

#pragma once

#if !defined(GRAYLIB_INSIDE) && !defined(GRAYLIB_COMPILATION)
#error "Only <graylib.h> can be included directly."
#endif

#include <glib-object.h>
#include "../grl-version.h"
#include "../grl-types.h"
#include "../grl-enums.h"

G_BEGIN_DECLS

#define GRL_TYPE_GIF_WRITER (grl_gif_writer_get_type ())

GRL_AVAILABLE_IN_ALL
G_DECLARE_FINAL_TYPE (GrlGifWriter, grl_gif_writer, GRL, GIF_WRITER, GObject)

/**
 * GrlGifError:
 * @GRL_GIF_ERROR_OPEN: The output file could not be opened.
 * @GRL_GIF_ERROR_WRITE: A write to the output file failed.
 * @GRL_GIF_ERROR_CLOSED: The writer has already been closed.
 *
 * Errors that can occur while writing an animated GIF.
 */
typedef enum
{
    GRL_GIF_ERROR_OPEN,
    GRL_GIF_ERROR_WRITE,
    GRL_GIF_ERROR_CLOSED
} GrlGifError;

#define GRL_GIF_ERROR (grl_gif_error_quark ())

GRL_AVAILABLE_IN_ALL
GQuark grl_gif_error_quark (void);

/**
 * grl_gif_writer_new:
 * @filename: (type filename): Output GIF path.
 * @width: Canvas width in pixels.
 * @height: Canvas height in pixels.
 * @loop_count: Number of times to loop (0 = loop forever).
 * @error: (nullable): Return location for error, or %NULL.
 *
 * Creates a streaming animated GIF writer and opens @filename for writing,
 * emitting the GIF header. Add frames with grl_gif_writer_add_frame() and
 * finish with grl_gif_writer_close().
 *
 * Returns: (transfer full) (nullable): A new #GrlGifWriter, or %NULL on error.
 */
GRL_AVAILABLE_IN_ALL
GrlGifWriter *      grl_gif_writer_new          (const gchar  *filename,
                                                 gint          width,
                                                 gint          height,
                                                 gint          loop_count,
                                                 GError      **error);

/**
 * grl_gif_writer_add_frame:
 * @self: A #GrlGifWriter.
 * @frame: The image for this frame.
 * @delay_centiseconds: Display time for this frame, in 1/100ths of a second.
 * @error: (nullable): Return location for error, or %NULL.
 *
 * Appends @frame to the GIF. The image is converted to RGBA and, if its size
 * differs from the writer's canvas, scaled to fit.
 *
 * Returns: %TRUE on success.
 */
GRL_AVAILABLE_IN_ALL
gboolean            grl_gif_writer_add_frame    (GrlGifWriter *self,
                                                 GrlImage     *frame,
                                                 gint          delay_centiseconds,
                                                 GError      **error);

/**
 * grl_gif_writer_close:
 * @self: A #GrlGifWriter.
 * @error: (nullable): Return location for error, or %NULL.
 *
 * Writes the GIF trailer and closes the output file. After this the writer can
 * no longer accept frames. Closing is idempotent.
 *
 * Returns: %TRUE on success.
 */
GRL_AVAILABLE_IN_ALL
gboolean            grl_gif_writer_close        (GrlGifWriter *self,
                                                 GError      **error);

/*
 * Quantization and dithering controls
 *
 * All setters must be called before the first grl_gif_writer_add_frame(). The
 * defaults reproduce the exact byte-for-byte output of the original encoder
 * (web-safe palette, no dithering, global palette scope, 256 colours, no
 * transparency).
 */

/**
 * grl_gif_writer_set_quantizer:
 * @self: A #GrlGifWriter.
 * @quantizer: The palette-building strategy to use.
 *
 * Sets the quantizer used to build the colour palette.
 *
 * The default is %GRL_GIF_QUANTIZER_WEB_SAFE, which produces output
 * byte-for-byte identical to the original encoder. %GRL_GIF_QUANTIZER_MEDIAN_CUT
 * builds an adaptive palette from the image data; in that mode the Global
 * Color Table is deferred and written on the first frame.
 */
GRL_AVAILABLE_IN_ALL
void                grl_gif_writer_set_quantizer    (GrlGifWriter     *self,
                                                     GrlGifQuantizer   quantizer);

/**
 * grl_gif_writer_set_dither:
 * @self: A #GrlGifWriter.
 * @dither: The dithering mode to apply during index mapping.
 *
 * Sets the dithering mode. Only has an effect when the quantizer is
 * %GRL_GIF_QUANTIZER_MEDIAN_CUT. The default is %GRL_GIF_DITHER_NONE.
 */
GRL_AVAILABLE_IN_ALL
void                grl_gif_writer_set_dither       (GrlGifWriter     *self,
                                                     GrlGifDither      dither);

/**
 * grl_gif_writer_set_palette_scope:
 * @self: A #GrlGifWriter.
 * @scope: Whether to use a single global palette or a per-frame local palette.
 *
 * Sets the palette scope for median-cut encoding.
 *
 * %GRL_GIF_PALETTE_SCOPE_GLOBAL (default): one palette is built from the
 * first frame and used for all subsequent frames. %GRL_GIF_PALETTE_SCOPE_PER_FRAME:
 * each frame gets its own Local Color Table. Ignored when the quantizer is
 * %GRL_GIF_QUANTIZER_WEB_SAFE.
 */
GRL_AVAILABLE_IN_ALL
void                grl_gif_writer_set_palette_scope (GrlGifWriter     *self,
                                                      GrlGifPaletteScope scope);

/**
 * grl_gif_writer_set_max_colors:
 * @self: A #GrlGifWriter.
 * @max_colors: Maximum number of palette entries. Clamped to [2, 256].
 *
 * Sets the maximum palette size for median-cut quantization. Ignored when
 * the quantizer is %GRL_GIF_QUANTIZER_WEB_SAFE. The default is 256.
 *
 * The GIF format requires the palette size to be a power of two; the encoder
 * rounds up to the next power of two internally and pads unused entries with
 * black.
 */
GRL_AVAILABLE_IN_ALL
void                grl_gif_writer_set_max_colors   (GrlGifWriter *self,
                                                     gint          max_colors);

/**
 * grl_gif_writer_set_transparency:
 * @self: A #GrlGifWriter.
 * @enabled: %TRUE to enable transparency support.
 * @alpha_threshold: Pixels with alpha strictly below this value are treated as
 *   fully transparent. Ignored when @enabled is %FALSE.
 *
 * Enables or disables transparency. When enabled, one palette slot is reserved
 * as the transparent index (the last slot in the palette), the Graphic Control
 * Extension transparency flag is set, and the disposal method is changed to 2
 * (restore to background colour) so subsequent frames do not bleed through
 * transparent regions. Only meaningful with %GRL_GIF_QUANTIZER_MEDIAN_CUT.
 *
 * The default is disabled with an alpha threshold of 128.
 */
GRL_AVAILABLE_IN_ALL
void                grl_gif_writer_set_transparency (GrlGifWriter *self,
                                                     gboolean      enabled,
                                                     guint8        alpha_threshold);

G_END_DECLS
