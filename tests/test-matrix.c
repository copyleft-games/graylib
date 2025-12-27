/* test-matrix.c
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Unit tests for GrlMatrix.
 */

#include <glib.h>
#include <math.h>
#include "src/math/grl-matrix.h"
#include "src/math/grl-vector3.h"

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
test_matrix_new (void)
{
    g_autoptr(GrlMatrix) m = grl_matrix_new ();

    g_assert_nonnull (m);
}

static void
test_matrix_copy (void)
{
    g_autoptr(GrlMatrix) m1 = grl_matrix_new_identity ();
    g_autoptr(GrlMatrix) m2 = grl_matrix_copy (m1);

    g_assert_nonnull (m2);
    g_assert_true (m1 != m2);
    g_assert_true (float_equal (m2->m0, 1.0f));
    g_assert_true (float_equal (m2->m5, 1.0f));
    g_assert_true (float_equal (m2->m10, 1.0f));
    g_assert_true (float_equal (m2->m15, 1.0f));
}

static void
test_matrix_copy_null (void)
{
    GrlMatrix *m = grl_matrix_copy (NULL);

    g_assert_null (m);
}

static void
test_matrix_new_identity (void)
{
    g_autoptr(GrlMatrix) m = grl_matrix_new_identity ();

    g_assert_nonnull (m);

    /* Diagonal elements should be 1 */
    g_assert_true (float_equal (m->m0, 1.0f));
    g_assert_true (float_equal (m->m5, 1.0f));
    g_assert_true (float_equal (m->m10, 1.0f));
    g_assert_true (float_equal (m->m15, 1.0f));

    /* Off-diagonal elements should be 0 */
    g_assert_true (float_equal (m->m1, 0.0f));
    g_assert_true (float_equal (m->m2, 0.0f));
    g_assert_true (float_equal (m->m3, 0.0f));
    g_assert_true (float_equal (m->m4, 0.0f));
    g_assert_true (float_equal (m->m6, 0.0f));
    g_assert_true (float_equal (m->m7, 0.0f));
    g_assert_true (float_equal (m->m8, 0.0f));
    g_assert_true (float_equal (m->m9, 0.0f));
    g_assert_true (float_equal (m->m11, 0.0f));
    g_assert_true (float_equal (m->m12, 0.0f));
    g_assert_true (float_equal (m->m13, 0.0f));
    g_assert_true (float_equal (m->m14, 0.0f));
}

static void
test_matrix_new_translate (void)
{
    g_autoptr(GrlMatrix) m = grl_matrix_new_translate (10.0f, 20.0f, 30.0f);

    g_assert_nonnull (m);

    /* Translation components are in m12, m13, m14 (column-major order) */
    g_assert_true (float_equal (m->m12, 10.0f));
    g_assert_true (float_equal (m->m13, 20.0f));
    g_assert_true (float_equal (m->m14, 30.0f));

    /* Diagonal should still be 1 */
    g_assert_true (float_equal (m->m0, 1.0f));
    g_assert_true (float_equal (m->m5, 1.0f));
    g_assert_true (float_equal (m->m10, 1.0f));
    g_assert_true (float_equal (m->m15, 1.0f));
}

static void
test_matrix_new_scale (void)
{
    g_autoptr(GrlMatrix) m = grl_matrix_new_scale (2.0f, 3.0f, 4.0f);

    g_assert_nonnull (m);

    /* Scale components on diagonal */
    g_assert_true (float_equal (m->m0, 2.0f));
    g_assert_true (float_equal (m->m5, 3.0f));
    g_assert_true (float_equal (m->m10, 4.0f));
    g_assert_true (float_equal (m->m15, 1.0f));
}

static void
test_matrix_new_rotate (void)
{
    /* Rotate 90 degrees around Z axis */
    g_autoptr(GrlVector3) axis = grl_vector3_new (0.0f, 0.0f, 1.0f);
    g_autoptr(GrlMatrix) m = grl_matrix_new_rotate (axis, PI / 2.0f);

    g_assert_nonnull (m);

    /* After 90 degree rotation around Z:
     * X axis (1,0,0) becomes (0,1,0)
     * Y axis (0,1,0) becomes (-1,0,0)
     */
    g_assert_true (float_equal (m->m0, 0.0f));   /* cos(90) */
    g_assert_true (float_equal (m->m1, 1.0f));   /* sin(90) */
    g_assert_true (float_equal (m->m4, -1.0f));  /* -sin(90) */
    g_assert_true (float_equal (m->m5, 0.0f));   /* cos(90) */
}

static void
test_matrix_new_rotate_xyz (void)
{
    /* Rotate 90 degrees around each axis */
    g_autoptr(GrlMatrix) m = grl_matrix_new_rotate_xyz (PI / 2.0f, 0.0f, 0.0f);

    g_assert_nonnull (m);

    /* After 90 degree X rotation:
     * Y axis becomes Z axis
     * Z axis becomes -Y axis
     */
    g_assert_true (float_equal (m->m0, 1.0f));
}

/*
 * Test operations
 */

static void
test_matrix_multiply_identity (void)
{
    g_autoptr(GrlMatrix) ident = grl_matrix_new_identity ();
    g_autoptr(GrlMatrix) trans = grl_matrix_new_translate (5.0f, 10.0f, 15.0f);
    g_autoptr(GrlMatrix) result = grl_matrix_multiply (ident, trans);

    g_assert_nonnull (result);

    /* Identity * M = M */
    g_assert_true (float_equal (result->m12, 5.0f));
    g_assert_true (float_equal (result->m13, 10.0f));
    g_assert_true (float_equal (result->m14, 15.0f));
}

static void
test_matrix_multiply_scale_translate (void)
{
    /* Multiply scale and translate matrices */
    g_autoptr(GrlMatrix) scale = grl_matrix_new_scale (2.0f, 2.0f, 2.0f);
    g_autoptr(GrlMatrix) trans = grl_matrix_new_translate (5.0f, 10.0f, 15.0f);
    g_autoptr(GrlMatrix) result = grl_matrix_multiply (scale, trans);

    g_assert_nonnull (result);

    /* Scale should be preserved on diagonal */
    g_assert_true (float_equal (result->m0, 2.0f));
    g_assert_true (float_equal (result->m5, 2.0f));
    g_assert_true (float_equal (result->m10, 2.0f));
    g_assert_true (float_equal (result->m15, 1.0f));
}

static void
test_matrix_transpose (void)
{
    g_autoptr(GrlMatrix) m = grl_matrix_new_translate (1.0f, 2.0f, 3.0f);
    g_autoptr(GrlMatrix) t = grl_matrix_transpose (m);

    g_assert_nonnull (t);

    /* In a translation matrix, m12=1, m13=2, m14=3
     * After transpose, these become m3, m7, m11 */
    g_assert_true (float_equal (t->m3, 1.0f));
    g_assert_true (float_equal (t->m7, 2.0f));
    g_assert_true (float_equal (t->m11, 3.0f));
}

static void
test_matrix_transpose_identity (void)
{
    g_autoptr(GrlMatrix) ident = grl_matrix_new_identity ();
    g_autoptr(GrlMatrix) t = grl_matrix_transpose (ident);

    g_assert_nonnull (t);

    /* Transpose of identity is identity */
    g_assert_true (float_equal (t->m0, 1.0f));
    g_assert_true (float_equal (t->m5, 1.0f));
    g_assert_true (float_equal (t->m10, 1.0f));
    g_assert_true (float_equal (t->m15, 1.0f));
}

static void
test_matrix_determinant_identity (void)
{
    g_autoptr(GrlMatrix) ident = grl_matrix_new_identity ();
    gfloat det = grl_matrix_determinant (ident);

    /* Determinant of identity matrix is 1 */
    g_assert_true (float_equal (det, 1.0f));
}

static void
test_matrix_determinant_scale (void)
{
    g_autoptr(GrlMatrix) scale = grl_matrix_new_scale (2.0f, 3.0f, 4.0f);
    gfloat det = grl_matrix_determinant (scale);

    /* Determinant of scale matrix is product of scale factors */
    g_assert_true (float_equal (det, 24.0f));
}

static void
test_matrix_trace_identity (void)
{
    g_autoptr(GrlMatrix) ident = grl_matrix_new_identity ();
    gfloat trace = grl_matrix_trace (ident);

    /* Trace of identity is 4 (sum of 1+1+1+1) */
    g_assert_true (float_equal (trace, 4.0f));
}

static void
test_matrix_trace_scale (void)
{
    g_autoptr(GrlMatrix) scale = grl_matrix_new_scale (2.0f, 3.0f, 4.0f);
    gfloat trace = grl_matrix_trace (scale);

    /* Trace is 2 + 3 + 4 + 1 = 10 */
    g_assert_true (float_equal (trace, 10.0f));
}

static void
test_matrix_invert_identity (void)
{
    g_autoptr(GrlMatrix) ident = grl_matrix_new_identity ();
    g_autoptr(GrlMatrix) inv = grl_matrix_invert (ident);

    g_assert_nonnull (inv);

    /* Inverse of identity is identity */
    g_assert_true (float_equal (inv->m0, 1.0f));
    g_assert_true (float_equal (inv->m5, 1.0f));
    g_assert_true (float_equal (inv->m10, 1.0f));
    g_assert_true (float_equal (inv->m15, 1.0f));
}

static void
test_matrix_invert_translate (void)
{
    g_autoptr(GrlMatrix) trans = grl_matrix_new_translate (5.0f, 10.0f, 15.0f);
    g_autoptr(GrlMatrix) inv = grl_matrix_invert (trans);

    g_assert_nonnull (inv);

    /* Inverse of translate(5,10,15) is translate(-5,-10,-15) */
    g_assert_true (float_equal (inv->m12, -5.0f));
    g_assert_true (float_equal (inv->m13, -10.0f));
    g_assert_true (float_equal (inv->m14, -15.0f));
}

static void
test_matrix_invert_multiply_identity (void)
{
    g_autoptr(GrlMatrix) trans = grl_matrix_new_translate (5.0f, 10.0f, 15.0f);
    g_autoptr(GrlMatrix) inv = grl_matrix_invert (trans);
    g_autoptr(GrlMatrix) result = grl_matrix_multiply (trans, inv);

    g_assert_nonnull (result);

    /* M * M^-1 should be identity */
    g_assert_true (float_equal (result->m0, 1.0f));
    g_assert_true (float_equal (result->m5, 1.0f));
    g_assert_true (float_equal (result->m10, 1.0f));
    g_assert_true (float_equal (result->m15, 1.0f));
    g_assert_true (float_equal (result->m12, 0.0f));
    g_assert_true (float_equal (result->m13, 0.0f));
    g_assert_true (float_equal (result->m14, 0.0f));
}

/*
 * Test projection matrices
 */

static void
test_matrix_new_perspective (void)
{
    g_autoptr(GrlMatrix) m = grl_matrix_new_perspective (PI / 4.0, 16.0 / 9.0, 0.1, 100.0);

    g_assert_nonnull (m);

    /* Perspective matrix should have m15 = 0 and m11 = -1 for typical setup */
    g_assert_true (float_equal (m->m15, 0.0f));
    g_assert_true (float_equal (m->m11, -1.0f));
}

static void
test_matrix_new_ortho (void)
{
    g_autoptr(GrlMatrix) m = grl_matrix_new_ortho (-10.0, 10.0, -10.0, 10.0, 0.1, 100.0);

    g_assert_nonnull (m);

    /* Orthographic matrix should have m15 = 1 */
    g_assert_true (float_equal (m->m15, 1.0f));
}

static void
test_matrix_new_look_at (void)
{
    g_autoptr(GrlVector3) eye = grl_vector3_new (0.0f, 0.0f, 5.0f);
    g_autoptr(GrlVector3) target = grl_vector3_new (0.0f, 0.0f, 0.0f);
    g_autoptr(GrlVector3) up = grl_vector3_new (0.0f, 1.0f, 0.0f);
    g_autoptr(GrlMatrix) m = grl_matrix_new_look_at (eye, target, up);

    g_assert_nonnull (m);

    /* Looking down -Z from (0,0,5) at origin should have translation in m14 */
    g_assert_true (float_equal (m->m14, -5.0f));
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
    g_test_add_func ("/matrix/new", test_matrix_new);
    g_test_add_func ("/matrix/copy", test_matrix_copy);
    g_test_add_func ("/matrix/copy-null", test_matrix_copy_null);
    g_test_add_func ("/matrix/new-identity", test_matrix_new_identity);
    g_test_add_func ("/matrix/new-translate", test_matrix_new_translate);
    g_test_add_func ("/matrix/new-scale", test_matrix_new_scale);
    g_test_add_func ("/matrix/new-rotate", test_matrix_new_rotate);
    g_test_add_func ("/matrix/new-rotate-xyz", test_matrix_new_rotate_xyz);

    /* Operations */
    g_test_add_func ("/matrix/multiply-identity", test_matrix_multiply_identity);
    g_test_add_func ("/matrix/multiply-scale-translate", test_matrix_multiply_scale_translate);
    g_test_add_func ("/matrix/transpose", test_matrix_transpose);
    g_test_add_func ("/matrix/transpose-identity", test_matrix_transpose_identity);
    g_test_add_func ("/matrix/determinant-identity", test_matrix_determinant_identity);
    g_test_add_func ("/matrix/determinant-scale", test_matrix_determinant_scale);
    g_test_add_func ("/matrix/trace-identity", test_matrix_trace_identity);
    g_test_add_func ("/matrix/trace-scale", test_matrix_trace_scale);
    g_test_add_func ("/matrix/invert-identity", test_matrix_invert_identity);
    g_test_add_func ("/matrix/invert-translate", test_matrix_invert_translate);
    g_test_add_func ("/matrix/invert-multiply-identity", test_matrix_invert_multiply_identity);

    /* Projection matrices */
    g_test_add_func ("/matrix/new-perspective", test_matrix_new_perspective);
    g_test_add_func ("/matrix/new-ortho", test_matrix_new_ortho);
    g_test_add_func ("/matrix/new-look-at", test_matrix_new_look_at);

    return g_test_run ();
}
