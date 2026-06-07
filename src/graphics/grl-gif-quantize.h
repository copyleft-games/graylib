/* grl-gif-quantize.h
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Internal palette-building and index-mapping routines for GrlGifWriter.
 * NOT part of the public API; not installed.
 */

#pragma once

#include <glib.h>
#include "../grl-enums.h"

G_BEGIN_DECLS

/**
 * grl_gif_median_cut:
 * @rgba: Packed RGBA8 pixel buffer (4 bytes per pixel, no padding).
 * @n_pixels: Number of pixels in @rgba.
 * @max_colors: Maximum palette entries to produce. Clamped to [2, 256].
 * @palette: (out caller-allocates) (array fixed-size=768): Output buffer for
 *   the palette. Must be at least 768 bytes (256 * 3). Each entry is
 *   three consecutive bytes: R, G, B. The buffer is zero-initialised for
 *   unused slots beyond the returned count.
 *
 * Builds a palette of at most @max_colors RGB entries from @rgba using a
 * median-cut algorithm. Alpha is used only for transparency gating (pixels
 * that are fully transparent, alpha == 0, are excluded from colour sampling
 * so they do not pollute the palette with the fill colour). The palette slots
 * themselves contain opaque RGB triples; transparency is communicated through
 * a dedicated index reserved by the caller via grl_gif_map_indices().
 *
 * Returns: The number of palette entries written (>= 1, <= @max_colors).
 */
gint grl_gif_median_cut (const guint8 *rgba,
                         gint          n_pixels,
                         gint          max_colors,
                         guint8        palette[768]);

/**
 * grl_gif_map_indices:
 * @rgba: Packed RGBA8 pixel buffer (4 bytes per pixel).
 * @width: Image width in pixels.
 * @height: Image height in pixels.
 * @palette: RGB palette (3 bytes per entry).
 * @palette_len: Number of entries in @palette.
 * @dither: Dithering mode to apply.
 * @transparent_index: If >= 0, pixels with alpha < @alpha_threshold map to
 *   this palette index rather than to their nearest colour. Set to -1 to
 *   disable transparency mapping.
 * @alpha_threshold: Alpha value below which a pixel is considered transparent.
 *   Ignored when @transparent_index < 0.
 * @out_indices: (out caller-allocates) (array): Output index buffer,
 *   @width * @height bytes. Each byte is the palette index for the
 *   corresponding pixel, in row-major order.
 *
 * Maps each pixel in @rgba to the nearest entry in @palette (Euclidean
 * distance in RGB space). When @dither is %GRL_GIF_DITHER_FLOYD_STEINBERG the
 * quantisation error is diffused to the right, lower-left, lower, and
 * lower-right neighbours with the standard 7/16, 3/16, 5/16, 1/16 weights.
 * The error buffer is allocated internally and freed on return.
 */
void grl_gif_map_indices (const guint8  *rgba,
                          gint           width,
                          gint           height,
                          const guint8  *palette,
                          gint           palette_len,
                          GrlGifDither   dither,
                          gint           transparent_index,
                          guint8         alpha_threshold,
                          guint8        *out_indices);

G_END_DECLS
