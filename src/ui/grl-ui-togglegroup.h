/* grl-ui-togglegroup.h
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * ToggleGroup control for selecting one option from a group.
 */

#pragma once

#if !defined(GRAYLIB_INSIDE) && !defined(GRAYLIB_COMPILATION)
#error "Only <graylib.h> can be included directly."
#endif

#include <glib-object.h>
#include "../grl-version.h"
#include "grl-ui-control.h"

G_BEGIN_DECLS

#define GRL_TYPE_UI_TOGGLEGROUP (grl_ui_togglegroup_get_type ())

GRL_AVAILABLE_IN_ALL
G_DECLARE_FINAL_TYPE (GrlUiToggleGroup, grl_ui_togglegroup, GRL, UI_TOGGLEGROUP, GrlUiControl)

/*
 * Constructors
 */

/**
 * grl_ui_togglegroup_new:
 * @items: (nullable): Semicolon-separated list of items
 *
 * Creates a new toggle group control.
 *
 * Returns: (transfer full): A new #GrlUiToggleGroup
 */
GRL_AVAILABLE_IN_ALL
GrlUiToggleGroup *  grl_ui_togglegroup_new              (const gchar    *items);

/**
 * grl_ui_togglegroup_new_with_bounds:
 * @x: X position
 * @y: Y position
 * @width: Width of each toggle button
 * @height: Height of the toggle buttons
 * @items: (nullable): Semicolon-separated list of items
 *
 * Creates a new toggle group control with specified bounds.
 * The width specifies the width of each individual toggle button.
 *
 * Returns: (transfer full): A new #GrlUiToggleGroup
 */
GRL_AVAILABLE_IN_ALL
GrlUiToggleGroup *  grl_ui_togglegroup_new_with_bounds  (gfloat          x,
                                                         gfloat          y,
                                                         gfloat          width,
                                                         gfloat          height,
                                                         const gchar    *items);

/*
 * Properties
 */

/**
 * grl_ui_togglegroup_get_items:
 * @self: A #GrlUiToggleGroup
 *
 * Gets the semicolon-separated list of items.
 *
 * Returns: (transfer none) (nullable): The items string
 */
GRL_AVAILABLE_IN_ALL
const gchar *       grl_ui_togglegroup_get_items        (GrlUiToggleGroup *self);

/**
 * grl_ui_togglegroup_set_items:
 * @self: A #GrlUiToggleGroup
 * @items: (nullable): Semicolon-separated list of items
 *
 * Sets the semicolon-separated list of items.
 */
GRL_AVAILABLE_IN_ALL
void                grl_ui_togglegroup_set_items        (GrlUiToggleGroup *self,
                                                         const gchar      *items);

/**
 * grl_ui_togglegroup_get_active:
 * @self: A #GrlUiToggleGroup
 *
 * Gets the index of the currently selected item.
 *
 * Returns: The active item index (0-based)
 */
GRL_AVAILABLE_IN_ALL
gint                grl_ui_togglegroup_get_active       (GrlUiToggleGroup *self);

/**
 * grl_ui_togglegroup_set_active:
 * @self: A #GrlUiToggleGroup
 * @active: The index of the item to select
 *
 * Sets the currently selected item by index.
 */
GRL_AVAILABLE_IN_ALL
void                grl_ui_togglegroup_set_active       (GrlUiToggleGroup *self,
                                                         gint              active);

/*
 * Methods
 */

/**
 * grl_ui_togglegroup_get_item_count:
 * @self: A #GrlUiToggleGroup
 *
 * Gets the number of items in the toggle group.
 *
 * Returns: The number of items
 */
GRL_AVAILABLE_IN_ALL
gint                grl_ui_togglegroup_get_item_count   (GrlUiToggleGroup *self);

/**
 * grl_ui_togglegroup_get_active_text:
 * @self: A #GrlUiToggleGroup
 *
 * Gets the text of the currently selected item.
 *
 * Returns: (transfer full) (nullable): The active item text
 */
GRL_AVAILABLE_IN_ALL
gchar *             grl_ui_togglegroup_get_active_text  (GrlUiToggleGroup *self);

G_END_DECLS
