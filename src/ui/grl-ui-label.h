/* grl-ui-label.h
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Label control for displaying text.
 */

#pragma once

#if !defined(GRAYLIB_INSIDE) && !defined(GRAYLIB_COMPILATION)
#error "Only <graylib.h> can be included directly."
#endif

#include <glib-object.h>
#include "../grl-version.h"
#include "grl-ui-control.h"
#include "grl-ui-enums.h"

G_BEGIN_DECLS

#define GRL_TYPE_UI_LABEL (grl_ui_label_get_type ())

GRL_AVAILABLE_IN_ALL
G_DECLARE_FINAL_TYPE (GrlUiLabel, grl_ui_label, GRL, UI_LABEL, GrlUiControl)

/*
 * Constructors
 */

/**
 * grl_ui_label_new:
 * @text: (nullable): The label text
 *
 * Creates a new label control.
 *
 * Returns: (transfer full): A new #GrlUiLabel
 */
GRL_AVAILABLE_IN_ALL
GrlUiLabel *        grl_ui_label_new            (const gchar    *text);

/**
 * grl_ui_label_new_with_bounds:
 * @x: X position
 * @y: Y position
 * @width: Width of the label
 * @height: Height of the label
 * @text: (nullable): The label text
 *
 * Creates a new label control with specified bounds.
 *
 * Returns: (transfer full): A new #GrlUiLabel
 */
GRL_AVAILABLE_IN_ALL
GrlUiLabel *        grl_ui_label_new_with_bounds (gfloat          x,
                                                  gfloat          y,
                                                  gfloat          width,
                                                  gfloat          height,
                                                  const gchar    *text);

/*
 * Properties
 */

/**
 * grl_ui_label_get_text:
 * @self: A #GrlUiLabel
 *
 * Gets the label text.
 *
 * Returns: (transfer none) (nullable): The label text
 */
GRL_AVAILABLE_IN_ALL
const gchar *       grl_ui_label_get_text       (GrlUiLabel     *self);

/**
 * grl_ui_label_set_text:
 * @self: A #GrlUiLabel
 * @text: (nullable): The new text
 *
 * Sets the label text.
 */
GRL_AVAILABLE_IN_ALL
void                grl_ui_label_set_text       (GrlUiLabel     *self,
                                                 const gchar    *text);

/**
 * grl_ui_label_get_text_alignment:
 * @self: A #GrlUiLabel
 *
 * Gets the horizontal text alignment.
 *
 * Returns: The #GrlUiTextAlignment
 */
GRL_AVAILABLE_IN_ALL
GrlUiTextAlignment  grl_ui_label_get_text_alignment (GrlUiLabel *self);

/**
 * grl_ui_label_set_text_alignment:
 * @self: A #GrlUiLabel
 * @alignment: The horizontal text alignment
 *
 * Sets the horizontal text alignment.
 */
GRL_AVAILABLE_IN_ALL
void                grl_ui_label_set_text_alignment (GrlUiLabel         *self,
                                                     GrlUiTextAlignment  alignment);

G_END_DECLS
