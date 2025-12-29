/* grl-collision.c
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#include "config.h"
#include "grl-collision.h"
#include <raylib.h>
#include <math.h>

/**
 * SECTION:grl-collision
 * @Title: Collision Detection
 * @Short_description: 2D and 3D collision detection utilities
 *
 * This module provides collision detection functions for common shapes
 * in both 2D and 3D. It wraps raylib's collision detection with a
 * GLib-friendly API.
 *
 * # 2D Collision Detection
 *
 * For 2D games, you can check collisions between points, rectangles,
 * circles, triangles, lines, and polygons.
 *
 * # 3D Collision Detection
 *
 * For 3D games, you can check collisions between points, spheres,
 * bounding boxes, and rays.
 *
 * # Example
 *
 * |[<!-- language="C" -->
 * g_autoptr(GrlVector2) player_pos = grl_vector2_new (100.0f, 100.0f);
 * g_autoptr(GrlRectangle) wall = grl_rectangle_new (200.0f, 50.0f, 50.0f, 200.0f);
 *
 * // Check if player point is inside wall rectangle
 * if (grl_collision_point_rect (player_pos, wall))
 * {
 *     g_print ("Collision detected!\n");
 * }
 * ]|
 */

/*
 * =============================================================================
 * GrlRay Boxed Type
 * =============================================================================
 */

G_DEFINE_BOXED_TYPE (GrlRay, grl_ray, grl_ray_copy, grl_ray_free)

/**
 * grl_ray_new:
 * @position: Ray origin position
 * @direction: Ray direction (will be used as-is, should be normalized)
 *
 * Creates a new ray.
 *
 * Returns: (transfer full): A new #GrlRay
 */
GrlRay *
grl_ray_new (const GrlVector3 *position,
             const GrlVector3 *direction)
{
    GrlRay *self;

    g_return_val_if_fail (position != NULL, NULL);
    g_return_val_if_fail (direction != NULL, NULL);

    self = g_slice_new (GrlRay);
    self->position.x = position->x;
    self->position.y = position->y;
    self->position.z = position->z;
    self->direction.x = direction->x;
    self->direction.y = direction->y;
    self->direction.z = direction->z;

    return self;
}

/**
 * grl_ray_copy:
 * @self: (nullable): A #GrlRay
 *
 * Copies a ray.
 *
 * Returns: (transfer full) (nullable): A copy, or %NULL
 */
GrlRay *
grl_ray_copy (const GrlRay *self)
{
    GrlRay *copy;

    if (self == NULL)
        return NULL;

    copy = g_slice_new (GrlRay);
    *copy = *self;

    return copy;
}

/**
 * grl_ray_free:
 * @self: (nullable): A #GrlRay
 *
 * Frees a ray.
 */
void
grl_ray_free (GrlRay *self)
{
    if (self != NULL)
        g_slice_free (GrlRay, self);
}

/*
 * =============================================================================
 * GrlRayCollision Boxed Type
 * =============================================================================
 */

G_DEFINE_BOXED_TYPE (GrlRayCollision, grl_ray_collision,
                     grl_ray_collision_copy, grl_ray_collision_free)

/**
 * grl_ray_collision_copy:
 * @self: (nullable): A #GrlRayCollision
 *
 * Copies a ray collision result.
 *
 * Returns: (transfer full) (nullable): A copy, or %NULL
 */
GrlRayCollision *
grl_ray_collision_copy (const GrlRayCollision *self)
{
    GrlRayCollision *copy;

    if (self == NULL)
        return NULL;

    copy = g_slice_new (GrlRayCollision);
    *copy = *self;

    return copy;
}

/**
 * grl_ray_collision_free:
 * @self: (nullable): A #GrlRayCollision
 *
 * Frees a ray collision result.
 */
void
grl_ray_collision_free (GrlRayCollision *self)
{
    if (self != NULL)
        g_slice_free (GrlRayCollision, self);
}

/*
 * Helper to create GrlRayCollision from raylib RayCollision
 */
static GrlRayCollision *
grl_ray_collision_from_raylib (RayCollision rc)
{
    GrlRayCollision *self;
    unsigned char raw_hit;

    self = g_slice_new (GrlRayCollision);

    /* Fix bool/gboolean ABI mismatch */
    raw_hit = rc.hit;
    self->hit = raw_hit != 0;

    self->distance = rc.distance;
    self->point.x = rc.point.x;
    self->point.y = rc.point.y;
    self->point.z = rc.point.z;
    self->normal.x = rc.normal.x;
    self->normal.y = rc.normal.y;
    self->normal.z = rc.normal.z;

    return self;
}

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
gboolean
grl_collision_point_rect (const GrlVector2   *point,
                          const GrlRectangle *rect)
{
    Vector2 p;
    Rectangle r;
    unsigned char raw;

    g_return_val_if_fail (point != NULL, FALSE);
    g_return_val_if_fail (rect != NULL, FALSE);

    p.x = point->x;
    p.y = point->y;

    r.x = rect->x;
    r.y = rect->y;
    r.width = rect->width;
    r.height = rect->height;

    /* Fix bool/gboolean ABI mismatch */
    raw = CheckCollisionPointRec (p, r);
    return raw != 0;
}

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
gboolean
grl_collision_point_circle (const GrlVector2 *point,
                            const GrlVector2 *center,
                            gfloat            radius)
{
    Vector2 p, c;
    unsigned char raw;

    g_return_val_if_fail (point != NULL, FALSE);
    g_return_val_if_fail (center != NULL, FALSE);

    p.x = point->x;
    p.y = point->y;
    c.x = center->x;
    c.y = center->y;

    /* Fix bool/gboolean ABI mismatch */
    raw = CheckCollisionPointCircle (p, c, radius);
    return raw != 0;
}

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
gboolean
grl_collision_point_triangle (const GrlVector2 *point,
                              const GrlVector2 *p1,
                              const GrlVector2 *p2,
                              const GrlVector2 *p3)
{
    Vector2 pt, v1, v2, v3;
    unsigned char raw;

    g_return_val_if_fail (point != NULL, FALSE);
    g_return_val_if_fail (p1 != NULL, FALSE);
    g_return_val_if_fail (p2 != NULL, FALSE);
    g_return_val_if_fail (p3 != NULL, FALSE);

    pt.x = point->x;
    pt.y = point->y;
    v1.x = p1->x;
    v1.y = p1->y;
    v2.x = p2->x;
    v2.y = p2->y;
    v3.x = p3->x;
    v3.y = p3->y;

    /* Fix bool/gboolean ABI mismatch */
    raw = CheckCollisionPointTriangle (pt, v1, v2, v3);
    return raw != 0;
}

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
gboolean
grl_collision_point_poly (const GrlVector2 *point,
                          const GrlVector2 *points,
                          gint              point_count)
{
    Vector2 pt;
    Vector2 *poly;
    gboolean result;
    gint i;
    unsigned char raw;

    g_return_val_if_fail (point != NULL, FALSE);
    g_return_val_if_fail (points != NULL, FALSE);
    g_return_val_if_fail (point_count >= 3, FALSE);

    pt.x = point->x;
    pt.y = point->y;

    /* Convert polygon points */
    poly = g_new (Vector2, point_count);
    for (i = 0; i < point_count; i++)
    {
        poly[i].x = points[i].x;
        poly[i].y = points[i].y;
    }

    /* Fix bool/gboolean ABI mismatch */
    raw = CheckCollisionPointPoly (pt, poly, point_count);
    result = raw != 0;

    g_free (poly);

    return result;
}

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
gboolean
grl_collision_point_line (const GrlVector2 *point,
                          const GrlVector2 *p1,
                          const GrlVector2 *p2,
                          gint              threshold)
{
    Vector2 pt, v1, v2;
    unsigned char raw;

    g_return_val_if_fail (point != NULL, FALSE);
    g_return_val_if_fail (p1 != NULL, FALSE);
    g_return_val_if_fail (p2 != NULL, FALSE);

    pt.x = point->x;
    pt.y = point->y;
    v1.x = p1->x;
    v1.y = p1->y;
    v2.x = p2->x;
    v2.y = p2->y;

    /* Fix bool/gboolean ABI mismatch */
    raw = CheckCollisionPointLine (pt, v1, v2, threshold);
    return raw != 0;
}

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
gboolean
grl_collision_rects (const GrlRectangle *rect1,
                     const GrlRectangle *rect2)
{
    Rectangle r1, r2;
    unsigned char raw;

    g_return_val_if_fail (rect1 != NULL, FALSE);
    g_return_val_if_fail (rect2 != NULL, FALSE);

    r1.x = rect1->x;
    r1.y = rect1->y;
    r1.width = rect1->width;
    r1.height = rect1->height;

    r2.x = rect2->x;
    r2.y = rect2->y;
    r2.width = rect2->width;
    r2.height = rect2->height;

    /* Fix bool/gboolean ABI mismatch */
    raw = CheckCollisionRecs (r1, r2);
    return raw != 0;
}

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
gboolean
grl_collision_circles (const GrlVector2 *center1,
                       gfloat            radius1,
                       const GrlVector2 *center2,
                       gfloat            radius2)
{
    Vector2 c1, c2;
    unsigned char raw;

    g_return_val_if_fail (center1 != NULL, FALSE);
    g_return_val_if_fail (center2 != NULL, FALSE);

    c1.x = center1->x;
    c1.y = center1->y;
    c2.x = center2->x;
    c2.y = center2->y;

    /* Fix bool/gboolean ABI mismatch */
    raw = CheckCollisionCircles (c1, radius1, c2, radius2);
    return raw != 0;
}

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
gboolean
grl_collision_circle_rect (const GrlVector2   *center,
                           gfloat              radius,
                           const GrlRectangle *rect)
{
    Vector2 c;
    Rectangle r;
    unsigned char raw;

    g_return_val_if_fail (center != NULL, FALSE);
    g_return_val_if_fail (rect != NULL, FALSE);

    c.x = center->x;
    c.y = center->y;

    r.x = rect->x;
    r.y = rect->y;
    r.width = rect->width;
    r.height = rect->height;

    /* Fix bool/gboolean ABI mismatch */
    raw = CheckCollisionCircleRec (c, radius, r);
    return raw != 0;
}

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
gboolean
grl_collision_circle_line (const GrlVector2 *center,
                           gfloat            radius,
                           const GrlVector2 *p1,
                           const GrlVector2 *p2)
{
    Vector2 c, v1, v2;
    unsigned char raw;

    g_return_val_if_fail (center != NULL, FALSE);
    g_return_val_if_fail (p1 != NULL, FALSE);
    g_return_val_if_fail (p2 != NULL, FALSE);

    c.x = center->x;
    c.y = center->y;
    v1.x = p1->x;
    v1.y = p1->y;
    v2.x = p2->x;
    v2.y = p2->y;

    /* Fix bool/gboolean ABI mismatch */
    raw = CheckCollisionCircleLine (c, radius, v1, v2);
    return raw != 0;
}

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
gboolean
grl_collision_lines (const GrlVector2 *start1,
                     const GrlVector2 *end1,
                     const GrlVector2 *start2,
                     const GrlVector2 *end2,
                     GrlVector2       *collision_point)
{
    Vector2 s1, e1, s2, e2;
    Vector2 cp;
    gboolean result;
    unsigned char raw;

    g_return_val_if_fail (start1 != NULL, FALSE);
    g_return_val_if_fail (end1 != NULL, FALSE);
    g_return_val_if_fail (start2 != NULL, FALSE);
    g_return_val_if_fail (end2 != NULL, FALSE);

    s1.x = start1->x;
    s1.y = start1->y;
    e1.x = end1->x;
    e1.y = end1->y;
    s2.x = start2->x;
    s2.y = start2->y;
    e2.x = end2->x;
    e2.y = end2->y;

    /* Fix bool/gboolean ABI mismatch */
    raw = CheckCollisionLines (s1, e1, s2, e2, &cp);
    result = raw != 0;

    if (result && collision_point != NULL)
    {
        collision_point->x = cp.x;
        collision_point->y = cp.y;
    }

    return result;
}

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
GrlRectangle *
grl_collision_get_rect_overlap (const GrlRectangle *rect1,
                                const GrlRectangle *rect2)
{
    Rectangle r1, r2, overlap;

    g_return_val_if_fail (rect1 != NULL, NULL);
    g_return_val_if_fail (rect2 != NULL, NULL);

    r1.x = rect1->x;
    r1.y = rect1->y;
    r1.width = rect1->width;
    r1.height = rect1->height;

    r2.x = rect2->x;
    r2.y = rect2->y;
    r2.width = rect2->width;
    r2.height = rect2->height;

    /* Check if they overlap first */
    if (!CheckCollisionRecs (r1, r2))
        return NULL;

    overlap = GetCollisionRec (r1, r2);

    return grl_rectangle_new (overlap.x, overlap.y, overlap.width, overlap.height);
}

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
gboolean
grl_collision_point_sphere (const GrlVector3 *point,
                            const GrlVector3 *center,
                            gfloat            radius)
{
    gfloat dx, dy, dz, dist_sq;

    g_return_val_if_fail (point != NULL, FALSE);
    g_return_val_if_fail (center != NULL, FALSE);

    dx = point->x - center->x;
    dy = point->y - center->y;
    dz = point->z - center->z;
    dist_sq = dx * dx + dy * dy + dz * dz;

    return dist_sq <= (radius * radius);
}

/**
 * grl_collision_point_box:
 * @point: The point to test
 * @box: The bounding box to test against
 *
 * Checks if a point is inside a bounding box.
 *
 * Returns: %TRUE if the point is inside the box
 */
gboolean
grl_collision_point_box (const GrlVector3     *point,
                         const GrlBoundingBox *box)
{
    g_return_val_if_fail (point != NULL, FALSE);
    g_return_val_if_fail (box != NULL, FALSE);

    return grl_bounding_box_contains_point (box, point);
}

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
gboolean
grl_collision_spheres (const GrlVector3 *center1,
                       gfloat            radius1,
                       const GrlVector3 *center2,
                       gfloat            radius2)
{
    Vector3 c1, c2;
    unsigned char raw;

    g_return_val_if_fail (center1 != NULL, FALSE);
    g_return_val_if_fail (center2 != NULL, FALSE);

    c1.x = center1->x;
    c1.y = center1->y;
    c1.z = center1->z;

    c2.x = center2->x;
    c2.y = center2->y;
    c2.z = center2->z;

    /* Fix bool/gboolean ABI mismatch */
    raw = CheckCollisionSpheres (c1, radius1, c2, radius2);
    return raw != 0;
}

/**
 * grl_collision_boxes:
 * @box1: First bounding box
 * @box2: Second bounding box
 *
 * Checks collision between two bounding boxes (AABB).
 *
 * Returns: %TRUE if the boxes overlap
 */
gboolean
grl_collision_boxes (const GrlBoundingBox *box1,
                     const GrlBoundingBox *box2)
{
    BoundingBox b1, b2;
    unsigned char raw;

    g_return_val_if_fail (box1 != NULL, FALSE);
    g_return_val_if_fail (box2 != NULL, FALSE);

    b1.min.x = box1->min.x;
    b1.min.y = box1->min.y;
    b1.min.z = box1->min.z;
    b1.max.x = box1->max.x;
    b1.max.y = box1->max.y;
    b1.max.z = box1->max.z;

    b2.min.x = box2->min.x;
    b2.min.y = box2->min.y;
    b2.min.z = box2->min.z;
    b2.max.x = box2->max.x;
    b2.max.y = box2->max.y;
    b2.max.z = box2->max.z;

    /* Fix bool/gboolean ABI mismatch */
    raw = CheckCollisionBoxes (b1, b2);
    return raw != 0;
}

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
gboolean
grl_collision_box_sphere (const GrlBoundingBox *box,
                          const GrlVector3     *center,
                          gfloat                radius)
{
    BoundingBox b;
    Vector3 c;
    unsigned char raw;

    g_return_val_if_fail (box != NULL, FALSE);
    g_return_val_if_fail (center != NULL, FALSE);

    b.min.x = box->min.x;
    b.min.y = box->min.y;
    b.min.z = box->min.z;
    b.max.x = box->max.x;
    b.max.y = box->max.y;
    b.max.z = box->max.z;

    c.x = center->x;
    c.y = center->y;
    c.z = center->z;

    /* Fix bool/gboolean ABI mismatch */
    raw = CheckCollisionBoxSphere (b, c, radius);
    return raw != 0;
}

/*
 * =============================================================================
 * 3D Ray Collision Detection
 * =============================================================================
 */

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
GrlRayCollision *
grl_collision_ray_sphere (const GrlRay     *ray,
                          const GrlVector3 *center,
                          gfloat            radius)
{
    Ray r;
    Vector3 c;
    RayCollision rc;

    g_return_val_if_fail (ray != NULL, NULL);
    g_return_val_if_fail (center != NULL, NULL);

    r.position.x = ray->position.x;
    r.position.y = ray->position.y;
    r.position.z = ray->position.z;
    r.direction.x = ray->direction.x;
    r.direction.y = ray->direction.y;
    r.direction.z = ray->direction.z;

    c.x = center->x;
    c.y = center->y;
    c.z = center->z;

    rc = GetRayCollisionSphere (r, c, radius);

    return grl_ray_collision_from_raylib (rc);
}

/**
 * grl_collision_ray_box:
 * @ray: The ray to test
 * @box: Bounding box to test against
 *
 * Gets collision info for a ray vs bounding box.
 *
 * Returns: (transfer full): Collision result
 */
GrlRayCollision *
grl_collision_ray_box (const GrlRay         *ray,
                       const GrlBoundingBox *box)
{
    Ray r;
    BoundingBox b;
    RayCollision rc;

    g_return_val_if_fail (ray != NULL, NULL);
    g_return_val_if_fail (box != NULL, NULL);

    r.position.x = ray->position.x;
    r.position.y = ray->position.y;
    r.position.z = ray->position.z;
    r.direction.x = ray->direction.x;
    r.direction.y = ray->direction.y;
    r.direction.z = ray->direction.z;

    b.min.x = box->min.x;
    b.min.y = box->min.y;
    b.min.z = box->min.z;
    b.max.x = box->max.x;
    b.max.y = box->max.y;
    b.max.z = box->max.z;

    rc = GetRayCollisionBox (r, b);

    return grl_ray_collision_from_raylib (rc);
}

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
GrlRayCollision *
grl_collision_ray_triangle (const GrlRay     *ray,
                            const GrlVector3 *p1,
                            const GrlVector3 *p2,
                            const GrlVector3 *p3)
{
    Ray r;
    Vector3 v1, v2, v3;
    RayCollision rc;

    g_return_val_if_fail (ray != NULL, NULL);
    g_return_val_if_fail (p1 != NULL, NULL);
    g_return_val_if_fail (p2 != NULL, NULL);
    g_return_val_if_fail (p3 != NULL, NULL);

    r.position.x = ray->position.x;
    r.position.y = ray->position.y;
    r.position.z = ray->position.z;
    r.direction.x = ray->direction.x;
    r.direction.y = ray->direction.y;
    r.direction.z = ray->direction.z;

    v1.x = p1->x;
    v1.y = p1->y;
    v1.z = p1->z;
    v2.x = p2->x;
    v2.y = p2->y;
    v2.z = p2->z;
    v3.x = p3->x;
    v3.y = p3->y;
    v3.z = p3->z;

    rc = GetRayCollisionTriangle (r, v1, v2, v3);

    return grl_ray_collision_from_raylib (rc);
}

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
GrlRayCollision *
grl_collision_ray_quad (const GrlRay     *ray,
                        const GrlVector3 *p1,
                        const GrlVector3 *p2,
                        const GrlVector3 *p3,
                        const GrlVector3 *p4)
{
    Ray r;
    Vector3 v1, v2, v3, v4;
    RayCollision rc;

    g_return_val_if_fail (ray != NULL, NULL);
    g_return_val_if_fail (p1 != NULL, NULL);
    g_return_val_if_fail (p2 != NULL, NULL);
    g_return_val_if_fail (p3 != NULL, NULL);
    g_return_val_if_fail (p4 != NULL, NULL);

    r.position.x = ray->position.x;
    r.position.y = ray->position.y;
    r.position.z = ray->position.z;
    r.direction.x = ray->direction.x;
    r.direction.y = ray->direction.y;
    r.direction.z = ray->direction.z;

    v1.x = p1->x;
    v1.y = p1->y;
    v1.z = p1->z;
    v2.x = p2->x;
    v2.y = p2->y;
    v2.z = p2->z;
    v3.x = p3->x;
    v3.y = p3->y;
    v3.z = p3->z;
    v4.x = p4->x;
    v4.y = p4->y;
    v4.z = p4->z;

    rc = GetRayCollisionQuad (r, v1, v2, v3, v4);

    return grl_ray_collision_from_raylib (rc);
}

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
GrlRayCollision *
grl_collision_ray_plane (const GrlRay     *ray,
                         const GrlVector3 *plane_normal,
                         gfloat            plane_distance)
{
    GrlRayCollision *result;
    gfloat denom, t;
    Vector3 n;

    g_return_val_if_fail (ray != NULL, NULL);
    g_return_val_if_fail (plane_normal != NULL, NULL);

    result = g_slice_new0 (GrlRayCollision);

    n.x = plane_normal->x;
    n.y = plane_normal->y;
    n.z = plane_normal->z;

    /* Calculate denominator (dot product of ray direction and plane normal) */
    denom = ray->direction.x * n.x +
            ray->direction.y * n.y +
            ray->direction.z * n.z;

    /* If denom is close to 0, ray is parallel to plane */
    if (fabsf (denom) < 0.00001f)
    {
        result->hit = FALSE;
        return result;
    }

    /* Calculate t (distance along ray to intersection) */
    t = (plane_distance -
         (ray->position.x * n.x +
          ray->position.y * n.y +
          ray->position.z * n.z)) / denom;

    /* Check if intersection is behind ray origin */
    if (t < 0.0f)
    {
        result->hit = FALSE;
        return result;
    }

    result->hit = TRUE;
    result->distance = t;
    result->point.x = ray->position.x + ray->direction.x * t;
    result->point.y = ray->position.y + ray->direction.y * t;
    result->point.z = ray->position.z + ray->direction.z * t;
    result->normal.x = n.x;
    result->normal.y = n.y;
    result->normal.z = n.z;

    return result;
}

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
GrlRay *
grl_collision_get_ray_from_screen (const GrlVector2 *screen_pos,
                                   const GrlVector3 *camera_position,
                                   const GrlVector3 *camera_target,
                                   const GrlVector3 *camera_up,
                                   gfloat            fovy,
                                   gint              screen_width,
                                   gint              screen_height)
{
    Camera cam;
    Vector2 sp;
    Ray r;
    GrlRay *result;

    g_return_val_if_fail (screen_pos != NULL, NULL);
    g_return_val_if_fail (camera_position != NULL, NULL);
    g_return_val_if_fail (camera_target != NULL, NULL);
    g_return_val_if_fail (camera_up != NULL, NULL);

    cam.position.x = camera_position->x;
    cam.position.y = camera_position->y;
    cam.position.z = camera_position->z;
    cam.target.x = camera_target->x;
    cam.target.y = camera_target->y;
    cam.target.z = camera_target->z;
    cam.up.x = camera_up->x;
    cam.up.y = camera_up->y;
    cam.up.z = camera_up->z;
    cam.fovy = fovy;
    cam.projection = CAMERA_PERSPECTIVE;

    sp.x = screen_pos->x;
    sp.y = screen_pos->y;

    /* Use raylib's GetScreenToWorldRay instead for proper implementation */
    r = GetScreenToWorldRay (sp, cam);

    result = g_slice_new (GrlRay);
    result->position.x = r.position.x;
    result->position.y = r.position.y;
    result->position.z = r.position.z;
    result->direction.x = r.direction.x;
    result->direction.y = r.direction.y;
    result->direction.z = r.direction.z;

    return result;
}
