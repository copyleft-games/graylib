/* grl-ui-colorpicker.h
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * ColorPicker control for selecting colors with interactive UI.
 */

#pragma once

#if !defined(GRAYLIB_INSIDE) && !defined(GRAYLIB_COMPILATION)
#error "Only <graylib.h> can be included directly."
#endif

#include <glib-object.h>
#include "../grl-version.h"
#include "grl-ui-control.h"
#include "../math/grl-color.h"

G_BEGIN_DECLS

#define GRL_TYPE_UI_COLORPICKER (grl_ui_colorpicker_get_type ())

GRL_AVAILABLE_IN_ALL
G_DECLARE_FINAL_TYPE (GrlUiColorPicker, grl_ui_colorpicker, GRL, UI_COLORPICKER, GrlUiControl)

/*
 * Constructors
 */

/**
 * grl_ui_colorpicker_new:
 * @color: (nullable): Initial color, or %NULL for red
 *
 * Creates a new color picker control.
 *
 * Returns: (transfer full): A new #GrlUiColorPicker
 */
GRL_AVAILABLE_IN_ALL
GrlUiColorPicker *  grl_ui_colorpicker_new              (GrlColor       *color);

/**
 * grl_ui_colorpicker_new_with_bounds:
 * @x: X position
 * @y: Y position
 * @width: Width of the color picker
 * @height: Height of the color picker
 * @color: (nullable): Initial color, or %NULL for red
 *
 * Creates a new color picker control with specified bounds.
 *
 * Returns: (transfer full): A new #GrlUiColorPicker
 */
GRL_AVAILABLE_IN_ALL
GrlUiColorPicker *  grl_ui_colorpicker_new_with_bounds  (gfloat          x,
                                                         gfloat          y,
                                                         gfloat          width,
                                                         gfloat          height,
                                                         GrlColor       *color);

/*
 * Properties
 */

/**
 * grl_ui_colorpicker_get_color:
 * @self: A #GrlUiColorPicker
 *
 * Gets the currently selected color.
 *
 * Returns: (transfer full): The current color
 */
GRL_AVAILABLE_IN_ALL
GrlColor *          grl_ui_colorpicker_get_color        (GrlUiColorPicker *self);

/**
 * grl_ui_colorpicker_set_color:
 * @self: A #GrlUiColorPicker
 * @color: The color to set
 *
 * Sets the currently selected color.
 */
GRL_AVAILABLE_IN_ALL
void                grl_ui_colorpicker_set_color        (GrlUiColorPicker *self,
                                                         GrlColor         *color);

/**
 * grl_ui_colorpicker_get_text:
 * @self: A #GrlUiColorPicker
 *
 * Gets the label text displayed above the color picker.
 *
 * Returns: (transfer none) (nullable): The label text
 */
GRL_AVAILABLE_IN_ALL
const gchar *       grl_ui_colorpicker_get_text         (GrlUiColorPicker *self);

/**
 * grl_ui_colorpicker_set_text:
 * @self: A #GrlUiColorPicker
 * @text: (nullable): The label text to display
 *
 * Sets the label text displayed above the color picker.
 */
GRL_AVAILABLE_IN_ALL
void                grl_ui_colorpicker_set_text         (GrlUiColorPicker *self,
                                                         const gchar      *text);

/*
 * Methods
 */

/**
 * grl_ui_colorpicker_get_rgb:
 * @self: A #GrlUiColorPicker
 * @r: (out): Red component (0-255)
 * @g: (out): Green component (0-255)
 * @b: (out): Blue component (0-255)
 *
 * Gets the RGB components of the current color.
 */
GRL_AVAILABLE_IN_ALL
void                grl_ui_colorpicker_get_rgb          (GrlUiColorPicker *self,
                                                         guint8           *r,
                                                         guint8           *g,
                                                         guint8           *b);

/**
 * grl_ui_colorpicker_set_rgb:
 * @self: A #GrlUiColorPicker
 * @r: Red component (0-255)
 * @g: Green component (0-255)
 * @b: Blue component (0-255)
 *
 * Sets the current color using RGB components.
 */
GRL_AVAILABLE_IN_ALL
void                grl_ui_colorpicker_set_rgb          (GrlUiColorPicker *self,
                                                         guint8            r,
                                                         guint8            g,
                                                         guint8            b);

/**
 * grl_ui_colorpicker_get_rgba:
 * @self: A #GrlUiColorPicker
 * @r: (out): Red component (0-255)
 * @g: (out): Green component (0-255)
 * @b: (out): Blue component (0-255)
 * @a: (out): Alpha component (0-255)
 *
 * Gets the RGBA components of the current color.
 */
GRL_AVAILABLE_IN_ALL
void                grl_ui_colorpicker_get_rgba         (GrlUiColorPicker *self,
                                                         guint8           *r,
                                                         guint8           *g,
                                                         guint8           *b,
                                                         guint8           *a);

/**
 * grl_ui_colorpicker_set_rgba:
 * @self: A #GrlUiColorPicker
 * @r: Red component (0-255)
 * @g: Green component (0-255)
 * @b: Blue component (0-255)
 * @a: Alpha component (0-255)
 *
 * Sets the current color using RGBA components.
 */
GRL_AVAILABLE_IN_ALL
void                grl_ui_colorpicker_set_rgba         (GrlUiColorPicker *self,
                                                         guint8            r,
                                                         guint8            g,
                                                         guint8            b,
                                                         guint8            a);

G_END_DECLS
