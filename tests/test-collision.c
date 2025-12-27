/* test-collision.c
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Unit tests for collision detection functions.
 */

#include <glib.h>
#include <math.h>
#include "src/collision/grl-collision.h"
#include "src/math/grl-vector2.h"
#include "src/math/grl-vector3.h"
#include "src/math/grl-rectangle.h"
#include "src/math/grl-bounding-box.h"

#define EPSILON 0.0001f

static gboolean
float_equal (gfloat a, gfloat b)
{
    return fabsf (a - b) < EPSILON;
}

/*
 * =============================================================================
 * 2D Point Tests
 * =============================================================================
 */

static void
test_collision_point_rect_inside (void)
{
    g_autoptr(GrlVector2) point = grl_vector2_new (5.0f, 5.0f);
    g_autoptr(GrlRectangle) rect = grl_rectangle_new (0.0f, 0.0f, 10.0f, 10.0f);

    g_assert_true (grl_collision_point_rect (point, rect));
}

static void
test_collision_point_rect_outside (void)
{
    g_autoptr(GrlVector2) point = grl_vector2_new (15.0f, 5.0f);
    g_autoptr(GrlRectangle) rect = grl_rectangle_new (0.0f, 0.0f, 10.0f, 10.0f);

    g_assert_false (grl_collision_point_rect (point, rect));
}

static void
test_collision_point_rect_edge (void)
{
    /* Test point on the origin corner (top-left) - should be inside */
    g_autoptr(GrlVector2) point = grl_vector2_new (0.0f, 0.0f);
    g_autoptr(GrlRectangle) rect = grl_rectangle_new (0.0f, 0.0f, 10.0f, 10.0f);

    g_assert_true (grl_collision_point_rect (point, rect));
}

static void
test_collision_point_rect_exclusive_edge (void)
{
    /* Right/bottom edges use exclusive bounds in raylib */
    g_autoptr(GrlVector2) point = grl_vector2_new (10.0f, 10.0f);
    g_autoptr(GrlRectangle) rect = grl_rectangle_new (0.0f, 0.0f, 10.0f, 10.0f);

    g_assert_false (grl_collision_point_rect (point, rect));
}

static void
test_collision_point_circle_inside (void)
{
    g_autoptr(GrlVector2) point = grl_vector2_new (1.0f, 1.0f);
    g_autoptr(GrlVector2) center = grl_vector2_new (0.0f, 0.0f);

    g_assert_true (grl_collision_point_circle (point, center, 5.0f));
}

static void
test_collision_point_circle_outside (void)
{
    g_autoptr(GrlVector2) point = grl_vector2_new (10.0f, 0.0f);
    g_autoptr(GrlVector2) center = grl_vector2_new (0.0f, 0.0f);

    g_assert_false (grl_collision_point_circle (point, center, 5.0f));
}

static void
test_collision_point_circle_on_edge (void)
{
    g_autoptr(GrlVector2) point = grl_vector2_new (5.0f, 0.0f);
    g_autoptr(GrlVector2) center = grl_vector2_new (0.0f, 0.0f);

    g_assert_true (grl_collision_point_circle (point, center, 5.0f));
}

static void
test_collision_point_triangle_inside (void)
{
    g_autoptr(GrlVector2) point = grl_vector2_new (0.5f, 0.5f);
    g_autoptr(GrlVector2) p1 = grl_vector2_new (0.0f, 0.0f);
    g_autoptr(GrlVector2) p2 = grl_vector2_new (2.0f, 0.0f);
    g_autoptr(GrlVector2) p3 = grl_vector2_new (1.0f, 2.0f);

    g_assert_true (grl_collision_point_triangle (point, p1, p2, p3));
}

static void
test_collision_point_triangle_outside (void)
{
    g_autoptr(GrlVector2) point = grl_vector2_new (-1.0f, -1.0f);
    g_autoptr(GrlVector2) p1 = grl_vector2_new (0.0f, 0.0f);
    g_autoptr(GrlVector2) p2 = grl_vector2_new (2.0f, 0.0f);
    g_autoptr(GrlVector2) p3 = grl_vector2_new (1.0f, 2.0f);

    g_assert_false (grl_collision_point_triangle (point, p1, p2, p3));
}

/*
 * =============================================================================
 * 2D Shape vs Shape Tests
 * =============================================================================
 */

static void
test_collision_rects_overlap (void)
{
    g_autoptr(GrlRectangle) rect1 = grl_rectangle_new (0.0f, 0.0f, 10.0f, 10.0f);
    g_autoptr(GrlRectangle) rect2 = grl_rectangle_new (5.0f, 5.0f, 10.0f, 10.0f);

    g_assert_true (grl_collision_rects (rect1, rect2));
}

static void
test_collision_rects_no_overlap (void)
{
    g_autoptr(GrlRectangle) rect1 = grl_rectangle_new (0.0f, 0.0f, 5.0f, 5.0f);
    g_autoptr(GrlRectangle) rect2 = grl_rectangle_new (10.0f, 10.0f, 5.0f, 5.0f);

    g_assert_false (grl_collision_rects (rect1, rect2));
}

static void
test_collision_rects_touching (void)
{
    /* Rectangles that share an edge but don't overlap */
    g_autoptr(GrlRectangle) rect1 = grl_rectangle_new (0.0f, 0.0f, 5.0f, 5.0f);
    g_autoptr(GrlRectangle) rect2 = grl_rectangle_new (5.0f, 0.0f, 5.0f, 5.0f);

    /* In raylib, touching edges do NOT count as overlapping */
    g_assert_false (grl_collision_rects (rect1, rect2));
}

static void
test_collision_circles_overlap (void)
{
    g_autoptr(GrlVector2) c1 = grl_vector2_new (0.0f, 0.0f);
    g_autoptr(GrlVector2) c2 = grl_vector2_new (5.0f, 0.0f);

    /* Two circles with radius 3 that are 5 units apart should overlap */
    g_assert_true (grl_collision_circles (c1, 3.0f, c2, 3.0f));
}

static void
test_collision_circles_no_overlap (void)
{
    g_autoptr(GrlVector2) c1 = grl_vector2_new (0.0f, 0.0f);
    g_autoptr(GrlVector2) c2 = grl_vector2_new (10.0f, 0.0f);

    /* Two circles with radius 3 that are 10 units apart should not overlap */
    g_assert_false (grl_collision_circles (c1, 3.0f, c2, 3.0f));
}

static void
test_collision_circles_concentric (void)
{
    g_autoptr(GrlVector2) c1 = grl_vector2_new (0.0f, 0.0f);
    g_autoptr(GrlVector2) c2 = grl_vector2_new (0.0f, 0.0f);

    /* Concentric circles always overlap */
    g_assert_true (grl_collision_circles (c1, 5.0f, c2, 3.0f));
}

static void
test_collision_circle_rect_overlap (void)
{
    g_autoptr(GrlVector2) center = grl_vector2_new (5.0f, 5.0f);
    g_autoptr(GrlRectangle) rect = grl_rectangle_new (0.0f, 0.0f, 10.0f, 10.0f);

    g_assert_true (grl_collision_circle_rect (center, 3.0f, rect));
}

static void
test_collision_circle_rect_no_overlap (void)
{
    g_autoptr(GrlVector2) center = grl_vector2_new (20.0f, 20.0f);
    g_autoptr(GrlRectangle) rect = grl_rectangle_new (0.0f, 0.0f, 10.0f, 10.0f);

    g_assert_false (grl_collision_circle_rect (center, 3.0f, rect));
}

static void
test_collision_get_rect_overlap (void)
{
    g_autoptr(GrlRectangle) rect1 = grl_rectangle_new (0.0f, 0.0f, 10.0f, 10.0f);
    g_autoptr(GrlRectangle) rect2 = grl_rectangle_new (5.0f, 5.0f, 10.0f, 10.0f);
    g_autoptr(GrlRectangle) overlap = grl_collision_get_rect_overlap (rect1, rect2);

    g_assert_nonnull (overlap);
    g_assert_true (float_equal (overlap->x, 5.0f));
    g_assert_true (float_equal (overlap->y, 5.0f));
    g_assert_true (float_equal (overlap->width, 5.0f));
    g_assert_true (float_equal (overlap->height, 5.0f));
}

static void
test_collision_get_rect_overlap_null (void)
{
    g_autoptr(GrlRectangle) rect1 = grl_rectangle_new (0.0f, 0.0f, 5.0f, 5.0f);
    g_autoptr(GrlRectangle) rect2 = grl_rectangle_new (10.0f, 10.0f, 5.0f, 5.0f);
    GrlRectangle *overlap = grl_collision_get_rect_overlap (rect1, rect2);

    g_assert_null (overlap);
}

/*
 * =============================================================================
 * 3D Point Tests
 * =============================================================================
 */

static void
test_collision_point_sphere_inside (void)
{
    g_autoptr(GrlVector3) point = grl_vector3_new (1.0f, 1.0f, 1.0f);
    g_autoptr(GrlVector3) center = grl_vector3_new (0.0f, 0.0f, 0.0f);

    g_assert_true (grl_collision_point_sphere (point, center, 5.0f));
}

static void
test_collision_point_sphere_outside (void)
{
    g_autoptr(GrlVector3) point = grl_vector3_new (10.0f, 0.0f, 0.0f);
    g_autoptr(GrlVector3) center = grl_vector3_new (0.0f, 0.0f, 0.0f);

    g_assert_false (grl_collision_point_sphere (point, center, 5.0f));
}

static void
test_collision_point_box_inside (void)
{
    g_autoptr(GrlVector3) point = grl_vector3_new (0.0f, 0.0f, 0.0f);
    g_autoptr(GrlBoundingBox) box = grl_bounding_box_new_xyz (-5.0f, -5.0f, -5.0f,
                                                               5.0f, 5.0f, 5.0f);

    g_assert_true (grl_collision_point_box (point, box));
}

static void
test_collision_point_box_outside (void)
{
    g_autoptr(GrlVector3) point = grl_vector3_new (10.0f, 10.0f, 10.0f);
    g_autoptr(GrlBoundingBox) box = grl_bounding_box_new_xyz (-5.0f, -5.0f, -5.0f,
                                                               5.0f, 5.0f, 5.0f);

    g_assert_false (grl_collision_point_box (point, box));
}

/*
 * =============================================================================
 * 3D Shape vs Shape Tests
 * =============================================================================
 */

static void
test_collision_spheres_overlap (void)
{
    g_autoptr(GrlVector3) c1 = grl_vector3_new (0.0f, 0.0f, 0.0f);
    g_autoptr(GrlVector3) c2 = grl_vector3_new (5.0f, 0.0f, 0.0f);

    g_assert_true (grl_collision_spheres (c1, 3.0f, c2, 3.0f));
}

static void
test_collision_spheres_no_overlap (void)
{
    g_autoptr(GrlVector3) c1 = grl_vector3_new (0.0f, 0.0f, 0.0f);
    g_autoptr(GrlVector3) c2 = grl_vector3_new (10.0f, 0.0f, 0.0f);

    g_assert_false (grl_collision_spheres (c1, 3.0f, c2, 3.0f));
}

static void
test_collision_boxes_overlap (void)
{
    g_autoptr(GrlBoundingBox) box1 = grl_bounding_box_new_xyz (0.0f, 0.0f, 0.0f,
                                                                5.0f, 5.0f, 5.0f);
    g_autoptr(GrlBoundingBox) box2 = grl_bounding_box_new_xyz (3.0f, 3.0f, 3.0f,
                                                                8.0f, 8.0f, 8.0f);

    g_assert_true (grl_collision_boxes (box1, box2));
}

static void
test_collision_boxes_no_overlap (void)
{
    g_autoptr(GrlBoundingBox) box1 = grl_bounding_box_new_xyz (0.0f, 0.0f, 0.0f,
                                                                5.0f, 5.0f, 5.0f);
    g_autoptr(GrlBoundingBox) box2 = grl_bounding_box_new_xyz (10.0f, 10.0f, 10.0f,
                                                                15.0f, 15.0f, 15.0f);

    g_assert_false (grl_collision_boxes (box1, box2));
}

static void
test_collision_box_sphere_overlap (void)
{
    g_autoptr(GrlBoundingBox) box = grl_bounding_box_new_xyz (0.0f, 0.0f, 0.0f,
                                                               5.0f, 5.0f, 5.0f);
    g_autoptr(GrlVector3) center = grl_vector3_new (2.5f, 2.5f, 2.5f);

    g_assert_true (grl_collision_box_sphere (box, center, 2.0f));
}

static void
test_collision_box_sphere_no_overlap (void)
{
    g_autoptr(GrlBoundingBox) box = grl_bounding_box_new_xyz (0.0f, 0.0f, 0.0f,
                                                               5.0f, 5.0f, 5.0f);
    g_autoptr(GrlVector3) center = grl_vector3_new (20.0f, 20.0f, 20.0f);

    g_assert_false (grl_collision_box_sphere (box, center, 2.0f));
}

/*
 * =============================================================================
 * Ray Tests
 * =============================================================================
 */

static void
test_ray_new (void)
{
    g_autoptr(GrlVector3) pos = grl_vector3_new (0.0f, 0.0f, 0.0f);
    g_autoptr(GrlVector3) dir = grl_vector3_new (0.0f, 0.0f, -1.0f);
    GrlRay *ray = grl_ray_new (pos, dir);

    g_assert_nonnull (ray);
    g_assert_true (float_equal (ray->position.x, 0.0f));
    g_assert_true (float_equal (ray->direction.z, -1.0f));

    grl_ray_free (ray);
}

static void
test_ray_copy (void)
{
    g_autoptr(GrlVector3) pos = grl_vector3_new (1.0f, 2.0f, 3.0f);
    g_autoptr(GrlVector3) dir = grl_vector3_new (0.0f, 1.0f, 0.0f);
    GrlRay *ray1 = grl_ray_new (pos, dir);
    GrlRay *ray2 = grl_ray_copy (ray1);

    g_assert_nonnull (ray2);
    g_assert_true (ray1 != ray2);
    g_assert_true (float_equal (ray2->position.x, 1.0f));

    grl_ray_free (ray1);
    grl_ray_free (ray2);
}

static void
test_collision_ray_sphere_hit (void)
{
    g_autoptr(GrlVector3) pos = grl_vector3_new (0.0f, 0.0f, 10.0f);
    g_autoptr(GrlVector3) dir = grl_vector3_new (0.0f, 0.0f, -1.0f);
    g_autoptr(GrlVector3) center = grl_vector3_new (0.0f, 0.0f, 0.0f);
    GrlRay *ray = grl_ray_new (pos, dir);
    GrlRayCollision *collision = grl_collision_ray_sphere (ray, center, 2.0f);

    g_assert_nonnull (collision);
    g_assert_true (collision->hit);
    g_assert_true (float_equal (collision->distance, 8.0f));  /* 10 - 2 = 8 */

    grl_ray_free (ray);
    grl_ray_collision_free (collision);
}

static void
test_collision_ray_sphere_miss (void)
{
    g_autoptr(GrlVector3) pos = grl_vector3_new (0.0f, 10.0f, 10.0f);
    g_autoptr(GrlVector3) dir = grl_vector3_new (0.0f, 0.0f, -1.0f);
    g_autoptr(GrlVector3) center = grl_vector3_new (0.0f, 0.0f, 0.0f);
    GrlRay *ray = grl_ray_new (pos, dir);
    GrlRayCollision *collision = grl_collision_ray_sphere (ray, center, 2.0f);

    g_assert_nonnull (collision);
    g_assert_false (collision->hit);

    grl_ray_free (ray);
    grl_ray_collision_free (collision);
}

static void
test_collision_ray_box_hit (void)
{
    g_autoptr(GrlVector3) pos = grl_vector3_new (0.0f, 0.0f, 10.0f);
    g_autoptr(GrlVector3) dir = grl_vector3_new (0.0f, 0.0f, -1.0f);
    g_autoptr(GrlBoundingBox) box = grl_bounding_box_new_xyz (-2.0f, -2.0f, -2.0f,
                                                               2.0f, 2.0f, 2.0f);
    GrlRay *ray = grl_ray_new (pos, dir);
    GrlRayCollision *collision = grl_collision_ray_box (ray, box);

    g_assert_nonnull (collision);
    g_assert_true (collision->hit);

    grl_ray_free (ray);
    grl_ray_collision_free (collision);
}

static void
test_collision_ray_box_miss (void)
{
    g_autoptr(GrlVector3) pos = grl_vector3_new (10.0f, 0.0f, 10.0f);
    g_autoptr(GrlVector3) dir = grl_vector3_new (0.0f, 0.0f, -1.0f);
    g_autoptr(GrlBoundingBox) box = grl_bounding_box_new_xyz (-2.0f, -2.0f, -2.0f,
                                                               2.0f, 2.0f, 2.0f);
    GrlRay *ray = grl_ray_new (pos, dir);
    GrlRayCollision *collision = grl_collision_ray_box (ray, box);

    g_assert_nonnull (collision);
    g_assert_false (collision->hit);

    grl_ray_free (ray);
    grl_ray_collision_free (collision);
}

/*
 * Test registration
 */

int
main (int   argc,
      char *argv[])
{
    g_test_init (&argc, &argv, NULL);

    /* 2D Point tests */
    g_test_add_func ("/collision/point-rect-inside", test_collision_point_rect_inside);
    g_test_add_func ("/collision/point-rect-outside", test_collision_point_rect_outside);
    g_test_add_func ("/collision/point-rect-edge", test_collision_point_rect_edge);
    g_test_add_func ("/collision/point-rect-exclusive-edge", test_collision_point_rect_exclusive_edge);
    g_test_add_func ("/collision/point-circle-inside", test_collision_point_circle_inside);
    g_test_add_func ("/collision/point-circle-outside", test_collision_point_circle_outside);
    g_test_add_func ("/collision/point-circle-on-edge", test_collision_point_circle_on_edge);
    g_test_add_func ("/collision/point-triangle-inside", test_collision_point_triangle_inside);
    g_test_add_func ("/collision/point-triangle-outside", test_collision_point_triangle_outside);

    /* 2D Shape vs Shape tests */
    g_test_add_func ("/collision/rects-overlap", test_collision_rects_overlap);
    g_test_add_func ("/collision/rects-no-overlap", test_collision_rects_no_overlap);
    g_test_add_func ("/collision/rects-touching", test_collision_rects_touching);
    g_test_add_func ("/collision/circles-overlap", test_collision_circles_overlap);
    g_test_add_func ("/collision/circles-no-overlap", test_collision_circles_no_overlap);
    g_test_add_func ("/collision/circles-concentric", test_collision_circles_concentric);
    g_test_add_func ("/collision/circle-rect-overlap", test_collision_circle_rect_overlap);
    g_test_add_func ("/collision/circle-rect-no-overlap", test_collision_circle_rect_no_overlap);
    g_test_add_func ("/collision/get-rect-overlap", test_collision_get_rect_overlap);
    g_test_add_func ("/collision/get-rect-overlap-null", test_collision_get_rect_overlap_null);

    /* 3D Point tests */
    g_test_add_func ("/collision/point-sphere-inside", test_collision_point_sphere_inside);
    g_test_add_func ("/collision/point-sphere-outside", test_collision_point_sphere_outside);
    g_test_add_func ("/collision/point-box-inside", test_collision_point_box_inside);
    g_test_add_func ("/collision/point-box-outside", test_collision_point_box_outside);

    /* 3D Shape vs Shape tests */
    g_test_add_func ("/collision/spheres-overlap", test_collision_spheres_overlap);
    g_test_add_func ("/collision/spheres-no-overlap", test_collision_spheres_no_overlap);
    g_test_add_func ("/collision/boxes-overlap", test_collision_boxes_overlap);
    g_test_add_func ("/collision/boxes-no-overlap", test_collision_boxes_no_overlap);
    g_test_add_func ("/collision/box-sphere-overlap", test_collision_box_sphere_overlap);
    g_test_add_func ("/collision/box-sphere-no-overlap", test_collision_box_sphere_no_overlap);

    /* Ray tests */
    g_test_add_func ("/collision/ray-new", test_ray_new);
    g_test_add_func ("/collision/ray-copy", test_ray_copy);
    g_test_add_func ("/collision/ray-sphere-hit", test_collision_ray_sphere_hit);
    g_test_add_func ("/collision/ray-sphere-miss", test_collision_ray_sphere_miss);
    g_test_add_func ("/collision/ray-box-hit", test_collision_ray_box_hit);
    g_test_add_func ("/collision/ray-box-miss", test_collision_ray_box_miss);

    return g_test_run ();
}
