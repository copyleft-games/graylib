/* grl-ui-button.h
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Button control that emits a signal when clicked.
 */

#pragma once

#if !defined(GRAYLIB_INSIDE) && !defined(GRAYLIB_COMPILATION)
#error "Only <graylib.h> can be included directly."
#endif

#include <glib-object.h>
#include "../grl-version.h"
#include "grl-ui-control.h"

G_BEGIN_DECLS

#define GRL_TYPE_UI_BUTTON (grl_ui_button_get_type ())

GRL_AVAILABLE_IN_ALL
G_DECLARE_FINAL_TYPE (GrlUiButton, grl_ui_button, GRL, UI_BUTTON, GrlUiControl)

/*
 * Constructors
 */

/**
 * grl_ui_button_new:
 * @text: (nullable): The button text
 *
 * Creates a new button control.
 *
 * Returns: (transfer full): A new #GrlUiButton
 */
GRL_AVAILABLE_IN_ALL
GrlUiButton *       grl_ui_button_new               (const gchar    *text);

/**
 * grl_ui_button_new_with_bounds:
 * @x: X position
 * @y: Y position
 * @width: Width of the button
 * @height: Height of the button
 * @text: (nullable): The button text
 *
 * Creates a new button control with specified bounds.
 *
 * Returns: (transfer full): A new #GrlUiButton
 */
GRL_AVAILABLE_IN_ALL
GrlUiButton *       grl_ui_button_new_with_bounds   (gfloat          x,
                                                     gfloat          y,
                                                     gfloat          width,
                                                     gfloat          height,
                                                     const gchar    *text);

/*
 * Properties
 */

/**
 * grl_ui_button_get_text:
 * @self: A #GrlUiButton
 *
 * Gets the button text.
 *
 * Returns: (transfer none) (nullable): The button text
 */
GRL_AVAILABLE_IN_ALL
const gchar *       grl_ui_button_get_text          (GrlUiButton    *self);

/**
 * grl_ui_button_set_text:
 * @self: A #GrlUiButton
 * @text: (nullable): The new text
 *
 * Sets the button text.
 */
GRL_AVAILABLE_IN_ALL
void                grl_ui_button_set_text          (GrlUiButton    *self,
                                                     const gchar    *text);

/**
 * grl_ui_button_get_icon:
 * @self: A #GrlUiButton
 *
 * Gets the button icon ID.
 *
 * Returns: The icon ID, or -1 if no icon
 */
GRL_AVAILABLE_IN_ALL
gint                grl_ui_button_get_icon          (GrlUiButton    *self);

/**
 * grl_ui_button_set_icon:
 * @self: A #GrlUiButton
 * @icon_id: The icon ID, or -1 for no icon
 *
 * Sets the button icon. The icon is prepended to the text.
 */
GRL_AVAILABLE_IN_ALL
void                grl_ui_button_set_icon          (GrlUiButton    *self,
                                                     gint            icon_id);

G_END_DECLS
