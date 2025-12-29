/* grl-ui-spinner.h
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Spinner control for integer input with +/- buttons.
 */

#pragma once

#if !defined(GRAYLIB_INSIDE) && !defined(GRAYLIB_COMPILATION)
#error "Only <graylib.h> can be included directly."
#endif

#include <glib-object.h>
#include "../grl-version.h"
#include "grl-ui-control.h"

G_BEGIN_DECLS

#define GRL_TYPE_UI_SPINNER (grl_ui_spinner_get_type ())

GRL_AVAILABLE_IN_ALL
G_DECLARE_FINAL_TYPE (GrlUiSpinner, grl_ui_spinner, GRL, UI_SPINNER, GrlUiControl)

/*
 * Constructors
 */

/**
 * grl_ui_spinner_new:
 * @min_value: Minimum value
 * @max_value: Maximum value
 *
 * Creates a new spinner control.
 *
 * Returns: (transfer full): A new #GrlUiSpinner
 */
GRL_AVAILABLE_IN_ALL
GrlUiSpinner *      grl_ui_spinner_new              (gint            min_value,
                                                     gint            max_value);

/**
 * grl_ui_spinner_new_with_bounds:
 * @x: X position
 * @y: Y position
 * @width: Width of the spinner
 * @height: Height of the spinner
 * @min_value: Minimum value
 * @max_value: Maximum value
 *
 * Creates a new spinner control with specified bounds.
 *
 * Returns: (transfer full): A new #GrlUiSpinner
 */
GRL_AVAILABLE_IN_ALL
GrlUiSpinner *      grl_ui_spinner_new_with_bounds  (gfloat          x,
                                                     gfloat          y,
                                                     gfloat          width,
                                                     gfloat          height,
                                                     gint            min_value,
                                                     gint            max_value);

/*
 * Properties
 */

/**
 * grl_ui_spinner_get_value:
 * @self: A #GrlUiSpinner
 *
 * Gets the current value.
 *
 * Returns: The current value
 */
GRL_AVAILABLE_IN_ALL
gint                grl_ui_spinner_get_value        (GrlUiSpinner   *self);

/**
 * grl_ui_spinner_set_value:
 * @self: A #GrlUiSpinner
 * @value: The new value
 *
 * Sets the current value.
 */
GRL_AVAILABLE_IN_ALL
void                grl_ui_spinner_set_value        (GrlUiSpinner   *self,
                                                     gint            value);

/**
 * grl_ui_spinner_get_min_value:
 * @self: A #GrlUiSpinner
 *
 * Gets the minimum value.
 *
 * Returns: The minimum value
 */
GRL_AVAILABLE_IN_ALL
gint                grl_ui_spinner_get_min_value    (GrlUiSpinner   *self);

/**
 * grl_ui_spinner_set_min_value:
 * @self: A #GrlUiSpinner
 * @min_value: The new minimum value
 *
 * Sets the minimum value.
 */
GRL_AVAILABLE_IN_ALL
void                grl_ui_spinner_set_min_value    (GrlUiSpinner   *self,
                                                     gint            min_value);

/**
 * grl_ui_spinner_get_max_value:
 * @self: A #GrlUiSpinner
 *
 * Gets the maximum value.
 *
 * Returns: The maximum value
 */
GRL_AVAILABLE_IN_ALL
gint                grl_ui_spinner_get_max_value    (GrlUiSpinner   *self);

/**
 * grl_ui_spinner_set_max_value:
 * @self: A #GrlUiSpinner
 * @max_value: The new maximum value
 *
 * Sets the maximum value.
 */
GRL_AVAILABLE_IN_ALL
void                grl_ui_spinner_set_max_value    (GrlUiSpinner   *self,
                                                     gint            max_value);

/**
 * grl_ui_spinner_get_text:
 * @self: A #GrlUiSpinner
 *
 * Gets the spinner label text.
 *
 * Returns: (transfer none) (nullable): The label text
 */
GRL_AVAILABLE_IN_ALL
const gchar *       grl_ui_spinner_get_text         (GrlUiSpinner   *self);

/**
 * grl_ui_spinner_set_text:
 * @self: A #GrlUiSpinner
 * @text: (nullable): The new label text
 *
 * Sets the spinner label text.
 */
GRL_AVAILABLE_IN_ALL
void                grl_ui_spinner_set_text         (GrlUiSpinner   *self,
                                                     const gchar    *text);

/**
 * grl_ui_spinner_get_edit_mode:
 * @self: A #GrlUiSpinner
 *
 * Gets whether the spinner is in edit mode.
 *
 * Returns: %TRUE if in edit mode
 */
GRL_AVAILABLE_IN_ALL
gboolean            grl_ui_spinner_get_edit_mode    (GrlUiSpinner   *self);

/**
 * grl_ui_spinner_set_edit_mode:
 * @self: A #GrlUiSpinner
 * @edit_mode: Whether to enter edit mode
 *
 * Sets whether the spinner is in edit mode.
 */
GRL_AVAILABLE_IN_ALL
void                grl_ui_spinner_set_edit_mode    (GrlUiSpinner   *self,
                                                     gboolean        edit_mode);

/*
 * Methods
 */

/**
 * grl_ui_spinner_increment:
 * @self: A #GrlUiSpinner
 *
 * Increments the value by one (if not at max).
 */
GRL_AVAILABLE_IN_ALL
void                grl_ui_spinner_increment        (GrlUiSpinner   *self);

/**
 * grl_ui_spinner_decrement:
 * @self: A #GrlUiSpinner
 *
 * Decrements the value by one (if not at min).
 */
GRL_AVAILABLE_IN_ALL
void                grl_ui_spinner_decrement        (GrlUiSpinner   *self);

G_END_DECLS
