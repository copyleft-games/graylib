/* test-image-noise-overlay.c
 *
 * Copyright 2026 Ben Doty
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * TDD suite for grl_image_apply_noise. Red-first: each test must
 * fail with missing-symbol before implementation exists.
 */

#include <glib.h>
#include <math.h>
#include <stdlib.h>
#include "src/graphics/grl-image.h"
#include "src/math/grl-color.h"

#define W 64
#define H 64

static GrlImage *
make_gray_canvas (void)
{
    g_autoptr (GrlColor) bg = grl_color_new (128, 128, 128, 255);
    return grl_image_new_color (W, H, bg);
}

/* Helper: count pixels that differ between two images by more than `tol`. */
static gint
diff_pixel_count (GrlImage *a, GrlImage *b, gint tol)
{
    gint count = 0;
    gint x, y;
    for (y = 0; y < H; y++) {
        for (x = 0; x < W; x++) {
            g_autoptr (GrlColor) ca = grl_image_get_pixel (a, x, y);
            g_autoptr (GrlColor) cb = grl_image_get_pixel (b, x, y);
            gint dr = (gint) ca->r - (gint) cb->r;
            gint dg = (gint) ca->g - (gint) cb->g;
            gint db = (gint) ca->b - (gint) cb->b;
            if (abs (dr) > tol || abs (dg) > tol || abs (db) > tol)
                count++;
        }
    }
    return count;
}

/* amplitude=0 → unchanged image */
static void
test_noise_zero_amplitude_unchanged (void)
{
    g_autoptr (GrlImage) a = make_gray_canvas ();
    g_autoptr (GrlImage) b = make_gray_canvas ();
    grl_image_apply_noise (b, GRL_NOISE_BLEND_ADDITIVE, 0.0f, 0.1f, 1234);
    g_assert_cmpint (diff_pixel_count (a, b, 0), ==, 0);
}

/* amplitude>0 → most pixels change */
static void
test_noise_nonzero_amplitude_changes (void)
{
    g_autoptr (GrlImage) a = make_gray_canvas ();
    g_autoptr (GrlImage) b = make_gray_canvas ();
    grl_image_apply_noise (b, GRL_NOISE_BLEND_ADDITIVE, 30.0f, 0.5f, 1234);
    /* expect majority of pixels changed */
    g_assert_cmpint (diff_pixel_count (a, b, 0), >, W * H / 2);
}

/* same seed → identical output */
static void
test_noise_seed_deterministic (void)
{
    g_autoptr (GrlImage) a = make_gray_canvas ();
    g_autoptr (GrlImage) b = make_gray_canvas ();
    grl_image_apply_noise (a, GRL_NOISE_BLEND_ADDITIVE, 30.0f, 0.5f, 7);
    grl_image_apply_noise (b, GRL_NOISE_BLEND_ADDITIVE, 30.0f, 0.5f, 7);
    g_assert_cmpint (diff_pixel_count (a, b, 0), ==, 0);
}

/* different seed → different output */
static void
test_noise_seed_differs (void)
{
    g_autoptr (GrlImage) a = make_gray_canvas ();
    g_autoptr (GrlImage) b = make_gray_canvas ();
    grl_image_apply_noise (a, GRL_NOISE_BLEND_ADDITIVE, 30.0f, 0.5f, 1);
    grl_image_apply_noise (b, GRL_NOISE_BLEND_ADDITIVE, 30.0f, 0.5f, 2);
    g_assert_cmpint (diff_pixel_count (a, b, 0), >, W * H / 4);
}

/* MULTIPLY mode never brightens above the base on midtone-gray */
static void
test_noise_multiply_darkens (void)
{
    g_autoptr (GrlImage) a = make_gray_canvas ();
    gint x, y;
    gint brighter = 0;
    grl_image_apply_noise (a, GRL_NOISE_BLEND_MULTIPLY, 1.0f, 0.5f, 99);
    for (y = 0; y < H; y++) {
        for (x = 0; x < W; x++) {
            g_autoptr (GrlColor) c = grl_image_get_pixel (a, x, y);
            if (c->r > 128) brighter++;
        }
    }
    /* multiply with noise in [0,1] should never brighten pure gray */
    g_assert_cmpint (brighter, ==, 0);
}

int
main (int argc, char **argv)
{
    g_test_init (&argc, &argv, NULL);
    g_test_add_func ("/noise/zero-amplitude-unchanged",  test_noise_zero_amplitude_unchanged);
    g_test_add_func ("/noise/nonzero-amplitude-changes", test_noise_nonzero_amplitude_changes);
    g_test_add_func ("/noise/seed-deterministic",        test_noise_seed_deterministic);
    g_test_add_func ("/noise/seed-differs",              test_noise_seed_differs);
    g_test_add_func ("/noise/multiply-darkens",          test_noise_multiply_darkens);
    return g_test_run ();
}
