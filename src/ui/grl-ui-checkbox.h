/* grl-ui-checkbox.h
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * CheckBox control for boolean options.
 */

#pragma once

#if !defined(GRAYLIB_INSIDE) && !defined(GRAYLIB_COMPILATION)
#error "Only <graylib.h> can be included directly."
#endif

#include <glib-object.h>
#include "../grl-version.h"
#include "grl-ui-control.h"

G_BEGIN_DECLS

#define GRL_TYPE_UI_CHECKBOX (grl_ui_checkbox_get_type ())

GRL_AVAILABLE_IN_ALL
G_DECLARE_FINAL_TYPE (GrlUiCheckBox, grl_ui_checkbox, GRL, UI_CHECKBOX, GrlUiControl)

/*
 * Constructors
 */

/**
 * grl_ui_checkbox_new:
 * @text: (nullable): The checkbox label text
 *
 * Creates a new checkbox control.
 *
 * Returns: (transfer full): A new #GrlUiCheckBox
 */
GRL_AVAILABLE_IN_ALL
GrlUiCheckBox *     grl_ui_checkbox_new             (const gchar    *text);

/**
 * grl_ui_checkbox_new_with_bounds:
 * @x: X position
 * @y: Y position
 * @width: Width of the checkbox
 * @height: Height of the checkbox
 * @text: (nullable): The checkbox label text
 *
 * Creates a new checkbox control with specified bounds.
 *
 * Returns: (transfer full): A new #GrlUiCheckBox
 */
GRL_AVAILABLE_IN_ALL
GrlUiCheckBox *     grl_ui_checkbox_new_with_bounds (gfloat          x,
                                                     gfloat          y,
                                                     gfloat          width,
                                                     gfloat          height,
                                                     const gchar    *text);

/*
 * Properties
 */

/**
 * grl_ui_checkbox_get_text:
 * @self: A #GrlUiCheckBox
 *
 * Gets the checkbox label text.
 *
 * Returns: (transfer none) (nullable): The checkbox text
 */
GRL_AVAILABLE_IN_ALL
const gchar *       grl_ui_checkbox_get_text        (GrlUiCheckBox  *self);

/**
 * grl_ui_checkbox_set_text:
 * @self: A #GrlUiCheckBox
 * @text: (nullable): The new text
 *
 * Sets the checkbox label text.
 */
GRL_AVAILABLE_IN_ALL
void                grl_ui_checkbox_set_text        (GrlUiCheckBox  *self,
                                                     const gchar    *text);

/**
 * grl_ui_checkbox_get_checked:
 * @self: A #GrlUiCheckBox
 *
 * Gets whether the checkbox is checked.
 *
 * Returns: %TRUE if checked, %FALSE otherwise
 */
GRL_AVAILABLE_IN_ALL
gboolean            grl_ui_checkbox_get_checked     (GrlUiCheckBox  *self);

/**
 * grl_ui_checkbox_set_checked:
 * @self: A #GrlUiCheckBox
 * @checked: Whether the checkbox should be checked
 *
 * Sets whether the checkbox is checked.
 */
GRL_AVAILABLE_IN_ALL
void                grl_ui_checkbox_set_checked     (GrlUiCheckBox  *self,
                                                     gboolean        checked);

/**
 * grl_ui_checkbox_toggle:
 * @self: A #GrlUiCheckBox
 *
 * Toggles the checkbox state.
 */
GRL_AVAILABLE_IN_ALL
void                grl_ui_checkbox_toggle          (GrlUiCheckBox  *self);

G_END_DECLS
