/* grl-collidable.h
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Interface for collision-enabled objects.
 */

#pragma once

#if !defined(GRAYLIB_INSIDE) && !defined(GRAYLIB_COMPILATION)
#error "Only <graylib.h> can be included directly."
#endif

#include <glib-object.h>
#include "../grl-version.h"
#include "../math/grl-rectangle.h"

G_BEGIN_DECLS

#define GRL_TYPE_COLLIDABLE (grl_collidable_get_type ())

GRL_AVAILABLE_IN_ALL
G_DECLARE_INTERFACE (GrlCollidable, grl_collidable, GRL, COLLIDABLE, GObject)

/**
 * GrlCollidableInterface:
 * @parent_iface: The parent interface
 * @get_bounds: Virtual method to get the collision bounds
 * @get_collision_enabled: Virtual method to check if collision is enabled
 * @set_collision_enabled: Virtual method to enable/disable collision
 * @on_collision: Virtual method called when a collision occurs
 *
 * Interface for objects that participate in collision detection.
 *
 * Objects implementing this interface provide bounding rectangles
 * for collision checks and receive callbacks when collisions occur.
 */
struct _GrlCollidableInterface
{
    GTypeInterface parent_iface;

    /* Virtual methods */
    GrlRectangle *  (*get_bounds)               (GrlCollidable  *self);
    gboolean        (*get_collision_enabled)    (GrlCollidable  *self);
    void            (*set_collision_enabled)    (GrlCollidable  *self,
                                                 gboolean        enabled);
    void            (*on_collision)             (GrlCollidable  *self,
                                                 GrlCollidable  *other);
};

/**
 * grl_collidable_get_bounds:
 * @self: A #GrlCollidable
 *
 * Gets the collision bounding rectangle for this object.
 * The bounds should be in world coordinates.
 *
 * Returns: (transfer full): A new #GrlRectangle with the collision bounds
 */
GRL_AVAILABLE_IN_ALL
GrlRectangle *  grl_collidable_get_bounds           (GrlCollidable  *self);

/**
 * grl_collidable_get_collision_enabled:
 * @self: A #GrlCollidable
 *
 * Checks if collision detection is enabled for this object.
 *
 * Returns: %TRUE if collision is enabled
 */
GRL_AVAILABLE_IN_ALL
gboolean        grl_collidable_get_collision_enabled (GrlCollidable *self);

/**
 * grl_collidable_set_collision_enabled:
 * @self: A #GrlCollidable
 * @enabled: Whether collision should be enabled
 *
 * Enables or disables collision detection for this object.
 * Disabled objects will not be checked for collisions and
 * will not receive on_collision callbacks.
 */
GRL_AVAILABLE_IN_ALL
void            grl_collidable_set_collision_enabled (GrlCollidable *self,
                                                      gboolean       enabled);

/**
 * grl_collidable_on_collision:
 * @self: A #GrlCollidable
 * @other: The other #GrlCollidable that was hit
 *
 * Called when a collision is detected between this object
 * and another collidable object.
 *
 * Note: This is called on both objects involved in the collision.
 */
GRL_AVAILABLE_IN_ALL
void            grl_collidable_on_collision         (GrlCollidable  *self,
                                                     GrlCollidable  *other);

/**
 * grl_collidable_check_collision:
 * @self: A #GrlCollidable
 * @other: Another #GrlCollidable to check against
 *
 * Checks if two collidable objects are colliding.
 * This uses axis-aligned bounding box (AABB) collision detection.
 *
 * Returns: %TRUE if the objects are colliding
 */
GRL_AVAILABLE_IN_ALL
gboolean        grl_collidable_check_collision      (GrlCollidable  *self,
                                                     GrlCollidable  *other);

G_END_DECLS
