/* grl-ui-dropdownbox.h
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * DropdownBox control for selecting from a dropdown list.
 */

#pragma once

#if !defined(GRAYLIB_INSIDE) && !defined(GRAYLIB_COMPILATION)
#error "Only <graylib.h> can be included directly."
#endif

#include <glib-object.h>
#include "../grl-version.h"
#include "grl-ui-control.h"

G_BEGIN_DECLS

#define GRL_TYPE_UI_DROPDOWNBOX (grl_ui_dropdownbox_get_type ())

GRL_AVAILABLE_IN_ALL
G_DECLARE_FINAL_TYPE (GrlUiDropdownBox, grl_ui_dropdownbox, GRL, UI_DROPDOWNBOX, GrlUiControl)

/*
 * Constructors
 */

/**
 * grl_ui_dropdownbox_new:
 * @items: (nullable): Semicolon-separated list of items
 *
 * Creates a new dropdown box control.
 *
 * Returns: (transfer full): A new #GrlUiDropdownBox
 */
GRL_AVAILABLE_IN_ALL
GrlUiDropdownBox *  grl_ui_dropdownbox_new              (const gchar    *items);

/**
 * grl_ui_dropdownbox_new_with_bounds:
 * @x: X position
 * @y: Y position
 * @width: Width of the dropdown box
 * @height: Height of the dropdown box
 * @items: (nullable): Semicolon-separated list of items
 *
 * Creates a new dropdown box control with specified bounds.
 *
 * Returns: (transfer full): A new #GrlUiDropdownBox
 */
GRL_AVAILABLE_IN_ALL
GrlUiDropdownBox *  grl_ui_dropdownbox_new_with_bounds  (gfloat          x,
                                                         gfloat          y,
                                                         gfloat          width,
                                                         gfloat          height,
                                                         const gchar    *items);

/*
 * Properties
 */

/**
 * grl_ui_dropdownbox_get_items:
 * @self: A #GrlUiDropdownBox
 *
 * Gets the semicolon-separated list of items.
 *
 * Returns: (transfer none) (nullable): The items string
 */
GRL_AVAILABLE_IN_ALL
const gchar *       grl_ui_dropdownbox_get_items        (GrlUiDropdownBox *self);

/**
 * grl_ui_dropdownbox_set_items:
 * @self: A #GrlUiDropdownBox
 * @items: (nullable): Semicolon-separated list of items
 *
 * Sets the semicolon-separated list of items.
 */
GRL_AVAILABLE_IN_ALL
void                grl_ui_dropdownbox_set_items        (GrlUiDropdownBox *self,
                                                         const gchar      *items);

/**
 * grl_ui_dropdownbox_get_active:
 * @self: A #GrlUiDropdownBox
 *
 * Gets the index of the currently selected item.
 *
 * Returns: The active item index (0-based)
 */
GRL_AVAILABLE_IN_ALL
gint                grl_ui_dropdownbox_get_active       (GrlUiDropdownBox *self);

/**
 * grl_ui_dropdownbox_set_active:
 * @self: A #GrlUiDropdownBox
 * @active: The index of the item to select
 *
 * Sets the currently selected item by index.
 */
GRL_AVAILABLE_IN_ALL
void                grl_ui_dropdownbox_set_active       (GrlUiDropdownBox *self,
                                                         gint              active);

/**
 * grl_ui_dropdownbox_get_edit_mode:
 * @self: A #GrlUiDropdownBox
 *
 * Gets whether the dropdown is expanded.
 *
 * Returns: %TRUE if dropdown is expanded
 */
GRL_AVAILABLE_IN_ALL
gboolean            grl_ui_dropdownbox_get_edit_mode    (GrlUiDropdownBox *self);

/**
 * grl_ui_dropdownbox_set_edit_mode:
 * @self: A #GrlUiDropdownBox
 * @edit_mode: Whether to expand the dropdown
 *
 * Sets whether the dropdown is expanded.
 */
GRL_AVAILABLE_IN_ALL
void                grl_ui_dropdownbox_set_edit_mode    (GrlUiDropdownBox *self,
                                                         gboolean          edit_mode);

/*
 * Methods
 */

/**
 * grl_ui_dropdownbox_get_item_count:
 * @self: A #GrlUiDropdownBox
 *
 * Gets the number of items in the dropdown box.
 *
 * Returns: The number of items
 */
GRL_AVAILABLE_IN_ALL
gint                grl_ui_dropdownbox_get_item_count   (GrlUiDropdownBox *self);

/**
 * grl_ui_dropdownbox_get_active_text:
 * @self: A #GrlUiDropdownBox
 *
 * Gets the text of the currently selected item.
 *
 * Returns: (transfer full) (nullable): The active item text
 */
GRL_AVAILABLE_IN_ALL
gchar *             grl_ui_dropdownbox_get_active_text  (GrlUiDropdownBox *self);

/**
 * grl_ui_dropdownbox_toggle:
 * @self: A #GrlUiDropdownBox
 *
 * Toggles the dropdown open/closed.
 */
GRL_AVAILABLE_IN_ALL
void                grl_ui_dropdownbox_toggle           (GrlUiDropdownBox *self);

G_END_DECLS
