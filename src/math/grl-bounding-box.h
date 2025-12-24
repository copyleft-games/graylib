/* grl-bounding-box.h
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Axis-aligned bounding box (AABB) type.
 */

#pragma once

#if !defined(GRAYLIB_INSIDE) && !defined(GRAYLIB_COMPILATION)
#error "Only <graylib.h> can be included directly."
#endif

#include <glib-object.h>
#include "../grl-version.h"
#include "grl-vector3.h"

G_BEGIN_DECLS

typedef struct _GrlBoundingBox GrlBoundingBox;

/**
 * GrlBoundingBox:
 * @min: Minimum corner of the bounding box
 * @max: Maximum corner of the bounding box
 *
 * An axis-aligned bounding box (AABB) defined by minimum and maximum corners.
 */
struct _GrlBoundingBox
{
    GrlVector3 min;
    GrlVector3 max;
};

#define GRL_TYPE_BOUNDING_BOX (grl_bounding_box_get_type())

GRL_AVAILABLE_IN_ALL
GType               grl_bounding_box_get_type       (void) G_GNUC_CONST;

/**
 * grl_bounding_box_new:
 * @min: (transfer none): Minimum corner
 * @max: (transfer none): Maximum corner
 *
 * Creates a new bounding box from minimum and maximum corners.
 *
 * Returns: (transfer full): A newly allocated #GrlBoundingBox
 */
GRL_AVAILABLE_IN_ALL
GrlBoundingBox *    grl_bounding_box_new            (const GrlVector3       *min,
                                                     const GrlVector3       *max);

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
GRL_AVAILABLE_IN_ALL
GrlBoundingBox *    grl_bounding_box_new_xyz        (gfloat                  min_x,
                                                     gfloat                  min_y,
                                                     gfloat                  min_z,
                                                     gfloat                  max_x,
                                                     gfloat                  max_y,
                                                     gfloat                  max_z);

/**
 * grl_bounding_box_copy:
 * @self: (nullable): A #GrlBoundingBox
 *
 * Creates a copy of the bounding box.
 *
 * Returns: (transfer full) (nullable): A copy, or %NULL
 */
GRL_AVAILABLE_IN_ALL
GrlBoundingBox *    grl_bounding_box_copy           (const GrlBoundingBox   *self);

/**
 * grl_bounding_box_free:
 * @self: (nullable): A #GrlBoundingBox
 *
 * Frees a bounding box.
 */
GRL_AVAILABLE_IN_ALL
void                grl_bounding_box_free           (GrlBoundingBox         *self);

G_DEFINE_AUTOPTR_CLEANUP_FUNC (GrlBoundingBox, grl_bounding_box_free)

/**
 * grl_bounding_box_get_min:
 * @self: A #GrlBoundingBox
 *
 * Gets the minimum corner of the bounding box.
 *
 * Returns: (transfer full): The minimum corner
 */
GRL_AVAILABLE_IN_ALL
GrlVector3 *        grl_bounding_box_get_min        (const GrlBoundingBox   *self);

/**
 * grl_bounding_box_get_max:
 * @self: A #GrlBoundingBox
 *
 * Gets the maximum corner of the bounding box.
 *
 * Returns: (transfer full): The maximum corner
 */
GRL_AVAILABLE_IN_ALL
GrlVector3 *        grl_bounding_box_get_max        (const GrlBoundingBox   *self);

/**
 * grl_bounding_box_get_center:
 * @self: A #GrlBoundingBox
 *
 * Gets the center point of the bounding box.
 *
 * Returns: (transfer full): The center point
 */
GRL_AVAILABLE_IN_ALL
GrlVector3 *        grl_bounding_box_get_center     (const GrlBoundingBox   *self);

/**
 * grl_bounding_box_get_size:
 * @self: A #GrlBoundingBox
 *
 * Gets the size (dimensions) of the bounding box.
 *
 * Returns: (transfer full): A vector with width, height, depth
 */
GRL_AVAILABLE_IN_ALL
GrlVector3 *        grl_bounding_box_get_size       (const GrlBoundingBox   *self);

/**
 * grl_bounding_box_contains_point:
 * @self: A #GrlBoundingBox
 * @point: The point to check
 *
 * Checks if a point is inside the bounding box.
 *
 * Returns: %TRUE if the point is inside
 */
GRL_AVAILABLE_IN_ALL
gboolean            grl_bounding_box_contains_point (const GrlBoundingBox   *self,
                                                     const GrlVector3       *point);

/**
 * grl_bounding_box_intersects:
 * @a: First bounding box
 * @b: Second bounding box
 *
 * Checks if two bounding boxes intersect.
 *
 * Returns: %TRUE if they intersect
 */
GRL_AVAILABLE_IN_ALL
gboolean            grl_bounding_box_intersects     (const GrlBoundingBox   *a,
                                                     const GrlBoundingBox   *b);

/**
 * grl_bounding_box_expand:
 * @self: A #GrlBoundingBox
 * @point: Point to include
 *
 * Expands the bounding box to include the given point.
 *
 * Returns: (transfer full): A new expanded bounding box
 */
GRL_AVAILABLE_IN_ALL
GrlBoundingBox *    grl_bounding_box_expand         (const GrlBoundingBox   *self,
                                                     const GrlVector3       *point);

/**
 * grl_bounding_box_merge:
 * @a: First bounding box
 * @b: Second bounding box
 *
 * Creates a bounding box that contains both input boxes.
 *
 * Returns: (transfer full): A new merged bounding box
 */
GRL_AVAILABLE_IN_ALL
GrlBoundingBox *    grl_bounding_box_merge          (const GrlBoundingBox   *a,
                                                     const GrlBoundingBox   *b);

G_END_DECLS
