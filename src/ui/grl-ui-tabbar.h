/* grl-ui-tabbar.h
 *
 * Copyright 2026 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * TabBar control: a row of tabs, one of them active, with optional close
 * buttons (raygui 5 GuiTabBar()).
 */

#pragma once

#if !defined(GRAYLIB_INSIDE) && !defined(GRAYLIB_COMPILATION)
#error "Only <graylib.h> can be included directly."
#endif

#include <glib-object.h>
#include "../grl-version.h"
#include "grl-ui-control.h"

G_BEGIN_DECLS

#define GRL_TYPE_UI_TABBAR (grl_ui_tabbar_get_type ())

GRL_AVAILABLE_IN_ALL
G_DECLARE_FINAL_TYPE (GrlUiTabBar, grl_ui_tabbar, GRL, UI_TABBAR, GrlUiControl)

/*
 * Constructors
 */

/**
 * grl_ui_tabbar_new:
 * @items: (nullable): Semicolon-separated tab titles
 *
 * Creates a new tab bar.
 *
 * Returns: (transfer full): A new #GrlUiTabBar
 */
GRL_AVAILABLE_IN_ALL
GrlUiTabBar *       grl_ui_tabbar_new                   (const gchar    *items);

/**
 * grl_ui_tabbar_new_with_bounds:
 * @x: X position
 * @y: Y position
 * @width: Width of the whole bar (the line under or over the tabs)
 * @height: Height of the tabs
 * @items: (nullable): Semicolon-separated tab titles
 *
 * Creates a new tab bar with the given bounds. Each tab is
 * #GrlUiTabBar:tab-width wide.
 *
 * Returns: (transfer full): A new #GrlUiTabBar
 */
GRL_AVAILABLE_IN_ALL
GrlUiTabBar *       grl_ui_tabbar_new_with_bounds       (gfloat          x,
                                                         gfloat          y,
                                                         gfloat          width,
                                                         gfloat          height,
                                                         const gchar    *items);

/*
 * Properties
 */

/**
 * grl_ui_tabbar_get_items:
 * @self: A #GrlUiTabBar
 *
 * Gets the semicolon-separated tab titles.
 *
 * Returns: (transfer none) (nullable): The items string
 */
GRL_AVAILABLE_IN_ALL
const gchar *       grl_ui_tabbar_get_items             (GrlUiTabBar    *self);

/**
 * grl_ui_tabbar_set_items:
 * @self: A #GrlUiTabBar
 * @items: (nullable): Semicolon-separated tab titles
 *
 * Sets the tab titles. The active index is clamped to the new count.
 */
GRL_AVAILABLE_IN_ALL
void                grl_ui_tabbar_set_items             (GrlUiTabBar    *self,
                                                         const gchar    *items);

/**
 * grl_ui_tabbar_get_item_count:
 * @self: A #GrlUiTabBar
 *
 * Gets the number of tabs.
 *
 * Returns: The number of tabs
 */
GRL_AVAILABLE_IN_ALL
gint                grl_ui_tabbar_get_item_count        (GrlUiTabBar    *self);

/**
 * grl_ui_tabbar_get_active:
 * @self: A #GrlUiTabBar
 *
 * Gets the index of the active tab.
 *
 * Returns: The active tab index (0-based)
 */
GRL_AVAILABLE_IN_ALL
gint                grl_ui_tabbar_get_active            (GrlUiTabBar    *self);

/**
 * grl_ui_tabbar_set_active:
 * @self: A #GrlUiTabBar
 * @active: Index of the tab to activate (clamped to the tab count)
 *
 * Sets the active tab.
 */
GRL_AVAILABLE_IN_ALL
void                grl_ui_tabbar_set_active            (GrlUiTabBar    *self,
                                                         gint            active);

/**
 * grl_ui_tabbar_get_tab_width:
 * @self: A #GrlUiTabBar
 *
 * Gets the width of each tab.
 *
 * Returns: The tab width in pixels, or 0 to use the style's
 *   (%GRL_UI_CONTROL_TABBAR property 16, TAB_ITEMS_WIDTH)
 */
GRL_AVAILABLE_IN_ALL
gint                grl_ui_tabbar_get_tab_width         (GrlUiTabBar    *self);

/**
 * grl_ui_tabbar_set_tab_width:
 * @self: A #GrlUiTabBar
 * @tab_width: Width of each tab in pixels, or 0 for the style's
 *
 * Sets the width of each tab.
 */
GRL_AVAILABLE_IN_ALL
void                grl_ui_tabbar_set_tab_width         (GrlUiTabBar    *self,
                                                         gint            tab_width);

/**
 * grl_ui_tabbar_get_show_close_buttons:
 * @self: A #GrlUiTabBar
 *
 * Gets whether every tab shows a close button.
 *
 * Returns: %TRUE when close buttons are shown
 */
GRL_AVAILABLE_IN_ALL
gboolean            grl_ui_tabbar_get_show_close_buttons (GrlUiTabBar   *self);

/**
 * grl_ui_tabbar_set_show_close_buttons:
 * @self: A #GrlUiTabBar
 * @show: Whether to show a close button on every tab
 *
 * Sets whether tabs show a close button. A middle click on a tab requests
 * closing it either way.
 */
GRL_AVAILABLE_IN_ALL
void                grl_ui_tabbar_set_show_close_buttons (GrlUiTabBar   *self,
                                                          gboolean       show);

/**
 * grl_ui_tabbar_get_line_on_top:
 * @self: A #GrlUiTabBar
 *
 * Gets whether the bar's line is drawn above the tabs (tabs hanging from
 * the top of a panel) instead of below them.
 *
 * Returns: %TRUE when the line is on top
 */
GRL_AVAILABLE_IN_ALL
gboolean            grl_ui_tabbar_get_line_on_top       (GrlUiTabBar    *self);

/**
 * grl_ui_tabbar_set_line_on_top:
 * @self: A #GrlUiTabBar
 * @on_top: %TRUE to draw the line above the tabs
 *
 * Sets which side of the tabs the bar's line is drawn on.
 */
GRL_AVAILABLE_IN_ALL
void                grl_ui_tabbar_set_line_on_top       (GrlUiTabBar    *self,
                                                         gboolean        on_top);

/*
 * Methods
 */

/**
 * grl_ui_tabbar_get_active_text:
 * @self: A #GrlUiTabBar
 *
 * Gets the title of the active tab.
 *
 * Returns: (transfer full) (nullable): The active tab title
 */
GRL_AVAILABLE_IN_ALL
gchar *             grl_ui_tabbar_get_active_text       (GrlUiTabBar    *self);

/**
 * grl_ui_tabbar_tab_at:
 * @self: A #GrlUiTabBar
 * @x: X coordinate (screen space)
 * @y: Y coordinate (screen space)
 *
 * Finds the tab under a point, using the same layout raygui draws.
 *
 * Returns: The tab index, or -1 when no tab is at (@x, @y)
 */
GRL_AVAILABLE_IN_ALL
gint                grl_ui_tabbar_tab_at                (GrlUiTabBar    *self,
                                                         gfloat          x,
                                                         gfloat          y);

G_END_DECLS
