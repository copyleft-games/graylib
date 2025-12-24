/* grl-collidable.c
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Interface for collision-enabled objects.
 */

#include "config.h"
#include "grl-collidable.h"

/**
 * SECTION:grl-collidable
 * @title: GrlCollidable
 * @short_description: Interface for collision-enabled objects
 *
 * #GrlCollidable is an interface for objects that participate in
 * collision detection. It provides methods for getting bounding
 * rectangles and handling collision events.
 *
 * # Implementing GrlCollidable
 *
 * |[<!-- language="C" -->
 * static GrlRectangle *
 * my_enemy_get_bounds (GrlCollidable *collidable)
 * {
 *     MyEnemy *self = MY_ENEMY (collidable);
 *
 *     return grl_rectangle_new (self->x, self->y, self->width, self->height);
 * }
 *
 * static void
 * my_enemy_on_collision (GrlCollidable *collidable, GrlCollidable *other)
 * {
 *     MyEnemy *self = MY_ENEMY (collidable);
 *
 *     // Check what we hit and respond accordingly
 *     if (MY_IS_PROJECTILE (other))
 *         my_enemy_take_damage (self, 10);
 * }
 *
 * static void
 * my_enemy_collidable_init (GrlCollidableInterface *iface)
 * {
 *     iface->get_bounds = my_enemy_get_bounds;
 *     iface->on_collision = my_enemy_on_collision;
 * }
 *
 * G_DEFINE_TYPE_WITH_CODE (MyEnemy, my_enemy, GRL_TYPE_ENTITY,
 *     G_IMPLEMENT_INTERFACE (GRL_TYPE_COLLIDABLE, my_enemy_collidable_init))
 * ]|
 */

G_DEFINE_INTERFACE (GrlCollidable, grl_collidable, G_TYPE_OBJECT)

/* Quark for storing collision enabled state */
static GQuark collision_enabled_quark = 0;

static void
grl_collidable_default_init (GrlCollidableInterface *iface)
{
    collision_enabled_quark = g_quark_from_static_string ("grl-collidable-enabled");
}

/**
 * grl_collidable_get_bounds:
 * @self: A #GrlCollidable
 *
 * Gets the collision bounding rectangle for this object.
 *
 * Returns: (transfer full): A new #GrlRectangle with the collision bounds
 */
GrlRectangle *
grl_collidable_get_bounds (GrlCollidable *self)
{
    GrlCollidableInterface *iface;

    g_return_val_if_fail (GRL_IS_COLLIDABLE (self), NULL);

    iface = GRL_COLLIDABLE_GET_IFACE (self);

    if (iface->get_bounds != NULL)
        return iface->get_bounds (self);

    /* Default: empty rectangle */
    return grl_rectangle_new_empty ();
}

/**
 * grl_collidable_get_collision_enabled:
 * @self: A #GrlCollidable
 *
 * Checks if collision detection is enabled for this object.
 *
 * Returns: %TRUE if collision is enabled
 */
gboolean
grl_collidable_get_collision_enabled (GrlCollidable *self)
{
    GrlCollidableInterface *iface;
    gpointer data;

    g_return_val_if_fail (GRL_IS_COLLIDABLE (self), FALSE);

    iface = GRL_COLLIDABLE_GET_IFACE (self);

    if (iface->get_collision_enabled != NULL)
        return iface->get_collision_enabled (self);

    /* Default implementation using qdata */
    data = g_object_get_qdata (G_OBJECT (self), collision_enabled_quark);

    /* Default to enabled if not set */
    if (data == NULL)
        return TRUE;

    return GPOINTER_TO_INT (data) != 0;
}

/**
 * grl_collidable_set_collision_enabled:
 * @self: A #GrlCollidable
 * @enabled: Whether collision should be enabled
 *
 * Enables or disables collision detection for this object.
 */
void
grl_collidable_set_collision_enabled (GrlCollidable *self,
                                      gboolean       enabled)
{
    GrlCollidableInterface *iface;

    g_return_if_fail (GRL_IS_COLLIDABLE (self));

    iface = GRL_COLLIDABLE_GET_IFACE (self);

    if (iface->set_collision_enabled != NULL)
    {
        iface->set_collision_enabled (self, enabled);
        return;
    }

    /* Default implementation using qdata */
    /* Store as 1 or 2 to distinguish from NULL (unset) */
    g_object_set_qdata (G_OBJECT (self), collision_enabled_quark,
                        GINT_TO_POINTER (enabled ? 1 : 2));
}

/**
 * grl_collidable_on_collision:
 * @self: A #GrlCollidable
 * @other: The other #GrlCollidable that was hit
 *
 * Called when a collision is detected between this object
 * and another collidable object.
 */
void
grl_collidable_on_collision (GrlCollidable *self,
                             GrlCollidable *other)
{
    GrlCollidableInterface *iface;

    g_return_if_fail (GRL_IS_COLLIDABLE (self));
    g_return_if_fail (GRL_IS_COLLIDABLE (other));

    iface = GRL_COLLIDABLE_GET_IFACE (self);

    if (iface->on_collision != NULL)
        iface->on_collision (self, other);
}

/**
 * grl_collidable_check_collision:
 * @self: A #GrlCollidable
 * @other: Another #GrlCollidable to check against
 *
 * Checks if two collidable objects are colliding using AABB.
 *
 * Returns: %TRUE if the objects are colliding
 */
gboolean
grl_collidable_check_collision (GrlCollidable *self,
                                GrlCollidable *other)
{
    g_autoptr(GrlRectangle) bounds_a = NULL;
    g_autoptr(GrlRectangle) bounds_b = NULL;

    g_return_val_if_fail (GRL_IS_COLLIDABLE (self), FALSE);
    g_return_val_if_fail (GRL_IS_COLLIDABLE (other), FALSE);

    /* Check if both have collision enabled */
    if (!grl_collidable_get_collision_enabled (self))
        return FALSE;

    if (!grl_collidable_get_collision_enabled (other))
        return FALSE;

    /* Get bounds */
    bounds_a = grl_collidable_get_bounds (self);
    bounds_b = grl_collidable_get_bounds (other);

    if (bounds_a == NULL || bounds_b == NULL)
        return FALSE;

    /* AABB collision check */
    return grl_rectangle_intersects (bounds_a, bounds_b);
}
