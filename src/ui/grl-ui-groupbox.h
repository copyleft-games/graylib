/* grl-ui-groupbox.h
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * GroupBox control for visually grouping related controls with a border and label.
 */

#pragma once

#if !defined(GRAYLIB_INSIDE) && !defined(GRAYLIB_COMPILATION)
#error "Only <graylib.h> can be included directly."
#endif

#include <glib-object.h>
#include "../grl-version.h"
#include "grl-ui-control.h"

G_BEGIN_DECLS

#define GRL_TYPE_UI_GROUPBOX (grl_ui_groupbox_get_type ())

GRL_AVAILABLE_IN_ALL
G_DECLARE_FINAL_TYPE (GrlUiGroupBox, grl_ui_groupbox, GRL, UI_GROUPBOX, GrlUiControl)

/*
 * Constructors
 */

/**
 * grl_ui_groupbox_new:
 * @text: (nullable): Label text for the group box
 *
 * Creates a new group box control.
 *
 * Returns: (transfer full): A new #GrlUiGroupBox
 */
GRL_AVAILABLE_IN_ALL
GrlUiGroupBox *     grl_ui_groupbox_new             (const gchar    *text);

/**
 * grl_ui_groupbox_new_with_bounds:
 * @x: X position
 * @y: Y position
 * @width: Width of the group box
 * @height: Height of the group box
 * @text: (nullable): Label text for the group box
 *
 * Creates a new group box control with specified bounds.
 *
 * Returns: (transfer full): A new #GrlUiGroupBox
 */
GRL_AVAILABLE_IN_ALL
GrlUiGroupBox *     grl_ui_groupbox_new_with_bounds (gfloat          x,
                                                     gfloat          y,
                                                     gfloat          width,
                                                     gfloat          height,
                                                     const gchar    *text);

/*
 * Properties
 */

/**
 * grl_ui_groupbox_get_text:
 * @self: A #GrlUiGroupBox
 *
 * Gets the group box label text.
 *
 * Returns: (transfer none) (nullable): The label text
 */
GRL_AVAILABLE_IN_ALL
const gchar *       grl_ui_groupbox_get_text        (GrlUiGroupBox  *self);

/**
 * grl_ui_groupbox_set_text:
 * @self: A #GrlUiGroupBox
 * @text: (nullable): The label text to display
 *
 * Sets the group box label text.
 */
GRL_AVAILABLE_IN_ALL
void                grl_ui_groupbox_set_text        (GrlUiGroupBox  *self,
                                                     const gchar    *text);

/*
 * Methods
 */

/**
 * grl_ui_groupbox_get_content_bounds:
 * @self: A #GrlUiGroupBox
 *
 * Gets the bounds of the content area (inside the group box).
 * This is useful for positioning child controls.
 *
 * Returns: (transfer full): The content area bounds
 */
GRL_AVAILABLE_IN_ALL
GrlRectangle *      grl_ui_groupbox_get_content_bounds (GrlUiGroupBox *self);

G_END_DECLS
