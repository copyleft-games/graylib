/* test-quaternion.c
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Unit tests for GrlQuaternion.
 */

#include <glib.h>
#include <math.h>
#include "src/math/grl-quaternion.h"
#include "src/math/grl-vector3.h"
#include "src/math/grl-matrix.h"

#define EPSILON 0.0001f
#define PI 3.14159265358979323846f

static gboolean
float_equal (gfloat a, gfloat b)
{
    return fabsf (a - b) < EPSILON;
}

/*
 * Test constructors
 */

static void
test_quaternion_new (void)
{
    g_autoptr(GrlQuaternion) q = grl_quaternion_new (0.0f, 0.0f, 0.0f, 1.0f);

    g_assert_nonnull (q);
    g_assert_true (float_equal (q->x, 0.0f));
    g_assert_true (float_equal (q->y, 0.0f));
    g_assert_true (float_equal (q->z, 0.0f));
    g_assert_true (float_equal (q->w, 1.0f));
}

static void
test_quaternion_new_identity (void)
{
    g_autoptr(GrlQuaternion) q = grl_quaternion_new_identity ();

    g_assert_nonnull (q);
    g_assert_true (float_equal (q->x, 0.0f));
    g_assert_true (float_equal (q->y, 0.0f));
    g_assert_true (float_equal (q->z, 0.0f));
    g_assert_true (float_equal (q->w, 1.0f));
}

static void
test_quaternion_copy (void)
{
    g_autoptr(GrlQuaternion) q1 = grl_quaternion_new (1.0f, 2.0f, 3.0f, 4.0f);
    g_autoptr(GrlQuaternion) q2 = grl_quaternion_copy (q1);

    g_assert_nonnull (q2);
    g_assert_true (q1 != q2);
    g_assert_true (float_equal (q2->x, 1.0f));
    g_assert_true (float_equal (q2->y, 2.0f));
    g_assert_true (float_equal (q2->z, 3.0f));
    g_assert_true (float_equal (q2->w, 4.0f));
}

static void
test_quaternion_copy_null (void)
{
    GrlQuaternion *q = grl_quaternion_copy (NULL);

    g_assert_null (q);
}

static void
test_quaternion_new_from_axis_angle (void)
{
    /* 90 degree rotation around Y axis */
    g_autoptr(GrlVector3) axis = grl_vector3_new (0.0f, 1.0f, 0.0f);
    g_autoptr(GrlQuaternion) q = grl_quaternion_new_from_axis_angle (axis, PI / 2.0f);
    gfloat len;

    g_assert_nonnull (q);

    /* Quaternion for 90° around Y: (0, sin(45°), 0, cos(45°)) */
    g_assert_true (float_equal (q->x, 0.0f));
    g_assert_true (float_equal (q->y, sinf (PI / 4.0f)));
    g_assert_true (float_equal (q->z, 0.0f));
    g_assert_true (float_equal (q->w, cosf (PI / 4.0f)));

    /* Should be unit quaternion */
    len = grl_quaternion_length (q);
    g_assert_true (float_equal (len, 1.0f));
}

static void
test_quaternion_new_from_euler (void)
{
    /* Create quaternion from zero rotation */
    g_autoptr(GrlQuaternion) q = grl_quaternion_new_from_euler (0.0f, 0.0f, 0.0f);
    gfloat len;

    g_assert_nonnull (q);

    /* Should be identity quaternion */
    g_assert_true (float_equal (q->x, 0.0f));
    g_assert_true (float_equal (q->y, 0.0f));
    g_assert_true (float_equal (q->z, 0.0f));
    g_assert_true (float_equal (q->w, 1.0f));

    /* Should be unit quaternion */
    len = grl_quaternion_length (q);
    g_assert_true (float_equal (len, 1.0f));
}

/*
 * Test arithmetic operations
 */

static void
test_quaternion_add (void)
{
    g_autoptr(GrlQuaternion) q1 = grl_quaternion_new (1.0f, 2.0f, 3.0f, 4.0f);
    g_autoptr(GrlQuaternion) q2 = grl_quaternion_new (5.0f, 6.0f, 7.0f, 8.0f);
    g_autoptr(GrlQuaternion) result = grl_quaternion_add (q1, q2);

    g_assert_nonnull (result);
    g_assert_true (float_equal (result->x, 6.0f));
    g_assert_true (float_equal (result->y, 8.0f));
    g_assert_true (float_equal (result->z, 10.0f));
    g_assert_true (float_equal (result->w, 12.0f));
}

static void
test_quaternion_subtract (void)
{
    g_autoptr(GrlQuaternion) q1 = grl_quaternion_new (5.0f, 7.0f, 9.0f, 11.0f);
    g_autoptr(GrlQuaternion) q2 = grl_quaternion_new (1.0f, 2.0f, 3.0f, 4.0f);
    g_autoptr(GrlQuaternion) result = grl_quaternion_subtract (q1, q2);

    g_assert_nonnull (result);
    g_assert_true (float_equal (result->x, 4.0f));
    g_assert_true (float_equal (result->y, 5.0f));
    g_assert_true (float_equal (result->z, 6.0f));
    g_assert_true (float_equal (result->w, 7.0f));
}

static void
test_quaternion_scale (void)
{
    g_autoptr(GrlQuaternion) q = grl_quaternion_new (1.0f, 2.0f, 3.0f, 4.0f);
    g_autoptr(GrlQuaternion) result = grl_quaternion_scale (q, 2.0f);

    g_assert_nonnull (result);
    g_assert_true (float_equal (result->x, 2.0f));
    g_assert_true (float_equal (result->y, 4.0f));
    g_assert_true (float_equal (result->z, 6.0f));
    g_assert_true (float_equal (result->w, 8.0f));
}

static void
test_quaternion_negate (void)
{
    g_autoptr(GrlQuaternion) q = grl_quaternion_new (1.0f, -2.0f, 3.0f, -4.0f);
    g_autoptr(GrlQuaternion) result = grl_quaternion_negate (q);

    g_assert_nonnull (result);
    g_assert_true (float_equal (result->x, -1.0f));
    g_assert_true (float_equal (result->y, 2.0f));
    g_assert_true (float_equal (result->z, -3.0f));
    g_assert_true (float_equal (result->w, 4.0f));
}

/*
 * Test quaternion operations
 */

static void
test_quaternion_length (void)
{
    /* Quaternion (1, 2, 2, 4) has length sqrt(1 + 4 + 4 + 16) = 5 */
    g_autoptr(GrlQuaternion) q = grl_quaternion_new (1.0f, 2.0f, 2.0f, 4.0f);
    gfloat len = grl_quaternion_length (q);

    g_assert_true (float_equal (len, 5.0f));
}

static void
test_quaternion_normalize (void)
{
    g_autoptr(GrlQuaternion) q = grl_quaternion_new (0.0f, 0.0f, 0.0f, 2.0f);
    g_autoptr(GrlQuaternion) result = grl_quaternion_normalize (q);
    gfloat len;

    g_assert_nonnull (result);

    len = grl_quaternion_length (result);
    g_assert_true (float_equal (len, 1.0f));

    /* For (0,0,0,2), normalized should be (0,0,0,1) */
    g_assert_true (float_equal (result->x, 0.0f));
    g_assert_true (float_equal (result->y, 0.0f));
    g_assert_true (float_equal (result->z, 0.0f));
    g_assert_true (float_equal (result->w, 1.0f));
}

static void
test_quaternion_multiply_identity (void)
{
    g_autoptr(GrlQuaternion) q = grl_quaternion_new (1.0f, 2.0f, 3.0f, 4.0f);
    g_autoptr(GrlQuaternion) identity = grl_quaternion_new_identity ();
    g_autoptr(GrlQuaternion) result = grl_quaternion_multiply (q, identity);

    g_assert_nonnull (result);

    /* Multiplying by identity should give the same quaternion */
    g_assert_true (float_equal (result->x, q->x));
    g_assert_true (float_equal (result->y, q->y));
    g_assert_true (float_equal (result->z, q->z));
    g_assert_true (float_equal (result->w, q->w));
}

static void
test_quaternion_invert (void)
{
    g_autoptr(GrlQuaternion) q = grl_quaternion_new_identity ();
    g_autoptr(GrlQuaternion) inv = grl_quaternion_invert (q);
    g_autoptr(GrlQuaternion) result = grl_quaternion_multiply (q, inv);

    g_assert_nonnull (inv);
    g_assert_nonnull (result);

    /* q * q^-1 should give identity */
    g_assert_true (float_equal (result->x, 0.0f));
    g_assert_true (float_equal (result->y, 0.0f));
    g_assert_true (float_equal (result->z, 0.0f));
    g_assert_true (float_equal (result->w, 1.0f));
}

/*
 * Test interpolation
 */

static void
test_quaternion_lerp_endpoints (void)
{
    g_autoptr(GrlQuaternion) q1 = grl_quaternion_new (0.0f, 0.0f, 0.0f, 1.0f);
    g_autoptr(GrlQuaternion) q2 = grl_quaternion_new (0.0f, 1.0f, 0.0f, 0.0f);
    g_autoptr(GrlQuaternion) at_zero = grl_quaternion_lerp (q1, q2, 0.0f);
    g_autoptr(GrlQuaternion) at_one = grl_quaternion_lerp (q1, q2, 1.0f);

    /* At t=0, should equal q1 */
    g_assert_true (float_equal (at_zero->x, q1->x));
    g_assert_true (float_equal (at_zero->y, q1->y));
    g_assert_true (float_equal (at_zero->z, q1->z));
    g_assert_true (float_equal (at_zero->w, q1->w));

    /* At t=1, should equal q2 */
    g_assert_true (float_equal (at_one->x, q2->x));
    g_assert_true (float_equal (at_one->y, q2->y));
    g_assert_true (float_equal (at_one->z, q2->z));
    g_assert_true (float_equal (at_one->w, q2->w));
}

static void
test_quaternion_nlerp (void)
{
    g_autoptr(GrlQuaternion) q1 = grl_quaternion_new_identity ();
    g_autoptr(GrlQuaternion) q2 = grl_quaternion_new (0.0f, 1.0f, 0.0f, 0.0f);
    g_autoptr(GrlQuaternion) result = grl_quaternion_nlerp (q1, q2, 0.5f);
    gfloat len;

    g_assert_nonnull (result);

    /* nlerp should return a unit quaternion */
    len = grl_quaternion_length (result);
    g_assert_true (float_equal (len, 1.0f));
}

static void
test_quaternion_slerp (void)
{
    g_autoptr(GrlQuaternion) q1 = grl_quaternion_new_identity ();
    g_autoptr(GrlQuaternion) q2 = grl_quaternion_new (0.0f, 1.0f, 0.0f, 0.0f);
    g_autoptr(GrlQuaternion) result = grl_quaternion_slerp (q1, q2, 0.5f);
    gfloat len;

    g_assert_nonnull (result);

    /* slerp should return a unit quaternion */
    len = grl_quaternion_length (result);
    g_assert_true (float_equal (len, 1.0f));
}

static void
test_quaternion_slerp_endpoints (void)
{
    g_autoptr(GrlQuaternion) q1 = grl_quaternion_new (0.0f, 0.0f, 0.0f, 1.0f);
    g_autoptr(GrlQuaternion) q2 = grl_quaternion_new (0.0f, 1.0f, 0.0f, 0.0f);
    g_autoptr(GrlQuaternion) at_zero = grl_quaternion_slerp (q1, q2, 0.0f);
    g_autoptr(GrlQuaternion) at_one = grl_quaternion_slerp (q1, q2, 1.0f);

    /* At t=0, should equal q1 */
    g_assert_true (float_equal (at_zero->x, q1->x));
    g_assert_true (float_equal (at_zero->y, q1->y));
    g_assert_true (float_equal (at_zero->z, q1->z));
    g_assert_true (float_equal (at_zero->w, q1->w));

    /* At t=1, should equal q2 */
    g_assert_true (float_equal (at_one->x, q2->x));
    g_assert_true (float_equal (at_one->y, q2->y));
    g_assert_true (float_equal (at_one->z, q2->z));
    g_assert_true (float_equal (at_one->w, q2->w));
}

/*
 * Test conversions
 */

static void
test_quaternion_to_euler_identity (void)
{
    g_autoptr(GrlQuaternion) q = grl_quaternion_new_identity ();
    gfloat pitch, yaw, roll;

    grl_quaternion_to_euler (q, &pitch, &yaw, &roll);

    /* Identity quaternion should give zero rotation */
    g_assert_true (float_equal (pitch, 0.0f));
    g_assert_true (float_equal (yaw, 0.0f));
    g_assert_true (float_equal (roll, 0.0f));
}

static void
test_quaternion_euler_roundtrip (void)
{
    gfloat original_pitch, original_yaw, original_roll;
    gfloat result_pitch, result_yaw, result_roll;
    g_autoptr(GrlQuaternion) q = NULL;

    original_pitch = PI / 4.0f;  /* 45 degrees */
    original_yaw = 0.0f;
    original_roll = 0.0f;

    q = grl_quaternion_new_from_euler (original_pitch, original_yaw, original_roll);
    grl_quaternion_to_euler (q, &result_pitch, &result_yaw, &result_roll);

    /* Should get back similar values (note: there can be representation differences) */
    g_assert_true (float_equal (result_pitch, original_pitch));
    g_assert_true (float_equal (result_yaw, original_yaw));
    g_assert_true (float_equal (result_roll, original_roll));
}

static void
test_quaternion_to_matrix (void)
{
    g_autoptr(GrlQuaternion) q = grl_quaternion_new_identity ();
    g_autoptr(GrlMatrix) m = grl_quaternion_to_matrix (q);

    g_assert_nonnull (m);

    /* Identity quaternion should give identity matrix */
    /* Check diagonal elements are 1 */
    g_assert_true (float_equal (m->m0, 1.0f));
    g_assert_true (float_equal (m->m5, 1.0f));
    g_assert_true (float_equal (m->m10, 1.0f));
    g_assert_true (float_equal (m->m15, 1.0f));

    /* Check off-diagonal elements are 0 */
    g_assert_true (float_equal (m->m1, 0.0f));
    g_assert_true (float_equal (m->m2, 0.0f));
    g_assert_true (float_equal (m->m4, 0.0f));
}

/*
 * Test comparison
 */

static void
test_quaternion_equal (void)
{
    g_autoptr(GrlQuaternion) q1 = grl_quaternion_new (1.0f, 2.0f, 3.0f, 4.0f);
    g_autoptr(GrlQuaternion) q2 = grl_quaternion_new (1.0f, 2.0f, 3.0f, 4.0f);
    g_autoptr(GrlQuaternion) q3 = grl_quaternion_new (5.0f, 6.0f, 7.0f, 8.0f);

    g_assert_true (grl_quaternion_equal (q1, q2));
    g_assert_false (grl_quaternion_equal (q1, q3));
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
    g_test_add_func ("/quaternion/new", test_quaternion_new);
    g_test_add_func ("/quaternion/new-identity", test_quaternion_new_identity);
    g_test_add_func ("/quaternion/copy", test_quaternion_copy);
    g_test_add_func ("/quaternion/copy-null", test_quaternion_copy_null);
    g_test_add_func ("/quaternion/new-from-axis-angle", test_quaternion_new_from_axis_angle);
    g_test_add_func ("/quaternion/new-from-euler", test_quaternion_new_from_euler);

    /* Arithmetic */
    g_test_add_func ("/quaternion/add", test_quaternion_add);
    g_test_add_func ("/quaternion/subtract", test_quaternion_subtract);
    g_test_add_func ("/quaternion/scale", test_quaternion_scale);
    g_test_add_func ("/quaternion/negate", test_quaternion_negate);

    /* Operations */
    g_test_add_func ("/quaternion/length", test_quaternion_length);
    g_test_add_func ("/quaternion/normalize", test_quaternion_normalize);
    g_test_add_func ("/quaternion/multiply-identity", test_quaternion_multiply_identity);
    g_test_add_func ("/quaternion/invert", test_quaternion_invert);

    /* Interpolation */
    g_test_add_func ("/quaternion/lerp-endpoints", test_quaternion_lerp_endpoints);
    g_test_add_func ("/quaternion/nlerp", test_quaternion_nlerp);
    g_test_add_func ("/quaternion/slerp", test_quaternion_slerp);
    g_test_add_func ("/quaternion/slerp-endpoints", test_quaternion_slerp_endpoints);

    /* Conversions */
    g_test_add_func ("/quaternion/to-euler-identity", test_quaternion_to_euler_identity);
    g_test_add_func ("/quaternion/euler-roundtrip", test_quaternion_euler_roundtrip);
    g_test_add_func ("/quaternion/to-matrix", test_quaternion_to_matrix);

    /* Comparison */
    g_test_add_func ("/quaternion/equal", test_quaternion_equal);

    return g_test_run ();
}
