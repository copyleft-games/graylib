/* test-color-oklab.c
 *
 * Copyright 2026 Ben Doty
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * TDD suite for grl_color_lerp_oklab — perceptual-space color interpolation.
 * Red phase: each test must fail before the symbol exists.
 */

#include <glib.h>
#include <math.h>
#include <stdlib.h>
#include "src/math/grl-color.h"

/* Lerp at 0 returns a; lerp at 1 returns b. Boundary identity. */
static void
test_oklab_lerp_boundaries (void)
{
    g_autoptr (GrlColor) a = grl_color_new (10, 50, 200, 255);
    g_autoptr (GrlColor) b = grl_color_new (240, 200, 30, 255);
    g_autoptr (GrlColor) r0 = grl_color_lerp_oklab (a, b, 0.0f);
    g_autoptr (GrlColor) r1 = grl_color_lerp_oklab (a, b, 1.0f);

    g_assert_cmpint (abs ((int) r0->r - (int) a->r), <=, 1);
    g_assert_cmpint (abs ((int) r0->g - (int) a->g), <=, 1);
    g_assert_cmpint (abs ((int) r0->b - (int) a->b), <=, 1);
    g_assert_cmpint (abs ((int) r1->r - (int) b->r), <=, 1);
    g_assert_cmpint (abs ((int) r1->g - (int) b->g), <=, 1);
    g_assert_cmpint (abs ((int) r1->b - (int) b->b), <=, 1);
}

/* Blue→Yellow midpoint differs significantly between RGB and OKLab.
 * RGB lerp gives muddy gray (~128,128,128). OKLab gives a perceptually
 * brighter, chromatic midpoint (greenish). The two midpoints must differ
 * in at least one channel by >= 20. */
static void
test_oklab_blue_yellow_midpoint_differs (void)
{
    g_autoptr (GrlColor) blue   = grl_color_new (0, 0, 255, 255);
    g_autoptr (GrlColor) yellow = grl_color_new (255, 255, 0, 255);
    g_autoptr (GrlColor) rgb_mid    = grl_color_lerp       (blue, yellow, 0.5f);
    g_autoptr (GrlColor) oklab_mid  = grl_color_lerp_oklab (blue, yellow, 0.5f);

    int dr = abs ((int) rgb_mid->r - (int) oklab_mid->r);
    int dg = abs ((int) rgb_mid->g - (int) oklab_mid->g);
    int db = abs ((int) rgb_mid->b - (int) oklab_mid->b);
    int total = dr + dg + db;
    g_assert_cmpint (total, >=, 20);
}

/* Alpha is interpolated linearly in OKLab too (not channel-warped). */
static void
test_oklab_alpha_linear (void)
{
    g_autoptr (GrlColor) a = grl_color_new (255, 0, 0, 0);
    g_autoptr (GrlColor) b = grl_color_new (0, 255, 0, 200);
    g_autoptr (GrlColor) mid = grl_color_lerp_oklab (a, b, 0.5f);

    g_assert_cmpint (abs ((int) mid->a - 100), <=, 1);
}

/* Same color at both ends → identical color at t=0.5. */
static void
test_oklab_lerp_identity (void)
{
    g_autoptr (GrlColor) c = grl_color_new (87, 142, 33, 255);
    g_autoptr (GrlColor) r = grl_color_lerp_oklab (c, c, 0.5f);

    g_assert_cmpint (abs ((int) r->r - (int) c->r), <=, 1);
    g_assert_cmpint (abs ((int) r->g - (int) c->g), <=, 1);
    g_assert_cmpint (abs ((int) r->b - (int) c->b), <=, 1);
}

/* Determinism: same inputs → same output, repeatable. */
static void
test_oklab_lerp_deterministic (void)
{
    g_autoptr (GrlColor) a = grl_color_new (12, 34, 56, 255);
    g_autoptr (GrlColor) b = grl_color_new (200, 180, 90, 255);
    g_autoptr (GrlColor) r1 = grl_color_lerp_oklab (a, b, 0.37f);
    g_autoptr (GrlColor) r2 = grl_color_lerp_oklab (a, b, 0.37f);

    g_assert_cmpint (r1->r, ==, r2->r);
    g_assert_cmpint (r1->g, ==, r2->g);
    g_assert_cmpint (r1->b, ==, r2->b);
    g_assert_cmpint (r1->a, ==, r2->a);
}

int
main (int argc, char **argv)
{
    g_test_init (&argc, &argv, NULL);
    g_test_add_func ("/color-oklab/boundaries",        test_oklab_lerp_boundaries);
    g_test_add_func ("/color-oklab/blue-yellow-midpoint", test_oklab_blue_yellow_midpoint_differs);
    g_test_add_func ("/color-oklab/alpha-linear",      test_oklab_alpha_linear);
    g_test_add_func ("/color-oklab/identity",          test_oklab_lerp_identity);
    g_test_add_func ("/color-oklab/deterministic",     test_oklab_lerp_deterministic);
    return g_test_run ();
}
