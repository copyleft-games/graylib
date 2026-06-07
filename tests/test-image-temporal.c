/* test-image-temporal.c
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Unit tests for grl_image_onion_skin() and grl_image_temporal_aa().
 * All tests run headless (no window / GL context required).
 */

#include <glib.h>
#include <math.h>
#include "src/graphics/grl-image-temporal.h"
#include "src/graphics/grl-image-accumulator.h"
#include "src/graphics/grl-image.h"
#include "src/grl-enums.h"
#include "src/math/grl-color.h"

/* ---------------------------------------------------------------------------
 * Helpers
 * --------------------------------------------------------------------------- */

/* Assert that pixel (x, y) matches (r, g, b, a) within tolerance @tol. */
static void
assert_pixel (GrlImage *img,
              gint      x,
              gint      y,
              gint      r,
              gint      g,
              gint      b,
              gint      a,
              gint      tol)
{
    g_autoptr(GrlColor) c = grl_image_get_pixel (img, x, y);

    g_assert_nonnull (c);
    g_assert_cmpint (ABS ((gint)c->r - r), <=, tol);
    g_assert_cmpint (ABS ((gint)c->g - g), <=, tol);
    g_assert_cmpint (ABS ((gint)c->b - b), <=, tol);
    g_assert_cmpint (ABS ((gint)c->a - a), <=, tol);
}

/* Create a solid R8G8B8A8 image of the given dimensions. */
static GrlImage *
make_solid_rgba (gint width, gint height,
                 guint8 r, guint8 g, guint8 b, guint8 a)
{
    g_autoptr(GrlColor) col = grl_color_new (r, g, b, a);
    return grl_image_new_color (width, height, col);
}

/*
 * Create a solid image in an RGB (non-RGBA) format.
 *
 * We construct an RGBA image and then convert it to R8G8B8 using
 * grl_image_set_format(), which is available on GrlImage.
 */
static GrlImage *
make_solid_rgb (gint width, gint height,
                guint8 r, guint8 g, guint8 b)
{
    GrlImage *img = make_solid_rgba (width, height, r, g, b, 255);
    grl_image_set_format (img, GRL_PIXELFORMAT_UNCOMPRESSED_R8G8B8);
    return img;
}

/* ---------------------------------------------------------------------------
 * onion_skin tests
 * --------------------------------------------------------------------------- */

/*
 * Both ghosts are NULL → current is completely unchanged.
 */
static void
test_onion_skin_no_ghosts_noop (void)
{
    g_autoptr(GrlImage) current = make_solid_rgba (8, 8, 100, 150, 200, 255);

    grl_image_onion_skin (current, NULL, NULL, 0.5f, 0.5f, NULL, NULL);

    /* Pixel must be exactly the original colour. */
    assert_pixel (current, 4, 4, 100, 150, 200, 255, 0);
}

/*
 * Both ghosts non-NULL but opacity 0.0 → current is unchanged.
 */
static void
test_onion_skin_zero_opacity_noop (void)
{
    g_autoptr(GrlImage) current = make_solid_rgba (8, 8, 100, 150, 200, 255);
    g_autoptr(GrlImage) prev    = make_solid_rgba (8, 8, 255,   0,   0, 255);
    g_autoptr(GrlImage) next    = make_solid_rgba (8, 8,   0,   0, 255, 255);

    grl_image_onion_skin (current, prev, next, 0.0f, 0.0f, NULL, NULL);

    assert_pixel (current, 4, 4, 100, 150, 200, 255, 0);
}

/*
 * Full-opacity prev ghost (opacity 1.0) completely overwrites current.
 *
 * With OVER blending and an alpha-255 source over an opaque destination the
 * result is the source colour (to within the OVER compositing rounding of 1
 * LSB per channel, because intermediate values are kept at 8-bit precision).
 */
static void
test_onion_skin_full_opacity_overwrites (void)
{
    g_autoptr(GrlImage) current = make_solid_rgba (8, 8, 100, 150, 200, 255);
    g_autoptr(GrlImage) prev    = make_solid_rgba (8, 8, 200,  50,  50, 255);

    grl_image_onion_skin (current, prev, NULL, 1.0f, 0.0f, NULL, NULL);

    /*
     * OVER with alpha=255 source onto any opaque destination produces the
     * source colour (the destination is fully replaced).  Allow tol=1 for
     * OVER rounding at 8-bit precision.
     */
    assert_pixel (current, 4, 4, 200, 50, 50, 255, 1);
}

/*
 * Zero-opacity prev ghost leaves current unchanged (opacity parameter path,
 * as distinct from test_onion_skin_no_ghosts_noop which tests a NULL ghost).
 */
static void
test_onion_skin_zero_prev_opacity_unchanged (void)
{
    g_autoptr(GrlImage) current = make_solid_rgba (8, 8, 100, 150, 200, 255);
    g_autoptr(GrlImage) prev    = make_solid_rgba (8, 8, 255,   0,   0, 255);

    grl_image_onion_skin (current, prev, NULL, 0.0f, 0.0f, NULL, NULL);

    assert_pixel (current, 4, 4, 100, 150, 200, 255, 0);
}

/*
 * A red tint on the prev ghost shifts the composited pixel towards red.
 *
 * We use a half-opacity white prev and a full-red tint.  Over an opaque
 * white destination we expect the red channel to be dominant.
 *
 * prev = white (255,255,255,255), tint = red (255,0,0) with alpha 128
 * Tinted source = (255*255/255, 255*0/255, 255*0/255, 255*128/255)
 *               = (255, 0, 0, 128)
 * OVER (255,0,0,128) onto (255,255,255,255):
 *   R_out = 255 * (128/255) + 255 * (1 - 128/255) = 255
 *   G_out =   0 * (128/255) + 255 * (1 - 128/255) ≈ 127
 *   B_out =   0 * (128/255) + 255 * (1 - 128/255) ≈ 127
 *
 * Assertion: R > G and R > B, with sensible absolute values.
 */
static void
test_onion_skin_tint_shifts_channel (void)
{
    g_autoptr(GrlImage) current   = make_solid_rgba (8, 8, 255, 255, 255, 255);
    g_autoptr(GrlImage) prev      = make_solid_rgba (8, 8, 255, 255, 255, 255);
    g_autoptr(GrlColor) red_tint  = grl_color_new (255, 0, 0, 255);
    g_autoptr(GrlColor) px        = NULL;

    grl_image_onion_skin (current, prev, NULL, 0.5f, 0.0f, red_tint, NULL);

    px = grl_image_get_pixel (current, 4, 4);
    g_assert_nonnull (px);

    /* Red channel must be higher than green and blue. */
    g_assert_cmpint ((gint)px->r, >, (gint)px->g);
    g_assert_cmpint ((gint)px->r, >, (gint)px->b);

    /* Red must be close to 255 (opaque white destination, red-tinted source). */
    g_assert_cmpint (ABS ((gint)px->r - 255), <=, 2);
}

/*
 * Passing a non-RGBA (R8G8B8) image must not crash.
 *
 * The function should detect the format mismatch and return without modifying
 * @current, per the silent-degrade contract documented in the header.
 * We assert the pixel is unchanged (same as the original solid colour), and
 * most importantly that we did not segfault.
 */
static void
test_onion_skin_non_rgba_no_crash (void)
{
    g_autoptr(GrlImage) current = make_solid_rgb (8, 8, 128, 64, 32);
    g_autoptr(GrlImage) prev    = make_solid_rgba (8, 8, 255, 0, 0, 255);

    /* This must not crash. */
    grl_image_onion_skin (current, prev, NULL, 0.5f, 0.0f, NULL, NULL);

    /*
     * Because grl_image_get_pixel() works on any format and converts to RGBA,
     * we just check the function didn't crash and the pixel is still close to
     * the original R8G8B8 colour.  Tol=2 for the RGB→RGBA round-trip.
     */
    assert_pixel (current, 4, 4, 128, 64, 32, 255, 2);
}

/* ---------------------------------------------------------------------------
 * temporal_aa tests
 * --------------------------------------------------------------------------- */

/*
 * NULL accumulator → returns NULL (guard).
 */
static void
test_temporal_aa_null_acc_returns_null (void)
{
    g_autoptr(GrlImage) sample = make_solid_rgba (8, 8, 128, 128, 128, 255);
    GrlImage *samples[1];
    GrlImage *result;

    samples[0] = sample;
    g_test_expect_message (NULL, G_LOG_LEVEL_CRITICAL, "*acc != NULL*");
    result = grl_image_temporal_aa (NULL, samples, 1);
    g_test_assert_expected_messages ();
    g_assert_null (result);
}

/*
 * n_samples < 1 → returns NULL (guard).
 */
static void
test_temporal_aa_zero_samples_returns_null (void)
{
    g_autoptr(GrlImageAccumulator) acc = grl_image_accumulator_new (8, 8, FALSE);
    GrlImage *result;

    g_test_expect_message (NULL, G_LOG_LEVEL_CRITICAL, "*n_samples*");
    result = grl_image_temporal_aa (acc, NULL, 0);
    g_test_assert_expected_messages ();
    g_assert_null (result);
}

/*
 * Single sample: resolved image must equal the sample within tol 1.
 */
static void
test_temporal_aa_single_sample (void)
{
    g_autoptr(GrlImageAccumulator) acc    = grl_image_accumulator_new (8, 8, FALSE);
    g_autoptr(GrlImage)            sample = make_solid_rgba (8, 8, 80, 160, 40, 255);
    GrlImage *samples[1];
    GrlImage *result;

    samples[0] = sample;
    result = grl_image_temporal_aa (acc, samples, 1);
    g_assert_nonnull (result);

    assert_pixel (result, 4, 4, 80, 160, 40, 255, 1);
    g_object_unref (result);
}

/*
 * Average of black and white in gamma mode (linear=FALSE).
 *
 * (0 + 255) / 2 = 127 or 128 depending on rounding; we allow tol 1.
 */
static void
test_temporal_aa_average_black_white_gamma (void)
{
    g_autoptr(GrlImageAccumulator) acc   = grl_image_accumulator_new (8, 8, FALSE);
    g_autoptr(GrlImage)            black = make_solid_rgba (8, 8, 0,   0,   0,   255);
    g_autoptr(GrlImage)            white = make_solid_rgba (8, 8, 255, 255, 255, 255);
    GrlImage *samples[2];
    GrlImage *result;

    samples[0] = black;
    samples[1] = white;
    result = grl_image_temporal_aa (acc, samples, 2);
    g_assert_nonnull (result);

    /* Gamma average: (0 + 255) / 2 == 128 (±1 for rounding). */
    assert_pixel (result, 4, 4, 128, 128, 128, 255, 1);
    g_object_unref (result);
}

/*
 * The accumulator is properly reset between calls: a second call with a
 * different single sample should return the new sample, not a blend.
 */
static void
test_temporal_aa_reuses_accumulator (void)
{
    g_autoptr(GrlImageAccumulator) acc    = grl_image_accumulator_new (8, 8, FALSE);
    g_autoptr(GrlImage)            first  = make_solid_rgba (8, 8, 200, 200, 200, 255);
    g_autoptr(GrlImage)            second = make_solid_rgba (8, 8,  50,  50,  50, 255);
    GrlImage *samples[1];
    GrlImage *result;

    samples[0] = first;
    result = grl_image_temporal_aa (acc, samples, 1);
    g_assert_nonnull (result);
    g_object_unref (result);

    /* Second call: different sample; accumulator must have been reset. */
    samples[0] = second;
    result = grl_image_temporal_aa (acc, samples, 1);
    g_assert_nonnull (result);

    assert_pixel (result, 4, 4, 50, 50, 50, 255, 1);
    g_object_unref (result);
}

/* ---------------------------------------------------------------------------
 * main
 * --------------------------------------------------------------------------- */

int
main (int   argc,
      char *argv[])
{
    g_test_init (&argc, &argv, NULL);

    /* onion_skin */
    g_test_add_func ("/image-temporal/onion-skin/no-ghosts-noop",
                     test_onion_skin_no_ghosts_noop);
    g_test_add_func ("/image-temporal/onion-skin/zero-opacity-noop",
                     test_onion_skin_zero_opacity_noop);
    g_test_add_func ("/image-temporal/onion-skin/full-opacity-overwrites",
                     test_onion_skin_full_opacity_overwrites);
    g_test_add_func ("/image-temporal/onion-skin/zero-prev-opacity-unchanged",
                     test_onion_skin_zero_prev_opacity_unchanged);
    g_test_add_func ("/image-temporal/onion-skin/tint-shifts-channel",
                     test_onion_skin_tint_shifts_channel);
    g_test_add_func ("/image-temporal/onion-skin/non-rgba-no-crash",
                     test_onion_skin_non_rgba_no_crash);

    /* temporal_aa */
    g_test_add_func ("/image-temporal/temporal-aa/null-acc-returns-null",
                     test_temporal_aa_null_acc_returns_null);
    g_test_add_func ("/image-temporal/temporal-aa/zero-samples-returns-null",
                     test_temporal_aa_zero_samples_returns_null);
    g_test_add_func ("/image-temporal/temporal-aa/single-sample",
                     test_temporal_aa_single_sample);
    g_test_add_func ("/image-temporal/temporal-aa/average-black-white-gamma",
                     test_temporal_aa_average_black_white_gamma);
    g_test_add_func ("/image-temporal/temporal-aa/reuses-accumulator",
                     test_temporal_aa_reuses_accumulator);

    return g_test_run ();
}
