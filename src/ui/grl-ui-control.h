/* grl-ui-control.h
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Base class for all UI controls.
 */

#pragma once

#if !defined(GRAYLIB_INSIDE) && !defined(GRAYLIB_COMPILATION)
#error "Only <graylib.h> can be included directly."
#endif

#include <glib-object.h>
#include "../grl-version.h"
#include "../math/grl-rectangle.h"
#include "grl-ui-enums.h"

G_BEGIN_DECLS

#define GRL_TYPE_UI_CONTROL (grl_ui_control_get_type ())

GRL_AVAILABLE_IN_ALL
G_DECLARE_DERIVABLE_TYPE (GrlUiControl, grl_ui_control, GRL, UI_CONTROL, GObject)

/**
 * GrlUiControlClass:
 * @parent_class: The parent class
 * @draw: Virtual method to draw the control
 * @handle_input: Virtual method to handle input (returns TRUE if input was handled)
 * @get_preferred_size: Virtual method to get the preferred size of the control
 *
 * The class structure for #GrlUiControl.
 */
struct _GrlUiControlClass
{
    GObjectClass parent_class;

    /* Virtual methods */
    void        (*draw)                 (GrlUiControl   *self);
    gboolean    (*handle_input)         (GrlUiControl   *self);
    void        (*get_preferred_size)   (GrlUiControl   *self,
                                         gfloat         *width,
                                         gfloat         *height);

    /*< private >*/
    gpointer _reserved[8];
};

/*
 * Properties
 */

/**
 * grl_ui_control_get_bounds:
 * @self: A #GrlUiControl
 *
 * Gets the bounds (position and size) of the control.
 *
 * Returns: (transfer full): The control bounds as a #GrlRectangle
 */
GRL_AVAILABLE_IN_ALL
GrlRectangle *      grl_ui_control_get_bounds       (GrlUiControl   *self);

/**
 * grl_ui_control_set_bounds:
 * @self: A #GrlUiControl
 * @bounds: The new bounds
 *
 * Sets the bounds (position and size) of the control.
 */
GRL_AVAILABLE_IN_ALL
void                grl_ui_control_set_bounds       (GrlUiControl   *self,
                                                     GrlRectangle   *bounds);

/**
 * grl_ui_control_get_x:
 * @self: A #GrlUiControl
 *
 * Gets the X position of the control.
 *
 * Returns: The X position
 */
GRL_AVAILABLE_IN_ALL
gfloat              grl_ui_control_get_x            (GrlUiControl   *self);

/**
 * grl_ui_control_set_x:
 * @self: A #GrlUiControl
 * @x: The new X position
 *
 * Sets the X position of the control.
 */
GRL_AVAILABLE_IN_ALL
void                grl_ui_control_set_x            (GrlUiControl   *self,
                                                     gfloat          x);

/**
 * grl_ui_control_get_y:
 * @self: A #GrlUiControl
 *
 * Gets the Y position of the control.
 *
 * Returns: The Y position
 */
GRL_AVAILABLE_IN_ALL
gfloat              grl_ui_control_get_y            (GrlUiControl   *self);

/**
 * grl_ui_control_set_y:
 * @self: A #GrlUiControl
 * @y: The new Y position
 *
 * Sets the Y position of the control.
 */
GRL_AVAILABLE_IN_ALL
void                grl_ui_control_set_y            (GrlUiControl   *self,
                                                     gfloat          y);

/**
 * grl_ui_control_get_width:
 * @self: A #GrlUiControl
 *
 * Gets the width of the control.
 *
 * Returns: The width
 */
GRL_AVAILABLE_IN_ALL
gfloat              grl_ui_control_get_width        (GrlUiControl   *self);

/**
 * grl_ui_control_set_width:
 * @self: A #GrlUiControl
 * @width: The new width
 *
 * Sets the width of the control.
 */
GRL_AVAILABLE_IN_ALL
void                grl_ui_control_set_width        (GrlUiControl   *self,
                                                     gfloat          width);

/**
 * grl_ui_control_get_height:
 * @self: A #GrlUiControl
 *
 * Gets the height of the control.
 *
 * Returns: The height
 */
GRL_AVAILABLE_IN_ALL
gfloat              grl_ui_control_get_height       (GrlUiControl   *self);

/**
 * grl_ui_control_set_height:
 * @self: A #GrlUiControl
 * @height: The new height
 *
 * Sets the height of the control.
 */
GRL_AVAILABLE_IN_ALL
void                grl_ui_control_set_height       (GrlUiControl   *self,
                                                     gfloat          height);

/**
 * grl_ui_control_get_enabled:
 * @self: A #GrlUiControl
 *
 * Gets whether the control is enabled (accepts input).
 *
 * Returns: %TRUE if the control is enabled
 */
GRL_AVAILABLE_IN_ALL
gboolean            grl_ui_control_get_enabled      (GrlUiControl   *self);

/**
 * grl_ui_control_set_enabled:
 * @self: A #GrlUiControl
 * @enabled: Whether the control should be enabled
 *
 * Sets whether the control is enabled (accepts input).
 */
GRL_AVAILABLE_IN_ALL
void                grl_ui_control_set_enabled      (GrlUiControl   *self,
                                                     gboolean        enabled);

/**
 * grl_ui_control_get_visible:
 * @self: A #GrlUiControl
 *
 * Gets whether the control is visible (is drawn).
 *
 * Returns: %TRUE if the control is visible
 */
GRL_AVAILABLE_IN_ALL
gboolean            grl_ui_control_get_visible      (GrlUiControl   *self);

/**
 * grl_ui_control_set_visible:
 * @self: A #GrlUiControl
 * @visible: Whether the control should be visible
 *
 * Sets whether the control is visible (is drawn).
 */
GRL_AVAILABLE_IN_ALL
void                grl_ui_control_set_visible      (GrlUiControl   *self,
                                                     gboolean        visible);

/**
 * grl_ui_control_get_state:
 * @self: A #GrlUiControl
 *
 * Gets the current visual state of the control.
 *
 * Returns: The current #GrlUiState
 */
GRL_AVAILABLE_IN_ALL
GrlUiState          grl_ui_control_get_state        (GrlUiControl   *self);

/**
 * grl_ui_control_get_tooltip:
 * @self: A #GrlUiControl
 *
 * Gets the tooltip text for the control.
 *
 * Returns: (nullable) (transfer none): The tooltip text, or %NULL
 */
GRL_AVAILABLE_IN_ALL
const gchar *       grl_ui_control_get_tooltip      (GrlUiControl   *self);

/**
 * grl_ui_control_set_tooltip:
 * @self: A #GrlUiControl
 * @tooltip: (nullable): The tooltip text, or %NULL
 *
 * Sets the tooltip text for the control.
 */
GRL_AVAILABLE_IN_ALL
void                grl_ui_control_set_tooltip      (GrlUiControl   *self,
                                                     const gchar    *tooltip);

/*
 * Methods
 */

/**
 * grl_ui_control_draw:
 * @self: A #GrlUiControl
 *
 * Draws the control. This calls the virtual draw() method
 * if the control is visible.
 */
GRL_AVAILABLE_IN_ALL
void                grl_ui_control_draw             (GrlUiControl   *self);

/**
 * grl_ui_control_handle_input:
 * @self: A #GrlUiControl
 *
 * Processes input for the control. This calls the virtual
 * handle_input() method if the control is enabled.
 *
 * Returns: %TRUE if the input was handled
 */
GRL_AVAILABLE_IN_ALL
gboolean            grl_ui_control_handle_input     (GrlUiControl   *self);

/**
 * grl_ui_control_get_preferred_size:
 * @self: A #GrlUiControl
 * @width: (out) (nullable): Return location for preferred width
 * @height: (out) (nullable): Return location for preferred height
 *
 * Gets the preferred size of the control based on its content.
 */
GRL_AVAILABLE_IN_ALL
void                grl_ui_control_get_preferred_size (GrlUiControl *self,
                                                       gfloat       *width,
                                                       gfloat       *height);

/**
 * grl_ui_control_contains_point:
 * @self: A #GrlUiControl
 * @x: X coordinate to test
 * @y: Y coordinate to test
 *
 * Tests whether a point is within the control's bounds.
 *
 * Returns: %TRUE if the point is inside the control
 */
GRL_AVAILABLE_IN_ALL
gboolean            grl_ui_control_contains_point   (GrlUiControl   *self,
                                                     gfloat          x,
                                                     gfloat          y);

G_END_DECLS
