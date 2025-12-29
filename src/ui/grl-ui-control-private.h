/* grl-ui-control-private.h
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Private header for UI control internals.
 *
 * This header is for internal use by UI controls only.
 * It should not be installed or included by applications.
 */

#pragma once

#include "grl-ui-control.h"

G_BEGIN_DECLS

/**
 * grl_ui_current_edit_control:
 *
 * Global pointer to the currently editing control.
 *
 * Only one control can be in edit mode at a time (TextBox, Spinner,
 * ValueBox, DropdownBox). When a control enters edit mode, it should
 * set this pointer to itself. When it exits edit mode, it should
 * set this to %NULL.
 *
 * Other controls should check this pointer before attempting to
 * enter edit mode and exit the current control first.
 */
extern GrlUiControl *grl_ui_current_edit_control;

/**
 * grl_ui_control_set_editing:
 * @self: (nullable): A #GrlUiControl, or %NULL to clear
 * @editing: Whether the control is entering edit mode
 *
 * Sets or clears the global edit mode for a control.
 *
 * When @editing is %TRUE:
 * - If another control is currently editing, it will be forced out
 *   of edit mode first (by emitting "editing-finished" signal if supported)
 * - @self becomes the current edit control
 *
 * When @editing is %FALSE:
 * - If @self is the current edit control, the edit mode is cleared
 * - If @self is not the current edit control, this is a no-op
 */
void        grl_ui_control_set_editing      (GrlUiControl   *self,
                                             gboolean        editing);

/**
 * grl_ui_control_is_editing:
 * @self: A #GrlUiControl
 *
 * Checks if this control is currently in edit mode.
 *
 * Returns: %TRUE if @self is the current edit control
 */
gboolean    grl_ui_control_is_editing       (GrlUiControl   *self);

/**
 * grl_ui_control_get_current_edit:
 *
 * Gets the control currently in edit mode, if any.
 *
 * Returns: (nullable) (transfer none): The current edit control, or %NULL
 */
GrlUiControl * grl_ui_control_get_current_edit (void);

/**
 * grl_ui_control_clear_edit_mode:
 *
 * Clears the current edit mode without notifying the control.
 *
 * This is a low-level function that should only be used when
 * the current edit control is being destroyed.
 */
void        grl_ui_control_clear_edit_mode  (void);

/**
 * grl_ui_control_update_state:
 * @self: A #GrlUiControl
 *
 * Updates the control's visual state based on mouse position
 * and input. This should be called at the start of draw/handle_input.
 *
 * Sets the state property to NORMAL, FOCUSED, PRESSED, or DISABLED
 * based on:
 * - enabled property
 * - mouse hover
 * - mouse button state
 */
void        grl_ui_control_update_state     (GrlUiControl   *self);

/**
 * grl_ui_control_set_state:
 * @self: A #GrlUiControl
 * @state: The new state
 *
 * Directly sets the control's visual state.
 *
 * This is a protected function for subclass use.
 */
void        grl_ui_control_set_state        (GrlUiControl   *self,
                                             GrlUiState      state);

G_END_DECLS
