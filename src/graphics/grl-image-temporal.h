/* grl-image-temporal.h
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Onion-skinning and temporal anti-aliasing helpers built on GrlImage and
 * GrlImageAccumulator.
 *
 * These are free functions — no new GObject type is introduced.
 *
 * # Onion-skinning
 *
 * grl_image_onion_skin() composites semi-transparent ghosts of the previous
 * and/or next animation frames onto a current frame in place.  It is designed
 * for animation authoring tools that need to see neighbour frames as coloured
 * overlays while drawing.  The target image must be R8G8B8A8; the call is a
 * no-op (with a debug warning) on other pixel formats.
 *
 * # Temporal AA (jittered super-sampling resolve)
 *
 * grl_image_temporal_aa() is a thin helper over #GrlImageAccumulator for the
 * common "accumulate N jittered samples → resolve" pattern.  The caller is
 * responsible for rendering each sample with a sub-pixel offset, typically
 * drawn from a Halton(2,3) or rotated-grid sequence in [-0.5, +0.5] pixels.
 *
 * |[<!-- language="C" -->
 * GrlImageAccumulator *acc = grl_image_accumulator_new (w, h, TRUE);
 * GrlImage *samples[N];
 * for (i = 0; i < N; i++)
 *     samples[i] = render_jittered (dx[i], dy[i]);
 * GrlImage *out = grl_image_temporal_aa (acc, samples, N);
 * for (i = 0; i < N; i++)
 *     g_object_unref (samples[i]);
 * ]|
 */

#pragma once

#if !defined(GRAYLIB_INSIDE) && !defined(GRAYLIB_COMPILATION)
#error "Only <graylib.h> can be included directly."
#endif

#include <glib-object.h>
#include "../grl-version.h"
#include "../grl-types.h"
#include "../math/grl-color.h"

G_BEGIN_DECLS

/**
 * grl_image_onion_skin:
 * @current: The target frame that will be modified in place.
 * @prev: (nullable): The previous frame to ghost, or %NULL to skip.
 * @next: (nullable): The next frame to ghost, or %NULL to skip.
 * @prev_opacity: Opacity of the previous-frame ghost, in [0, 1].
 *   Values ≤ 0 skip the previous ghost entirely.
 * @next_opacity: Opacity of the next-frame ghost, in [0, 1].
 *   Values ≤ 0 skip the next ghost entirely.
 * @prev_tint: (nullable): Base tint for the previous ghost, or %NULL for
 *   opaque white.  The tint alpha is overridden by @prev_opacity.
 * @next_tint: (nullable): Base tint for the next ghost, or %NULL for
 *   opaque white.  The tint alpha is overridden by @next_opacity.
 *
 * Composites semi-transparent ghosts of @prev and @next onto @current in
 * place using source-over (OVER) blending.
 *
 * The ghost alpha is derived by multiplying @prev_opacity (or @next_opacity)
 * by 255 and clamping to [0, 255]; the tint's own alpha component is
 * replaced by this value.  This allows the caller to supply a pure hue tint
 * (e.g. red for previous, blue for next) without having to pre-set its alpha.
 *
 * @current must be a valid #GrlImage with pixel format
 * %GRL_PIXELFORMAT_UNCOMPRESSED_R8G8B8A8.  On any other format the function
 * logs a debug message and returns without modifying @current; this
 * matches the silent-degrade contract of the rest of the image drawing API.
 *
 * The previous blend mode of @current is saved and restored so that the
 * caller's drawing state is not affected.
 *
 * NULL @prev or @next, or an opacity ≤ 0, means that ghost is skipped.
 */
GRL_AVAILABLE_IN_ALL
void        grl_image_onion_skin    (GrlImage           *current,
                                     GrlImage           *prev,
                                     GrlImage           *next,
                                     gfloat              prev_opacity,
                                     gfloat              next_opacity,
                                     const GrlColor     *prev_tint,
                                     const GrlColor     *next_tint);

/**
 * grl_image_temporal_aa:
 * @acc: A #GrlImageAccumulator.  It is reset before accumulation, so any
 *   previously accumulated data is discarded.
 * @samples: (array length=n_samples): Array of #GrlImage frames rendered with
 *   sub-pixel jitter offsets.  Each sample contributes with weight 1.0.  The
 *   caller retains ownership of every element.
 * @n_samples: Number of elements in @samples.  Must be ≥ 1.
 *
 * Resolves a set of jittered sample images into a temporally anti-aliased
 * output via #GrlImageAccumulator.
 *
 * The function resets @acc, adds each element of @samples with weight 1.0,
 * and returns the resolved average image.  The caller is responsible for
 * having rendered each sample with a distinct sub-pixel offset (e.g. a
 * Halton(2,3) or rotated-grid sequence in [−0.5, +0.5] pixels) so that the
 * average approximates the ideal filtered sample.
 *
 * With @n_samples = 1 the output image is identical to the single sample
 * (within the accumulator's rounding tolerance of 1 LSB).
 *
 * Returns: (transfer full) (nullable): A new resolved #GrlImage, or %NULL if
 *   @acc is %NULL, @n_samples is < 1, or the accumulator resolve fails.
 */
GRL_AVAILABLE_IN_ALL
GrlImage *  grl_image_temporal_aa   (GrlImageAccumulator *acc,
                                     GrlImage           **samples,
                                     gint                 n_samples);

G_END_DECLS
