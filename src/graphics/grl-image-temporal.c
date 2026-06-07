/* grl-image-temporal.c
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Onion-skinning and temporal anti-aliasing helpers.
 */

#include "config.h"
#include "grl-image-temporal.h"
#include "grl-image.h"
#include "grl-image-accumulator.h"
#include "../grl-enums.h"
#include "../math/grl-color.h"
#include "../math/grl-rectangle.h"

/* ---------------------------------------------------------------------------
 * Internal helpers
 * --------------------------------------------------------------------------- */

/*
 * make_ghost_tint:
 * @base:    Base tint from the caller (NULL = opaque white).
 * @opacity: Scalar opacity in [0, 1].
 *
 * Returns a GrlColor whose RGB channels come from @base (or 255,255,255 if
 * NULL) and whose alpha is (guint8)(CLAMP(opacity, 0.0f, 1.0f) * 255.0f + 0.5f).
 *
 * This is intentionally a value type returned on the stack; callers pass its
 * address to the drawing API.
 */
static GrlColor
make_ghost_tint (const GrlColor *base,
                 gfloat          opacity)
{
    GrlColor tint;
    guint8   alpha;
    gfloat   clamped;

    clamped = CLAMP (opacity, 0.0f, 1.0f);
    alpha   = (guint8)(clamped * 255.0f + 0.5f);

    if (base != NULL)
    {
        tint.r = base->r;
        tint.g = base->g;
        tint.b = base->b;
    }
    else
    {
        tint.r = 255;
        tint.g = 255;
        tint.b = 255;
    }

    tint.a = alpha;
    return tint;
}

/*
 * draw_ghost:
 * @current: Destination image (blend mode already set to OVER by caller).
 * @ghost:   Source image to composite.
 * @tint:    Tint colour including pre-computed alpha.
 *
 * Composites @ghost onto @current at its natural size (full-image dst_rect).
 * NULL @ghost is silently skipped.
 */
static void
draw_ghost (GrlImage       *current,
            GrlImage       *ghost,
            const GrlColor *tint)
{
    GrlRectangle dst;

    if (ghost == NULL)
        return;

    /* dst covers the entire destination canvas. */
    dst.x      = 0.0f;
    dst.y      = 0.0f;
    dst.width  = (gfloat)grl_image_get_width  (current);
    dst.height = (gfloat)grl_image_get_height (current);

    /*
     * src_rect is NULL → full source image.
     * tint encodes the ghost opacity in its alpha channel.
     * GRL_IMAGE_BLEND_OVER on @current ensures alpha-correct compositing.
     */
    grl_image_draw_image (current, ghost, NULL, &dst, tint);
}

/* ---------------------------------------------------------------------------
 * Public API
 * --------------------------------------------------------------------------- */

/**
 * grl_image_onion_skin:
 * @current: The target frame that will be modified in place.
 * @prev: (nullable): The previous frame to ghost.
 * @next: (nullable): The next frame to ghost.
 * @prev_opacity: Opacity of the previous-frame ghost, in [0, 1].
 * @next_opacity: Opacity of the next-frame ghost, in [0, 1].
 * @prev_tint: (nullable): Hue tint for the previous ghost (NULL = white).
 * @next_tint: (nullable): Hue tint for the next ghost (NULL = white).
 *
 * Composites semi-transparent ghosts of @prev and @next onto @current in
 * place using OVER blending.  See grl-image-temporal.h for full contract.
 */
void
grl_image_onion_skin (GrlImage           *current,
                      GrlImage           *prev,
                      GrlImage           *next,
                      gfloat              prev_opacity,
                      gfloat              next_opacity,
                      const GrlColor     *prev_tint,
                      const GrlColor     *next_tint)
{
    GrlImageBlendMode saved_mode;
    GrlColor          prev_col;
    GrlColor          next_col;

    g_return_if_fail (GRL_IS_IMAGE (current));

    /*
     * OVER blending requires R8G8B8A8.  Silently degrade on other formats so
     * callers don't crash when passing an RGB or grayscale image — the same
     * contract used by the rest of the image drawing API (see grl-image.c
     * drawing-state comment: "Non-REPLACE blending … require R8G8B8A8; on any
     * other format the draw silently falls back to REPLACE").  We go one step
     * further and skip the draw entirely so that the REPLACE-mode fallback
     * does not produce visible artefacts (the ghost would be drawn opaque).
     */
    if (grl_image_get_format (current) != GRL_PIXELFORMAT_UNCOMPRESSED_R8G8B8A8)
    {
        g_debug ("grl_image_onion_skin: @current is not R8G8B8A8; skipping ghost composite");
        return;
    }

    /* Nothing to do if both ghosts are invisible. */
    if ((prev == NULL || prev_opacity <= 0.0f) &&
        (next == NULL || next_opacity <= 0.0f))
        return;

    /* Save and switch blend mode. */
    saved_mode = grl_image_get_blend_mode (current);
    grl_image_set_blend_mode (current, GRL_IMAGE_BLEND_OVER);

    /* Draw previous ghost. */
    if (prev != NULL && prev_opacity > 0.0f)
    {
        prev_col = make_ghost_tint (prev_tint, prev_opacity);
        draw_ghost (current, prev, &prev_col);
    }

    /* Draw next ghost. */
    if (next != NULL && next_opacity > 0.0f)
    {
        next_col = make_ghost_tint (next_tint, next_opacity);
        draw_ghost (current, next, &next_col);
    }

    /* Restore caller's blend mode. */
    grl_image_set_blend_mode (current, saved_mode);
}

/**
 * grl_image_temporal_aa:
 * @acc: A #GrlImageAccumulator.
 * @samples: (array length=n_samples): Jittered sample images.
 * @n_samples: Number of samples (must be ≥ 1).
 *
 * Resets @acc, accumulates each sample at weight 1.0, and returns the
 * resolved average.  See grl-image-temporal.h for full contract.
 *
 * Returns: (transfer full) (nullable): Resolved #GrlImage, or %NULL.
 */
GrlImage *
grl_image_temporal_aa (GrlImageAccumulator *acc,
                       GrlImage           **samples,
                       gint                 n_samples)
{
    gint i;

    g_return_val_if_fail (acc != NULL, NULL);
    g_return_val_if_fail (n_samples >= 1, NULL);

    grl_image_accumulator_reset (acc);

    for (i = 0; i < n_samples; i++)
        grl_image_accumulator_add (acc, samples[i], 1.0f);

    return grl_image_accumulator_resolve (acc);
}
