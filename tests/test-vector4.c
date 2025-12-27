/* test-vector4.c
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Unit tests for GrlVector4.
 */

#include <glib.h>
#include <math.h>
#include "src/math/grl-vector4.h"

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
test_vector4_new (void)
{
    g_autoptr(GrlVector4) v = grl_vector4_new (1.0f, 2.0f, 3.0f, 4.0f);

    g_assert_nonnull (v);
    g_assert_true (float_equal (v->x, 1.0f));
    g_assert_true (float_equal (v->y, 2.0f));
    g_assert_true (float_equal (v->z, 3.0f));
    g_assert_true (float_equal (v->w, 4.0f));
}

static void
test_vector4_new_zero (void)
{
    g_autoptr(GrlVector4) v = grl_vector4_new_zero ();

    g_assert_nonnull (v);
    g_assert_true (float_equal (v->x, 0.0f));
    g_assert_true (float_equal (v->y, 0.0f));
    g_assert_true (float_equal (v->z, 0.0f));
    g_assert_true (float_equal (v->w, 0.0f));
}

static void
test_vector4_new_one (void)
{
    g_autoptr(GrlVector4) v = grl_vector4_new_one ();

    g_assert_nonnull (v);
    g_assert_true (float_equal (v->x, 1.0f));
    g_assert_true (float_equal (v->y, 1.0f));
    g_assert_true (float_equal (v->z, 1.0f));
    g_assert_true (float_equal (v->w, 1.0f));
}

static void
test_vector4_new_identity (void)
{
    /* Identity quaternion is (0, 0, 0, 1) */
    g_autoptr(GrlVector4) v = grl_vector4_new_identity ();

    g_assert_nonnull (v);
    g_assert_true (float_equal (v->x, 0.0f));
    g_assert_true (float_equal (v->y, 0.0f));
    g_assert_true (float_equal (v->z, 0.0f));
    g_assert_true (float_equal (v->w, 1.0f));
}

static void
test_vector4_copy (void)
{
    g_autoptr(GrlVector4) v1 = grl_vector4_new (5.0f, 6.0f, 7.0f, 8.0f);
    g_autoptr(GrlVector4) v2 = grl_vector4_copy (v1);

    g_assert_nonnull (v2);
    g_assert_true (v1 != v2);
    g_assert_true (float_equal (v2->x, 5.0f));
    g_assert_true (float_equal (v2->y, 6.0f));
    g_assert_true (float_equal (v2->z, 7.0f));
    g_assert_true (float_equal (v2->w, 8.0f));
}

static void
test_vector4_copy_null (void)
{
    GrlVector4 *v = grl_vector4_copy (NULL);

    g_assert_null (v);
}

/*
 * Test operations
 */

static void
test_vector4_add (void)
{
    g_autoptr(GrlVector4) v1 = grl_vector4_new (1.0f, 2.0f, 3.0f, 4.0f);
    g_autoptr(GrlVector4) v2 = grl_vector4_new (5.0f, 6.0f, 7.0f, 8.0f);
    g_autoptr(GrlVector4) result = grl_vector4_add (v1, v2);

    g_assert_nonnull (result);
    g_assert_true (float_equal (result->x, 6.0f));
    g_assert_true (float_equal (result->y, 8.0f));
    g_assert_true (float_equal (result->z, 10.0f));
    g_assert_true (float_equal (result->w, 12.0f));
}

static void
test_vector4_subtract (void)
{
    g_autoptr(GrlVector4) v1 = grl_vector4_new (5.0f, 7.0f, 9.0f, 11.0f);
    g_autoptr(GrlVector4) v2 = grl_vector4_new (2.0f, 3.0f, 4.0f, 5.0f);
    g_autoptr(GrlVector4) result = grl_vector4_subtract (v1, v2);

    g_assert_nonnull (result);
    g_assert_true (float_equal (result->x, 3.0f));
    g_assert_true (float_equal (result->y, 4.0f));
    g_assert_true (float_equal (result->z, 5.0f));
    g_assert_true (float_equal (result->w, 6.0f));
}

static void
test_vector4_scale (void)
{
    g_autoptr(GrlVector4) v = grl_vector4_new (2.0f, 3.0f, 4.0f, 5.0f);
    g_autoptr(GrlVector4) result = grl_vector4_scale (v, 2.0f);

    g_assert_nonnull (result);
    g_assert_true (float_equal (result->x, 4.0f));
    g_assert_true (float_equal (result->y, 6.0f));
    g_assert_true (float_equal (result->z, 8.0f));
    g_assert_true (float_equal (result->w, 10.0f));
}

static void
test_vector4_negate (void)
{
    g_autoptr(GrlVector4) v = grl_vector4_new (5.0f, -3.0f, 7.0f, -2.0f);
    g_autoptr(GrlVector4) result = grl_vector4_negate (v);

    g_assert_nonnull (result);
    g_assert_true (float_equal (result->x, -5.0f));
    g_assert_true (float_equal (result->y, 3.0f));
    g_assert_true (float_equal (result->z, -7.0f));
    g_assert_true (float_equal (result->w, 2.0f));
}

static void
test_vector4_length (void)
{
    /* Vector (1, 2, 2, 2) has length sqrt(1 + 4 + 4 + 4) = sqrt(13) ~ 3.606 */
    g_autoptr(GrlVector4) v = grl_vector4_new (1.0f, 2.0f, 2.0f, 2.0f);
    gfloat len = grl_vector4_length (v);

    g_assert_true (float_equal (len, sqrtf (13.0f)));
}

static void
test_vector4_length_sqr (void)
{
    g_autoptr(GrlVector4) v = grl_vector4_new (1.0f, 2.0f, 2.0f, 2.0f);
    gfloat len_sqr = grl_vector4_length_sqr (v);

    g_assert_true (float_equal (len_sqr, 13.0f));
}

static void
test_vector4_dot (void)
{
    g_autoptr(GrlVector4) v1 = grl_vector4_new (1.0f, 2.0f, 3.0f, 4.0f);
    g_autoptr(GrlVector4) v2 = grl_vector4_new (5.0f, 6.0f, 7.0f, 8.0f);
    gfloat dot = grl_vector4_dot (v1, v2);

    /* 1*5 + 2*6 + 3*7 + 4*8 = 5 + 12 + 21 + 32 = 70 */
    g_assert_true (float_equal (dot, 70.0f));
}

static void
test_vector4_normalize (void)
{
    g_autoptr(GrlVector4) v = grl_vector4_new (3.0f, 0.0f, 4.0f, 0.0f);
    g_autoptr(GrlVector4) result = grl_vector4_normalize (v);
    gfloat len;

    g_assert_nonnull (result);

    len = grl_vector4_length (result);
    g_assert_true (float_equal (len, 1.0f));
}

static void
test_vector4_lerp (void)
{
    g_autoptr(GrlVector4) v1 = grl_vector4_new (0.0f, 0.0f, 0.0f, 0.0f);
    g_autoptr(GrlVector4) v2 = grl_vector4_new (10.0f, 20.0f, 30.0f, 40.0f);
    g_autoptr(GrlVector4) result = grl_vector4_lerp (v1, v2, 0.5f);

    g_assert_nonnull (result);
    g_assert_true (float_equal (result->x, 5.0f));
    g_assert_true (float_equal (result->y, 10.0f));
    g_assert_true (float_equal (result->z, 15.0f));
    g_assert_true (float_equal (result->w, 20.0f));
}

static void
test_vector4_lerp_endpoints (void)
{
    g_autoptr(GrlVector4) v1 = grl_vector4_new (1.0f, 2.0f, 3.0f, 4.0f);
    g_autoptr(GrlVector4) v2 = grl_vector4_new (10.0f, 20.0f, 30.0f, 40.0f);
    g_autoptr(GrlVector4) at_zero = grl_vector4_lerp (v1, v2, 0.0f);
    g_autoptr(GrlVector4) at_one = grl_vector4_lerp (v1, v2, 1.0f);

    /* At t=0, should equal v1 */
    g_assert_true (float_equal (at_zero->x, 1.0f));
    g_assert_true (float_equal (at_zero->y, 2.0f));
    g_assert_true (float_equal (at_zero->z, 3.0f));
    g_assert_true (float_equal (at_zero->w, 4.0f));

    /* At t=1, should equal v2 */
    g_assert_true (float_equal (at_one->x, 10.0f));
    g_assert_true (float_equal (at_one->y, 20.0f));
    g_assert_true (float_equal (at_one->z, 30.0f));
    g_assert_true (float_equal (at_one->w, 40.0f));
}

static void
test_vector4_equal (void)
{
    g_autoptr(GrlVector4) v1 = grl_vector4_new (1.0f, 2.0f, 3.0f, 4.0f);
    g_autoptr(GrlVector4) v2 = grl_vector4_new (1.0f, 2.0f, 3.0f, 4.0f);
    g_autoptr(GrlVector4) v3 = grl_vector4_new (5.0f, 6.0f, 7.0f, 8.0f);

    g_assert_true (grl_vector4_equal (v1, v2));
    g_assert_false (grl_vector4_equal (v1, v3));
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
    g_test_add_func ("/vector4/new", test_vector4_new);
    g_test_add_func ("/vector4/new-zero", test_vector4_new_zero);
    g_test_add_func ("/vector4/new-one", test_vector4_new_one);
    g_test_add_func ("/vector4/new-identity", test_vector4_new_identity);
    g_test_add_func ("/vector4/copy", test_vector4_copy);
    g_test_add_func ("/vector4/copy-null", test_vector4_copy_null);

    /* Operations */
    g_test_add_func ("/vector4/add", test_vector4_add);
    g_test_add_func ("/vector4/subtract", test_vector4_subtract);
    g_test_add_func ("/vector4/scale", test_vector4_scale);
    g_test_add_func ("/vector4/negate", test_vector4_negate);
    g_test_add_func ("/vector4/length", test_vector4_length);
    g_test_add_func ("/vector4/length-sqr", test_vector4_length_sqr);
    g_test_add_func ("/vector4/dot", test_vector4_dot);
    g_test_add_func ("/vector4/normalize", test_vector4_normalize);
    g_test_add_func ("/vector4/lerp", test_vector4_lerp);
    g_test_add_func ("/vector4/lerp-endpoints", test_vector4_lerp_endpoints);

    /* Comparison */
    g_test_add_func ("/vector4/equal", test_vector4_equal);

    return g_test_run ();
}
