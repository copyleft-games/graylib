/* grl-drawable.c
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Interface for drawable objects.
 */

#include "config.h"
#include "grl-drawable.h"

/**
 * SECTION:grl-drawable
 * @title: GrlDrawable
 * @short_description: Interface for drawable objects
 *
 * #GrlDrawable is an interface for objects that can be drawn to the screen.
 * It provides a common API for visibility control and draw ordering via
 * z-index values.
 *
 * # Implementing GrlDrawable
 *
 * To implement this interface, you need to provide at minimum a draw()
 * implementation. The visibility and z-index methods have default
 * implementations that store state in qdata.
 *
 * |[<!-- language="C" -->
 * static void
 * my_object_draw (GrlDrawable *drawable)
 * {
 *     MyObject *self = MY_OBJECT (drawable);
 *
 *     // Draw the object
 *     grl_draw_rectangle (self->x, self->y, self->width, self->height, self->color);
 * }
 *
 * static void
 * my_object_drawable_init (GrlDrawableInterface *iface)
 * {
 *     iface->draw = my_object_draw;
 * }
 *
 * G_DEFINE_TYPE_WITH_CODE (MyObject, my_object, G_TYPE_OBJECT,
 *     G_IMPLEMENT_INTERFACE (GRL_TYPE_DRAWABLE, my_object_drawable_init))
 * ]|
 */

G_DEFINE_INTERFACE (GrlDrawable, grl_drawable, G_TYPE_OBJECT)

/* Quark for storing visibility state */
static GQuark visible_quark = 0;
static GQuark z_index_quark = 0;

static void
grl_drawable_default_init (GrlDrawableInterface *iface)
{
    visible_quark = g_quark_from_static_string ("grl-drawable-visible");
    z_index_quark = g_quark_from_static_string ("grl-drawable-z-index");
}

/**
 * grl_drawable_draw:
 * @self: A #GrlDrawable
 *
 * Draws the object. This should only be called between
 * grl_window_begin_drawing() and grl_window_end_drawing().
 */
void
grl_drawable_draw (GrlDrawable *self)
{
    GrlDrawableInterface *iface;

    g_return_if_fail (GRL_IS_DRAWABLE (self));

    iface = GRL_DRAWABLE_GET_IFACE (self);

    /* Check visibility first */
    if (!grl_drawable_get_visible (self))
        return;

    if (iface->draw != NULL)
        iface->draw (self);
}

/**
 * grl_drawable_get_visible:
 * @self: A #GrlDrawable
 *
 * Checks if the object is visible.
 *
 * Returns: %TRUE if the object is visible
 */
gboolean
grl_drawable_get_visible (GrlDrawable *self)
{
    GrlDrawableInterface *iface;
    gpointer data;

    g_return_val_if_fail (GRL_IS_DRAWABLE (self), FALSE);

    iface = GRL_DRAWABLE_GET_IFACE (self);

    if (iface->get_visible != NULL)
        return iface->get_visible (self);

    /* Default implementation using qdata */
    data = g_object_get_qdata (G_OBJECT (self), visible_quark);

    /* Default to visible if not set */
    if (data == NULL)
        return TRUE;

    return GPOINTER_TO_INT (data) != 0;
}

/**
 * grl_drawable_set_visible:
 * @self: A #GrlDrawable
 * @visible: Whether the object should be visible
 *
 * Sets the visibility of the object.
 */
void
grl_drawable_set_visible (GrlDrawable *self,
                          gboolean     visible)
{
    GrlDrawableInterface *iface;

    g_return_if_fail (GRL_IS_DRAWABLE (self));

    iface = GRL_DRAWABLE_GET_IFACE (self);

    if (iface->set_visible != NULL)
    {
        iface->set_visible (self, visible);
        return;
    }

    /* Default implementation using qdata */
    /* Store as 1 or 2 to distinguish from NULL (unset) */
    g_object_set_qdata (G_OBJECT (self), visible_quark,
                        GINT_TO_POINTER (visible ? 1 : 2));
}

/**
 * grl_drawable_get_z_index:
 * @self: A #GrlDrawable
 *
 * Gets the z-index (draw order) of the object.
 *
 * Returns: The z-index
 */
gint
grl_drawable_get_z_index (GrlDrawable *self)
{
    GrlDrawableInterface *iface;
    gpointer data;

    g_return_val_if_fail (GRL_IS_DRAWABLE (self), 0);

    iface = GRL_DRAWABLE_GET_IFACE (self);

    if (iface->get_z_index != NULL)
        return iface->get_z_index (self);

    /* Default implementation using qdata */
    data = g_object_get_qdata (G_OBJECT (self), z_index_quark);

    return GPOINTER_TO_INT (data);
}

/**
 * grl_drawable_set_z_index:
 * @self: A #GrlDrawable
 * @z_index: The z-index value
 *
 * Sets the z-index (draw order) of the object.
 */
void
grl_drawable_set_z_index (GrlDrawable *self,
                          gint         z_index)
{
    GrlDrawableInterface *iface;

    g_return_if_fail (GRL_IS_DRAWABLE (self));

    iface = GRL_DRAWABLE_GET_IFACE (self);

    if (iface->set_z_index != NULL)
    {
        iface->set_z_index (self, z_index);
        return;
    }

    /* Default implementation using qdata */
    g_object_set_qdata (G_OBJECT (self), z_index_quark,
                        GINT_TO_POINTER (z_index));
}
