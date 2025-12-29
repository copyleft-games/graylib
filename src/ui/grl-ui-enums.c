/* grl-ui-enums.c
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * UI enumeration type registration.
 */

#include "config.h"
#include "grl-ui-enums.h"

/**
 * grl_ui_state_get_type:
 *
 * Returns the #GType for #GrlUiState.
 *
 * Returns: The #GType for #GrlUiState
 */
GType
grl_ui_state_get_type (void)
{
    static GType type = 0;

    if (g_once_init_enter (&type))
    {
        static const GEnumValue values[] = {
            { GRL_UI_STATE_NORMAL, "GRL_UI_STATE_NORMAL", "normal" },
            { GRL_UI_STATE_FOCUSED, "GRL_UI_STATE_FOCUSED", "focused" },
            { GRL_UI_STATE_PRESSED, "GRL_UI_STATE_PRESSED", "pressed" },
            { GRL_UI_STATE_DISABLED, "GRL_UI_STATE_DISABLED", "disabled" },
            { 0, NULL, NULL }
        };

        GType new_type = g_enum_register_static ("GrlUiState", values);
        g_once_init_leave (&type, new_type);
    }

    return type;
}

/**
 * grl_ui_text_alignment_get_type:
 *
 * Returns the #GType for #GrlUiTextAlignment.
 *
 * Returns: The #GType for #GrlUiTextAlignment
 */
GType
grl_ui_text_alignment_get_type (void)
{
    static GType type = 0;

    if (g_once_init_enter (&type))
    {
        static const GEnumValue values[] = {
            { GRL_UI_TEXT_ALIGN_LEFT, "GRL_UI_TEXT_ALIGN_LEFT", "left" },
            { GRL_UI_TEXT_ALIGN_CENTER, "GRL_UI_TEXT_ALIGN_CENTER", "center" },
            { GRL_UI_TEXT_ALIGN_RIGHT, "GRL_UI_TEXT_ALIGN_RIGHT", "right" },
            { 0, NULL, NULL }
        };

        GType new_type = g_enum_register_static ("GrlUiTextAlignment", values);
        g_once_init_leave (&type, new_type);
    }

    return type;
}

/**
 * grl_ui_text_alignment_vertical_get_type:
 *
 * Returns the #GType for #GrlUiTextAlignmentVertical.
 *
 * Returns: The #GType for #GrlUiTextAlignmentVertical
 */
GType
grl_ui_text_alignment_vertical_get_type (void)
{
    static GType type = 0;

    if (g_once_init_enter (&type))
    {
        static const GEnumValue values[] = {
            { GRL_UI_TEXT_ALIGN_TOP, "GRL_UI_TEXT_ALIGN_TOP", "top" },
            { GRL_UI_TEXT_ALIGN_MIDDLE, "GRL_UI_TEXT_ALIGN_MIDDLE", "middle" },
            { GRL_UI_TEXT_ALIGN_BOTTOM, "GRL_UI_TEXT_ALIGN_BOTTOM", "bottom" },
            { 0, NULL, NULL }
        };

        GType new_type = g_enum_register_static ("GrlUiTextAlignmentVertical", values);
        g_once_init_leave (&type, new_type);
    }

    return type;
}

/**
 * grl_ui_text_wrap_mode_get_type:
 *
 * Returns the #GType for #GrlUiTextWrapMode.
 *
 * Returns: The #GType for #GrlUiTextWrapMode
 */
GType
grl_ui_text_wrap_mode_get_type (void)
{
    static GType type = 0;

    if (g_once_init_enter (&type))
    {
        static const GEnumValue values[] = {
            { GRL_UI_TEXT_WRAP_NONE, "GRL_UI_TEXT_WRAP_NONE", "none" },
            { GRL_UI_TEXT_WRAP_CHAR, "GRL_UI_TEXT_WRAP_CHAR", "char" },
            { GRL_UI_TEXT_WRAP_WORD, "GRL_UI_TEXT_WRAP_WORD", "word" },
            { 0, NULL, NULL }
        };

        GType new_type = g_enum_register_static ("GrlUiTextWrapMode", values);
        g_once_init_leave (&type, new_type);
    }

    return type;
}

/**
 * grl_ui_control_type_get_type:
 *
 * Returns the #GType for #GrlUiControlType.
 *
 * Returns: The #GType for #GrlUiControlType
 */
GType
grl_ui_control_type_get_type (void)
{
    static GType type = 0;

    if (g_once_init_enter (&type))
    {
        static const GEnumValue values[] = {
            { GRL_UI_CONTROL_DEFAULT, "GRL_UI_CONTROL_DEFAULT", "default" },
            { GRL_UI_CONTROL_LABEL, "GRL_UI_CONTROL_LABEL", "label" },
            { GRL_UI_CONTROL_BUTTON, "GRL_UI_CONTROL_BUTTON", "button" },
            { GRL_UI_CONTROL_TOGGLE, "GRL_UI_CONTROL_TOGGLE", "toggle" },
            { GRL_UI_CONTROL_SLIDER, "GRL_UI_CONTROL_SLIDER", "slider" },
            { GRL_UI_CONTROL_PROGRESSBAR, "GRL_UI_CONTROL_PROGRESSBAR", "progressbar" },
            { GRL_UI_CONTROL_CHECKBOX, "GRL_UI_CONTROL_CHECKBOX", "checkbox" },
            { GRL_UI_CONTROL_COMBOBOX, "GRL_UI_CONTROL_COMBOBOX", "combobox" },
            { GRL_UI_CONTROL_DROPDOWNBOX, "GRL_UI_CONTROL_DROPDOWNBOX", "dropdownbox" },
            { GRL_UI_CONTROL_TEXTBOX, "GRL_UI_CONTROL_TEXTBOX", "textbox" },
            { GRL_UI_CONTROL_VALUEBOX, "GRL_UI_CONTROL_VALUEBOX", "valuebox" },
            { GRL_UI_CONTROL_SPINNER, "GRL_UI_CONTROL_SPINNER", "spinner" },
            { GRL_UI_CONTROL_LISTVIEW, "GRL_UI_CONTROL_LISTVIEW", "listview" },
            { GRL_UI_CONTROL_COLORPICKER, "GRL_UI_CONTROL_COLORPICKER", "colorpicker" },
            { GRL_UI_CONTROL_SCROLLBAR, "GRL_UI_CONTROL_SCROLLBAR", "scrollbar" },
            { GRL_UI_CONTROL_STATUSBAR, "GRL_UI_CONTROL_STATUSBAR", "statusbar" },
            { 0, NULL, NULL }
        };

        GType new_type = g_enum_register_static ("GrlUiControlType", values);
        g_once_init_leave (&type, new_type);
    }

    return type;
}

/**
 * grl_ui_property_get_type:
 *
 * Returns the #GType for #GrlUiProperty.
 *
 * Returns: The #GType for #GrlUiProperty
 */
GType
grl_ui_property_get_type (void)
{
    static GType type = 0;

    if (g_once_init_enter (&type))
    {
        static const GEnumValue values[] = {
            { GRL_UI_PROP_BORDER_COLOR_NORMAL, "GRL_UI_PROP_BORDER_COLOR_NORMAL", "border-color-normal" },
            { GRL_UI_PROP_BASE_COLOR_NORMAL, "GRL_UI_PROP_BASE_COLOR_NORMAL", "base-color-normal" },
            { GRL_UI_PROP_TEXT_COLOR_NORMAL, "GRL_UI_PROP_TEXT_COLOR_NORMAL", "text-color-normal" },
            { GRL_UI_PROP_BORDER_COLOR_FOCUSED, "GRL_UI_PROP_BORDER_COLOR_FOCUSED", "border-color-focused" },
            { GRL_UI_PROP_BASE_COLOR_FOCUSED, "GRL_UI_PROP_BASE_COLOR_FOCUSED", "base-color-focused" },
            { GRL_UI_PROP_TEXT_COLOR_FOCUSED, "GRL_UI_PROP_TEXT_COLOR_FOCUSED", "text-color-focused" },
            { GRL_UI_PROP_BORDER_COLOR_PRESSED, "GRL_UI_PROP_BORDER_COLOR_PRESSED", "border-color-pressed" },
            { GRL_UI_PROP_BASE_COLOR_PRESSED, "GRL_UI_PROP_BASE_COLOR_PRESSED", "base-color-pressed" },
            { GRL_UI_PROP_TEXT_COLOR_PRESSED, "GRL_UI_PROP_TEXT_COLOR_PRESSED", "text-color-pressed" },
            { GRL_UI_PROP_BORDER_COLOR_DISABLED, "GRL_UI_PROP_BORDER_COLOR_DISABLED", "border-color-disabled" },
            { GRL_UI_PROP_BASE_COLOR_DISABLED, "GRL_UI_PROP_BASE_COLOR_DISABLED", "base-color-disabled" },
            { GRL_UI_PROP_TEXT_COLOR_DISABLED, "GRL_UI_PROP_TEXT_COLOR_DISABLED", "text-color-disabled" },
            { GRL_UI_PROP_BORDER_WIDTH, "GRL_UI_PROP_BORDER_WIDTH", "border-width" },
            { GRL_UI_PROP_TEXT_PADDING, "GRL_UI_PROP_TEXT_PADDING", "text-padding" },
            { GRL_UI_PROP_TEXT_ALIGNMENT, "GRL_UI_PROP_TEXT_ALIGNMENT", "text-alignment" },
            { 0, NULL, NULL }
        };

        GType new_type = g_enum_register_static ("GrlUiProperty", values);
        g_once_init_leave (&type, new_type);
    }

    return type;
}

/**
 * grl_ui_default_property_get_type:
 *
 * Returns the #GType for #GrlUiDefaultProperty.
 *
 * Returns: The #GType for #GrlUiDefaultProperty
 */
GType
grl_ui_default_property_get_type (void)
{
    static GType type = 0;

    if (g_once_init_enter (&type))
    {
        static const GEnumValue values[] = {
            { GRL_UI_DEFAULT_TEXT_SIZE, "GRL_UI_DEFAULT_TEXT_SIZE", "text-size" },
            { GRL_UI_DEFAULT_TEXT_SPACING, "GRL_UI_DEFAULT_TEXT_SPACING", "text-spacing" },
            { GRL_UI_DEFAULT_LINE_COLOR, "GRL_UI_DEFAULT_LINE_COLOR", "line-color" },
            { GRL_UI_DEFAULT_BACKGROUND_COLOR, "GRL_UI_DEFAULT_BACKGROUND_COLOR", "background-color" },
            { GRL_UI_DEFAULT_TEXT_LINE_SPACING, "GRL_UI_DEFAULT_TEXT_LINE_SPACING", "text-line-spacing" },
            { GRL_UI_DEFAULT_TEXT_ALIGNMENT_VERTICAL, "GRL_UI_DEFAULT_TEXT_ALIGNMENT_VERTICAL", "text-alignment-vertical" },
            { GRL_UI_DEFAULT_TEXT_WRAP_MODE, "GRL_UI_DEFAULT_TEXT_WRAP_MODE", "text-wrap-mode" },
            { 0, NULL, NULL }
        };

        GType new_type = g_enum_register_static ("GrlUiDefaultProperty", values);
        g_once_init_leave (&type, new_type);
    }

    return type;
}
