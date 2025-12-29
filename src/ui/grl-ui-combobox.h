/* grl-ui-combobox.h
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * ComboBox control for selecting from a list of options.
 */

#pragma once

#if !defined(GRAYLIB_INSIDE) && !defined(GRAYLIB_COMPILATION)
#error "Only <graylib.h> can be included directly."
#endif

#include <glib-object.h>
#include "../grl-version.h"
#include "grl-ui-control.h"

G_BEGIN_DECLS

#define GRL_TYPE_UI_COMBOBOX (grl_ui_combobox_get_type ())

GRL_AVAILABLE_IN_ALL
G_DECLARE_FINAL_TYPE (GrlUiComboBox, grl_ui_combobox, GRL, UI_COMBOBOX, GrlUiControl)

/*
 * Constructors
 */

/**
 * grl_ui_combobox_new:
 * @items: (nullable): Semicolon-separated list of items
 *
 * Creates a new combo box control.
 *
 * Returns: (transfer full): A new #GrlUiComboBox
 */
GRL_AVAILABLE_IN_ALL
GrlUiComboBox *     grl_ui_combobox_new             (const gchar    *items);

/**
 * grl_ui_combobox_new_with_bounds:
 * @x: X position
 * @y: Y position
 * @width: Width of the combo box
 * @height: Height of the combo box
 * @items: (nullable): Semicolon-separated list of items
 *
 * Creates a new combo box control with specified bounds.
 *
 * Returns: (transfer full): A new #GrlUiComboBox
 */
GRL_AVAILABLE_IN_ALL
GrlUiComboBox *     grl_ui_combobox_new_with_bounds (gfloat          x,
                                                     gfloat          y,
                                                     gfloat          width,
                                                     gfloat          height,
                                                     const gchar    *items);

/*
 * Properties
 */

/**
 * grl_ui_combobox_get_items:
 * @self: A #GrlUiComboBox
 *
 * Gets the semicolon-separated list of items.
 *
 * Returns: (transfer none) (nullable): The items string
 */
GRL_AVAILABLE_IN_ALL
const gchar *       grl_ui_combobox_get_items       (GrlUiComboBox  *self);

/**
 * grl_ui_combobox_set_items:
 * @self: A #GrlUiComboBox
 * @items: (nullable): Semicolon-separated list of items
 *
 * Sets the semicolon-separated list of items.
 */
GRL_AVAILABLE_IN_ALL
void                grl_ui_combobox_set_items       (GrlUiComboBox  *self,
                                                     const gchar    *items);

/**
 * grl_ui_combobox_get_active:
 * @self: A #GrlUiComboBox
 *
 * Gets the index of the currently selected item.
 *
 * Returns: The active item index (0-based)
 */
GRL_AVAILABLE_IN_ALL
gint                grl_ui_combobox_get_active      (GrlUiComboBox  *self);

/**
 * grl_ui_combobox_set_active:
 * @self: A #GrlUiComboBox
 * @active: The index of the item to select
 *
 * Sets the currently selected item by index.
 */
GRL_AVAILABLE_IN_ALL
void                grl_ui_combobox_set_active      (GrlUiComboBox  *self,
                                                     gint            active);

/*
 * Methods
 */

/**
 * grl_ui_combobox_get_item_count:
 * @self: A #GrlUiComboBox
 *
 * Gets the number of items in the combo box.
 *
 * Returns: The number of items
 */
GRL_AVAILABLE_IN_ALL
gint                grl_ui_combobox_get_item_count  (GrlUiComboBox  *self);

/**
 * grl_ui_combobox_get_active_text:
 * @self: A #GrlUiComboBox
 *
 * Gets the text of the currently selected item.
 *
 * Returns: (transfer full) (nullable): The active item text
 */
GRL_AVAILABLE_IN_ALL
gchar *             grl_ui_combobox_get_active_text (GrlUiComboBox  *self);

G_END_DECLS
