/* grl-ui-windowbox.h
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * WindowBox control - a closeable window container with title bar.
 */

#pragma once

#if !defined(GRAYLIB_INSIDE) && !defined(GRAYLIB_COMPILATION)
#error "Only <graylib.h> can be included directly."
#endif

#include <glib-object.h>
#include "../grl-version.h"
#include "grl-ui-control.h"

G_BEGIN_DECLS

#define GRL_TYPE_UI_WINDOWBOX (grl_ui_windowbox_get_type ())

GRL_AVAILABLE_IN_ALL
G_DECLARE_FINAL_TYPE (GrlUiWindowBox, grl_ui_windowbox, GRL, UI_WINDOWBOX, GrlUiControl)

/*
 * Constructors
 */

/**
 * grl_ui_windowbox_new:
 * @title: (nullable): Window title
 *
 * Creates a new window box control.
 *
 * Returns: (transfer full): A new #GrlUiWindowBox
 */
GRL_AVAILABLE_IN_ALL
GrlUiWindowBox *    grl_ui_windowbox_new                (const gchar    *title);

/**
 * grl_ui_windowbox_new_with_bounds:
 * @x: X position
 * @y: Y position
 * @width: Width of the window box
 * @height: Height of the window box
 * @title: (nullable): Window title
 *
 * Creates a new window box control with specified bounds.
 *
 * Returns: (transfer full): A new #GrlUiWindowBox
 */
GRL_AVAILABLE_IN_ALL
GrlUiWindowBox *    grl_ui_windowbox_new_with_bounds    (gfloat          x,
                                                         gfloat          y,
                                                         gfloat          width,
                                                         gfloat          height,
                                                         const gchar    *title);

/*
 * Properties
 */

/**
 * grl_ui_windowbox_get_title:
 * @self: A #GrlUiWindowBox
 *
 * Gets the window title.
 *
 * Returns: (transfer none) (nullable): The window title
 */
GRL_AVAILABLE_IN_ALL
const gchar *       grl_ui_windowbox_get_title          (GrlUiWindowBox *self);

/**
 * grl_ui_windowbox_set_title:
 * @self: A #GrlUiWindowBox
 * @title: (nullable): The window title
 *
 * Sets the window title.
 */
GRL_AVAILABLE_IN_ALL
void                grl_ui_windowbox_set_title          (GrlUiWindowBox *self,
                                                         const gchar    *title);

/**
 * grl_ui_windowbox_get_open:
 * @self: A #GrlUiWindowBox
 *
 * Gets whether the window is open (visible).
 *
 * Returns: %TRUE if the window is open
 */
GRL_AVAILABLE_IN_ALL
gboolean            grl_ui_windowbox_get_open           (GrlUiWindowBox *self);

/**
 * grl_ui_windowbox_set_open:
 * @self: A #GrlUiWindowBox
 * @open: %TRUE to open the window, %FALSE to close it
 *
 * Sets whether the window is open.
 */
GRL_AVAILABLE_IN_ALL
void                grl_ui_windowbox_set_open           (GrlUiWindowBox *self,
                                                         gboolean        open);

/*
 * Methods
 */

/**
 * grl_ui_windowbox_close:
 * @self: A #GrlUiWindowBox
 *
 * Closes the window box.
 */
GRL_AVAILABLE_IN_ALL
void                grl_ui_windowbox_close              (GrlUiWindowBox *self);

/**
 * grl_ui_windowbox_open:
 * @self: A #GrlUiWindowBox
 *
 * Opens the window box.
 */
GRL_AVAILABLE_IN_ALL
void                grl_ui_windowbox_open               (GrlUiWindowBox *self);

/**
 * grl_ui_windowbox_toggle:
 * @self: A #GrlUiWindowBox
 *
 * Toggles the window box open/closed state.
 */
GRL_AVAILABLE_IN_ALL
void                grl_ui_windowbox_toggle             (GrlUiWindowBox *self);

/**
 * grl_ui_windowbox_get_content_bounds:
 * @self: A #GrlUiWindowBox
 *
 * Gets the bounds of the content area (inside the window).
 * This is useful for positioning child controls.
 *
 * Returns: (transfer full): The content area bounds
 */
GRL_AVAILABLE_IN_ALL
GrlRectangle *      grl_ui_windowbox_get_content_bounds (GrlUiWindowBox *self);

G_END_DECLS
