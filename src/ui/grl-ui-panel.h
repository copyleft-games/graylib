/* grl-ui-panel.h
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Panel control for grouping other controls with optional text header.
 */

#pragma once

#if !defined(GRAYLIB_INSIDE) && !defined(GRAYLIB_COMPILATION)
#error "Only <graylib.h> can be included directly."
#endif

#include <glib-object.h>
#include "../grl-version.h"
#include "grl-ui-control.h"

G_BEGIN_DECLS

#define GRL_TYPE_UI_PANEL (grl_ui_panel_get_type ())

GRL_AVAILABLE_IN_ALL
G_DECLARE_FINAL_TYPE (GrlUiPanel, grl_ui_panel, GRL, UI_PANEL, GrlUiControl)

/*
 * Constructors
 */

/**
 * grl_ui_panel_new:
 * @text: (nullable): Optional header text
 *
 * Creates a new panel control.
 *
 * Returns: (transfer full): A new #GrlUiPanel
 */
GRL_AVAILABLE_IN_ALL
GrlUiPanel *        grl_ui_panel_new                (const gchar    *text);

/**
 * grl_ui_panel_new_with_bounds:
 * @x: X position
 * @y: Y position
 * @width: Width of the panel
 * @height: Height of the panel
 * @text: (nullable): Optional header text
 *
 * Creates a new panel control with specified bounds.
 *
 * Returns: (transfer full): A new #GrlUiPanel
 */
GRL_AVAILABLE_IN_ALL
GrlUiPanel *        grl_ui_panel_new_with_bounds    (gfloat          x,
                                                     gfloat          y,
                                                     gfloat          width,
                                                     gfloat          height,
                                                     const gchar    *text);

/*
 * Properties
 */

/**
 * grl_ui_panel_get_text:
 * @self: A #GrlUiPanel
 *
 * Gets the panel header text.
 *
 * Returns: (transfer none) (nullable): The header text
 */
GRL_AVAILABLE_IN_ALL
const gchar *       grl_ui_panel_get_text           (GrlUiPanel     *self);

/**
 * grl_ui_panel_set_text:
 * @self: A #GrlUiPanel
 * @text: (nullable): The header text to display
 *
 * Sets the panel header text.
 */
GRL_AVAILABLE_IN_ALL
void                grl_ui_panel_set_text           (GrlUiPanel     *self,
                                                     const gchar    *text);

/*
 * Methods
 */

/**
 * grl_ui_panel_get_content_bounds:
 * @self: A #GrlUiPanel
 *
 * Gets the bounds of the content area (inside the panel).
 * This is useful for positioning child controls.
 *
 * Returns: (transfer full): The content area bounds
 */
GRL_AVAILABLE_IN_ALL
GrlRectangle *      grl_ui_panel_get_content_bounds (GrlUiPanel     *self);

G_END_DECLS
