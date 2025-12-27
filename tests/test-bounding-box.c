/* test-bounding-box.c
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Unit tests for GrlBoundingBox.
 */

#include <glib.h>
#include <math.h>
#include "src/math/grl-bounding-box.h"
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
test_bounding_box_new (void)
{
    g_autoptr(GrlVector3) min = grl_vector3_new (-1.0f, -2.0f, -3.0f);
    g_autoptr(GrlVector3) max = grl_vector3_new (1.0f, 2.0f, 3.0f);
    g_autoptr(GrlBoundingBox) bb = grl_bounding_box_new (min, max);

    g_assert_nonnull (bb);
    g_assert_true (float_equal (bb->min.x, -1.0f));
    g_assert_true (float_equal (bb->min.y, -2.0f));
    g_assert_true (float_equal (bb->min.z, -3.0f));
    g_assert_true (float_equal (bb->max.x, 1.0f));
    g_assert_true (float_equal (bb->max.y, 2.0f));
    g_assert_true (float_equal (bb->max.z, 3.0f));
}

static void
test_bounding_box_new_xyz (void)
{
    g_autoptr(GrlBoundingBox) bb = grl_bounding_box_new_xyz (0.0f, 0.0f, 0.0f,
                                                              10.0f, 20.0f, 30.0f);

    g_assert_nonnull (bb);
    g_assert_true (float_equal (bb->min.x, 0.0f));
    g_assert_true (float_equal (bb->min.y, 0.0f));
    g_assert_true (float_equal (bb->min.z, 0.0f));
    g_assert_true (float_equal (bb->max.x, 10.0f));
    g_assert_true (float_equal (bb->max.y, 20.0f));
    g_assert_true (float_equal (bb->max.z, 30.0f));
}

static void
test_bounding_box_copy (void)
{
    g_autoptr(GrlBoundingBox) bb1 = grl_bounding_box_new_xyz (-5.0f, -5.0f, -5.0f,
                                                               5.0f, 5.0f, 5.0f);
    g_autoptr(GrlBoundingBox) bb2 = grl_bounding_box_copy (bb1);

    g_assert_nonnull (bb2);
    g_assert_true (bb1 != bb2);
    g_assert_true (float_equal (bb2->min.x, -5.0f));
    g_assert_true (float_equal (bb2->max.x, 5.0f));
}

static void
test_bounding_box_copy_null (void)
{
    GrlBoundingBox *bb = grl_bounding_box_copy (NULL);

    g_assert_null (bb);
}

/*
 * Test accessors
 */

static void
test_bounding_box_get_min (void)
{
    g_autoptr(GrlBoundingBox) bb = grl_bounding_box_new_xyz (-1.0f, -2.0f, -3.0f,
                                                              1.0f, 2.0f, 3.0f);
    g_autoptr(GrlVector3) min = grl_bounding_box_get_min (bb);

    g_assert_nonnull (min);
    g_assert_true (float_equal (min->x, -1.0f));
    g_assert_true (float_equal (min->y, -2.0f));
    g_assert_true (float_equal (min->z, -3.0f));
}

static void
test_bounding_box_get_max (void)
{
    g_autoptr(GrlBoundingBox) bb = grl_bounding_box_new_xyz (-1.0f, -2.0f, -3.0f,
                                                              1.0f, 2.0f, 3.0f);
    g_autoptr(GrlVector3) max = grl_bounding_box_get_max (bb);

    g_assert_nonnull (max);
    g_assert_true (float_equal (max->x, 1.0f));
    g_assert_true (float_equal (max->y, 2.0f));
    g_assert_true (float_equal (max->z, 3.0f));
}

static void
test_bounding_box_get_center (void)
{
    g_autoptr(GrlBoundingBox) bb = grl_bounding_box_new_xyz (-2.0f, -4.0f, -6.0f,
                                                              2.0f, 4.0f, 6.0f);
    g_autoptr(GrlVector3) center = grl_bounding_box_get_center (bb);

    g_assert_nonnull (center);
    g_assert_true (float_equal (center->x, 0.0f));
    g_assert_true (float_equal (center->y, 0.0f));
    g_assert_true (float_equal (center->z, 0.0f));
}

static void
test_bounding_box_get_center_offset (void)
{
    g_autoptr(GrlBoundingBox) bb = grl_bounding_box_new_xyz (0.0f, 0.0f, 0.0f,
                                                              10.0f, 20.0f, 30.0f);
    g_autoptr(GrlVector3) center = grl_bounding_box_get_center (bb);

    g_assert_nonnull (center);
    g_assert_true (float_equal (center->x, 5.0f));
    g_assert_true (float_equal (center->y, 10.0f));
    g_assert_true (float_equal (center->z, 15.0f));
}

static void
test_bounding_box_get_size (void)
{
    g_autoptr(GrlBoundingBox) bb = grl_bounding_box_new_xyz (-1.0f, -2.0f, -3.0f,
                                                              1.0f, 2.0f, 3.0f);
    g_autoptr(GrlVector3) size = grl_bounding_box_get_size (bb);

    g_assert_nonnull (size);
    g_assert_true (float_equal (size->x, 2.0f));
    g_assert_true (float_equal (size->y, 4.0f));
    g_assert_true (float_equal (size->z, 6.0f));
}

/*
 * Test queries
 */

static void
test_bounding_box_contains_point_inside (void)
{
    g_autoptr(GrlBoundingBox) bb = grl_bounding_box_new_xyz (-5.0f, -5.0f, -5.0f,
                                                              5.0f, 5.0f, 5.0f);
    g_autoptr(GrlVector3) point = grl_vector3_new (0.0f, 0.0f, 0.0f);

    g_assert_true (grl_bounding_box_contains_point (bb, point));
}

static void
test_bounding_box_contains_point_outside (void)
{
    g_autoptr(GrlBoundingBox) bb = grl_bounding_box_new_xyz (-5.0f, -5.0f, -5.0f,
                                                              5.0f, 5.0f, 5.0f);
    g_autoptr(GrlVector3) point = grl_vector3_new (10.0f, 0.0f, 0.0f);

    g_assert_false (grl_bounding_box_contains_point (bb, point));
}

static void
test_bounding_box_contains_point_edge (void)
{
    g_autoptr(GrlBoundingBox) bb = grl_bounding_box_new_xyz (-5.0f, -5.0f, -5.0f,
                                                              5.0f, 5.0f, 5.0f);
    g_autoptr(GrlVector3) point = grl_vector3_new (5.0f, 5.0f, 5.0f);

    /* Points on edge should be considered inside (inclusive bounds) */
    g_assert_true (grl_bounding_box_contains_point (bb, point));
}

static void
test_bounding_box_intersects_true (void)
{
    g_autoptr(GrlBoundingBox) bb1 = grl_bounding_box_new_xyz (-5.0f, -5.0f, -5.0f,
                                                               5.0f, 5.0f, 5.0f);
    g_autoptr(GrlBoundingBox) bb2 = grl_bounding_box_new_xyz (0.0f, 0.0f, 0.0f,
                                                               10.0f, 10.0f, 10.0f);

    g_assert_true (grl_bounding_box_intersects (bb1, bb2));
}

static void
test_bounding_box_intersects_false (void)
{
    g_autoptr(GrlBoundingBox) bb1 = grl_bounding_box_new_xyz (-5.0f, -5.0f, -5.0f,
                                                               0.0f, 0.0f, 0.0f);
    g_autoptr(GrlBoundingBox) bb2 = grl_bounding_box_new_xyz (10.0f, 10.0f, 10.0f,
                                                               20.0f, 20.0f, 20.0f);

    g_assert_false (grl_bounding_box_intersects (bb1, bb2));
}

static void
test_bounding_box_intersects_touching (void)
{
    g_autoptr(GrlBoundingBox) bb1 = grl_bounding_box_new_xyz (0.0f, 0.0f, 0.0f,
                                                               5.0f, 5.0f, 5.0f);
    g_autoptr(GrlBoundingBox) bb2 = grl_bounding_box_new_xyz (5.0f, 0.0f, 0.0f,
                                                               10.0f, 5.0f, 5.0f);

    /* Touching boxes should intersect */
    g_assert_true (grl_bounding_box_intersects (bb1, bb2));
}

/*
 * Test operations
 */

static void
test_bounding_box_expand (void)
{
    g_autoptr(GrlBoundingBox) bb = grl_bounding_box_new_xyz (0.0f, 0.0f, 0.0f,
                                                              5.0f, 5.0f, 5.0f);
    g_autoptr(GrlVector3) point = grl_vector3_new (10.0f, 10.0f, 10.0f);
    g_autoptr(GrlBoundingBox) expanded = grl_bounding_box_expand (bb, point);

    g_assert_nonnull (expanded);
    g_assert_true (float_equal (expanded->min.x, 0.0f));
    g_assert_true (float_equal (expanded->min.y, 0.0f));
    g_assert_true (float_equal (expanded->min.z, 0.0f));
    g_assert_true (float_equal (expanded->max.x, 10.0f));
    g_assert_true (float_equal (expanded->max.y, 10.0f));
    g_assert_true (float_equal (expanded->max.z, 10.0f));
}

static void
test_bounding_box_expand_negative (void)
{
    g_autoptr(GrlBoundingBox) bb = grl_bounding_box_new_xyz (0.0f, 0.0f, 0.0f,
                                                              5.0f, 5.0f, 5.0f);
    g_autoptr(GrlVector3) point = grl_vector3_new (-5.0f, -5.0f, -5.0f);
    g_autoptr(GrlBoundingBox) expanded = grl_bounding_box_expand (bb, point);

    g_assert_nonnull (expanded);
    g_assert_true (float_equal (expanded->min.x, -5.0f));
    g_assert_true (float_equal (expanded->min.y, -5.0f));
    g_assert_true (float_equal (expanded->min.z, -5.0f));
    g_assert_true (float_equal (expanded->max.x, 5.0f));
    g_assert_true (float_equal (expanded->max.y, 5.0f));
    g_assert_true (float_equal (expanded->max.z, 5.0f));
}

static void
test_bounding_box_merge (void)
{
    g_autoptr(GrlBoundingBox) bb1 = grl_bounding_box_new_xyz (-5.0f, -5.0f, -5.0f,
                                                               0.0f, 0.0f, 0.0f);
    g_autoptr(GrlBoundingBox) bb2 = grl_bounding_box_new_xyz (0.0f, 0.0f, 0.0f,
                                                               5.0f, 5.0f, 5.0f);
    g_autoptr(GrlBoundingBox) merged = grl_bounding_box_merge (bb1, bb2);

    g_assert_nonnull (merged);
    g_assert_true (float_equal (merged->min.x, -5.0f));
    g_assert_true (float_equal (merged->min.y, -5.0f));
    g_assert_true (float_equal (merged->min.z, -5.0f));
    g_assert_true (float_equal (merged->max.x, 5.0f));
    g_assert_true (float_equal (merged->max.y, 5.0f));
    g_assert_true (float_equal (merged->max.z, 5.0f));
}

static void
test_bounding_box_merge_disjoint (void)
{
    g_autoptr(GrlBoundingBox) bb1 = grl_bounding_box_new_xyz (0.0f, 0.0f, 0.0f,
                                                               2.0f, 2.0f, 2.0f);
    g_autoptr(GrlBoundingBox) bb2 = grl_bounding_box_new_xyz (10.0f, 10.0f, 10.0f,
                                                               12.0f, 12.0f, 12.0f);
    g_autoptr(GrlBoundingBox) merged = grl_bounding_box_merge (bb1, bb2);

    g_assert_nonnull (merged);
    g_assert_true (float_equal (merged->min.x, 0.0f));
    g_assert_true (float_equal (merged->min.y, 0.0f));
    g_assert_true (float_equal (merged->min.z, 0.0f));
    g_assert_true (float_equal (merged->max.x, 12.0f));
    g_assert_true (float_equal (merged->max.y, 12.0f));
    g_assert_true (float_equal (merged->max.z, 12.0f));
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
    g_test_add_func ("/bounding-box/new", test_bounding_box_new);
    g_test_add_func ("/bounding-box/new-xyz", test_bounding_box_new_xyz);
    g_test_add_func ("/bounding-box/copy", test_bounding_box_copy);
    g_test_add_func ("/bounding-box/copy-null", test_bounding_box_copy_null);

    /* Accessors */
    g_test_add_func ("/bounding-box/get-min", test_bounding_box_get_min);
    g_test_add_func ("/bounding-box/get-max", test_bounding_box_get_max);
    g_test_add_func ("/bounding-box/get-center", test_bounding_box_get_center);
    g_test_add_func ("/bounding-box/get-center-offset", test_bounding_box_get_center_offset);
    g_test_add_func ("/bounding-box/get-size", test_bounding_box_get_size);

    /* Queries */
    g_test_add_func ("/bounding-box/contains-point-inside", test_bounding_box_contains_point_inside);
    g_test_add_func ("/bounding-box/contains-point-outside", test_bounding_box_contains_point_outside);
    g_test_add_func ("/bounding-box/contains-point-edge", test_bounding_box_contains_point_edge);
    g_test_add_func ("/bounding-box/intersects-true", test_bounding_box_intersects_true);
    g_test_add_func ("/bounding-box/intersects-false", test_bounding_box_intersects_false);
    g_test_add_func ("/bounding-box/intersects-touching", test_bounding_box_intersects_touching);

    /* Operations */
    g_test_add_func ("/bounding-box/expand", test_bounding_box_expand);
    g_test_add_func ("/bounding-box/expand-negative", test_bounding_box_expand_negative);
    g_test_add_func ("/bounding-box/merge", test_bounding_box_merge);
    g_test_add_func ("/bounding-box/merge-disjoint", test_bounding_box_merge_disjoint);

    return g_test_run ();
}
