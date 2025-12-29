/* test-math-utils.c
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Unit tests for math utility functions.
 */

#include <glib.h>
#include <math.h>
#include "src/math/grl-math-utils.h"

#define EPSILON 0.0001f

static gboolean
float_equal (gfloat a, gfloat b)
{
    return fabsf (a - b) < EPSILON;
}

/*
 * Test grl_math_clamp
 */

static void
test_math_clamp_within_range (void)
{
    gfloat result = grl_math_clamp (5.0f, 0.0f, 10.0f);
    g_assert_true (float_equal (result, 5.0f));
}

static void
test_math_clamp_below_min (void)
{
    gfloat result = grl_math_clamp (-5.0f, 0.0f, 10.0f);
    g_assert_true (float_equal (result, 0.0f));
}

static void
test_math_clamp_above_max (void)
{
    gfloat result = grl_math_clamp (15.0f, 0.0f, 10.0f);
    g_assert_true (float_equal (result, 10.0f));
}

static void
test_math_clamp_at_min (void)
{
    gfloat result = grl_math_clamp (0.0f, 0.0f, 10.0f);
    g_assert_true (float_equal (result, 0.0f));
}

static void
test_math_clamp_at_max (void)
{
    gfloat result = grl_math_clamp (10.0f, 0.0f, 10.0f);
    g_assert_true (float_equal (result, 10.0f));
}

/*
 * Test grl_math_clamp_int
 */

static void
test_math_clamp_int_within_range (void)
{
    gint result = grl_math_clamp_int (5, 0, 10);
    g_assert_cmpint (result, ==, 5);
}

static void
test_math_clamp_int_below_min (void)
{
    gint result = grl_math_clamp_int (-5, 0, 10);
    g_assert_cmpint (result, ==, 0);
}

static void
test_math_clamp_int_above_max (void)
{
    gint result = grl_math_clamp_int (15, 0, 10);
    g_assert_cmpint (result, ==, 10);
}

/*
 * Test grl_math_lerp
 */

static void
test_math_lerp_zero (void)
{
    gfloat result = grl_math_lerp (0.0f, 10.0f, 0.0f);
    g_assert_true (float_equal (result, 0.0f));
}

static void
test_math_lerp_one (void)
{
    gfloat result = grl_math_lerp (0.0f, 10.0f, 1.0f);
    g_assert_true (float_equal (result, 10.0f));
}

static void
test_math_lerp_half (void)
{
    gfloat result = grl_math_lerp (0.0f, 10.0f, 0.5f);
    g_assert_true (float_equal (result, 5.0f));
}

static void
test_math_lerp_quarter (void)
{
    gfloat result = grl_math_lerp (0.0f, 100.0f, 0.25f);
    g_assert_true (float_equal (result, 25.0f));
}

static void
test_math_lerp_negative_range (void)
{
    gfloat result = grl_math_lerp (-10.0f, 10.0f, 0.5f);
    g_assert_true (float_equal (result, 0.0f));
}

/*
 * Test grl_math_normalize
 */

static void
test_math_normalize_start (void)
{
    gfloat result = grl_math_normalize (0.0f, 0.0f, 10.0f);
    g_assert_true (float_equal (result, 0.0f));
}

static void
test_math_normalize_end (void)
{
    gfloat result = grl_math_normalize (10.0f, 0.0f, 10.0f);
    g_assert_true (float_equal (result, 1.0f));
}

static void
test_math_normalize_middle (void)
{
    gfloat result = grl_math_normalize (5.0f, 0.0f, 10.0f);
    g_assert_true (float_equal (result, 0.5f));
}

static void
test_math_normalize_negative_range (void)
{
    gfloat result = grl_math_normalize (0.0f, -10.0f, 10.0f);
    g_assert_true (float_equal (result, 0.5f));
}

/*
 * Test grl_math_remap
 */

static void
test_math_remap_identity (void)
{
    gfloat result = grl_math_remap (5.0f, 0.0f, 10.0f, 0.0f, 10.0f);
    g_assert_true (float_equal (result, 5.0f));
}

static void
test_math_remap_double (void)
{
    gfloat result = grl_math_remap (5.0f, 0.0f, 10.0f, 0.0f, 20.0f);
    g_assert_true (float_equal (result, 10.0f));
}

static void
test_math_remap_invert (void)
{
    gfloat result = grl_math_remap (2.0f, 0.0f, 10.0f, 10.0f, 0.0f);
    g_assert_true (float_equal (result, 8.0f));
}

static void
test_math_remap_offset (void)
{
    gfloat result = grl_math_remap (5.0f, 0.0f, 10.0f, 100.0f, 200.0f);
    g_assert_true (float_equal (result, 150.0f));
}

/*
 * Test grl_math_wrap
 */

static void
test_math_wrap_within_range (void)
{
    gfloat result = grl_math_wrap (5.0f, 0.0f, 10.0f);
    g_assert_true (float_equal (result, 5.0f));
}

static void
test_math_wrap_above_max (void)
{
    gfloat result = grl_math_wrap (15.0f, 0.0f, 10.0f);
    g_assert_true (float_equal (result, 5.0f));
}

static void
test_math_wrap_below_min (void)
{
    gfloat result = grl_math_wrap (-5.0f, 0.0f, 10.0f);
    g_assert_true (float_equal (result, 5.0f));
}

static void
test_math_wrap_at_max (void)
{
    gfloat result = grl_math_wrap (10.0f, 0.0f, 10.0f);
    g_assert_true (float_equal (result, 0.0f));
}

static void
test_math_wrap_multiple_cycles (void)
{
    gfloat result = grl_math_wrap (25.0f, 0.0f, 10.0f);
    g_assert_true (float_equal (result, 5.0f));
}

/*
 * Test grl_math_float_equals
 */

static void
test_math_float_equals_same (void)
{
    g_assert_true (grl_math_float_equals (1.0f, 1.0f));
}

static void
test_math_float_equals_close (void)
{
    g_assert_true (grl_math_float_equals (1.0f, 1.0000001f));
}

static void
test_math_float_equals_different (void)
{
    g_assert_false (grl_math_float_equals (1.0f, 2.0f));
}

static void
test_math_float_equals_zero (void)
{
    g_assert_true (grl_math_float_equals (0.0f, 0.0f));
}

/*
 * Test grl_math_deg_to_rad
 */

static void
test_math_deg_to_rad_zero (void)
{
    gfloat result = grl_math_deg_to_rad (0.0f);
    g_assert_true (float_equal (result, 0.0f));
}

static void
test_math_deg_to_rad_90 (void)
{
    gfloat result = grl_math_deg_to_rad (90.0f);
    g_assert_true (float_equal (result, G_PI / 2.0f));
}

static void
test_math_deg_to_rad_180 (void)
{
    gfloat result = grl_math_deg_to_rad (180.0f);
    g_assert_true (float_equal (result, G_PI));
}

static void
test_math_deg_to_rad_360 (void)
{
    gfloat result = grl_math_deg_to_rad (360.0f);
    g_assert_true (float_equal (result, 2.0f * G_PI));
}

static void
test_math_deg_to_rad_negative (void)
{
    gfloat result = grl_math_deg_to_rad (-90.0f);
    g_assert_true (float_equal (result, -G_PI / 2.0f));
}

/*
 * Test grl_math_rad_to_deg
 */

static void
test_math_rad_to_deg_zero (void)
{
    gfloat result = grl_math_rad_to_deg (0.0f);
    g_assert_true (float_equal (result, 0.0f));
}

static void
test_math_rad_to_deg_pi_half (void)
{
    gfloat result = grl_math_rad_to_deg (G_PI / 2.0f);
    g_assert_true (float_equal (result, 90.0f));
}

static void
test_math_rad_to_deg_pi (void)
{
    gfloat result = grl_math_rad_to_deg (G_PI);
    g_assert_true (float_equal (result, 180.0f));
}

static void
test_math_rad_to_deg_two_pi (void)
{
    gfloat result = grl_math_rad_to_deg (2.0f * G_PI);
    g_assert_true (float_equal (result, 360.0f));
}

static void
test_math_rad_to_deg_negative (void)
{
    gfloat result = grl_math_rad_to_deg (-G_PI / 2.0f);
    g_assert_true (float_equal (result, -90.0f));
}

/*
 * Test roundtrip conversions
 */

static void
test_math_deg_rad_roundtrip (void)
{
    gfloat degrees = 45.0f;
    gfloat radians = grl_math_deg_to_rad (degrees);
    gfloat result = grl_math_rad_to_deg (radians);

    g_assert_true (float_equal (result, degrees));
}

/*
 * Test registration
 */

int
main (int   argc,
      char *argv[])
{
    g_test_init (&argc, &argv, NULL);

    /* Clamp tests */
    g_test_add_func ("/math-utils/clamp/within-range", test_math_clamp_within_range);
    g_test_add_func ("/math-utils/clamp/below-min", test_math_clamp_below_min);
    g_test_add_func ("/math-utils/clamp/above-max", test_math_clamp_above_max);
    g_test_add_func ("/math-utils/clamp/at-min", test_math_clamp_at_min);
    g_test_add_func ("/math-utils/clamp/at-max", test_math_clamp_at_max);

    /* Clamp int tests */
    g_test_add_func ("/math-utils/clamp-int/within-range", test_math_clamp_int_within_range);
    g_test_add_func ("/math-utils/clamp-int/below-min", test_math_clamp_int_below_min);
    g_test_add_func ("/math-utils/clamp-int/above-max", test_math_clamp_int_above_max);

    /* Lerp tests */
    g_test_add_func ("/math-utils/lerp/zero", test_math_lerp_zero);
    g_test_add_func ("/math-utils/lerp/one", test_math_lerp_one);
    g_test_add_func ("/math-utils/lerp/half", test_math_lerp_half);
    g_test_add_func ("/math-utils/lerp/quarter", test_math_lerp_quarter);
    g_test_add_func ("/math-utils/lerp/negative-range", test_math_lerp_negative_range);

    /* Normalize tests */
    g_test_add_func ("/math-utils/normalize/start", test_math_normalize_start);
    g_test_add_func ("/math-utils/normalize/end", test_math_normalize_end);
    g_test_add_func ("/math-utils/normalize/middle", test_math_normalize_middle);
    g_test_add_func ("/math-utils/normalize/negative-range", test_math_normalize_negative_range);

    /* Remap tests */
    g_test_add_func ("/math-utils/remap/identity", test_math_remap_identity);
    g_test_add_func ("/math-utils/remap/double", test_math_remap_double);
    g_test_add_func ("/math-utils/remap/invert", test_math_remap_invert);
    g_test_add_func ("/math-utils/remap/offset", test_math_remap_offset);

    /* Wrap tests */
    g_test_add_func ("/math-utils/wrap/within-range", test_math_wrap_within_range);
    g_test_add_func ("/math-utils/wrap/above-max", test_math_wrap_above_max);
    g_test_add_func ("/math-utils/wrap/below-min", test_math_wrap_below_min);
    g_test_add_func ("/math-utils/wrap/at-max", test_math_wrap_at_max);
    g_test_add_func ("/math-utils/wrap/multiple-cycles", test_math_wrap_multiple_cycles);

    /* Float equals tests */
    g_test_add_func ("/math-utils/float-equals/same", test_math_float_equals_same);
    g_test_add_func ("/math-utils/float-equals/close", test_math_float_equals_close);
    g_test_add_func ("/math-utils/float-equals/different", test_math_float_equals_different);
    g_test_add_func ("/math-utils/float-equals/zero", test_math_float_equals_zero);

    /* Deg to rad tests */
    g_test_add_func ("/math-utils/deg-to-rad/zero", test_math_deg_to_rad_zero);
    g_test_add_func ("/math-utils/deg-to-rad/90", test_math_deg_to_rad_90);
    g_test_add_func ("/math-utils/deg-to-rad/180", test_math_deg_to_rad_180);
    g_test_add_func ("/math-utils/deg-to-rad/360", test_math_deg_to_rad_360);
    g_test_add_func ("/math-utils/deg-to-rad/negative", test_math_deg_to_rad_negative);

    /* Rad to deg tests */
    g_test_add_func ("/math-utils/rad-to-deg/zero", test_math_rad_to_deg_zero);
    g_test_add_func ("/math-utils/rad-to-deg/pi-half", test_math_rad_to_deg_pi_half);
    g_test_add_func ("/math-utils/rad-to-deg/pi", test_math_rad_to_deg_pi);
    g_test_add_func ("/math-utils/rad-to-deg/two-pi", test_math_rad_to_deg_two_pi);
    g_test_add_func ("/math-utils/rad-to-deg/negative", test_math_rad_to_deg_negative);

    /* Roundtrip tests */
    g_test_add_func ("/math-utils/deg-rad-roundtrip", test_math_deg_rad_roundtrip);

    return g_test_run ();
}
