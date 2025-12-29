/* grl-ui-valuebox.h
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * ValueBox control for integer input.
 */

#pragma once

#if !defined(GRAYLIB_INSIDE) && !defined(GRAYLIB_COMPILATION)
#error "Only <graylib.h> can be included directly."
#endif

#include <glib-object.h>
#include "../grl-version.h"
#include "grl-ui-control.h"

G_BEGIN_DECLS

#define GRL_TYPE_UI_VALUEBOX (grl_ui_valuebox_get_type ())

GRL_AVAILABLE_IN_ALL
G_DECLARE_FINAL_TYPE (GrlUiValueBox, grl_ui_valuebox, GRL, UI_VALUEBOX, GrlUiControl)

/*
 * Constructors
 */

/**
 * grl_ui_valuebox_new:
 * @min_value: Minimum value
 * @max_value: Maximum value
 *
 * Creates a new value box control.
 *
 * Returns: (transfer full): A new #GrlUiValueBox
 */
GRL_AVAILABLE_IN_ALL
GrlUiValueBox *     grl_ui_valuebox_new             (gint            min_value,
                                                     gint            max_value);

/**
 * grl_ui_valuebox_new_with_bounds:
 * @x: X position
 * @y: Y position
 * @width: Width of the value box
 * @height: Height of the value box
 * @min_value: Minimum value
 * @max_value: Maximum value
 *
 * Creates a new value box control with specified bounds.
 *
 * Returns: (transfer full): A new #GrlUiValueBox
 */
GRL_AVAILABLE_IN_ALL
GrlUiValueBox *     grl_ui_valuebox_new_with_bounds (gfloat          x,
                                                     gfloat          y,
                                                     gfloat          width,
                                                     gfloat          height,
                                                     gint            min_value,
                                                     gint            max_value);

/*
 * Properties
 */

/**
 * grl_ui_valuebox_get_value:
 * @self: A #GrlUiValueBox
 *
 * Gets the current value.
 *
 * Returns: The current value
 */
GRL_AVAILABLE_IN_ALL
gint                grl_ui_valuebox_get_value       (GrlUiValueBox  *self);

/**
 * grl_ui_valuebox_set_value:
 * @self: A #GrlUiValueBox
 * @value: The new value
 *
 * Sets the current value.
 */
GRL_AVAILABLE_IN_ALL
void                grl_ui_valuebox_set_value       (GrlUiValueBox  *self,
                                                     gint            value);

/**
 * grl_ui_valuebox_get_min_value:
 * @self: A #GrlUiValueBox
 *
 * Gets the minimum value.
 *
 * Returns: The minimum value
 */
GRL_AVAILABLE_IN_ALL
gint                grl_ui_valuebox_get_min_value   (GrlUiValueBox  *self);

/**
 * grl_ui_valuebox_set_min_value:
 * @self: A #GrlUiValueBox
 * @min_value: The new minimum value
 *
 * Sets the minimum value.
 */
GRL_AVAILABLE_IN_ALL
void                grl_ui_valuebox_set_min_value   (GrlUiValueBox  *self,
                                                     gint            min_value);

/**
 * grl_ui_valuebox_get_max_value:
 * @self: A #GrlUiValueBox
 *
 * Gets the maximum value.
 *
 * Returns: The maximum value
 */
GRL_AVAILABLE_IN_ALL
gint                grl_ui_valuebox_get_max_value   (GrlUiValueBox  *self);

/**
 * grl_ui_valuebox_set_max_value:
 * @self: A #GrlUiValueBox
 * @max_value: The new maximum value
 *
 * Sets the maximum value.
 */
GRL_AVAILABLE_IN_ALL
void                grl_ui_valuebox_set_max_value   (GrlUiValueBox  *self,
                                                     gint            max_value);

/**
 * grl_ui_valuebox_get_text:
 * @self: A #GrlUiValueBox
 *
 * Gets the value box label text.
 *
 * Returns: (transfer none) (nullable): The label text
 */
GRL_AVAILABLE_IN_ALL
const gchar *       grl_ui_valuebox_get_text        (GrlUiValueBox  *self);

/**
 * grl_ui_valuebox_set_text:
 * @self: A #GrlUiValueBox
 * @text: (nullable): The new label text
 *
 * Sets the value box label text.
 */
GRL_AVAILABLE_IN_ALL
void                grl_ui_valuebox_set_text        (GrlUiValueBox  *self,
                                                     const gchar    *text);

/**
 * grl_ui_valuebox_get_edit_mode:
 * @self: A #GrlUiValueBox
 *
 * Gets whether the value box is in edit mode.
 *
 * Returns: %TRUE if in edit mode
 */
GRL_AVAILABLE_IN_ALL
gboolean            grl_ui_valuebox_get_edit_mode   (GrlUiValueBox  *self);

/**
 * grl_ui_valuebox_set_edit_mode:
 * @self: A #GrlUiValueBox
 * @edit_mode: Whether to enter edit mode
 *
 * Sets whether the value box is in edit mode.
 */
GRL_AVAILABLE_IN_ALL
void                grl_ui_valuebox_set_edit_mode   (GrlUiValueBox  *self,
                                                     gboolean        edit_mode);

G_END_DECLS
