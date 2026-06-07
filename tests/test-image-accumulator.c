/* test-image-accumulator.c
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Unit tests for GrlImageAccumulator.  All tests run headless (no window /
 * GL context required).
 */

#include <glib.h>
#include <math.h>
#include "src/graphics/grl-image-accumulator.h"
#include "src/graphics/grl-image.h"
#include "src/math/grl-color.h"

/* ---------------------------------------------------------------------------
 * Helpers
 * --------------------------------------------------------------------------- */

/* Assert a pixel value (r,g,b,a) at (x,y) within tolerance @tol. */
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

/* Create a solid-colour GrlImage of the given dimensions. */
static GrlImage *
make_solid (gint width, gint height, guint8 r, guint8 g, guint8 b, guint8 a)
{
    g_autoptr(GrlColor) col = grl_color_new (r, g, b, a);
    return grl_image_new_color (width, height, col);
}

/* ---------------------------------------------------------------------------
 * Tests
 * --------------------------------------------------------------------------- */

/* Sanity: new accumulator is non-null with the right type. */
static void
test_new_sanity (void)
{
    g_autoptr(GrlImageAccumulator) acc = grl_image_accumulator_new (32, 32, FALSE);

    g_assert_nonnull (acc);
    g_assert_true (GRL_IS_IMAGE_ACCUMULATOR (acc));
}

/* resolve() before any add() returns NULL (no divide-by-zero). */
static void
test_resolve_empty_returns_null (void)
{
    g_autoptr(GrlImageAccumulator) acc = grl_image_accumulator_new (16, 16, FALSE);
    GrlImage *result;

    result = grl_image_accumulator_resolve (acc);
    g_assert_null (result);
}

/*
 * Single sample, gamma mode: add a solid-colour frame, resolve.
 * The resolved image must exactly equal the source colour (within tol 1 to
 * absorb the rounding on the guint8 conversion).
 */
static void
test_single_sample_gamma (void)
{
    g_autoptr(GrlImageAccumulator) acc = grl_image_accumulator_new (8, 8, FALSE);
    g_autoptr(GrlImage) frame = make_solid (8, 8, 100, 150, 200, 255);
    GrlImage *result;

    grl_image_accumulator_add (acc, frame, 1.0f);
    result = grl_image_accumulator_resolve (acc);
    g_assert_nonnull (result);

    assert_pixel (result, 4, 4, 100, 150, 200, 255, 1);
    g_object_unref (result);
}

/*
 * Single sample, linear mode: add a solid frame with weight 1, resolve.
 * Linear round-trip (sRGB -> linear -> sRGB) must reproduce the source
 * within tol 1.
 */
static void
test_single_sample_linear (void)
{
    g_autoptr(GrlImageAccumulator) acc = grl_image_accumulator_new (8, 8, TRUE);
    g_autoptr(GrlImage) frame = make_solid (8, 8, 100, 150, 200, 255);
    GrlImage *result;

    grl_image_accumulator_add (acc, frame, 1.0f);
    result = grl_image_accumulator_resolve (acc);
    g_assert_nonnull (result);

    /* Round-trip through sRGB<->linear must land within 1 LSB. */
    assert_pixel (result, 4, 4, 100, 150, 200, 255, 1);
    g_object_unref (result);
}

/*
 * Overflow safety: add 1024 identical solid-white frames with weight 1 each.
 * The float buffer must not wrap; the resolved output must still be white.
 */
static void
test_overflow_white (void)
{
    g_autoptr(GrlImageAccumulator) acc = grl_image_accumulator_new (4, 4, FALSE);
    g_autoptr(GrlImage) frame = make_solid (4, 4, 255, 255, 255, 255);
    GrlImage *result;
    gint      i;

    for (i = 0; i < 1024; i++)
        grl_image_accumulator_add (acc, frame, 1.0f);

    result = grl_image_accumulator_resolve (acc);
    g_assert_nonnull (result);

    /* Average of 1024 identical white frames is still white. */
    assert_pixel (result, 2, 2, 255, 255, 255, 255, 1);
    g_object_unref (result);
}

/*
 * Average of black and white, gamma mode.
 * Two frames, weight 1 each.  Expected mid-grey in gamma space: ~128.
 */
static void
test_average_black_white_gamma (void)
{
    g_autoptr(GrlImageAccumulator) acc = grl_image_accumulator_new (4, 4, FALSE);
    g_autoptr(GrlImage) black = make_solid (4, 4, 0, 0, 0, 255);
    g_autoptr(GrlImage) white = make_solid (4, 4, 255, 255, 255, 255);
    GrlImage *result;

    grl_image_accumulator_add (acc, black, 1.0f);
    grl_image_accumulator_add (acc, white, 1.0f);
    result = grl_image_accumulator_resolve (acc);
    g_assert_nonnull (result);

    /* Gamma average: (0 + 255) / 2 = 127 or 128 depending on rounding. */
    assert_pixel (result, 2, 2, 128, 128, 128, 255, 1);
    g_object_unref (result);
}

/*
 * Average of black and white, linear mode.
 * The perceptual midpoint of 0 and 1 in linear light is 0.5 linear, which
 * converts back to sRGB as approx. 188 (standard value for 50% luminance).
 */
static void
test_average_black_white_linear (void)
{
    g_autoptr(GrlImageAccumulator) acc = grl_image_accumulator_new (4, 4, TRUE);
    g_autoptr(GrlImage) black = make_solid (4, 4, 0, 0, 0, 255);
    g_autoptr(GrlImage) white = make_solid (4, 4, 255, 255, 255, 255);
    GrlImage *result;

    grl_image_accumulator_add (acc, black, 1.0f);
    grl_image_accumulator_add (acc, white, 1.0f);
    result = grl_image_accumulator_resolve (acc);
    g_assert_nonnull (result);

    /*
     * linear(0)=0, linear(255/255=1.0)=1.0  => avg=0.5 linear
     * linear_to_srgb(0.5) = 1.055*pow(0.5,1/2.4)-0.055 ~ 0.7354 => ~188
     */
    assert_pixel (result, 2, 2, 188, 188, 188, 255, 2);
    g_object_unref (result);
}

/*
 * Mismatched sub-frame size: add a frame of different dimensions.
 * resolve() must not crash and must return a canvas-sized image.
 */
static void
test_mismatched_size_no_crash (void)
{
    g_autoptr(GrlImageAccumulator) acc = grl_image_accumulator_new (16, 16, FALSE);
    g_autoptr(GrlImage) small = make_solid (5, 7, 200, 100, 50, 255);
    GrlImage *result;

    grl_image_accumulator_add (acc, small, 1.0f);
    result = grl_image_accumulator_resolve (acc);
    g_assert_nonnull (result);
    g_assert_cmpint (grl_image_get_width  (result), ==, 16);
    g_assert_cmpint (grl_image_get_height (result), ==, 16);
    g_object_unref (result);
}

/*
 * Fully transparent samples: the resolved alpha should be ~0.
 */
static void
test_fully_transparent_alpha (void)
{
    g_autoptr(GrlImageAccumulator) acc = grl_image_accumulator_new (4, 4, FALSE);
    g_autoptr(GrlImage) frame = make_solid (4, 4, 255, 255, 255, 0);
    GrlImage *result;

    grl_image_accumulator_add (acc, frame, 1.0f);
    result = grl_image_accumulator_resolve (acc);
    g_assert_nonnull (result);

    /* Alpha must be ~0. */
    assert_pixel (result, 2, 2, 255, 255, 255, 0, 1);
    g_object_unref (result);
}

/*
 * Reset: after reset(), resolve() returns NULL again.
 */
static void
test_reset_clears_state (void)
{
    g_autoptr(GrlImageAccumulator) acc = grl_image_accumulator_new (8, 8, FALSE);
    g_autoptr(GrlImage) frame = make_solid (8, 8, 128, 128, 128, 255);
    GrlImage *result;

    /* Add a frame so there is something accumulated. */
    grl_image_accumulator_add (acc, frame, 1.0f);
    result = grl_image_accumulator_resolve (acc);
    g_assert_nonnull (result);
    g_object_unref (result);

    /* After reset, resolve must return NULL. */
    grl_image_accumulator_reset (acc);
    result = grl_image_accumulator_resolve (acc);
    g_assert_null (result);
}

/* ---------------------------------------------------------------------------
 * main
 * --------------------------------------------------------------------------- */

int
main (int   argc,
      char *argv[])
{
    g_test_init (&argc, &argv, NULL);

    g_test_add_func ("/image-accumulator/new-sanity",
                     test_new_sanity);
    g_test_add_func ("/image-accumulator/resolve-empty-returns-null",
                     test_resolve_empty_returns_null);
    g_test_add_func ("/image-accumulator/single-sample-gamma",
                     test_single_sample_gamma);
    g_test_add_func ("/image-accumulator/single-sample-linear",
                     test_single_sample_linear);
    g_test_add_func ("/image-accumulator/overflow-white",
                     test_overflow_white);
    g_test_add_func ("/image-accumulator/average-black-white-gamma",
                     test_average_black_white_gamma);
    g_test_add_func ("/image-accumulator/average-black-white-linear",
                     test_average_black_white_linear);
    g_test_add_func ("/image-accumulator/mismatched-size-no-crash",
                     test_mismatched_size_no_crash);
    g_test_add_func ("/image-accumulator/fully-transparent-alpha",
                     test_fully_transparent_alpha);
    g_test_add_func ("/image-accumulator/reset-clears-state",
                     test_reset_clears_state);

    return g_test_run ();
}
