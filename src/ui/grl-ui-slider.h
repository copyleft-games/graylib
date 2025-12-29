/* grl-ui-slider.h
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Slider control for selecting a value within a range.
 */

#pragma once

#if !defined(GRAYLIB_INSIDE) && !defined(GRAYLIB_COMPILATION)
#error "Only <graylib.h> can be included directly."
#endif

#include <glib-object.h>
#include "../grl-version.h"
#include "grl-ui-control.h"

G_BEGIN_DECLS

#define GRL_TYPE_UI_SLIDER (grl_ui_slider_get_type ())

GRL_AVAILABLE_IN_ALL
G_DECLARE_FINAL_TYPE (GrlUiSlider, grl_ui_slider, GRL, UI_SLIDER, GrlUiControl)

/*
 * Constructors
 */

/**
 * grl_ui_slider_new:
 * @min_value: Minimum value
 * @max_value: Maximum value
 *
 * Creates a new slider control.
 *
 * Returns: (transfer full): A new #GrlUiSlider
 */
GRL_AVAILABLE_IN_ALL
GrlUiSlider *       grl_ui_slider_new               (gfloat          min_value,
                                                     gfloat          max_value);

/**
 * grl_ui_slider_new_with_bounds:
 * @x: X position
 * @y: Y position
 * @width: Width of the slider
 * @height: Height of the slider
 * @min_value: Minimum value
 * @max_value: Maximum value
 *
 * Creates a new slider control with specified bounds.
 *
 * Returns: (transfer full): A new #GrlUiSlider
 */
GRL_AVAILABLE_IN_ALL
GrlUiSlider *       grl_ui_slider_new_with_bounds   (gfloat          x,
                                                     gfloat          y,
                                                     gfloat          width,
                                                     gfloat          height,
                                                     gfloat          min_value,
                                                     gfloat          max_value);

/*
 * Properties
 */

/**
 * grl_ui_slider_get_value:
 * @self: A #GrlUiSlider
 *
 * Gets the current slider value.
 *
 * Returns: The current value
 */
GRL_AVAILABLE_IN_ALL
gfloat              grl_ui_slider_get_value         (GrlUiSlider    *self);

/**
 * grl_ui_slider_set_value:
 * @self: A #GrlUiSlider
 * @value: The new value
 *
 * Sets the slider value. The value will be clamped to the
 * min/max range.
 */
GRL_AVAILABLE_IN_ALL
void                grl_ui_slider_set_value         (GrlUiSlider    *self,
                                                     gfloat          value);

/**
 * grl_ui_slider_get_min_value:
 * @self: A #GrlUiSlider
 *
 * Gets the minimum slider value.
 *
 * Returns: The minimum value
 */
GRL_AVAILABLE_IN_ALL
gfloat              grl_ui_slider_get_min_value     (GrlUiSlider    *self);

/**
 * grl_ui_slider_set_min_value:
 * @self: A #GrlUiSlider
 * @min_value: The new minimum value
 *
 * Sets the minimum slider value.
 */
GRL_AVAILABLE_IN_ALL
void                grl_ui_slider_set_min_value     (GrlUiSlider    *self,
                                                     gfloat          min_value);

/**
 * grl_ui_slider_get_max_value:
 * @self: A #GrlUiSlider
 *
 * Gets the maximum slider value.
 *
 * Returns: The maximum value
 */
GRL_AVAILABLE_IN_ALL
gfloat              grl_ui_slider_get_max_value     (GrlUiSlider    *self);

/**
 * grl_ui_slider_set_max_value:
 * @self: A #GrlUiSlider
 * @max_value: The new maximum value
 *
 * Sets the maximum slider value.
 */
GRL_AVAILABLE_IN_ALL
void                grl_ui_slider_set_max_value     (GrlUiSlider    *self,
                                                     gfloat          max_value);

/**
 * grl_ui_slider_get_text_left:
 * @self: A #GrlUiSlider
 *
 * Gets the left label text.
 *
 * Returns: (transfer none) (nullable): The left label text
 */
GRL_AVAILABLE_IN_ALL
const gchar *       grl_ui_slider_get_text_left     (GrlUiSlider    *self);

/**
 * grl_ui_slider_set_text_left:
 * @self: A #GrlUiSlider
 * @text: (nullable): The left label text
 *
 * Sets the left label text.
 */
GRL_AVAILABLE_IN_ALL
void                grl_ui_slider_set_text_left     (GrlUiSlider    *self,
                                                     const gchar    *text);

/**
 * grl_ui_slider_get_text_right:
 * @self: A #GrlUiSlider
 *
 * Gets the right label text.
 *
 * Returns: (transfer none) (nullable): The right label text
 */
GRL_AVAILABLE_IN_ALL
const gchar *       grl_ui_slider_get_text_right    (GrlUiSlider    *self);

/**
 * grl_ui_slider_set_text_right:
 * @self: A #GrlUiSlider
 * @text: (nullable): The right label text
 *
 * Sets the right label text.
 */
GRL_AVAILABLE_IN_ALL
void                grl_ui_slider_set_text_right    (GrlUiSlider    *self,
                                                     const gchar    *text);

/**
 * grl_ui_slider_get_show_bar:
 * @self: A #GrlUiSlider
 *
 * Gets whether the slider shows as a bar (filled from left).
 *
 * Returns: %TRUE if showing as bar
 */
GRL_AVAILABLE_IN_ALL
gboolean            grl_ui_slider_get_show_bar      (GrlUiSlider    *self);

/**
 * grl_ui_slider_set_show_bar:
 * @self: A #GrlUiSlider
 * @show_bar: Whether to show as a bar
 *
 * Sets whether the slider shows as a bar (filled from left).
 */
GRL_AVAILABLE_IN_ALL
void                grl_ui_slider_set_show_bar      (GrlUiSlider    *self,
                                                     gboolean        show_bar);

G_END_DECLS
