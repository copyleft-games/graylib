/* grl-ui-enums.h
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Enumerations for the UI module.
 */

#pragma once

#if !defined(GRAYLIB_INSIDE) && !defined(GRAYLIB_COMPILATION)
#error "Only <graylib.h> can be included directly."
#endif

#include <glib-object.h>
#include "../grl-version.h"

G_BEGIN_DECLS

/**
 * GrlUiState:
 * @GRL_UI_STATE_NORMAL: Control is in normal state
 * @GRL_UI_STATE_FOCUSED: Control has keyboard focus or mouse hover
 * @GRL_UI_STATE_PRESSED: Control is being pressed/clicked
 * @GRL_UI_STATE_DISABLED: Control is disabled and not interactive
 *
 * The visual and interactive state of a UI control.
 */
typedef enum
{
    GRL_UI_STATE_NORMAL = 0,
    GRL_UI_STATE_FOCUSED,
    GRL_UI_STATE_PRESSED,
    GRL_UI_STATE_DISABLED
} GrlUiState;

GRL_AVAILABLE_IN_ALL
GType grl_ui_state_get_type (void) G_GNUC_CONST;
#define GRL_TYPE_UI_STATE (grl_ui_state_get_type ())

/**
 * GrlUiTextAlignment:
 * @GRL_UI_TEXT_ALIGN_LEFT: Text aligned to the left
 * @GRL_UI_TEXT_ALIGN_CENTER: Text centered horizontally
 * @GRL_UI_TEXT_ALIGN_RIGHT: Text aligned to the right
 *
 * Horizontal text alignment within a control.
 */
typedef enum
{
    GRL_UI_TEXT_ALIGN_LEFT = 0,
    GRL_UI_TEXT_ALIGN_CENTER,
    GRL_UI_TEXT_ALIGN_RIGHT
} GrlUiTextAlignment;

GRL_AVAILABLE_IN_ALL
GType grl_ui_text_alignment_get_type (void) G_GNUC_CONST;
#define GRL_TYPE_UI_TEXT_ALIGNMENT (grl_ui_text_alignment_get_type ())

/**
 * GrlUiTextAlignmentVertical:
 * @GRL_UI_TEXT_ALIGN_TOP: Text aligned to the top
 * @GRL_UI_TEXT_ALIGN_MIDDLE: Text centered vertically
 * @GRL_UI_TEXT_ALIGN_BOTTOM: Text aligned to the bottom
 *
 * Vertical text alignment within a control.
 */
typedef enum
{
    GRL_UI_TEXT_ALIGN_TOP = 0,
    GRL_UI_TEXT_ALIGN_MIDDLE,
    GRL_UI_TEXT_ALIGN_BOTTOM
} GrlUiTextAlignmentVertical;

GRL_AVAILABLE_IN_ALL
GType grl_ui_text_alignment_vertical_get_type (void) G_GNUC_CONST;
#define GRL_TYPE_UI_TEXT_ALIGNMENT_VERTICAL (grl_ui_text_alignment_vertical_get_type ())

/**
 * GrlUiTextWrapMode:
 * @GRL_UI_TEXT_WRAP_NONE: No text wrapping
 * @GRL_UI_TEXT_WRAP_CHAR: Wrap text at character boundaries
 * @GRL_UI_TEXT_WRAP_WORD: Wrap text at word boundaries
 *
 * Text wrapping mode for multiline text.
 */
typedef enum
{
    GRL_UI_TEXT_WRAP_NONE = 0,
    GRL_UI_TEXT_WRAP_CHAR,
    GRL_UI_TEXT_WRAP_WORD
} GrlUiTextWrapMode;

GRL_AVAILABLE_IN_ALL
GType grl_ui_text_wrap_mode_get_type (void) G_GNUC_CONST;
#define GRL_TYPE_UI_TEXT_WRAP_MODE (grl_ui_text_wrap_mode_get_type ())

/**
 * GrlUiControlType:
 * @GRL_UI_CONTROL_DEFAULT: Default control (applies to all controls)
 * @GRL_UI_CONTROL_LABEL: Label control
 * @GRL_UI_CONTROL_BUTTON: Button control
 * @GRL_UI_CONTROL_TOGGLE: Toggle control
 * @GRL_UI_CONTROL_SLIDER: Slider control
 * @GRL_UI_CONTROL_PROGRESSBAR: Progress bar control
 * @GRL_UI_CONTROL_CHECKBOX: Check box control
 * @GRL_UI_CONTROL_COMBOBOX: Combo box control
 * @GRL_UI_CONTROL_DROPDOWNBOX: Dropdown box control
 * @GRL_UI_CONTROL_TEXTBOX: Text box control
 * @GRL_UI_CONTROL_VALUEBOX: Value box control
 * @GRL_UI_CONTROL_SPINNER: Spinner control
 * @GRL_UI_CONTROL_LISTVIEW: List view control
 * @GRL_UI_CONTROL_COLORPICKER: Color picker control
 * @GRL_UI_CONTROL_SCROLLBAR: Scroll bar control
 * @GRL_UI_CONTROL_STATUSBAR: Status bar control
 *
 * Type identifiers for UI controls (used for styling).
 */
typedef enum
{
    GRL_UI_CONTROL_DEFAULT = 0,
    GRL_UI_CONTROL_LABEL,
    GRL_UI_CONTROL_BUTTON,
    GRL_UI_CONTROL_TOGGLE,
    GRL_UI_CONTROL_SLIDER,
    GRL_UI_CONTROL_PROGRESSBAR,
    GRL_UI_CONTROL_CHECKBOX,
    GRL_UI_CONTROL_COMBOBOX,
    GRL_UI_CONTROL_DROPDOWNBOX,
    GRL_UI_CONTROL_TEXTBOX,
    GRL_UI_CONTROL_VALUEBOX,
    GRL_UI_CONTROL_SPINNER,
    GRL_UI_CONTROL_LISTVIEW,
    GRL_UI_CONTROL_COLORPICKER,
    GRL_UI_CONTROL_SCROLLBAR,
    GRL_UI_CONTROL_STATUSBAR
} GrlUiControlType;

GRL_AVAILABLE_IN_ALL
GType grl_ui_control_type_get_type (void) G_GNUC_CONST;
#define GRL_TYPE_UI_CONTROL_TYPE (grl_ui_control_type_get_type ())

/**
 * GrlUiProperty:
 * @GRL_UI_PROP_BORDER_COLOR_NORMAL: Border color in normal state
 * @GRL_UI_PROP_BASE_COLOR_NORMAL: Base color in normal state
 * @GRL_UI_PROP_TEXT_COLOR_NORMAL: Text color in normal state
 * @GRL_UI_PROP_BORDER_COLOR_FOCUSED: Border color in focused state
 * @GRL_UI_PROP_BASE_COLOR_FOCUSED: Base color in focused state
 * @GRL_UI_PROP_TEXT_COLOR_FOCUSED: Text color in focused state
 * @GRL_UI_PROP_BORDER_COLOR_PRESSED: Border color in pressed state
 * @GRL_UI_PROP_BASE_COLOR_PRESSED: Base color in pressed state
 * @GRL_UI_PROP_TEXT_COLOR_PRESSED: Text color in pressed state
 * @GRL_UI_PROP_BORDER_COLOR_DISABLED: Border color in disabled state
 * @GRL_UI_PROP_BASE_COLOR_DISABLED: Base color in disabled state
 * @GRL_UI_PROP_TEXT_COLOR_DISABLED: Text color in disabled state
 * @GRL_UI_PROP_BORDER_WIDTH: Border width (0 for no border)
 * @GRL_UI_PROP_TEXT_PADDING: Text padding inside control
 * @GRL_UI_PROP_TEXT_ALIGNMENT: Text horizontal alignment
 *
 * Base properties that apply to all controls.
 */
typedef enum
{
    GRL_UI_PROP_BORDER_COLOR_NORMAL = 0,
    GRL_UI_PROP_BASE_COLOR_NORMAL,
    GRL_UI_PROP_TEXT_COLOR_NORMAL,
    GRL_UI_PROP_BORDER_COLOR_FOCUSED,
    GRL_UI_PROP_BASE_COLOR_FOCUSED,
    GRL_UI_PROP_TEXT_COLOR_FOCUSED,
    GRL_UI_PROP_BORDER_COLOR_PRESSED,
    GRL_UI_PROP_BASE_COLOR_PRESSED,
    GRL_UI_PROP_TEXT_COLOR_PRESSED,
    GRL_UI_PROP_BORDER_COLOR_DISABLED,
    GRL_UI_PROP_BASE_COLOR_DISABLED,
    GRL_UI_PROP_TEXT_COLOR_DISABLED,
    GRL_UI_PROP_BORDER_WIDTH = 12,
    GRL_UI_PROP_TEXT_PADDING = 13,
    GRL_UI_PROP_TEXT_ALIGNMENT = 14
} GrlUiProperty;

GRL_AVAILABLE_IN_ALL
GType grl_ui_property_get_type (void) G_GNUC_CONST;
#define GRL_TYPE_UI_PROPERTY (grl_ui_property_get_type ())

/**
 * GrlUiDefaultProperty:
 * @GRL_UI_DEFAULT_TEXT_SIZE: Text size (glyph max height)
 * @GRL_UI_DEFAULT_TEXT_SPACING: Text spacing between glyphs
 * @GRL_UI_DEFAULT_LINE_COLOR: Line control color
 * @GRL_UI_DEFAULT_BACKGROUND_COLOR: Background color
 * @GRL_UI_DEFAULT_TEXT_LINE_SPACING: Text spacing between lines
 * @GRL_UI_DEFAULT_TEXT_ALIGNMENT_VERTICAL: Text vertical alignment
 * @GRL_UI_DEFAULT_TEXT_WRAP_MODE: Text wrap mode
 *
 * Default extended properties that are global to all controls.
 */
typedef enum
{
    GRL_UI_DEFAULT_TEXT_SIZE = 16,
    GRL_UI_DEFAULT_TEXT_SPACING,
    GRL_UI_DEFAULT_LINE_COLOR,
    GRL_UI_DEFAULT_BACKGROUND_COLOR,
    GRL_UI_DEFAULT_TEXT_LINE_SPACING,
    GRL_UI_DEFAULT_TEXT_ALIGNMENT_VERTICAL,
    GRL_UI_DEFAULT_TEXT_WRAP_MODE
} GrlUiDefaultProperty;

GRL_AVAILABLE_IN_ALL
GType grl_ui_default_property_get_type (void) G_GNUC_CONST;
#define GRL_TYPE_UI_DEFAULT_PROPERTY (grl_ui_default_property_get_type ())

G_END_DECLS
