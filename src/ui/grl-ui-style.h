/* grl-ui-style.h
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * UI style manager singleton for theming controls.
 */

#pragma once

#if !defined(GRAYLIB_INSIDE) && !defined(GRAYLIB_COMPILATION)
#error "Only <graylib.h> can be included directly."
#endif

#include <glib-object.h>
#include "../grl-version.h"
#include "grl-ui-enums.h"

G_BEGIN_DECLS

#define GRL_TYPE_UI_STYLE (grl_ui_style_get_type ())

GRL_AVAILABLE_IN_ALL
G_DECLARE_FINAL_TYPE (GrlUiStyle, grl_ui_style, GRL, UI_STYLE, GObject)

/**
 * grl_ui_style_get_default:
 *
 * Gets the default UI style singleton instance.
 *
 * This is the global style manager that controls the appearance
 * of all UI controls. Changes made to this instance affect all
 * controls immediately.
 *
 * Returns: (transfer none): The default #GrlUiStyle instance
 */
GRL_AVAILABLE_IN_ALL
GrlUiStyle *        grl_ui_style_get_default        (void);

/**
 * grl_ui_style_load_file:
 * @self: A #GrlUiStyle
 * @filename: (type filename): Path to a .rgs style file
 * @error: (nullable): Return location for error, or %NULL
 *
 * Loads a raygui style file (.rgs format).
 *
 * Style files can define colors, fonts, and other visual properties
 * for all control types.
 *
 * Returns: %TRUE on success, %FALSE on error
 */
GRL_AVAILABLE_IN_ALL
gboolean            grl_ui_style_load_file          (GrlUiStyle     *self,
                                                     const gchar    *filename,
                                                     GError        **error);

/**
 * grl_ui_style_load_default:
 * @self: A #GrlUiStyle
 *
 * Resets the style to the default raygui theme.
 *
 * This restores all style properties to their built-in default values.
 */
GRL_AVAILABLE_IN_ALL
void                grl_ui_style_load_default       (GrlUiStyle     *self);

/**
 * grl_ui_style_set_control_property:
 * @self: A #GrlUiStyle
 * @control: The control type (use #GrlUiControlType or %GRL_UI_CONTROL_DEFAULT)
 * @property: The property to set (use #GrlUiProperty or #GrlUiDefaultProperty)
 * @value: The value to set
 *
 * Sets a style property for a specific control type.
 *
 * Use %GRL_UI_CONTROL_DEFAULT to set properties that apply to all controls.
 * For color properties, the value should be a 32-bit RGBA color packed as
 * an integer (use grl_color_to_int()).
 */
GRL_AVAILABLE_IN_ALL
void                grl_ui_style_set_control_property (GrlUiStyle   *self,
                                                       gint          control,
                                                       gint          property,
                                                       gint          value);

/**
 * grl_ui_style_get_control_property:
 * @self: A #GrlUiStyle
 * @control: The control type (use #GrlUiControlType or %GRL_UI_CONTROL_DEFAULT)
 * @property: The property to get (use #GrlUiProperty or #GrlUiDefaultProperty)
 *
 * Gets a style property for a specific control type.
 *
 * Returns: The property value
 */
GRL_AVAILABLE_IN_ALL
gint                grl_ui_style_get_control_property (GrlUiStyle   *self,
                                                       gint          control,
                                                       gint          property);

/**
 * grl_ui_style_set_font:
 * @self: A #GrlUiStyle
 * @font: (nullable): The font to use, or %NULL for default
 *
 * Sets the font used for all UI controls.
 *
 * Note: The font must remain valid for the lifetime of the style.
 */
GRL_AVAILABLE_IN_ALL
void                grl_ui_style_set_font           (GrlUiStyle     *self,
                                                     gpointer        font);

/**
 * grl_ui_style_enable:
 * @self: A #GrlUiStyle
 *
 * Enables the UI state (required before drawing controls).
 *
 * This is automatically called by #GrlUiControl before drawing,
 * but can be called manually if using raw raygui functions.
 */
GRL_AVAILABLE_IN_ALL
void                grl_ui_style_enable             (GrlUiStyle     *self);

/**
 * grl_ui_style_disable:
 * @self: A #GrlUiStyle
 *
 * Disables the UI state.
 *
 * This puts the UI into a disabled state where controls don't respond
 * to input and are drawn with disabled colors.
 */
GRL_AVAILABLE_IN_ALL
void                grl_ui_style_disable            (GrlUiStyle     *self);

/**
 * grl_ui_style_lock:
 * @self: A #GrlUiStyle
 *
 * Locks the UI from processing input.
 *
 * While locked, controls are drawn normally but do not respond to input.
 */
GRL_AVAILABLE_IN_ALL
void                grl_ui_style_lock               (GrlUiStyle     *self);

/**
 * grl_ui_style_unlock:
 * @self: A #GrlUiStyle
 *
 * Unlocks the UI to resume processing input.
 */
GRL_AVAILABLE_IN_ALL
void                grl_ui_style_unlock             (GrlUiStyle     *self);

/**
 * grl_ui_style_is_locked:
 * @self: A #GrlUiStyle
 *
 * Checks if the UI is locked.
 *
 * Returns: %TRUE if the UI is locked
 */
GRL_AVAILABLE_IN_ALL
gboolean            grl_ui_style_is_locked          (GrlUiStyle     *self);

/**
 * grl_ui_style_set_alpha:
 * @self: A #GrlUiStyle
 * @alpha: Alpha value (0.0 = transparent, 1.0 = opaque)
 *
 * Sets the global transparency for all UI controls.
 */
GRL_AVAILABLE_IN_ALL
void                grl_ui_style_set_alpha          (GrlUiStyle     *self,
                                                     gfloat          alpha);

/**
 * grl_ui_style_get_alpha:
 * @self: A #GrlUiStyle
 *
 * Gets the global transparency for UI controls.
 *
 * Returns: The current alpha value
 */
GRL_AVAILABLE_IN_ALL
gfloat              grl_ui_style_get_alpha          (GrlUiStyle     *self);

G_END_DECLS
