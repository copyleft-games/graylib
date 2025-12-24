/* grl-updatable.c
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Interface for objects that update each frame.
 */

#include "config.h"
#include "grl-updatable.h"

/**
 * SECTION:grl-updatable
 * @title: GrlUpdatable
 * @short_description: Interface for objects that update each frame
 *
 * #GrlUpdatable is an interface for objects that need to be updated
 * every frame. It provides a delta time parameter for frame-rate
 * independent behavior.
 *
 * # Implementing GrlUpdatable
 *
 * |[<!-- language="C" -->
 * static void
 * my_player_update (GrlUpdatable *updatable, gfloat delta)
 * {
 *     MyPlayer *self = MY_PLAYER (updatable);
 *
 *     // Frame-rate independent movement
 *     if (grl_input_is_key_down (GRL_KEY_RIGHT))
 *         self->x += self->speed * delta;
 *     if (grl_input_is_key_down (GRL_KEY_LEFT))
 *         self->x -= self->speed * delta;
 * }
 *
 * static void
 * my_player_updatable_init (GrlUpdatableInterface *iface)
 * {
 *     iface->update = my_player_update;
 * }
 *
 * G_DEFINE_TYPE_WITH_CODE (MyPlayer, my_player, GRL_TYPE_ENTITY,
 *     G_IMPLEMENT_INTERFACE (GRL_TYPE_UPDATABLE, my_player_updatable_init))
 * ]|
 */

G_DEFINE_INTERFACE (GrlUpdatable, grl_updatable, G_TYPE_OBJECT)

/* Quark for storing active state */
static GQuark active_quark = 0;

static void
grl_updatable_default_init (GrlUpdatableInterface *iface)
{
    active_quark = g_quark_from_static_string ("grl-updatable-active");
}

/**
 * grl_updatable_update:
 * @self: A #GrlUpdatable
 * @delta: Time elapsed since the last frame in seconds
 *
 * Updates the object's state.
 */
void
grl_updatable_update (GrlUpdatable *self,
                      gfloat        delta)
{
    GrlUpdatableInterface *iface;

    g_return_if_fail (GRL_IS_UPDATABLE (self));

    /* Check if active first */
    if (!grl_updatable_get_active (self))
        return;

    iface = GRL_UPDATABLE_GET_IFACE (self);

    if (iface->update != NULL)
        iface->update (self, delta);
}

/**
 * grl_updatable_get_active:
 * @self: A #GrlUpdatable
 *
 * Checks if updates are active for this object.
 *
 * Returns: %TRUE if updates are active
 */
gboolean
grl_updatable_get_active (GrlUpdatable *self)
{
    GrlUpdatableInterface *iface;
    gpointer data;

    g_return_val_if_fail (GRL_IS_UPDATABLE (self), FALSE);

    iface = GRL_UPDATABLE_GET_IFACE (self);

    if (iface->get_active != NULL)
        return iface->get_active (self);

    /* Default implementation using qdata */
    data = g_object_get_qdata (G_OBJECT (self), active_quark);

    /* Default to active if not set */
    if (data == NULL)
        return TRUE;

    return GPOINTER_TO_INT (data) != 0;
}

/**
 * grl_updatable_set_active:
 * @self: A #GrlUpdatable
 * @active: Whether updates should be active
 *
 * Enables or disables updates for this object.
 */
void
grl_updatable_set_active (GrlUpdatable *self,
                          gboolean      active)
{
    GrlUpdatableInterface *iface;

    g_return_if_fail (GRL_IS_UPDATABLE (self));

    iface = GRL_UPDATABLE_GET_IFACE (self);

    if (iface->set_active != NULL)
    {
        iface->set_active (self, active);
        return;
    }

    /* Default implementation using qdata */
    /* Store as 1 or 2 to distinguish from NULL (unset) */
    g_object_set_qdata (G_OBJECT (self), active_quark,
                        GINT_TO_POINTER (active ? 1 : 2));
}
