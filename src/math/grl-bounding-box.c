/* grl-bounding-box.c
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#include "grl-bounding-box.h"

/**
 * SECTION:grl-bounding-box
 * @Title: GrlBoundingBox
 * @Short_description: Axis-aligned bounding box
 *
 * #GrlBoundingBox represents an axis-aligned bounding box (AABB) defined
 * by minimum and maximum corner points. It is useful for collision detection,
 * frustum culling, and spatial queries.
 */

G_DEFINE_BOXED_TYPE (GrlBoundingBox, grl_bounding_box, grl_bounding_box_copy, grl_bounding_box_free)

/**
 * grl_bounding_box_new:
 * @min: (transfer none): Minimum corner
 * @max: (transfer none): Maximum corner
 *
 * Creates a new bounding box from minimum and maximum corners.
 *
 * Returns: (transfer full): A newly allocated #GrlBoundingBox
 */
GrlBoundingBox *
grl_bounding_box_new (const GrlVector3 *min,
                      const GrlVector3 *max)
{
    GrlBoundingBox *self;

    g_return_val_if_fail (min != NULL, NULL);
    g_return_val_if_fail (max != NULL, NULL);

    self = g_slice_new (GrlBoundingBox);
    self->min.x = min->x;
    self->min.y = min->y;
    self->min.z = min->z;
    self->max.x = max->x;
    self->max.y = max->y;
    self->max.z = max->z;

    return self;
}

/**
 * grl_bounding_box_new_xyz:
 * @min_x: Minimum X coordinate
 * @min_y: Minimum Y coordinate
 * @min_z: Minimum Z coordinate
 * @max_x: Maximum X coordinate
 * @max_y: Maximum Y coordinate
 * @max_z: Maximum Z coordinate
 *
 * Creates a new bounding box from individual coordinates.
 *
 * Returns: (transfer full): A newly allocated #GrlBoundingBox
 */
GrlBoundingBox *
grl_bounding_box_new_xyz (gfloat min_x,
                          gfloat min_y,
                          gfloat min_z,
                          gfloat max_x,
                          gfloat max_y,
                          gfloat max_z)
{
    GrlBoundingBox *self;

    self = g_slice_new (GrlBoundingBox);
    self->min.x = min_x;
    self->min.y = min_y;
    self->min.z = min_z;
    self->max.x = max_x;
    self->max.y = max_y;
    self->max.z = max_z;

    return self;
}

/**
 * grl_bounding_box_copy:
 * @self: (nullable): A #GrlBoundingBox
 *
 * Creates a copy of the bounding box.
 *
 * Returns: (transfer full) (nullable): A copy, or %NULL if @self is %NULL
 */
GrlBoundingBox *
grl_bounding_box_copy (const GrlBoundingBox *self)
{
    GrlBoundingBox *copy;

    if (self == NULL)
        return NULL;

    copy = g_slice_new (GrlBoundingBox);
    *copy = *self;

    return copy;
}

/**
 * grl_bounding_box_free:
 * @self: (nullable): A #GrlBoundingBox
 *
 * Frees a bounding box.
 */
void
grl_bounding_box_free (GrlBoundingBox *self)
{
    if (self != NULL)
        g_slice_free (GrlBoundingBox, self);
}

/**
 * grl_bounding_box_get_min:
 * @self: A #GrlBoundingBox
 *
 * Gets the minimum corner of the bounding box.
 *
 * Returns: (transfer full): The minimum corner
 */
GrlVector3 *
grl_bounding_box_get_min (const GrlBoundingBox *self)
{
    g_return_val_if_fail (self != NULL, NULL);

    return grl_vector3_new (self->min.x, self->min.y, self->min.z);
}

/**
 * grl_bounding_box_get_max:
 * @self: A #GrlBoundingBox
 *
 * Gets the maximum corner of the bounding box.
 *
 * Returns: (transfer full): The maximum corner
 */
GrlVector3 *
grl_bounding_box_get_max (const GrlBoundingBox *self)
{
    g_return_val_if_fail (self != NULL, NULL);

    return grl_vector3_new (self->max.x, self->max.y, self->max.z);
}

/**
 * grl_bounding_box_get_center:
 * @self: A #GrlBoundingBox
 *
 * Gets the center point of the bounding box.
 *
 * Returns: (transfer full): The center point
 */
GrlVector3 *
grl_bounding_box_get_center (const GrlBoundingBox *self)
{
    g_return_val_if_fail (self != NULL, NULL);

    return grl_vector3_new ((self->min.x + self->max.x) * 0.5f,
                            (self->min.y + self->max.y) * 0.5f,
                            (self->min.z + self->max.z) * 0.5f);
}

/**
 * grl_bounding_box_get_size:
 * @self: A #GrlBoundingBox
 *
 * Gets the size (dimensions) of the bounding box.
 *
 * Returns: (transfer full): A vector with width, height, depth
 */
GrlVector3 *
grl_bounding_box_get_size (const GrlBoundingBox *self)
{
    g_return_val_if_fail (self != NULL, NULL);

    return grl_vector3_new (self->max.x - self->min.x,
                            self->max.y - self->min.y,
                            self->max.z - self->min.z);
}

/**
 * grl_bounding_box_contains_point:
 * @self: A #GrlBoundingBox
 * @point: The point to check
 *
 * Checks if a point is inside the bounding box.
 *
 * Returns: %TRUE if the point is inside
 */
gboolean
grl_bounding_box_contains_point (const GrlBoundingBox *self,
                                 const GrlVector3     *point)
{
    g_return_val_if_fail (self != NULL, FALSE);
    g_return_val_if_fail (point != NULL, FALSE);

    return (point->x >= self->min.x && point->x <= self->max.x &&
            point->y >= self->min.y && point->y <= self->max.y &&
            point->z >= self->min.z && point->z <= self->max.z);
}

/**
 * grl_bounding_box_intersects:
 * @a: First bounding box
 * @b: Second bounding box
 *
 * Checks if two bounding boxes intersect.
 *
 * Returns: %TRUE if they intersect
 */
gboolean
grl_bounding_box_intersects (const GrlBoundingBox *a,
                             const GrlBoundingBox *b)
{
    g_return_val_if_fail (a != NULL, FALSE);
    g_return_val_if_fail (b != NULL, FALSE);

    /* Check for separation on each axis */
    if (a->max.x < b->min.x || a->min.x > b->max.x)
        return FALSE;
    if (a->max.y < b->min.y || a->min.y > b->max.y)
        return FALSE;
    if (a->max.z < b->min.z || a->min.z > b->max.z)
        return FALSE;

    return TRUE;
}

/**
 * grl_bounding_box_expand:
 * @self: A #GrlBoundingBox
 * @point: Point to include
 *
 * Expands the bounding box to include the given point.
 *
 * Returns: (transfer full): A new expanded bounding box
 */
GrlBoundingBox *
grl_bounding_box_expand (const GrlBoundingBox *self,
                         const GrlVector3     *point)
{
    GrlBoundingBox *result;

    g_return_val_if_fail (self != NULL, NULL);
    g_return_val_if_fail (point != NULL, NULL);

    result = grl_bounding_box_copy (self);

    if (point->x < result->min.x)
        result->min.x = point->x;
    if (point->y < result->min.y)
        result->min.y = point->y;
    if (point->z < result->min.z)
        result->min.z = point->z;

    if (point->x > result->max.x)
        result->max.x = point->x;
    if (point->y > result->max.y)
        result->max.y = point->y;
    if (point->z > result->max.z)
        result->max.z = point->z;

    return result;
}

/**
 * grl_bounding_box_merge:
 * @a: First bounding box
 * @b: Second bounding box
 *
 * Creates a bounding box that contains both input boxes.
 *
 * Returns: (transfer full): A new merged bounding box
 */
GrlBoundingBox *
grl_bounding_box_merge (const GrlBoundingBox *a,
                        const GrlBoundingBox *b)
{
    GrlBoundingBox *result;

    g_return_val_if_fail (a != NULL, NULL);
    g_return_val_if_fail (b != NULL, NULL);

    result = g_slice_new (GrlBoundingBox);

    /* Calculate minimum corner */
    result->min.x = (a->min.x < b->min.x) ? a->min.x : b->min.x;
    result->min.y = (a->min.y < b->min.y) ? a->min.y : b->min.y;
    result->min.z = (a->min.z < b->min.z) ? a->min.z : b->min.z;

    /* Calculate maximum corner */
    result->max.x = (a->max.x > b->max.x) ? a->max.x : b->max.x;
    result->max.y = (a->max.y > b->max.y) ? a->max.y : b->max.y;
    result->max.z = (a->max.z > b->max.z) ? a->max.z : b->max.z;

    return result;
}
