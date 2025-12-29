/* grl-ui-textbox.h
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * TextBox control for text input.
 */

#pragma once

#if !defined(GRAYLIB_INSIDE) && !defined(GRAYLIB_COMPILATION)
#error "Only <graylib.h> can be included directly."
#endif

#include <glib-object.h>
#include "../grl-version.h"
#include "grl-ui-control.h"

G_BEGIN_DECLS

#define GRL_TYPE_UI_TEXTBOX (grl_ui_textbox_get_type ())

GRL_AVAILABLE_IN_ALL
G_DECLARE_FINAL_TYPE (GrlUiTextBox, grl_ui_textbox, GRL, UI_TEXTBOX, GrlUiControl)

/*
 * Constructors
 */

/**
 * grl_ui_textbox_new:
 * @max_length: Maximum number of characters
 *
 * Creates a new textbox control.
 *
 * Returns: (transfer full): A new #GrlUiTextBox
 */
GRL_AVAILABLE_IN_ALL
GrlUiTextBox *      grl_ui_textbox_new              (gint            max_length);

/**
 * grl_ui_textbox_new_with_bounds:
 * @x: X position
 * @y: Y position
 * @width: Width of the textbox
 * @height: Height of the textbox
 * @max_length: Maximum number of characters
 *
 * Creates a new textbox control with specified bounds.
 *
 * Returns: (transfer full): A new #GrlUiTextBox
 */
GRL_AVAILABLE_IN_ALL
GrlUiTextBox *      grl_ui_textbox_new_with_bounds  (gfloat          x,
                                                     gfloat          y,
                                                     gfloat          width,
                                                     gfloat          height,
                                                     gint            max_length);

/*
 * Properties
 */

/**
 * grl_ui_textbox_get_text:
 * @self: A #GrlUiTextBox
 *
 * Gets the textbox text content.
 *
 * Returns: (transfer none): The text content
 */
GRL_AVAILABLE_IN_ALL
const gchar *       grl_ui_textbox_get_text         (GrlUiTextBox   *self);

/**
 * grl_ui_textbox_set_text:
 * @self: A #GrlUiTextBox
 * @text: (nullable): The new text content
 *
 * Sets the textbox text content.
 */
GRL_AVAILABLE_IN_ALL
void                grl_ui_textbox_set_text         (GrlUiTextBox   *self,
                                                     const gchar    *text);

/**
 * grl_ui_textbox_get_max_length:
 * @self: A #GrlUiTextBox
 *
 * Gets the maximum text length.
 *
 * Returns: The maximum number of characters
 */
GRL_AVAILABLE_IN_ALL
gint                grl_ui_textbox_get_max_length   (GrlUiTextBox   *self);

/**
 * grl_ui_textbox_set_max_length:
 * @self: A #GrlUiTextBox
 * @max_length: The maximum number of characters
 *
 * Sets the maximum text length.
 */
GRL_AVAILABLE_IN_ALL
void                grl_ui_textbox_set_max_length   (GrlUiTextBox   *self,
                                                     gint            max_length);

/**
 * grl_ui_textbox_get_edit_mode:
 * @self: A #GrlUiTextBox
 *
 * Gets whether the textbox is in edit mode.
 *
 * Returns: %TRUE if in edit mode
 */
GRL_AVAILABLE_IN_ALL
gboolean            grl_ui_textbox_get_edit_mode    (GrlUiTextBox   *self);

/**
 * grl_ui_textbox_set_edit_mode:
 * @self: A #GrlUiTextBox
 * @edit_mode: Whether to enter edit mode
 *
 * Sets whether the textbox is in edit mode.
 */
GRL_AVAILABLE_IN_ALL
void                grl_ui_textbox_set_edit_mode    (GrlUiTextBox   *self,
                                                     gboolean        edit_mode);

/**
 * grl_ui_textbox_get_read_only:
 * @self: A #GrlUiTextBox
 *
 * Gets whether the textbox is read-only.
 *
 * Returns: %TRUE if read-only
 */
GRL_AVAILABLE_IN_ALL
gboolean            grl_ui_textbox_get_read_only    (GrlUiTextBox   *self);

/**
 * grl_ui_textbox_set_read_only:
 * @self: A #GrlUiTextBox
 * @read_only: Whether the textbox should be read-only
 *
 * Sets whether the textbox is read-only.
 */
GRL_AVAILABLE_IN_ALL
void                grl_ui_textbox_set_read_only    (GrlUiTextBox   *self,
                                                     gboolean        read_only);

/*
 * Methods
 */

/**
 * grl_ui_textbox_clear:
 * @self: A #GrlUiTextBox
 *
 * Clears the textbox content.
 */
GRL_AVAILABLE_IN_ALL
void                grl_ui_textbox_clear            (GrlUiTextBox   *self);

/**
 * grl_ui_textbox_get_text_length:
 * @self: A #GrlUiTextBox
 *
 * Gets the current text length.
 *
 * Returns: The number of characters in the text
 */
GRL_AVAILABLE_IN_ALL
gint                grl_ui_textbox_get_text_length  (GrlUiTextBox   *self);

G_END_DECLS
