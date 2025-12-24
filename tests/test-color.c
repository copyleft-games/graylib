/* test-color.c
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Unit tests for GrlColor.
 */

#include <glib.h>
#include <math.h>
#include "src/math/grl-color.h"

#define EPSILON 0.0001f

static gboolean
float_equal (gfloat a, gfloat b)
{
    return fabsf (a - b) < EPSILON;
}

/*
 * Test constructors
 */

static void
test_color_new (void)
{
    g_autoptr(GrlColor) c = grl_color_new (100, 150, 200, 255);

    g_assert_nonnull (c);
    g_assert_cmpint (c->r, ==, 100);
    g_assert_cmpint (c->g, ==, 150);
    g_assert_cmpint (c->b, ==, 200);
    g_assert_cmpint (c->a, ==, 255);
}

static void
test_color_new_rgb (void)
{
    g_autoptr(GrlColor) c = grl_color_new_rgb (50, 100, 150);

    g_assert_nonnull (c);
    g_assert_cmpint (c->r, ==, 50);
    g_assert_cmpint (c->g, ==, 100);
    g_assert_cmpint (c->b, ==, 150);
    g_assert_cmpint (c->a, ==, 255);
}

static void
test_color_new_from_int (void)
{
    /* 0xFF5500FF = 255, 85, 0, 255 (RGBA) */
    g_autoptr(GrlColor) c = grl_color_new_from_int (0xFF5500FF);

    g_assert_nonnull (c);
    g_assert_cmpint (c->r, ==, 255);
    g_assert_cmpint (c->g, ==, 85);
    g_assert_cmpint (c->b, ==, 0);
    g_assert_cmpint (c->a, ==, 255);
}

static void
test_color_init (void)
{
    GrlColor c = grl_color_init (10, 20, 30, 40);

    g_assert_cmpint (c.r, ==, 10);
    g_assert_cmpint (c.g, ==, 20);
    g_assert_cmpint (c.b, ==, 30);
    g_assert_cmpint (c.a, ==, 40);
}

static void
test_color_copy (void)
{
    g_autoptr(GrlColor) c1 = grl_color_new (11, 22, 33, 44);
    g_autoptr(GrlColor) c2 = grl_color_copy (c1);

    g_assert_nonnull (c2);
    g_assert_true (c1 != c2);
    g_assert_cmpint (c2->r, ==, 11);
    g_assert_cmpint (c2->g, ==, 22);
    g_assert_cmpint (c2->b, ==, 33);
    g_assert_cmpint (c2->a, ==, 44);
}

/*
 * Test predefined colors
 */

static void
test_color_white (void)
{
    g_autoptr(GrlColor) c = grl_color_new_white ();

    g_assert_cmpint (c->r, ==, 255);
    g_assert_cmpint (c->g, ==, 255);
    g_assert_cmpint (c->b, ==, 255);
    g_assert_cmpint (c->a, ==, 255);
}

static void
test_color_black (void)
{
    g_autoptr(GrlColor) c = grl_color_new_black ();

    g_assert_cmpint (c->r, ==, 0);
    g_assert_cmpint (c->g, ==, 0);
    g_assert_cmpint (c->b, ==, 0);
    g_assert_cmpint (c->a, ==, 255);
}

static void
test_color_blank (void)
{
    g_autoptr(GrlColor) c = grl_color_new_blank ();

    g_assert_cmpint (c->r, ==, 0);
    g_assert_cmpint (c->g, ==, 0);
    g_assert_cmpint (c->b, ==, 0);
    g_assert_cmpint (c->a, ==, 0);
}

static void
test_color_red (void)
{
    g_autoptr(GrlColor) c = grl_color_new_red ();

    /* raylib's RED is (230, 41, 55, 255) */
    g_assert_cmpint (c->r, ==, 230);
    g_assert_cmpint (c->g, ==, 41);
    g_assert_cmpint (c->b, ==, 55);
    g_assert_cmpint (c->a, ==, 255);
}

/*
 * Test accessors
 */

static void
test_color_accessors (void)
{
    g_autoptr(GrlColor) c = grl_color_new (10, 20, 30, 40);

    g_assert_cmpint (grl_color_get_r (c), ==, 10);
    g_assert_cmpint (grl_color_get_g (c), ==, 20);
    g_assert_cmpint (grl_color_get_b (c), ==, 30);
    g_assert_cmpint (grl_color_get_a (c), ==, 40);

    grl_color_set (c, 50, 60, 70, 80);
    g_assert_cmpint (c->r, ==, 50);
    g_assert_cmpint (c->g, ==, 60);
    g_assert_cmpint (c->b, ==, 70);
    g_assert_cmpint (c->a, ==, 80);
}

/*
 * Test operations
 */

static void
test_color_fade (void)
{
    g_autoptr(GrlColor) c = grl_color_new (255, 128, 64, 255);
    g_autoptr(GrlColor) faded = grl_color_fade (c, 0.5f);

    g_assert_nonnull (faded);
    g_assert_cmpint (faded->r, ==, 255);
    g_assert_cmpint (faded->g, ==, 128);
    g_assert_cmpint (faded->b, ==, 64);
    g_assert_cmpint (faded->a, ==, 127);
}

static void
test_color_alpha (void)
{
    g_autoptr(GrlColor) c = grl_color_new (100, 100, 100, 100);
    g_autoptr(GrlColor) result = grl_color_alpha (c, 0.25f);

    g_assert_nonnull (result);
    g_assert_cmpint (result->r, ==, 100);
    g_assert_cmpint (result->g, ==, 100);
    g_assert_cmpint (result->b, ==, 100);
    g_assert_cmpint (result->a, ==, 63);
}

static void
test_color_lerp (void)
{
    g_autoptr(GrlColor) c1 = grl_color_new (0, 0, 0, 255);
    g_autoptr(GrlColor) c2 = grl_color_new (100, 200, 50, 255);
    g_autoptr(GrlColor) result = grl_color_lerp (c1, c2, 0.5f);

    g_assert_nonnull (result);
    g_assert_cmpint (result->r, ==, 50);
    g_assert_cmpint (result->g, ==, 100);
    g_assert_cmpint (result->b, ==, 25);
    g_assert_cmpint (result->a, ==, 255);
}

/*
 * Test conversions
 */

static void
test_color_to_int (void)
{
    g_autoptr(GrlColor) c = grl_color_new (255, 128, 64, 255);
    guint32 val = grl_color_to_int (c);

    /* 0xFF8040FF */
    g_assert_cmphex (val, ==, 0xFF8040FF);
}

static void
test_color_normalize (void)
{
    g_autoptr(GrlColor) c = grl_color_new (255, 128, 0, 255);
    gfloat r, g, b, a;

    grl_color_normalize (c, &r, &g, &b, &a);

    g_assert_true (float_equal (r, 1.0f));
    g_assert_true (float_equal (g, 128.0f / 255.0f));
    g_assert_true (float_equal (b, 0.0f));
    g_assert_true (float_equal (a, 1.0f));
}

/*
 * Test comparison
 */

static void
test_color_equal (void)
{
    g_autoptr(GrlColor) c1 = grl_color_new (100, 150, 200, 255);
    g_autoptr(GrlColor) c2 = grl_color_new (100, 150, 200, 255);
    g_autoptr(GrlColor) c3 = grl_color_new (200, 150, 100, 255);

    g_assert_true (grl_color_equal (c1, c2));
    g_assert_false (grl_color_equal (c1, c3));
}

/*
 * Test utility
 */

static void
test_color_to_string (void)
{
    g_autoptr(GrlColor) c = grl_color_new (100, 150, 200, 255);
    g_autofree gchar *str = grl_color_to_string (c);

    g_assert_nonnull (str);
    g_assert_true (g_str_has_prefix (str, "GrlColor("));
}

static void
test_color_to_hex (void)
{
    g_autoptr(GrlColor) c = grl_color_new (255, 128, 64, 255);
    g_autofree gchar *hex = grl_color_to_hex (c);

    g_assert_nonnull (hex);
    g_assert_cmpstr (hex, ==, "#FF8040FF");
}

/*
 * Test registration
 */

int
main (int   argc,
      char *argv[])
{
    g_test_init (&argc, &argv, NULL);

    /* Constructors */
    g_test_add_func ("/color/new", test_color_new);
    g_test_add_func ("/color/new-rgb", test_color_new_rgb);
    g_test_add_func ("/color/new-from-int", test_color_new_from_int);
    g_test_add_func ("/color/init", test_color_init);
    g_test_add_func ("/color/copy", test_color_copy);

    /* Predefined colors */
    g_test_add_func ("/color/white", test_color_white);
    g_test_add_func ("/color/black", test_color_black);
    g_test_add_func ("/color/blank", test_color_blank);
    g_test_add_func ("/color/red", test_color_red);

    /* Accessors */
    g_test_add_func ("/color/accessors", test_color_accessors);

    /* Operations */
    g_test_add_func ("/color/fade", test_color_fade);
    g_test_add_func ("/color/alpha", test_color_alpha);
    g_test_add_func ("/color/lerp", test_color_lerp);

    /* Conversions */
    g_test_add_func ("/color/to-int", test_color_to_int);
    g_test_add_func ("/color/normalize", test_color_normalize);

    /* Comparison */
    g_test_add_func ("/color/equal", test_color_equal);

    /* Utility */
    g_test_add_func ("/color/to-string", test_color_to_string);
    g_test_add_func ("/color/to-hex", test_color_to_hex);

    return g_test_run ();
}
