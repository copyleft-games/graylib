/* grl-image-accumulator.h
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Float-precision RGBA accumulation buffer for frame averaging.
 *
 * GrlImageAccumulator sums multiple GrlImage frames in float precision and
 * resolves the weighted average back to an 8-bit RGBA GrlImage.  Typical uses
 * are motion blur (accumulate N sub-frame renders, equal weight) and temporal
 * anti-aliasing (accumulate frames with exponential or history weights).
 *
 * When the @linear flag is TRUE at construction time each sRGB sample is
 * converted to linear light before accumulation, and the resolved average is
 * converted back to sRGB on resolve().  This avoids the perceptual darkening
 * that straight-gamma averaging produces.  When @linear is FALSE samples are
 * accumulated in their raw 8-bit domain (gamma space) — mathematically simpler
 * but less physically correct for colours other than grey.
 */

#pragma once

#if !defined(GRAYLIB_INSIDE) && !defined(GRAYLIB_COMPILATION)
#error "Only <graylib.h> can be included directly."
#endif

#include <glib-object.h>
#include "../grl-version.h"
#include "../grl-types.h"

G_BEGIN_DECLS

#define GRL_TYPE_IMAGE_ACCUMULATOR (grl_image_accumulator_get_type ())

GRL_AVAILABLE_IN_ALL
G_DECLARE_FINAL_TYPE (GrlImageAccumulator, grl_image_accumulator,
                      GRL, IMAGE_ACCUMULATOR, GObject)

/**
 * grl_image_accumulator_new:
 * @width: Canvas width in pixels (must be > 0).
 * @height: Canvas height in pixels (must be > 0).
 * @linear: If %TRUE, convert sRGB<->linear for each sample; if %FALSE,
 *   accumulate raw 8-bit values in gamma space.
 *
 * Creates a new #GrlImageAccumulator with a zeroed float buffer.
 *
 * Call grl_image_accumulator_add() for each sub-frame sample, then
 * grl_image_accumulator_resolve() to obtain the averaged output image.
 * Use grl_image_accumulator_reset() to start a new accumulation sequence
 * without allocating a new object.
 *
 * Returns: (transfer full): A new #GrlImageAccumulator.
 */
GRL_AVAILABLE_IN_ALL
GrlImageAccumulator *   grl_image_accumulator_new       (gint          width,
                                                         gint          height,
                                                         gboolean      linear);

/**
 * grl_image_accumulator_add:
 * @self: A #GrlImageAccumulator.
 * @frame: The source image to accumulate.
 * @weight: Contribution weight for this sample (typically 1.0).
 *
 * Adds @frame × @weight into the accumulation buffer and increments the
 * total weight by @weight.  If @frame has different dimensions from the
 * accumulator canvas it is resampled to the canvas size before accumulation
 * using grl_image_scaled_nearest().
 *
 * Samples are stored internally in the [0, 1] float domain (normalised from
 * the 8-bit source, regardless of the @linear flag).  When @linear is TRUE
 * the sRGB-encoded values are converted to linear light before being scaled
 * by @weight; when @linear is FALSE the raw normalised values are scaled
 * directly.
 *
 * The alpha channel is always treated as a linear coverage value and is
 * normalised identically in both modes.
 */
GRL_AVAILABLE_IN_ALL
void                    grl_image_accumulator_add       (GrlImageAccumulator *self,
                                                         GrlImage            *frame,
                                                         gfloat               weight);

/**
 * grl_image_accumulator_resolve:
 * @self: A #GrlImageAccumulator.
 *
 * Divides the accumulated sums by the total weight and produces an R8G8B8A8
 * #GrlImage containing the weighted average.  If the total weight is zero
 * (no samples have been added, or all weights were zero) the function returns
 * %NULL without touching the buffer — the accumulator remains valid and can
 * continue to receive samples.
 *
 * When @linear was TRUE at construction time, the averaged linear values are
 * converted back to sRGB before packing into the 8-bit output.  All channels
 * are clamped to [0, 255] before writing.
 *
 * Returns: (transfer full) (nullable): A new #GrlImage, or %NULL if the
 *   total weight is zero.
 */
GRL_AVAILABLE_IN_ALL
GrlImage *              grl_image_accumulator_resolve   (GrlImageAccumulator *self);

/**
 * grl_image_accumulator_reset:
 * @self: A #GrlImageAccumulator.
 *
 * Zeroes the accumulation buffer and resets the total weight to zero,
 * preparing the accumulator for the next output frame without re-allocating
 * memory.
 */
GRL_AVAILABLE_IN_ALL
void                    grl_image_accumulator_reset     (GrlImageAccumulator *self);

G_END_DECLS
