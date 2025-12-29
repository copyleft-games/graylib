/* grl-ui-listview.h
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * ListView control for displaying and selecting from a scrollable list.
 */

#pragma once

#if !defined(GRAYLIB_INSIDE) && !defined(GRAYLIB_COMPILATION)
#error "Only <graylib.h> can be included directly."
#endif

#include <glib-object.h>
#include "../grl-version.h"
#include "grl-ui-control.h"

G_BEGIN_DECLS

#define GRL_TYPE_UI_LISTVIEW (grl_ui_listview_get_type ())

GRL_AVAILABLE_IN_ALL
G_DECLARE_FINAL_TYPE (GrlUiListView, grl_ui_listview, GRL, UI_LISTVIEW, GrlUiControl)

/*
 * Constructors
 */

/**
 * grl_ui_listview_new:
 * @items: (nullable): Semicolon-separated list of items
 *
 * Creates a new list view control.
 *
 * Returns: (transfer full): A new #GrlUiListView
 */
GRL_AVAILABLE_IN_ALL
GrlUiListView *     grl_ui_listview_new             (const gchar    *items);

/**
 * grl_ui_listview_new_with_bounds:
 * @x: X position
 * @y: Y position
 * @width: Width of the list view
 * @height: Height of the list view
 * @items: (nullable): Semicolon-separated list of items
 *
 * Creates a new list view control with specified bounds.
 *
 * Returns: (transfer full): A new #GrlUiListView
 */
GRL_AVAILABLE_IN_ALL
GrlUiListView *     grl_ui_listview_new_with_bounds (gfloat          x,
                                                     gfloat          y,
                                                     gfloat          width,
                                                     gfloat          height,
                                                     const gchar    *items);

/*
 * Properties
 */

/**
 * grl_ui_listview_get_items:
 * @self: A #GrlUiListView
 *
 * Gets the semicolon-separated list of items.
 *
 * Returns: (transfer none) (nullable): The items string
 */
GRL_AVAILABLE_IN_ALL
const gchar *       grl_ui_listview_get_items       (GrlUiListView  *self);

/**
 * grl_ui_listview_set_items:
 * @self: A #GrlUiListView
 * @items: (nullable): Semicolon-separated list of items
 *
 * Sets the semicolon-separated list of items.
 */
GRL_AVAILABLE_IN_ALL
void                grl_ui_listview_set_items       (GrlUiListView  *self,
                                                     const gchar    *items);

/**
 * grl_ui_listview_get_active:
 * @self: A #GrlUiListView
 *
 * Gets the index of the currently selected item.
 *
 * Returns: The active item index (0-based), or -1 if no selection
 */
GRL_AVAILABLE_IN_ALL
gint                grl_ui_listview_get_active      (GrlUiListView  *self);

/**
 * grl_ui_listview_set_active:
 * @self: A #GrlUiListView
 * @active: The index of the item to select, or -1 for no selection
 *
 * Sets the currently selected item by index.
 */
GRL_AVAILABLE_IN_ALL
void                grl_ui_listview_set_active      (GrlUiListView  *self,
                                                     gint            active);

/**
 * grl_ui_listview_get_scroll_index:
 * @self: A #GrlUiListView
 *
 * Gets the current scroll position.
 *
 * Returns: The scroll index
 */
GRL_AVAILABLE_IN_ALL
gint                grl_ui_listview_get_scroll_index (GrlUiListView *self);

/**
 * grl_ui_listview_set_scroll_index:
 * @self: A #GrlUiListView
 * @scroll_index: The scroll position
 *
 * Sets the current scroll position.
 */
GRL_AVAILABLE_IN_ALL
void                grl_ui_listview_set_scroll_index (GrlUiListView *self,
                                                      gint           scroll_index);

/*
 * Methods
 */

/**
 * grl_ui_listview_get_item_count:
 * @self: A #GrlUiListView
 *
 * Gets the number of items in the list view.
 *
 * Returns: The number of items
 */
GRL_AVAILABLE_IN_ALL
gint                grl_ui_listview_get_item_count  (GrlUiListView  *self);

/**
 * grl_ui_listview_get_active_text:
 * @self: A #GrlUiListView
 *
 * Gets the text of the currently selected item.
 *
 * Returns: (transfer full) (nullable): The active item text
 */
GRL_AVAILABLE_IN_ALL
gchar *             grl_ui_listview_get_active_text (GrlUiListView  *self);

/**
 * grl_ui_listview_scroll_to_active:
 * @self: A #GrlUiListView
 *
 * Scrolls the list to make the active item visible.
 */
GRL_AVAILABLE_IN_ALL
void                grl_ui_listview_scroll_to_active (GrlUiListView *self);

/**
 * grl_ui_listview_clear_selection:
 * @self: A #GrlUiListView
 *
 * Clears the current selection.
 */
GRL_AVAILABLE_IN_ALL
void                grl_ui_listview_clear_selection (GrlUiListView  *self);

G_END_DECLS
