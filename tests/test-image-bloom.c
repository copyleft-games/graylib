/* test-image-bloom.c
 *
 * Copyright 2026 Ben Doty
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * TDD suite for grl_image_apply_bloom.
 */

#include <glib.h>
#include <math.h>
#include "src/graphics/grl-image.h"
#include "src/math/grl-color.h"

#define W 64
#define H 64

static GrlImage *
make_solid (guint8 r, guint8 g, guint8 b)
{
    g_autoptr (GrlColor) c = grl_color_new (r, g, b, 255);
    return grl_image_new_color (W, H, c);
}

/* Helper: average brightness across all pixels. */
static gdouble
avg_brightness (GrlImage *img)
{
    gdouble total = 0.0;
    gint x, y;
    for (y = 0; y < H; y++) {
        for (x = 0; x < W; x++) {
            g_autoptr (GrlColor) c = grl_image_get_pixel (img, x, y);
            total += (c->r + c->g + c->b) / 3.0;
        }
    }
    return total / (W * H);
}

/* Dark image (all below threshold) → bloom does nothing. */
static void
test_bloom_dark_unchanged (void)
{
    g_autoptr (GrlImage) before = make_solid (30, 30, 30);
    g_autoptr (GrlImage) after  = make_solid (30, 30, 30);
    grl_image_apply_bloom (after, 200, 4, 1.0f);
    gdouble db = avg_brightness (before);
    gdouble da = avg_brightness (after);
    /* expect no meaningful change — tolerance 2 */
    g_assert_cmpfloat (fabs (da - db), <, 2.0);
}

/* Bright image (all above threshold) → bloom brightens it. */
static void
test_bloom_bright_brightens (void)
{
    g_autoptr (GrlImage) before = make_solid (240, 240, 240);
    g_autoptr (GrlImage) after  = make_solid (240, 240, 240);
    grl_image_apply_bloom (after, 100, 4, 2.0f);
    gdouble db = avg_brightness (before);
    gdouble da = avg_brightness (after);
    /* bloom with intensity 2 should push brightness higher */
    g_assert_cmpfloat (da, >=, db);
}

/* intensity=0 → no change regardless of threshold */
static void
test_bloom_zero_intensity_unchanged (void)
{
    g_autoptr (GrlImage) before = make_solid (200, 200, 200);
    g_autoptr (GrlImage) after  = make_solid (200, 200, 200);
    grl_image_apply_bloom (after, 100, 4, 0.0f);
    gdouble db = avg_brightness (before);
    gdouble da = avg_brightness (after);
    g_assert_cmpfloat (fabs (da - db), <, 2.0);
}

/* threshold=255 → nothing passes threshold → no change */
static void
test_bloom_max_threshold_unchanged (void)
{
    g_autoptr (GrlImage) before = make_solid (200, 200, 200);
    g_autoptr (GrlImage) after  = make_solid (200, 200, 200);
    grl_image_apply_bloom (after, 255, 4, 1.0f);
    gdouble db = avg_brightness (before);
    gdouble da = avg_brightness (after);
    g_assert_cmpfloat (fabs (da - db), <, 2.0);
}

/* Result is always clamped — no channel exceeds 255. */
static void
test_bloom_clamps_255 (void)
{
    g_autoptr (GrlImage) img = make_solid (255, 255, 255);
    grl_image_apply_bloom (img, 0, 4, 10.0f);
    gint x, y;
    for (y = 0; y < H; y++) {
        for (x = 0; x < W; x++) {
            g_autoptr (GrlColor) c = grl_image_get_pixel (img, x, y);
            g_assert_cmpuint (c->r, <=, 255);
            g_assert_cmpuint (c->g, <=, 255);
            g_assert_cmpuint (c->b, <=, 255);
        }
    }
}

int
main (int argc, char **argv)
{
    g_test_init (&argc, &argv, NULL);
    g_test_add_func ("/bloom/dark-unchanged",        test_bloom_dark_unchanged);
    g_test_add_func ("/bloom/bright-brightens",      test_bloom_bright_brightens);
    g_test_add_func ("/bloom/zero-intensity",        test_bloom_zero_intensity_unchanged);
    g_test_add_func ("/bloom/max-threshold",         test_bloom_max_threshold_unchanged);
    g_test_add_func ("/bloom/clamps-255",            test_bloom_clamps_255);
    return g_test_run ();
}
