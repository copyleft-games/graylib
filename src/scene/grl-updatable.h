/* grl-updatable.h
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Interface for objects that update each frame.
 */

#pragma once

#if !defined(GRAYLIB_INSIDE) && !defined(GRAYLIB_COMPILATION)
#error "Only <graylib.h> can be included directly."
#endif

#include <glib-object.h>
#include "../grl-version.h"

G_BEGIN_DECLS

#define GRL_TYPE_UPDATABLE (grl_updatable_get_type ())

GRL_AVAILABLE_IN_ALL
G_DECLARE_INTERFACE (GrlUpdatable, grl_updatable, GRL, UPDATABLE, GObject)

/**
 * GrlUpdatableInterface:
 * @parent_iface: The parent interface
 * @update: Virtual method to update the object
 * @get_active: Virtual method to check if updates are active
 * @set_active: Virtual method to enable/disable updates
 *
 * Interface for objects that need to update each frame.
 *
 * Objects implementing this interface receive update calls
 * with the delta time (time since last frame) allowing for
 * frame-rate independent behavior.
 */
struct _GrlUpdatableInterface
{
    GTypeInterface parent_iface;

    /* Virtual methods */
    void        (*update)       (GrlUpdatable   *self,
                                 gfloat          delta);
    gboolean    (*get_active)   (GrlUpdatable   *self);
    void        (*set_active)   (GrlUpdatable   *self,
                                 gboolean        active);
};

/**
 * grl_updatable_update:
 * @self: A #GrlUpdatable
 * @delta: Time elapsed since the last frame in seconds
 *
 * Updates the object's state. This is typically called once
 * per frame before drawing.
 *
 * The delta time allows for frame-rate independent movement
 * and animation. For example:
 *
 * |[<!-- language="C" -->
 * // Move 100 pixels per second regardless of frame rate
 * position.x += velocity * delta;
 * ]|
 */
GRL_AVAILABLE_IN_ALL
void            grl_updatable_update        (GrlUpdatable   *self,
                                             gfloat          delta);

/**
 * grl_updatable_get_active:
 * @self: A #GrlUpdatable
 *
 * Checks if updates are active for this object.
 * Inactive objects will not have their update() method called.
 *
 * Returns: %TRUE if updates are active
 */
GRL_AVAILABLE_IN_ALL
gboolean        grl_updatable_get_active    (GrlUpdatable   *self);

/**
 * grl_updatable_set_active:
 * @self: A #GrlUpdatable
 * @active: Whether updates should be active
 *
 * Enables or disables updates for this object.
 * This is useful for pausing entities without removing them.
 */
GRL_AVAILABLE_IN_ALL
void            grl_updatable_set_active    (GrlUpdatable   *self,
                                             gboolean        active);

G_END_DECLS
