/* test-vector2.c
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Unit tests for GrlVector2.
 */

#include <glib.h>
#include <math.h>
#include "graylib/math/grl-vector2.h"

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
test_vector2_new (void)
{
    g_autoptr(GrlVector2) v = grl_vector2_new (3.0f, 4.0f);

    g_assert_nonnull (v);
    g_assert_true (float_equal (v->x, 3.0f));
    g_assert_true (float_equal (v->y, 4.0f));
}

static void
test_vector2_new_zero (void)
{
    g_autoptr(GrlVector2) v = grl_vector2_new_zero ();

    g_assert_nonnull (v);
    g_assert_true (float_equal (v->x, 0.0f));
    g_assert_true (float_equal (v->y, 0.0f));
}

static void
test_vector2_new_one (void)
{
    g_autoptr(GrlVector2) v = grl_vector2_new_one ();

    g_assert_nonnull (v);
    g_assert_true (float_equal (v->x, 1.0f));
    g_assert_true (float_equal (v->y, 1.0f));
}

static void
test_vector2_init (void)
{
    GrlVector2 v = grl_vector2_init (5.0f, 6.0f);

    g_assert_true (float_equal (v.x, 5.0f));
    g_assert_true (float_equal (v.y, 6.0f));
}

static void
test_vector2_copy (void)
{
    g_autoptr(GrlVector2) v1 = grl_vector2_new (7.0f, 8.0f);
    g_autoptr(GrlVector2) v2 = grl_vector2_copy (v1);

    g_assert_nonnull (v2);
    g_assert_true (v1 != v2);
    g_assert_true (float_equal (v2->x, 7.0f));
    g_assert_true (float_equal (v2->y, 8.0f));
}

/*
 * Test accessors
 */

static void
test_vector2_accessors (void)
{
    g_autoptr(GrlVector2) v = grl_vector2_new (10.0f, 20.0f);

    g_assert_true (float_equal (grl_vector2_get_x (v), 10.0f));
    g_assert_true (float_equal (grl_vector2_get_y (v), 20.0f));

    grl_vector2_set (v, 30.0f, 40.0f);
    g_assert_true (float_equal (v->x, 30.0f));
    g_assert_true (float_equal (v->y, 40.0f));
}

/*
 * Test operations
 */

static void
test_vector2_add (void)
{
    g_autoptr(GrlVector2) v1 = grl_vector2_new (1.0f, 2.0f);
    g_autoptr(GrlVector2) v2 = grl_vector2_new (3.0f, 4.0f);
    g_autoptr(GrlVector2) result = grl_vector2_add (v1, v2);

    g_assert_nonnull (result);
    g_assert_true (float_equal (result->x, 4.0f));
    g_assert_true (float_equal (result->y, 6.0f));
}

static void
test_vector2_subtract (void)
{
    g_autoptr(GrlVector2) v1 = grl_vector2_new (5.0f, 7.0f);
    g_autoptr(GrlVector2) v2 = grl_vector2_new (2.0f, 3.0f);
    g_autoptr(GrlVector2) result = grl_vector2_subtract (v1, v2);

    g_assert_nonnull (result);
    g_assert_true (float_equal (result->x, 3.0f));
    g_assert_true (float_equal (result->y, 4.0f));
}

static void
test_vector2_scale (void)
{
    g_autoptr(GrlVector2) v = grl_vector2_new (2.0f, 3.0f);
    g_autoptr(GrlVector2) result = grl_vector2_scale (v, 4.0f);

    g_assert_nonnull (result);
    g_assert_true (float_equal (result->x, 8.0f));
    g_assert_true (float_equal (result->y, 12.0f));
}

static void
test_vector2_negate (void)
{
    g_autoptr(GrlVector2) v = grl_vector2_new (5.0f, -3.0f);
    g_autoptr(GrlVector2) result = grl_vector2_negate (v);

    g_assert_nonnull (result);
    g_assert_true (float_equal (result->x, -5.0f));
    g_assert_true (float_equal (result->y, 3.0f));
}

static void
test_vector2_length (void)
{
    g_autoptr(GrlVector2) v = grl_vector2_new (3.0f, 4.0f);
    gfloat len = grl_vector2_length (v);

    g_assert_true (float_equal (len, 5.0f));
}

static void
test_vector2_length_sqr (void)
{
    g_autoptr(GrlVector2) v = grl_vector2_new (3.0f, 4.0f);
    gfloat len_sqr = grl_vector2_length_sqr (v);

    g_assert_true (float_equal (len_sqr, 25.0f));
}

static void
test_vector2_dot (void)
{
    g_autoptr(GrlVector2) v1 = grl_vector2_new (1.0f, 2.0f);
    g_autoptr(GrlVector2) v2 = grl_vector2_new (3.0f, 4.0f);
    gfloat dot = grl_vector2_dot (v1, v2);

    /* 1*3 + 2*4 = 11 */
    g_assert_true (float_equal (dot, 11.0f));
}

static void
test_vector2_normalize (void)
{
    g_autoptr(GrlVector2) v = grl_vector2_new (3.0f, 4.0f);
    g_autoptr(GrlVector2) result = grl_vector2_normalize (v);
    gfloat len;

    g_assert_nonnull (result);

    len = grl_vector2_length (result);
    g_assert_true (float_equal (len, 1.0f));
}

static void
test_vector2_distance (void)
{
    g_autoptr(GrlVector2) v1 = grl_vector2_new (0.0f, 0.0f);
    g_autoptr(GrlVector2) v2 = grl_vector2_new (3.0f, 4.0f);
    gfloat dist = grl_vector2_distance (v1, v2);

    g_assert_true (float_equal (dist, 5.0f));
}

static void
test_vector2_lerp (void)
{
    g_autoptr(GrlVector2) v1 = grl_vector2_new (0.0f, 0.0f);
    g_autoptr(GrlVector2) v2 = grl_vector2_new (10.0f, 20.0f);
    g_autoptr(GrlVector2) result = grl_vector2_lerp (v1, v2, 0.5f);

    g_assert_nonnull (result);
    g_assert_true (float_equal (result->x, 5.0f));
    g_assert_true (float_equal (result->y, 10.0f));
}

static void
test_vector2_equal (void)
{
    g_autoptr(GrlVector2) v1 = grl_vector2_new (1.0f, 2.0f);
    g_autoptr(GrlVector2) v2 = grl_vector2_new (1.0f, 2.0f);
    g_autoptr(GrlVector2) v3 = grl_vector2_new (3.0f, 4.0f);

    g_assert_true (grl_vector2_equal (v1, v2));
    g_assert_false (grl_vector2_equal (v1, v3));
}

static void
test_vector2_to_string (void)
{
    g_autoptr(GrlVector2) v = grl_vector2_new (1.5f, 2.5f);
    g_autofree gchar *str = grl_vector2_to_string (v);

    g_assert_nonnull (str);
    g_assert_true (g_str_has_prefix (str, "GrlVector2("));
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
    g_test_add_func ("/vector2/new", test_vector2_new);
    g_test_add_func ("/vector2/new-zero", test_vector2_new_zero);
    g_test_add_func ("/vector2/new-one", test_vector2_new_one);
    g_test_add_func ("/vector2/init", test_vector2_init);
    g_test_add_func ("/vector2/copy", test_vector2_copy);

    /* Accessors */
    g_test_add_func ("/vector2/accessors", test_vector2_accessors);

    /* Operations */
    g_test_add_func ("/vector2/add", test_vector2_add);
    g_test_add_func ("/vector2/subtract", test_vector2_subtract);
    g_test_add_func ("/vector2/scale", test_vector2_scale);
    g_test_add_func ("/vector2/negate", test_vector2_negate);
    g_test_add_func ("/vector2/length", test_vector2_length);
    g_test_add_func ("/vector2/length-sqr", test_vector2_length_sqr);
    g_test_add_func ("/vector2/dot", test_vector2_dot);
    g_test_add_func ("/vector2/normalize", test_vector2_normalize);
    g_test_add_func ("/vector2/distance", test_vector2_distance);
    g_test_add_func ("/vector2/lerp", test_vector2_lerp);

    /* Comparison */
    g_test_add_func ("/vector2/equal", test_vector2_equal);

    /* Utility */
    g_test_add_func ("/vector2/to-string", test_vector2_to_string);

    return g_test_run ();
}
