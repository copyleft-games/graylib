/* grl-collision.h
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Collision detection utility functions for 2D and 3D.
 */

#pragma once

#if !defined(GRAYLIB_INSIDE) && !defined(GRAYLIB_COMPILATION)
#error "Only <graylib.h> can be included directly."
#endif

#include <glib.h>
#include "../grl-version.h"
#include "../math/grl-vector2.h"
#include "../math/grl-vector3.h"
#include "../math/grl-rectangle.h"
#include "../math/grl-bounding-box.h"

G_BEGIN_DECLS

/*
 * =============================================================================
 * 2D Collision Detection - Point Tests
 * =============================================================================
 */

/**
 * grl_collision_point_rect:
 * @point: The point to test
 * @rect: The rectangle to test against
 *
 * Checks if a point is inside a rectangle.
 *
 * Returns: %TRUE if the point is inside the rectangle
 */
GRL_AVAILABLE_IN_ALL
gboolean    grl_collision_point_rect            (const GrlVector2   *point,
                                                 const GrlRectangle *rect);

/**
 * grl_collision_point_circle:
 * @point: The point to test
 * @center: Center of the circle
 * @radius: Radius of the circle
 *
 * Checks if a point is inside a circle.
 *
 * Returns: %TRUE if the point is inside the circle
 */
GRL_AVAILABLE_IN_ALL
gboolean    grl_collision_point_circle          (const GrlVector2   *point,
                                                 const GrlVector2   *center,
                                                 gfloat              radius);

/**
 * grl_collision_point_triangle:
 * @point: The point to test
 * @p1: First vertex of triangle
 * @p2: Second vertex of triangle
 * @p3: Third vertex of triangle
 *
 * Checks if a point is inside a triangle.
 *
 * Returns: %TRUE if the point is inside the triangle
 */
GRL_AVAILABLE_IN_ALL
gboolean    grl_collision_point_triangle        (const GrlVector2   *point,
                                                 const GrlVector2   *p1,
                                                 const GrlVector2   *p2,
                                                 const GrlVector2   *p3);

/**
 * grl_collision_point_poly:
 * @point: The point to test
 * @points: (array length=point_count): Array of polygon vertices
 * @point_count: Number of vertices in the polygon
 *
 * Checks if a point is inside a polygon.
 *
 * Returns: %TRUE if the point is inside the polygon
 */
GRL_AVAILABLE_IN_ALL
gboolean    grl_collision_point_poly            (const GrlVector2   *point,
                                                 const GrlVector2   *points,
                                                 gint                point_count);

/**
 * grl_collision_point_line:
 * @point: The point to test
 * @p1: Start of line segment
 * @p2: End of line segment
 * @threshold: Distance threshold for collision
 *
 * Checks if a point is on a line segment within a threshold distance.
 *
 * Returns: %TRUE if the point is on the line segment
 */
GRL_AVAILABLE_IN_ALL
gboolean    grl_collision_point_line            (const GrlVector2   *point,
                                                 const GrlVector2   *p1,
                                                 const GrlVector2   *p2,
                                                 gint                threshold);

/*
 * =============================================================================
 * 2D Collision Detection - Shape vs Shape
 * =============================================================================
 */

/**
 * grl_collision_rects:
 * @rect1: First rectangle
 * @rect2: Second rectangle
 *
 * Checks collision between two rectangles.
 *
 * Returns: %TRUE if the rectangles overlap
 */
GRL_AVAILABLE_IN_ALL
gboolean    grl_collision_rects                 (const GrlRectangle *rect1,
                                                 const GrlRectangle *rect2);

/**
 * grl_collision_circles:
 * @center1: Center of first circle
 * @radius1: Radius of first circle
 * @center2: Center of second circle
 * @radius2: Radius of second circle
 *
 * Checks collision between two circles.
 *
 * Returns: %TRUE if the circles overlap
 */
GRL_AVAILABLE_IN_ALL
gboolean    grl_collision_circles               (const GrlVector2   *center1,
                                                 gfloat              radius1,
                                                 const GrlVector2   *center2,
                                                 gfloat              radius2);

/**
 * grl_collision_circle_rect:
 * @center: Center of circle
 * @radius: Radius of circle
 * @rect: Rectangle to test against
 *
 * Checks collision between a circle and a rectangle.
 *
 * Returns: %TRUE if they overlap
 */
GRL_AVAILABLE_IN_ALL
gboolean    grl_collision_circle_rect           (const GrlVector2   *center,
                                                 gfloat              radius,
                                                 const GrlRectangle *rect);

/**
 * grl_collision_circle_line:
 * @center: Center of circle
 * @radius: Radius of circle
 * @p1: Start of line segment
 * @p2: End of line segment
 *
 * Checks collision between a circle and a line segment.
 *
 * Returns: %TRUE if they intersect
 */
GRL_AVAILABLE_IN_ALL
gboolean    grl_collision_circle_line           (const GrlVector2   *center,
                                                 gfloat              radius,
                                                 const GrlVector2   *p1,
                                                 const GrlVector2   *p2);

/**
 * grl_collision_lines:
 * @start1: Start of first line segment
 * @end1: End of first line segment
 * @start2: Start of second line segment
 * @end2: End of second line segment
 * @collision_point: (out) (optional): Output collision point
 *
 * Checks collision between two line segments.
 *
 * Returns: %TRUE if the lines intersect
 */
GRL_AVAILABLE_IN_ALL
gboolean    grl_collision_lines                 (const GrlVector2   *start1,
                                                 const GrlVector2   *end1,
                                                 const GrlVector2   *start2,
                                                 const GrlVector2   *end2,
                                                 GrlVector2         *collision_point);

/*
 * =============================================================================
 * 2D Collision Detection - Rectangle Operations
 * =============================================================================
 */

/**
 * grl_collision_get_rect_overlap:
 * @rect1: First rectangle
 * @rect2: Second rectangle
 *
 * Gets the overlapping rectangle of two rectangles.
 * Returns NULL if rectangles don't overlap.
 *
 * Returns: (transfer full) (nullable): Overlapping rectangle or %NULL
 */
GRL_AVAILABLE_IN_ALL
GrlRectangle *  grl_collision_get_rect_overlap  (const GrlRectangle *rect1,
                                                 const GrlRectangle *rect2);

/*
 * =============================================================================
 * 3D Collision Detection - Point Tests
 * =============================================================================
 */

/**
 * grl_collision_point_sphere:
 * @point: The point to test
 * @center: Center of the sphere
 * @radius: Radius of the sphere
 *
 * Checks if a point is inside a sphere.
 *
 * Returns: %TRUE if the point is inside the sphere
 */
GRL_AVAILABLE_IN_ALL
gboolean    grl_collision_point_sphere          (const GrlVector3   *point,
                                                 const GrlVector3   *center,
                                                 gfloat              radius);

/**
 * grl_collision_point_box:
 * @point: The point to test
 * @box: The bounding box to test against
 *
 * Checks if a point is inside a bounding box.
 *
 * Returns: %TRUE if the point is inside the box
 */
GRL_AVAILABLE_IN_ALL
gboolean    grl_collision_point_box             (const GrlVector3     *point,
                                                 const GrlBoundingBox *box);

/*
 * =============================================================================
 * 3D Collision Detection - Shape vs Shape
 * =============================================================================
 */

/**
 * grl_collision_spheres:
 * @center1: Center of first sphere
 * @radius1: Radius of first sphere
 * @center2: Center of second sphere
 * @radius2: Radius of second sphere
 *
 * Checks collision between two spheres.
 *
 * Returns: %TRUE if the spheres overlap
 */
GRL_AVAILABLE_IN_ALL
gboolean    grl_collision_spheres               (const GrlVector3   *center1,
                                                 gfloat              radius1,
                                                 const GrlVector3   *center2,
                                                 gfloat              radius2);

/**
 * grl_collision_boxes:
 * @box1: First bounding box
 * @box2: Second bounding box
 *
 * Checks collision between two bounding boxes (AABB).
 *
 * Returns: %TRUE if the boxes overlap
 */
GRL_AVAILABLE_IN_ALL
gboolean    grl_collision_boxes                 (const GrlBoundingBox *box1,
                                                 const GrlBoundingBox *box2);

/**
 * grl_collision_box_sphere:
 * @box: Bounding box
 * @center: Center of sphere
 * @radius: Radius of sphere
 *
 * Checks collision between a bounding box and a sphere.
 *
 * Returns: %TRUE if they overlap
 */
GRL_AVAILABLE_IN_ALL
gboolean    grl_collision_box_sphere            (const GrlBoundingBox *box,
                                                 const GrlVector3     *center,
                                                 gfloat                radius);

/*
 * =============================================================================
 * 3D Ray Collision Detection
 * =============================================================================
 */

/**
 * GrlRayCollision:
 * @hit: Whether the ray hit the target
 * @distance: Distance from ray origin to hit point
 * @point: Hit point position
 * @normal: Surface normal at hit point
 *
 * Information about a ray collision.
 */
typedef struct _GrlRayCollision GrlRayCollision;

struct _GrlRayCollision
{
    gboolean   hit;
    gfloat     distance;
    GrlVector3 point;
    GrlVector3 normal;
};

GRL_AVAILABLE_IN_ALL
GType           grl_ray_collision_get_type      (void) G_GNUC_CONST;

GRL_AVAILABLE_IN_ALL
GrlRayCollision *grl_ray_collision_copy         (const GrlRayCollision *self);

GRL_AVAILABLE_IN_ALL
void            grl_ray_collision_free          (GrlRayCollision *self);

/**
 * GrlRay:
 * @position: Ray origin position
 * @direction: Ray direction (normalized)
 *
 * A 3D ray defined by an origin point and direction.
 */
typedef struct _GrlRay GrlRay;

struct _GrlRay
{
    GrlVector3 position;
    GrlVector3 direction;
};

GRL_AVAILABLE_IN_ALL
GType           grl_ray_get_type                (void) G_GNUC_CONST;

GRL_AVAILABLE_IN_ALL
GrlRay *        grl_ray_new                     (const GrlVector3   *position,
                                                 const GrlVector3   *direction);

GRL_AVAILABLE_IN_ALL
GrlRay *        grl_ray_copy                    (const GrlRay       *self);

GRL_AVAILABLE_IN_ALL
void            grl_ray_free                    (GrlRay             *self);

/**
 * grl_collision_ray_sphere:
 * @ray: The ray to test
 * @center: Center of sphere
 * @radius: Radius of sphere
 *
 * Gets collision info for a ray vs sphere.
 *
 * Returns: (transfer full): Collision result
 */
GRL_AVAILABLE_IN_ALL
GrlRayCollision *   grl_collision_ray_sphere    (const GrlRay       *ray,
                                                 const GrlVector3   *center,
                                                 gfloat              radius);

/**
 * grl_collision_ray_box:
 * @ray: The ray to test
 * @box: Bounding box to test against
 *
 * Gets collision info for a ray vs bounding box.
 *
 * Returns: (transfer full): Collision result
 */
GRL_AVAILABLE_IN_ALL
GrlRayCollision *   grl_collision_ray_box       (const GrlRay         *ray,
                                                 const GrlBoundingBox *box);

/**
 * grl_collision_ray_triangle:
 * @ray: The ray to test
 * @p1: First vertex of triangle
 * @p2: Second vertex of triangle
 * @p3: Third vertex of triangle
 *
 * Gets collision info for a ray vs triangle.
 *
 * Returns: (transfer full): Collision result
 */
GRL_AVAILABLE_IN_ALL
GrlRayCollision *   grl_collision_ray_triangle  (const GrlRay       *ray,
                                                 const GrlVector3   *p1,
                                                 const GrlVector3   *p2,
                                                 const GrlVector3   *p3);

/**
 * grl_collision_ray_quad:
 * @ray: The ray to test
 * @p1: First vertex of quad
 * @p2: Second vertex of quad
 * @p3: Third vertex of quad
 * @p4: Fourth vertex of quad
 *
 * Gets collision info for a ray vs quad.
 *
 * Returns: (transfer full): Collision result
 */
GRL_AVAILABLE_IN_ALL
GrlRayCollision *   grl_collision_ray_quad      (const GrlRay       *ray,
                                                 const GrlVector3   *p1,
                                                 const GrlVector3   *p2,
                                                 const GrlVector3   *p3,
                                                 const GrlVector3   *p4);

/**
 * grl_collision_ray_plane:
 * @ray: The ray to test
 * @plane_normal: Normal vector of the plane
 * @plane_distance: Distance from origin to plane
 *
 * Gets collision info for a ray vs infinite plane.
 * The plane is defined by: dot(normal, point) = distance
 *
 * Returns: (transfer full): Collision result
 */
GRL_AVAILABLE_IN_ALL
GrlRayCollision *   grl_collision_ray_plane     (const GrlRay       *ray,
                                                 const GrlVector3   *plane_normal,
                                                 gfloat              plane_distance);

/*
 * =============================================================================
 * Screen/World Coordinate Conversion
 * =============================================================================
 */

/**
 * grl_collision_get_ray_from_screen:
 * @screen_pos: Screen position (mouse coordinates)
 * @camera_position: Camera position in world space
 * @camera_target: Camera target in world space
 * @camera_up: Camera up vector
 * @fovy: Camera field of view Y
 * @screen_width: Screen width in pixels
 * @screen_height: Screen height in pixels
 *
 * Gets a ray from a 2D screen position using a perspective camera.
 * Useful for mouse picking in 3D scenes.
 *
 * Returns: (transfer full): Ray from camera through screen point
 */
GRL_AVAILABLE_IN_ALL
GrlRay *    grl_collision_get_ray_from_screen   (const GrlVector2   *screen_pos,
                                                 const GrlVector3   *camera_position,
                                                 const GrlVector3   *camera_target,
                                                 const GrlVector3   *camera_up,
                                                 gfloat              fovy,
                                                 gint                screen_width,
                                                 gint                screen_height);

G_END_DECLS
