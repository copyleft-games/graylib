/* grl-drawable.h
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Interface for drawable objects.
 */

#pragma once

#if !defined(GRAYLIB_INSIDE) && !defined(GRAYLIB_COMPILATION)
#error "Only <graylib.h> can be included directly."
#endif

#include <glib-object.h>
#include "../grl-version.h"

G_BEGIN_DECLS

#define GRL_TYPE_DRAWABLE (grl_drawable_get_type ())

GRL_AVAILABLE_IN_ALL
G_DECLARE_INTERFACE (GrlDrawable, grl_drawable, GRL, DRAWABLE, GObject)

/**
 * GrlDrawableInterface:
 * @parent_iface: The parent interface
 * @draw: Virtual method to draw the object
 * @get_visible: Virtual method to check visibility
 * @set_visible: Virtual method to set visibility
 * @get_z_index: Virtual method to get the z-index (draw order)
 * @set_z_index: Virtual method to set the z-index
 *
 * Interface for objects that can be drawn to the screen.
 *
 * Objects implementing this interface can be added to scenes
 * and will be drawn automatically during the render phase.
 */
struct _GrlDrawableInterface
{
    GTypeInterface parent_iface;

    /* Virtual methods */
    void        (*draw)         (GrlDrawable    *self);
    gboolean    (*get_visible)  (GrlDrawable    *self);
    void        (*set_visible)  (GrlDrawable    *self,
                                 gboolean        visible);
    gint        (*get_z_index)  (GrlDrawable    *self);
    void        (*set_z_index)  (GrlDrawable    *self,
                                 gint            z_index);
};

/**
 * grl_drawable_draw:
 * @self: A #GrlDrawable
 *
 * Draws the object. This should only be called between
 * grl_window_begin_drawing() and grl_window_end_drawing().
 *
 * Objects with lower z-index values are drawn first (behind).
 */
GRL_AVAILABLE_IN_ALL
void            grl_drawable_draw           (GrlDrawable    *self);

/**
 * grl_drawable_get_visible:
 * @self: A #GrlDrawable
 *
 * Checks if the object is visible.
 *
 * Returns: %TRUE if the object is visible
 */
GRL_AVAILABLE_IN_ALL
gboolean        grl_drawable_get_visible    (GrlDrawable    *self);

/**
 * grl_drawable_set_visible:
 * @self: A #GrlDrawable
 * @visible: Whether the object should be visible
 *
 * Sets the visibility of the object. Invisible objects
 * are not drawn but may still be updated.
 */
GRL_AVAILABLE_IN_ALL
void            grl_drawable_set_visible    (GrlDrawable    *self,
                                             gboolean        visible);

/**
 * grl_drawable_get_z_index:
 * @self: A #GrlDrawable
 *
 * Gets the z-index (draw order) of the object.
 * Lower values are drawn first (behind higher values).
 *
 * Returns: The z-index
 */
GRL_AVAILABLE_IN_ALL
gint            grl_drawable_get_z_index    (GrlDrawable    *self);

/**
 * grl_drawable_set_z_index:
 * @self: A #GrlDrawable
 * @z_index: The z-index value
 *
 * Sets the z-index (draw order) of the object.
 * Lower values are drawn first (behind higher values).
 */
GRL_AVAILABLE_IN_ALL
void            grl_drawable_set_z_index    (GrlDrawable    *self,
                                             gint            z_index);

G_END_DECLS
