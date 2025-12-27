/* test-vector3.c
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Unit tests for GrlVector3.
 */

#include <glib.h>
#include <math.h>
#include "src/math/grl-vector3.h"

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
test_vector3_new (void)
{
    g_autoptr(GrlVector3) v = grl_vector3_new (3.0f, 4.0f, 5.0f);

    g_assert_nonnull (v);
    g_assert_true (float_equal (v->x, 3.0f));
    g_assert_true (float_equal (v->y, 4.0f));
    g_assert_true (float_equal (v->z, 5.0f));
}

static void
test_vector3_new_zero (void)
{
    g_autoptr(GrlVector3) v = grl_vector3_new_zero ();

    g_assert_nonnull (v);
    g_assert_true (float_equal (v->x, 0.0f));
    g_assert_true (float_equal (v->y, 0.0f));
    g_assert_true (float_equal (v->z, 0.0f));
}

static void
test_vector3_new_one (void)
{
    g_autoptr(GrlVector3) v = grl_vector3_new_one ();

    g_assert_nonnull (v);
    g_assert_true (float_equal (v->x, 1.0f));
    g_assert_true (float_equal (v->y, 1.0f));
    g_assert_true (float_equal (v->z, 1.0f));
}

static void
test_vector3_new_up (void)
{
    g_autoptr(GrlVector3) v = grl_vector3_new_up ();

    g_assert_nonnull (v);
    g_assert_true (float_equal (v->x, 0.0f));
    g_assert_true (float_equal (v->y, 1.0f));
    g_assert_true (float_equal (v->z, 0.0f));
}

static void
test_vector3_new_forward (void)
{
    g_autoptr(GrlVector3) v = grl_vector3_new_forward ();

    g_assert_nonnull (v);
    g_assert_true (float_equal (v->x, 0.0f));
    g_assert_true (float_equal (v->y, 0.0f));
    g_assert_true (float_equal (v->z, -1.0f));
}

static void
test_vector3_new_right (void)
{
    g_autoptr(GrlVector3) v = grl_vector3_new_right ();

    g_assert_nonnull (v);
    g_assert_true (float_equal (v->x, 1.0f));
    g_assert_true (float_equal (v->y, 0.0f));
    g_assert_true (float_equal (v->z, 0.0f));
}

static void
test_vector3_copy (void)
{
    g_autoptr(GrlVector3) v1 = grl_vector3_new (7.0f, 8.0f, 9.0f);
    g_autoptr(GrlVector3) v2 = grl_vector3_copy (v1);

    g_assert_nonnull (v2);
    g_assert_true (v1 != v2);
    g_assert_true (float_equal (v2->x, 7.0f));
    g_assert_true (float_equal (v2->y, 8.0f));
    g_assert_true (float_equal (v2->z, 9.0f));
}

static void
test_vector3_copy_null (void)
{
    GrlVector3 *v = grl_vector3_copy (NULL);

    g_assert_null (v);
}

/*
 * Test operations
 */

static void
test_vector3_add (void)
{
    g_autoptr(GrlVector3) v1 = grl_vector3_new (1.0f, 2.0f, 3.0f);
    g_autoptr(GrlVector3) v2 = grl_vector3_new (4.0f, 5.0f, 6.0f);
    g_autoptr(GrlVector3) result = grl_vector3_add (v1, v2);

    g_assert_nonnull (result);
    g_assert_true (float_equal (result->x, 5.0f));
    g_assert_true (float_equal (result->y, 7.0f));
    g_assert_true (float_equal (result->z, 9.0f));
}

static void
test_vector3_subtract (void)
{
    g_autoptr(GrlVector3) v1 = grl_vector3_new (5.0f, 7.0f, 9.0f);
    g_autoptr(GrlVector3) v2 = grl_vector3_new (2.0f, 3.0f, 4.0f);
    g_autoptr(GrlVector3) result = grl_vector3_subtract (v1, v2);

    g_assert_nonnull (result);
    g_assert_true (float_equal (result->x, 3.0f));
    g_assert_true (float_equal (result->y, 4.0f));
    g_assert_true (float_equal (result->z, 5.0f));
}

static void
test_vector3_scale (void)
{
    g_autoptr(GrlVector3) v = grl_vector3_new (2.0f, 3.0f, 4.0f);
    g_autoptr(GrlVector3) result = grl_vector3_scale (v, 2.0f);

    g_assert_nonnull (result);
    g_assert_true (float_equal (result->x, 4.0f));
    g_assert_true (float_equal (result->y, 6.0f));
    g_assert_true (float_equal (result->z, 8.0f));
}

static void
test_vector3_negate (void)
{
    g_autoptr(GrlVector3) v = grl_vector3_new (5.0f, -3.0f, 7.0f);
    g_autoptr(GrlVector3) result = grl_vector3_negate (v);

    g_assert_nonnull (result);
    g_assert_true (float_equal (result->x, -5.0f));
    g_assert_true (float_equal (result->y, 3.0f));
    g_assert_true (float_equal (result->z, -7.0f));
}

static void
test_vector3_length (void)
{
    /* Vector (1, 2, 2) has length sqrt(1 + 4 + 4) = 3 */
    g_autoptr(GrlVector3) v = grl_vector3_new (1.0f, 2.0f, 2.0f);
    gfloat len = grl_vector3_length (v);

    g_assert_true (float_equal (len, 3.0f));
}

static void
test_vector3_length_sqr (void)
{
    g_autoptr(GrlVector3) v = grl_vector3_new (1.0f, 2.0f, 2.0f);
    gfloat len_sqr = grl_vector3_length_sqr (v);

    g_assert_true (float_equal (len_sqr, 9.0f));
}

static void
test_vector3_dot (void)
{
    g_autoptr(GrlVector3) v1 = grl_vector3_new (1.0f, 2.0f, 3.0f);
    g_autoptr(GrlVector3) v2 = grl_vector3_new (4.0f, 5.0f, 6.0f);
    gfloat dot = grl_vector3_dot (v1, v2);

    /* 1*4 + 2*5 + 3*6 = 4 + 10 + 18 = 32 */
    g_assert_true (float_equal (dot, 32.0f));
}

static void
test_vector3_cross (void)
{
    g_autoptr(GrlVector3) v1 = grl_vector3_new (1.0f, 0.0f, 0.0f);
    g_autoptr(GrlVector3) v2 = grl_vector3_new (0.0f, 1.0f, 0.0f);
    g_autoptr(GrlVector3) result = grl_vector3_cross (v1, v2);

    /* i x j = k, so (1,0,0) x (0,1,0) = (0,0,1) */
    g_assert_nonnull (result);
    g_assert_true (float_equal (result->x, 0.0f));
    g_assert_true (float_equal (result->y, 0.0f));
    g_assert_true (float_equal (result->z, 1.0f));
}

static void
test_vector3_cross_anticommutative (void)
{
    g_autoptr(GrlVector3) v1 = grl_vector3_new (1.0f, 0.0f, 0.0f);
    g_autoptr(GrlVector3) v2 = grl_vector3_new (0.0f, 1.0f, 0.0f);
    g_autoptr(GrlVector3) result = grl_vector3_cross (v2, v1);

    /* j x i = -k, so (0,1,0) x (1,0,0) = (0,0,-1) */
    g_assert_nonnull (result);
    g_assert_true (float_equal (result->x, 0.0f));
    g_assert_true (float_equal (result->y, 0.0f));
    g_assert_true (float_equal (result->z, -1.0f));
}

static void
test_vector3_normalize (void)
{
    g_autoptr(GrlVector3) v = grl_vector3_new (3.0f, 0.0f, 4.0f);
    g_autoptr(GrlVector3) result = grl_vector3_normalize (v);
    gfloat len;

    g_assert_nonnull (result);

    len = grl_vector3_length (result);
    g_assert_true (float_equal (len, 1.0f));
}

static void
test_vector3_distance (void)
{
    g_autoptr(GrlVector3) v1 = grl_vector3_new (0.0f, 0.0f, 0.0f);
    g_autoptr(GrlVector3) v2 = grl_vector3_new (1.0f, 2.0f, 2.0f);
    gfloat dist = grl_vector3_distance (v1, v2);

    /* Distance = sqrt(1 + 4 + 4) = 3 */
    g_assert_true (float_equal (dist, 3.0f));
}

static void
test_vector3_lerp (void)
{
    g_autoptr(GrlVector3) v1 = grl_vector3_new (0.0f, 0.0f, 0.0f);
    g_autoptr(GrlVector3) v2 = grl_vector3_new (10.0f, 20.0f, 30.0f);
    g_autoptr(GrlVector3) result = grl_vector3_lerp (v1, v2, 0.5f);

    g_assert_nonnull (result);
    g_assert_true (float_equal (result->x, 5.0f));
    g_assert_true (float_equal (result->y, 10.0f));
    g_assert_true (float_equal (result->z, 15.0f));
}

static void
test_vector3_lerp_endpoints (void)
{
    g_autoptr(GrlVector3) v1 = grl_vector3_new (1.0f, 2.0f, 3.0f);
    g_autoptr(GrlVector3) v2 = grl_vector3_new (10.0f, 20.0f, 30.0f);
    g_autoptr(GrlVector3) at_zero = grl_vector3_lerp (v1, v2, 0.0f);
    g_autoptr(GrlVector3) at_one = grl_vector3_lerp (v1, v2, 1.0f);

    /* At t=0, should equal v1 */
    g_assert_true (float_equal (at_zero->x, 1.0f));
    g_assert_true (float_equal (at_zero->y, 2.0f));
    g_assert_true (float_equal (at_zero->z, 3.0f));

    /* At t=1, should equal v2 */
    g_assert_true (float_equal (at_one->x, 10.0f));
    g_assert_true (float_equal (at_one->y, 20.0f));
    g_assert_true (float_equal (at_one->z, 30.0f));
}

static void
test_vector3_equal (void)
{
    g_autoptr(GrlVector3) v1 = grl_vector3_new (1.0f, 2.0f, 3.0f);
    g_autoptr(GrlVector3) v2 = grl_vector3_new (1.0f, 2.0f, 3.0f);
    g_autoptr(GrlVector3) v3 = grl_vector3_new (3.0f, 4.0f, 5.0f);

    g_assert_true (grl_vector3_equal (v1, v2));
    g_assert_false (grl_vector3_equal (v1, v3));
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
    g_test_add_func ("/vector3/new", test_vector3_new);
    g_test_add_func ("/vector3/new-zero", test_vector3_new_zero);
    g_test_add_func ("/vector3/new-one", test_vector3_new_one);
    g_test_add_func ("/vector3/new-up", test_vector3_new_up);
    g_test_add_func ("/vector3/new-forward", test_vector3_new_forward);
    g_test_add_func ("/vector3/new-right", test_vector3_new_right);
    g_test_add_func ("/vector3/copy", test_vector3_copy);
    g_test_add_func ("/vector3/copy-null", test_vector3_copy_null);

    /* Operations */
    g_test_add_func ("/vector3/add", test_vector3_add);
    g_test_add_func ("/vector3/subtract", test_vector3_subtract);
    g_test_add_func ("/vector3/scale", test_vector3_scale);
    g_test_add_func ("/vector3/negate", test_vector3_negate);
    g_test_add_func ("/vector3/length", test_vector3_length);
    g_test_add_func ("/vector3/length-sqr", test_vector3_length_sqr);
    g_test_add_func ("/vector3/dot", test_vector3_dot);
    g_test_add_func ("/vector3/cross", test_vector3_cross);
    g_test_add_func ("/vector3/cross-anticommutative", test_vector3_cross_anticommutative);
    g_test_add_func ("/vector3/normalize", test_vector3_normalize);
    g_test_add_func ("/vector3/distance", test_vector3_distance);
    g_test_add_func ("/vector3/lerp", test_vector3_lerp);
    g_test_add_func ("/vector3/lerp-endpoints", test_vector3_lerp_endpoints);

    /* Comparison */
    g_test_add_func ("/vector3/equal", test_vector3_equal);

    return g_test_run ();
}
