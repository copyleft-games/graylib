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

G_END_DECLS
