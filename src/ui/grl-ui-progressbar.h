/* grl-ui-progressbar.h
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * ProgressBar control for displaying progress.
 */

#pragma once

#if !defined(GRAYLIB_INSIDE) && !defined(GRAYLIB_COMPILATION)
#error "Only <graylib.h> can be included directly."
#endif

#include <glib-object.h>
#include "../grl-version.h"
#include "grl-ui-control.h"

G_BEGIN_DECLS

#define GRL_TYPE_UI_PROGRESSBAR (grl_ui_progressbar_get_type ())

GRL_AVAILABLE_IN_ALL
G_DECLARE_FINAL_TYPE (GrlUiProgressBar, grl_ui_progressbar, GRL, UI_PROGRESSBAR, GrlUiControl)

/*
 * Constructors
 */

/**
 * grl_ui_progressbar_new:
 * @min_value: Minimum value
 * @max_value: Maximum value
 *
 * Creates a new progress bar control.
 *
 * Returns: (transfer full): A new #GrlUiProgressBar
 */
GRL_AVAILABLE_IN_ALL
GrlUiProgressBar *  grl_ui_progressbar_new              (gfloat          min_value,
                                                         gfloat          max_value);

/**
 * grl_ui_progressbar_new_with_bounds:
 * @x: X position
 * @y: Y position
 * @width: Width of the progress bar
 * @height: Height of the progress bar
 * @min_value: Minimum value
 * @max_value: Maximum value
 *
 * Creates a new progress bar control with specified bounds.
 *
 * Returns: (transfer full): A new #GrlUiProgressBar
 */
GRL_AVAILABLE_IN_ALL
GrlUiProgressBar *  grl_ui_progressbar_new_with_bounds  (gfloat          x,
                                                         gfloat          y,
                                                         gfloat          width,
                                                         gfloat          height,
                                                         gfloat          min_value,
                                                         gfloat          max_value);

/*
 * Properties
 */

/**
 * grl_ui_progressbar_get_value:
 * @self: A #GrlUiProgressBar
 *
 * Gets the current progress value.
 *
 * Returns: The current value
 */
GRL_AVAILABLE_IN_ALL
gfloat              grl_ui_progressbar_get_value        (GrlUiProgressBar *self);

/**
 * grl_ui_progressbar_set_value:
 * @self: A #GrlUiProgressBar
 * @value: The new value
 *
 * Sets the current progress value.
 */
GRL_AVAILABLE_IN_ALL
void                grl_ui_progressbar_set_value        (GrlUiProgressBar *self,
                                                         gfloat            value);

/**
 * grl_ui_progressbar_get_min_value:
 * @self: A #GrlUiProgressBar
 *
 * Gets the minimum value.
 *
 * Returns: The minimum value
 */
GRL_AVAILABLE_IN_ALL
gfloat              grl_ui_progressbar_get_min_value    (GrlUiProgressBar *self);

/**
 * grl_ui_progressbar_set_min_value:
 * @self: A #GrlUiProgressBar
 * @min_value: The new minimum value
 *
 * Sets the minimum value.
 */
GRL_AVAILABLE_IN_ALL
void                grl_ui_progressbar_set_min_value    (GrlUiProgressBar *self,
                                                         gfloat            min_value);

/**
 * grl_ui_progressbar_get_max_value:
 * @self: A #GrlUiProgressBar
 *
 * Gets the maximum value.
 *
 * Returns: The maximum value
 */
GRL_AVAILABLE_IN_ALL
gfloat              grl_ui_progressbar_get_max_value    (GrlUiProgressBar *self);

/**
 * grl_ui_progressbar_set_max_value:
 * @self: A #GrlUiProgressBar
 * @max_value: The new maximum value
 *
 * Sets the maximum value.
 */
GRL_AVAILABLE_IN_ALL
void                grl_ui_progressbar_set_max_value    (GrlUiProgressBar *self,
                                                         gfloat            max_value);

/**
 * grl_ui_progressbar_get_text_left:
 * @self: A #GrlUiProgressBar
 *
 * Gets the left text label.
 *
 * Returns: (transfer none) (nullable): The left text label
 */
GRL_AVAILABLE_IN_ALL
const gchar *       grl_ui_progressbar_get_text_left    (GrlUiProgressBar *self);

/**
 * grl_ui_progressbar_set_text_left:
 * @self: A #GrlUiProgressBar
 * @text: (nullable): The new left text label
 *
 * Sets the left text label.
 */
GRL_AVAILABLE_IN_ALL
void                grl_ui_progressbar_set_text_left    (GrlUiProgressBar *self,
                                                         const gchar      *text);

/**
 * grl_ui_progressbar_get_text_right:
 * @self: A #GrlUiProgressBar
 *
 * Gets the right text label.
 *
 * Returns: (transfer none) (nullable): The right text label
 */
GRL_AVAILABLE_IN_ALL
const gchar *       grl_ui_progressbar_get_text_right   (GrlUiProgressBar *self);

/**
 * grl_ui_progressbar_set_text_right:
 * @self: A #GrlUiProgressBar
 * @text: (nullable): The new right text label
 *
 * Sets the right text label.
 */
GRL_AVAILABLE_IN_ALL
void                grl_ui_progressbar_set_text_right   (GrlUiProgressBar *self,
                                                         const gchar      *text);

/*
 * Methods
 */

/**
 * grl_ui_progressbar_get_progress:
 * @self: A #GrlUiProgressBar
 *
 * Gets the progress as a percentage (0.0 to 1.0).
 *
 * Returns: The progress percentage
 */
GRL_AVAILABLE_IN_ALL
gfloat              grl_ui_progressbar_get_progress     (GrlUiProgressBar *self);

/**
 * grl_ui_progressbar_set_progress:
 * @self: A #GrlUiProgressBar
 * @progress: The progress as a percentage (0.0 to 1.0)
 *
 * Sets the progress as a percentage, mapping to the min/max range.
 */
GRL_AVAILABLE_IN_ALL
void                grl_ui_progressbar_set_progress     (GrlUiProgressBar *self,
                                                         gfloat            progress);

G_END_DECLS
