/* grl-ui-toggle.h
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Toggle Button control for on/off states.
 */

#pragma once

#if !defined(GRAYLIB_INSIDE) && !defined(GRAYLIB_COMPILATION)
#error "Only <graylib.h> can be included directly."
#endif

#include <glib-object.h>
#include "../grl-version.h"
#include "grl-ui-control.h"

G_BEGIN_DECLS

#define GRL_TYPE_UI_TOGGLE (grl_ui_toggle_get_type ())

GRL_AVAILABLE_IN_ALL
G_DECLARE_FINAL_TYPE (GrlUiToggle, grl_ui_toggle, GRL, UI_TOGGLE, GrlUiControl)

/*
 * Constructors
 */

/**
 * grl_ui_toggle_new:
 * @text: (nullable): The toggle label text
 *
 * Creates a new toggle button control.
 *
 * Returns: (transfer full): A new #GrlUiToggle
 */
GRL_AVAILABLE_IN_ALL
GrlUiToggle *       grl_ui_toggle_new               (const gchar    *text);

/**
 * grl_ui_toggle_new_with_bounds:
 * @x: X position
 * @y: Y position
 * @width: Width of the toggle
 * @height: Height of the toggle
 * @text: (nullable): The toggle label text
 *
 * Creates a new toggle button control with specified bounds.
 *
 * Returns: (transfer full): A new #GrlUiToggle
 */
GRL_AVAILABLE_IN_ALL
GrlUiToggle *       grl_ui_toggle_new_with_bounds   (gfloat          x,
                                                     gfloat          y,
                                                     gfloat          width,
                                                     gfloat          height,
                                                     const gchar    *text);

/*
 * Properties
 */

/**
 * grl_ui_toggle_get_text:
 * @self: A #GrlUiToggle
 *
 * Gets the toggle label text.
 *
 * Returns: (transfer none) (nullable): The label text
 */
GRL_AVAILABLE_IN_ALL
const gchar *       grl_ui_toggle_get_text          (GrlUiToggle    *self);

/**
 * grl_ui_toggle_set_text:
 * @self: A #GrlUiToggle
 * @text: (nullable): The new label text
 *
 * Sets the toggle label text.
 */
GRL_AVAILABLE_IN_ALL
void                grl_ui_toggle_set_text          (GrlUiToggle    *self,
                                                     const gchar    *text);

/**
 * grl_ui_toggle_get_active:
 * @self: A #GrlUiToggle
 *
 * Gets whether the toggle is active (on).
 *
 * Returns: %TRUE if active
 */
GRL_AVAILABLE_IN_ALL
gboolean            grl_ui_toggle_get_active        (GrlUiToggle    *self);

/**
 * grl_ui_toggle_set_active:
 * @self: A #GrlUiToggle
 * @active: Whether the toggle should be active
 *
 * Sets whether the toggle is active (on).
 */
GRL_AVAILABLE_IN_ALL
void                grl_ui_toggle_set_active        (GrlUiToggle    *self,
                                                     gboolean        active);

/*
 * Methods
 */

/**
 * grl_ui_toggle_toggle:
 * @self: A #GrlUiToggle
 *
 * Toggles the active state.
 */
GRL_AVAILABLE_IN_ALL
void                grl_ui_toggle_toggle            (GrlUiToggle    *self);

G_END_DECLS
