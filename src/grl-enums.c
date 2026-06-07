/* grl-enums.c
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * GType registration for Graylib enumerations.
 */

#include "grl-enums.h"

/*
 * GrlConfigFlags
 */
GType
grl_config_flags_get_type (void)
{
    static volatile gsize g_define_type_id__volatile = 0;

    if (g_once_init_enter (&g_define_type_id__volatile))
    {
        static const GFlagsValue values[] = {
            { GRL_FLAG_VSYNC_HINT, "GRL_FLAG_VSYNC_HINT", "vsync-hint" },
            { GRL_FLAG_FULLSCREEN_MODE, "GRL_FLAG_FULLSCREEN_MODE", "fullscreen-mode" },
            { GRL_FLAG_WINDOW_RESIZABLE, "GRL_FLAG_WINDOW_RESIZABLE", "window-resizable" },
            { GRL_FLAG_WINDOW_UNDECORATED, "GRL_FLAG_WINDOW_UNDECORATED", "window-undecorated" },
            { GRL_FLAG_WINDOW_HIDDEN, "GRL_FLAG_WINDOW_HIDDEN", "window-hidden" },
            { GRL_FLAG_WINDOW_MINIMIZED, "GRL_FLAG_WINDOW_MINIMIZED", "window-minimized" },
            { GRL_FLAG_WINDOW_MAXIMIZED, "GRL_FLAG_WINDOW_MAXIMIZED", "window-maximized" },
            { GRL_FLAG_WINDOW_UNFOCUSED, "GRL_FLAG_WINDOW_UNFOCUSED", "window-unfocused" },
            { GRL_FLAG_WINDOW_TOPMOST, "GRL_FLAG_WINDOW_TOPMOST", "window-topmost" },
            { GRL_FLAG_WINDOW_ALWAYS_RUN, "GRL_FLAG_WINDOW_ALWAYS_RUN", "window-always-run" },
            { GRL_FLAG_WINDOW_TRANSPARENT, "GRL_FLAG_WINDOW_TRANSPARENT", "window-transparent" },
            { GRL_FLAG_WINDOW_HIGHDPI, "GRL_FLAG_WINDOW_HIGHDPI", "window-highdpi" },
            { GRL_FLAG_WINDOW_MOUSE_PASSTHROUGH, "GRL_FLAG_WINDOW_MOUSE_PASSTHROUGH", "window-mouse-passthrough" },
            { GRL_FLAG_BORDERLESS_WINDOWED_MODE, "GRL_FLAG_BORDERLESS_WINDOWED_MODE", "borderless-windowed-mode" },
            { GRL_FLAG_MSAA_4X_HINT, "GRL_FLAG_MSAA_4X_HINT", "msaa-4x-hint" },
            { GRL_FLAG_INTERLACED_HINT, "GRL_FLAG_INTERLACED_HINT", "interlaced-hint" },
            { 0, NULL, NULL }
        };
        GType g_define_type_id = g_flags_register_static ("GrlConfigFlags", values);
        g_once_init_leave (&g_define_type_id__volatile, g_define_type_id);
    }

    return g_define_type_id__volatile;
}

/*
 * GrlLogLevel
 */
GType
grl_log_level_get_type (void)
{
    static volatile gsize g_define_type_id__volatile = 0;

    if (g_once_init_enter (&g_define_type_id__volatile))
    {
        static const GEnumValue values[] = {
            { GRL_LOG_ALL, "GRL_LOG_ALL", "all" },
            { GRL_LOG_TRACE, "GRL_LOG_TRACE", "trace" },
            { GRL_LOG_DEBUG, "GRL_LOG_DEBUG", "debug" },
            { GRL_LOG_INFO, "GRL_LOG_INFO", "info" },
            { GRL_LOG_WARNING, "GRL_LOG_WARNING", "warning" },
            { GRL_LOG_ERROR, "GRL_LOG_ERROR", "error" },
            { GRL_LOG_FATAL, "GRL_LOG_FATAL", "fatal" },
            { GRL_LOG_NONE, "GRL_LOG_NONE", "none" },
            { 0, NULL, NULL }
        };
        GType g_define_type_id = g_enum_register_static ("GrlLogLevel", values);
        g_once_init_leave (&g_define_type_id__volatile, g_define_type_id);
    }

    return g_define_type_id__volatile;
}

/*
 * GrlKey
 */
GType
grl_key_get_type (void)
{
    static volatile gsize g_define_type_id__volatile = 0;

    if (g_once_init_enter (&g_define_type_id__volatile))
    {
        static const GEnumValue values[] = {
            { GRL_KEY_NULL, "GRL_KEY_NULL", "null" },
            { GRL_KEY_APOSTROPHE, "GRL_KEY_APOSTROPHE", "apostrophe" },
            { GRL_KEY_COMMA, "GRL_KEY_COMMA", "comma" },
            { GRL_KEY_MINUS, "GRL_KEY_MINUS", "minus" },
            { GRL_KEY_PERIOD, "GRL_KEY_PERIOD", "period" },
            { GRL_KEY_SLASH, "GRL_KEY_SLASH", "slash" },
            { GRL_KEY_ZERO, "GRL_KEY_ZERO", "zero" },
            { GRL_KEY_ONE, "GRL_KEY_ONE", "one" },
            { GRL_KEY_TWO, "GRL_KEY_TWO", "two" },
            { GRL_KEY_THREE, "GRL_KEY_THREE", "three" },
            { GRL_KEY_FOUR, "GRL_KEY_FOUR", "four" },
            { GRL_KEY_FIVE, "GRL_KEY_FIVE", "five" },
            { GRL_KEY_SIX, "GRL_KEY_SIX", "six" },
            { GRL_KEY_SEVEN, "GRL_KEY_SEVEN", "seven" },
            { GRL_KEY_EIGHT, "GRL_KEY_EIGHT", "eight" },
            { GRL_KEY_NINE, "GRL_KEY_NINE", "nine" },
            { GRL_KEY_SEMICOLON, "GRL_KEY_SEMICOLON", "semicolon" },
            { GRL_KEY_EQUAL, "GRL_KEY_EQUAL", "equal" },
            { GRL_KEY_A, "GRL_KEY_A", "a" },
            { GRL_KEY_B, "GRL_KEY_B", "b" },
            { GRL_KEY_C, "GRL_KEY_C", "c" },
            { GRL_KEY_D, "GRL_KEY_D", "d" },
            { GRL_KEY_E, "GRL_KEY_E", "e" },
            { GRL_KEY_F, "GRL_KEY_F", "f" },
            { GRL_KEY_G, "GRL_KEY_G", "g" },
            { GRL_KEY_H, "GRL_KEY_H", "h" },
            { GRL_KEY_I, "GRL_KEY_I", "i" },
            { GRL_KEY_J, "GRL_KEY_J", "j" },
            { GRL_KEY_K, "GRL_KEY_K", "k" },
            { GRL_KEY_L, "GRL_KEY_L", "l" },
            { GRL_KEY_M, "GRL_KEY_M", "m" },
            { GRL_KEY_N, "GRL_KEY_N", "n" },
            { GRL_KEY_O, "GRL_KEY_O", "o" },
            { GRL_KEY_P, "GRL_KEY_P", "p" },
            { GRL_KEY_Q, "GRL_KEY_Q", "q" },
            { GRL_KEY_R, "GRL_KEY_R", "r" },
            { GRL_KEY_S, "GRL_KEY_S", "s" },
            { GRL_KEY_T, "GRL_KEY_T", "t" },
            { GRL_KEY_U, "GRL_KEY_U", "u" },
            { GRL_KEY_V, "GRL_KEY_V", "v" },
            { GRL_KEY_W, "GRL_KEY_W", "w" },
            { GRL_KEY_X, "GRL_KEY_X", "x" },
            { GRL_KEY_Y, "GRL_KEY_Y", "y" },
            { GRL_KEY_Z, "GRL_KEY_Z", "z" },
            { GRL_KEY_LEFT_BRACKET, "GRL_KEY_LEFT_BRACKET", "left-bracket" },
            { GRL_KEY_BACKSLASH, "GRL_KEY_BACKSLASH", "backslash" },
            { GRL_KEY_RIGHT_BRACKET, "GRL_KEY_RIGHT_BRACKET", "right-bracket" },
            { GRL_KEY_GRAVE, "GRL_KEY_GRAVE", "grave" },
            { GRL_KEY_SPACE, "GRL_KEY_SPACE", "space" },
            { GRL_KEY_ESCAPE, "GRL_KEY_ESCAPE", "escape" },
            { GRL_KEY_ENTER, "GRL_KEY_ENTER", "enter" },
            { GRL_KEY_TAB, "GRL_KEY_TAB", "tab" },
            { GRL_KEY_BACKSPACE, "GRL_KEY_BACKSPACE", "backspace" },
            { GRL_KEY_INSERT, "GRL_KEY_INSERT", "insert" },
            { GRL_KEY_DELETE, "GRL_KEY_DELETE", "delete" },
            { GRL_KEY_RIGHT, "GRL_KEY_RIGHT", "right" },
            { GRL_KEY_LEFT, "GRL_KEY_LEFT", "left" },
            { GRL_KEY_DOWN, "GRL_KEY_DOWN", "down" },
            { GRL_KEY_UP, "GRL_KEY_UP", "up" },
            { GRL_KEY_PAGE_UP, "GRL_KEY_PAGE_UP", "page-up" },
            { GRL_KEY_PAGE_DOWN, "GRL_KEY_PAGE_DOWN", "page-down" },
            { GRL_KEY_HOME, "GRL_KEY_HOME", "home" },
            { GRL_KEY_END, "GRL_KEY_END", "end" },
            { GRL_KEY_CAPS_LOCK, "GRL_KEY_CAPS_LOCK", "caps-lock" },
            { GRL_KEY_SCROLL_LOCK, "GRL_KEY_SCROLL_LOCK", "scroll-lock" },
            { GRL_KEY_NUM_LOCK, "GRL_KEY_NUM_LOCK", "num-lock" },
            { GRL_KEY_PRINT_SCREEN, "GRL_KEY_PRINT_SCREEN", "print-screen" },
            { GRL_KEY_PAUSE, "GRL_KEY_PAUSE", "pause" },
            { GRL_KEY_F1, "GRL_KEY_F1", "f1" },
            { GRL_KEY_F2, "GRL_KEY_F2", "f2" },
            { GRL_KEY_F3, "GRL_KEY_F3", "f3" },
            { GRL_KEY_F4, "GRL_KEY_F4", "f4" },
            { GRL_KEY_F5, "GRL_KEY_F5", "f5" },
            { GRL_KEY_F6, "GRL_KEY_F6", "f6" },
            { GRL_KEY_F7, "GRL_KEY_F7", "f7" },
            { GRL_KEY_F8, "GRL_KEY_F8", "f8" },
            { GRL_KEY_F9, "GRL_KEY_F9", "f9" },
            { GRL_KEY_F10, "GRL_KEY_F10", "f10" },
            { GRL_KEY_F11, "GRL_KEY_F11", "f11" },
            { GRL_KEY_F12, "GRL_KEY_F12", "f12" },
            { GRL_KEY_LEFT_SHIFT, "GRL_KEY_LEFT_SHIFT", "left-shift" },
            { GRL_KEY_LEFT_CONTROL, "GRL_KEY_LEFT_CONTROL", "left-control" },
            { GRL_KEY_LEFT_ALT, "GRL_KEY_LEFT_ALT", "left-alt" },
            { GRL_KEY_LEFT_SUPER, "GRL_KEY_LEFT_SUPER", "left-super" },
            { GRL_KEY_RIGHT_SHIFT, "GRL_KEY_RIGHT_SHIFT", "right-shift" },
            { GRL_KEY_RIGHT_CONTROL, "GRL_KEY_RIGHT_CONTROL", "right-control" },
            { GRL_KEY_RIGHT_ALT, "GRL_KEY_RIGHT_ALT", "right-alt" },
            { GRL_KEY_RIGHT_SUPER, "GRL_KEY_RIGHT_SUPER", "right-super" },
            { GRL_KEY_KB_MENU, "GRL_KEY_KB_MENU", "kb-menu" },
            { GRL_KEY_KP_0, "GRL_KEY_KP_0", "kp-0" },
            { GRL_KEY_KP_1, "GRL_KEY_KP_1", "kp-1" },
            { GRL_KEY_KP_2, "GRL_KEY_KP_2", "kp-2" },
            { GRL_KEY_KP_3, "GRL_KEY_KP_3", "kp-3" },
            { GRL_KEY_KP_4, "GRL_KEY_KP_4", "kp-4" },
            { GRL_KEY_KP_5, "GRL_KEY_KP_5", "kp-5" },
            { GRL_KEY_KP_6, "GRL_KEY_KP_6", "kp-6" },
            { GRL_KEY_KP_7, "GRL_KEY_KP_7", "kp-7" },
            { GRL_KEY_KP_8, "GRL_KEY_KP_8", "kp-8" },
            { GRL_KEY_KP_9, "GRL_KEY_KP_9", "kp-9" },
            { GRL_KEY_KP_DECIMAL, "GRL_KEY_KP_DECIMAL", "kp-decimal" },
            { GRL_KEY_KP_DIVIDE, "GRL_KEY_KP_DIVIDE", "kp-divide" },
            { GRL_KEY_KP_MULTIPLY, "GRL_KEY_KP_MULTIPLY", "kp-multiply" },
            { GRL_KEY_KP_SUBTRACT, "GRL_KEY_KP_SUBTRACT", "kp-subtract" },
            { GRL_KEY_KP_ADD, "GRL_KEY_KP_ADD", "kp-add" },
            { GRL_KEY_KP_ENTER, "GRL_KEY_KP_ENTER", "kp-enter" },
            { GRL_KEY_KP_EQUAL, "GRL_KEY_KP_EQUAL", "kp-equal" },
            { GRL_KEY_BACK, "GRL_KEY_BACK", "back" },
            { GRL_KEY_MENU, "GRL_KEY_MENU", "menu" },
            { GRL_KEY_VOLUME_UP, "GRL_KEY_VOLUME_UP", "volume-up" },
            { GRL_KEY_VOLUME_DOWN, "GRL_KEY_VOLUME_DOWN", "volume-down" },
            { 0, NULL, NULL }
        };
        GType g_define_type_id = g_enum_register_static ("GrlKey", values);
        g_once_init_leave (&g_define_type_id__volatile, g_define_type_id);
    }

    return g_define_type_id__volatile;
}

/*
 * GrlMouseButton
 */
GType
grl_mouse_button_get_type (void)
{
    static volatile gsize g_define_type_id__volatile = 0;

    if (g_once_init_enter (&g_define_type_id__volatile))
    {
        static const GEnumValue values[] = {
            { GRL_MOUSE_BUTTON_LEFT, "GRL_MOUSE_BUTTON_LEFT", "left" },
            { GRL_MOUSE_BUTTON_RIGHT, "GRL_MOUSE_BUTTON_RIGHT", "right" },
            { GRL_MOUSE_BUTTON_MIDDLE, "GRL_MOUSE_BUTTON_MIDDLE", "middle" },
            { GRL_MOUSE_BUTTON_SIDE, "GRL_MOUSE_BUTTON_SIDE", "side" },
            { GRL_MOUSE_BUTTON_EXTRA, "GRL_MOUSE_BUTTON_EXTRA", "extra" },
            { GRL_MOUSE_BUTTON_FORWARD, "GRL_MOUSE_BUTTON_FORWARD", "forward" },
            { GRL_MOUSE_BUTTON_BACK, "GRL_MOUSE_BUTTON_BACK", "back" },
            { 0, NULL, NULL }
        };
        GType g_define_type_id = g_enum_register_static ("GrlMouseButton", values);
        g_once_init_leave (&g_define_type_id__volatile, g_define_type_id);
    }

    return g_define_type_id__volatile;
}

/*
 * GrlMouseCursor
 */
GType
grl_mouse_cursor_get_type (void)
{
    static volatile gsize g_define_type_id__volatile = 0;

    if (g_once_init_enter (&g_define_type_id__volatile))
    {
        static const GEnumValue values[] = {
            { GRL_MOUSE_CURSOR_DEFAULT, "GRL_MOUSE_CURSOR_DEFAULT", "default" },
            { GRL_MOUSE_CURSOR_ARROW, "GRL_MOUSE_CURSOR_ARROW", "arrow" },
            { GRL_MOUSE_CURSOR_IBEAM, "GRL_MOUSE_CURSOR_IBEAM", "ibeam" },
            { GRL_MOUSE_CURSOR_CROSSHAIR, "GRL_MOUSE_CURSOR_CROSSHAIR", "crosshair" },
            { GRL_MOUSE_CURSOR_POINTING_HAND, "GRL_MOUSE_CURSOR_POINTING_HAND", "pointing-hand" },
            { GRL_MOUSE_CURSOR_RESIZE_EW, "GRL_MOUSE_CURSOR_RESIZE_EW", "resize-ew" },
            { GRL_MOUSE_CURSOR_RESIZE_NS, "GRL_MOUSE_CURSOR_RESIZE_NS", "resize-ns" },
            { GRL_MOUSE_CURSOR_RESIZE_NWSE, "GRL_MOUSE_CURSOR_RESIZE_NWSE", "resize-nwse" },
            { GRL_MOUSE_CURSOR_RESIZE_NESW, "GRL_MOUSE_CURSOR_RESIZE_NESW", "resize-nesw" },
            { GRL_MOUSE_CURSOR_RESIZE_ALL, "GRL_MOUSE_CURSOR_RESIZE_ALL", "resize-all" },
            { GRL_MOUSE_CURSOR_NOT_ALLOWED, "GRL_MOUSE_CURSOR_NOT_ALLOWED", "not-allowed" },
            { 0, NULL, NULL }
        };
        GType g_define_type_id = g_enum_register_static ("GrlMouseCursor", values);
        g_once_init_leave (&g_define_type_id__volatile, g_define_type_id);
    }

    return g_define_type_id__volatile;
}

/*
 * GrlGamepadButton
 */
GType
grl_gamepad_button_get_type (void)
{
    static volatile gsize g_define_type_id__volatile = 0;

    if (g_once_init_enter (&g_define_type_id__volatile))
    {
        static const GEnumValue values[] = {
            { GRL_GAMEPAD_BUTTON_UNKNOWN, "GRL_GAMEPAD_BUTTON_UNKNOWN", "unknown" },
            { GRL_GAMEPAD_BUTTON_LEFT_FACE_UP, "GRL_GAMEPAD_BUTTON_LEFT_FACE_UP", "left-face-up" },
            { GRL_GAMEPAD_BUTTON_LEFT_FACE_RIGHT, "GRL_GAMEPAD_BUTTON_LEFT_FACE_RIGHT", "left-face-right" },
            { GRL_GAMEPAD_BUTTON_LEFT_FACE_DOWN, "GRL_GAMEPAD_BUTTON_LEFT_FACE_DOWN", "left-face-down" },
            { GRL_GAMEPAD_BUTTON_LEFT_FACE_LEFT, "GRL_GAMEPAD_BUTTON_LEFT_FACE_LEFT", "left-face-left" },
            { GRL_GAMEPAD_BUTTON_RIGHT_FACE_UP, "GRL_GAMEPAD_BUTTON_RIGHT_FACE_UP", "right-face-up" },
            { GRL_GAMEPAD_BUTTON_RIGHT_FACE_RIGHT, "GRL_GAMEPAD_BUTTON_RIGHT_FACE_RIGHT", "right-face-right" },
            { GRL_GAMEPAD_BUTTON_RIGHT_FACE_DOWN, "GRL_GAMEPAD_BUTTON_RIGHT_FACE_DOWN", "right-face-down" },
            { GRL_GAMEPAD_BUTTON_RIGHT_FACE_LEFT, "GRL_GAMEPAD_BUTTON_RIGHT_FACE_LEFT", "right-face-left" },
            { GRL_GAMEPAD_BUTTON_LEFT_TRIGGER_1, "GRL_GAMEPAD_BUTTON_LEFT_TRIGGER_1", "left-trigger-1" },
            { GRL_GAMEPAD_BUTTON_LEFT_TRIGGER_2, "GRL_GAMEPAD_BUTTON_LEFT_TRIGGER_2", "left-trigger-2" },
            { GRL_GAMEPAD_BUTTON_RIGHT_TRIGGER_1, "GRL_GAMEPAD_BUTTON_RIGHT_TRIGGER_1", "right-trigger-1" },
            { GRL_GAMEPAD_BUTTON_RIGHT_TRIGGER_2, "GRL_GAMEPAD_BUTTON_RIGHT_TRIGGER_2", "right-trigger-2" },
            { GRL_GAMEPAD_BUTTON_MIDDLE_LEFT, "GRL_GAMEPAD_BUTTON_MIDDLE_LEFT", "middle-left" },
            { GRL_GAMEPAD_BUTTON_MIDDLE, "GRL_GAMEPAD_BUTTON_MIDDLE", "middle" },
            { GRL_GAMEPAD_BUTTON_MIDDLE_RIGHT, "GRL_GAMEPAD_BUTTON_MIDDLE_RIGHT", "middle-right" },
            { GRL_GAMEPAD_BUTTON_LEFT_THUMB, "GRL_GAMEPAD_BUTTON_LEFT_THUMB", "left-thumb" },
            { GRL_GAMEPAD_BUTTON_RIGHT_THUMB, "GRL_GAMEPAD_BUTTON_RIGHT_THUMB", "right-thumb" },
            { 0, NULL, NULL }
        };
        GType g_define_type_id = g_enum_register_static ("GrlGamepadButton", values);
        g_once_init_leave (&g_define_type_id__volatile, g_define_type_id);
    }

    return g_define_type_id__volatile;
}

/*
 * GrlGamepadAxis
 */
GType
grl_gamepad_axis_get_type (void)
{
    static volatile gsize g_define_type_id__volatile = 0;

    if (g_once_init_enter (&g_define_type_id__volatile))
    {
        static const GEnumValue values[] = {
            { GRL_GAMEPAD_AXIS_LEFT_X, "GRL_GAMEPAD_AXIS_LEFT_X", "left-x" },
            { GRL_GAMEPAD_AXIS_LEFT_Y, "GRL_GAMEPAD_AXIS_LEFT_Y", "left-y" },
            { GRL_GAMEPAD_AXIS_RIGHT_X, "GRL_GAMEPAD_AXIS_RIGHT_X", "right-x" },
            { GRL_GAMEPAD_AXIS_RIGHT_Y, "GRL_GAMEPAD_AXIS_RIGHT_Y", "right-y" },
            { GRL_GAMEPAD_AXIS_LEFT_TRIGGER, "GRL_GAMEPAD_AXIS_LEFT_TRIGGER", "left-trigger" },
            { GRL_GAMEPAD_AXIS_RIGHT_TRIGGER, "GRL_GAMEPAD_AXIS_RIGHT_TRIGGER", "right-trigger" },
            { 0, NULL, NULL }
        };
        GType g_define_type_id = g_enum_register_static ("GrlGamepadAxis", values);
        g_once_init_leave (&g_define_type_id__volatile, g_define_type_id);
    }

    return g_define_type_id__volatile;
}

/*
 * GrlBlendMode
 */
GType
grl_blend_mode_get_type (void)
{
    static volatile gsize g_define_type_id__volatile = 0;

    if (g_once_init_enter (&g_define_type_id__volatile))
    {
        static const GEnumValue values[] = {
            { GRL_BLEND_ALPHA, "GRL_BLEND_ALPHA", "alpha" },
            { GRL_BLEND_ADDITIVE, "GRL_BLEND_ADDITIVE", "additive" },
            { GRL_BLEND_MULTIPLIED, "GRL_BLEND_MULTIPLIED", "multiplied" },
            { GRL_BLEND_ADD_COLORS, "GRL_BLEND_ADD_COLORS", "add-colors" },
            { GRL_BLEND_SUBTRACT_COLORS, "GRL_BLEND_SUBTRACT_COLORS", "subtract-colors" },
            { GRL_BLEND_ALPHA_PREMULTIPLY, "GRL_BLEND_ALPHA_PREMULTIPLY", "alpha-premultiply" },
            { GRL_BLEND_CUSTOM, "GRL_BLEND_CUSTOM", "custom" },
            { GRL_BLEND_CUSTOM_SEPARATE, "GRL_BLEND_CUSTOM_SEPARATE", "custom-separate" },
            { 0, NULL, NULL }
        };
        GType g_define_type_id = g_enum_register_static ("GrlBlendMode", values);
        g_once_init_leave (&g_define_type_id__volatile, g_define_type_id);
    }

    return g_define_type_id__volatile;
}

/*
 * GrlImageBlendMode
 */
GType
grl_image_blend_mode_get_type (void)
{
    static volatile gsize g_define_type_id__volatile = 0;

    if (g_once_init_enter (&g_define_type_id__volatile))
    {
        static const GEnumValue values[] = {
            { GRL_IMAGE_BLEND_REPLACE, "GRL_IMAGE_BLEND_REPLACE", "replace" },
            { GRL_IMAGE_BLEND_OVER, "GRL_IMAGE_BLEND_OVER", "over" },
            { GRL_IMAGE_BLEND_ADD, "GRL_IMAGE_BLEND_ADD", "add" },
            { GRL_IMAGE_BLEND_MULTIPLY, "GRL_IMAGE_BLEND_MULTIPLY", "multiply" },
            { GRL_IMAGE_BLEND_SUBTRACT, "GRL_IMAGE_BLEND_SUBTRACT", "subtract" },
            { 0, NULL, NULL }
        };
        GType g_define_type_id = g_enum_register_static ("GrlImageBlendMode", values);
        g_once_init_leave (&g_define_type_id__volatile, g_define_type_id);
    }

    return g_define_type_id__volatile;
}

/*
 * GrlImageColorSpace
 */
GType
grl_image_color_space_get_type (void)
{
    static volatile gsize g_define_type_id__volatile = 0;

    if (g_once_init_enter (&g_define_type_id__volatile))
    {
        static const GEnumValue values[] = {
            { GRL_IMAGE_COLOR_SPACE_GAMMA, "GRL_IMAGE_COLOR_SPACE_GAMMA", "gamma" },
            { GRL_IMAGE_COLOR_SPACE_LINEAR, "GRL_IMAGE_COLOR_SPACE_LINEAR", "linear" },
            { 0, NULL, NULL }
        };
        GType g_define_type_id = g_enum_register_static ("GrlImageColorSpace", values);
        g_once_init_leave (&g_define_type_id__volatile, g_define_type_id);
    }

    return g_define_type_id__volatile;
}

/*
 * GrlGradientAxis
 */
GType
grl_gradient_axis_get_type (void)
{
    static volatile gsize g_define_type_id__volatile = 0;

    if (g_once_init_enter (&g_define_type_id__volatile))
    {
        static const GEnumValue values[] = {
            { GRL_GRADIENT_AXIS_HORIZONTAL, "GRL_GRADIENT_AXIS_HORIZONTAL", "horizontal" },
            { GRL_GRADIENT_AXIS_VERTICAL, "GRL_GRADIENT_AXIS_VERTICAL", "vertical" },
            { 0, NULL, NULL }
        };
        GType g_define_type_id = g_enum_register_static ("GrlGradientAxis", values);
        g_once_init_leave (&g_define_type_id__volatile, g_define_type_id);
    }

    return g_define_type_id__volatile;
}

/*
 * GrlTextureFilter
 */
GType
grl_texture_filter_get_type (void)
{
    static volatile gsize g_define_type_id__volatile = 0;

    if (g_once_init_enter (&g_define_type_id__volatile))
    {
        static const GEnumValue values[] = {
            { GRL_TEXTURE_FILTER_POINT, "GRL_TEXTURE_FILTER_POINT", "point" },
            { GRL_TEXTURE_FILTER_BILINEAR, "GRL_TEXTURE_FILTER_BILINEAR", "bilinear" },
            { GRL_TEXTURE_FILTER_TRILINEAR, "GRL_TEXTURE_FILTER_TRILINEAR", "trilinear" },
            { GRL_TEXTURE_FILTER_ANISOTROPIC_4X, "GRL_TEXTURE_FILTER_ANISOTROPIC_4X", "anisotropic-4x" },
            { GRL_TEXTURE_FILTER_ANISOTROPIC_8X, "GRL_TEXTURE_FILTER_ANISOTROPIC_8X", "anisotropic-8x" },
            { GRL_TEXTURE_FILTER_ANISOTROPIC_16X, "GRL_TEXTURE_FILTER_ANISOTROPIC_16X", "anisotropic-16x" },
            { 0, NULL, NULL }
        };
        GType g_define_type_id = g_enum_register_static ("GrlTextureFilter", values);
        g_once_init_leave (&g_define_type_id__volatile, g_define_type_id);
    }

    return g_define_type_id__volatile;
}

/*
 * GrlTextureWrap
 */
GType
grl_texture_wrap_get_type (void)
{
    static volatile gsize g_define_type_id__volatile = 0;

    if (g_once_init_enter (&g_define_type_id__volatile))
    {
        static const GEnumValue values[] = {
            { GRL_TEXTURE_WRAP_REPEAT, "GRL_TEXTURE_WRAP_REPEAT", "repeat" },
            { GRL_TEXTURE_WRAP_CLAMP, "GRL_TEXTURE_WRAP_CLAMP", "clamp" },
            { GRL_TEXTURE_WRAP_MIRROR_REPEAT, "GRL_TEXTURE_WRAP_MIRROR_REPEAT", "mirror-repeat" },
            { GRL_TEXTURE_WRAP_MIRROR_CLAMP, "GRL_TEXTURE_WRAP_MIRROR_CLAMP", "mirror-clamp" },
            { 0, NULL, NULL }
        };
        GType g_define_type_id = g_enum_register_static ("GrlTextureWrap", values);
        g_once_init_leave (&g_define_type_id__volatile, g_define_type_id);
    }

    return g_define_type_id__volatile;
}

/*
 * GrlPixelFormat
 */
GType
grl_pixel_format_get_type (void)
{
    static volatile gsize g_define_type_id__volatile = 0;

    if (g_once_init_enter (&g_define_type_id__volatile))
    {
        static const GEnumValue values[] = {
            { GRL_PIXELFORMAT_UNCOMPRESSED_GRAYSCALE, "GRL_PIXELFORMAT_UNCOMPRESSED_GRAYSCALE", "grayscale" },
            { GRL_PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA, "GRL_PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA", "gray-alpha" },
            { GRL_PIXELFORMAT_UNCOMPRESSED_R5G6B5, "GRL_PIXELFORMAT_UNCOMPRESSED_R5G6B5", "r5g6b5" },
            { GRL_PIXELFORMAT_UNCOMPRESSED_R8G8B8, "GRL_PIXELFORMAT_UNCOMPRESSED_R8G8B8", "r8g8b8" },
            { GRL_PIXELFORMAT_UNCOMPRESSED_R5G5B5A1, "GRL_PIXELFORMAT_UNCOMPRESSED_R5G5B5A1", "r5g5b5a1" },
            { GRL_PIXELFORMAT_UNCOMPRESSED_R4G4B4A4, "GRL_PIXELFORMAT_UNCOMPRESSED_R4G4B4A4", "r4g4b4a4" },
            { GRL_PIXELFORMAT_UNCOMPRESSED_R8G8B8A8, "GRL_PIXELFORMAT_UNCOMPRESSED_R8G8B8A8", "r8g8b8a8" },
            { GRL_PIXELFORMAT_UNCOMPRESSED_R32, "GRL_PIXELFORMAT_UNCOMPRESSED_R32", "r32" },
            { GRL_PIXELFORMAT_UNCOMPRESSED_R32G32B32, "GRL_PIXELFORMAT_UNCOMPRESSED_R32G32B32", "r32g32b32" },
            { GRL_PIXELFORMAT_UNCOMPRESSED_R32G32B32A32, "GRL_PIXELFORMAT_UNCOMPRESSED_R32G32B32A32", "r32g32b32a32" },
            { GRL_PIXELFORMAT_UNCOMPRESSED_R16, "GRL_PIXELFORMAT_UNCOMPRESSED_R16", "r16" },
            { GRL_PIXELFORMAT_UNCOMPRESSED_R16G16B16, "GRL_PIXELFORMAT_UNCOMPRESSED_R16G16B16", "r16g16b16" },
            { GRL_PIXELFORMAT_UNCOMPRESSED_R16G16B16A16, "GRL_PIXELFORMAT_UNCOMPRESSED_R16G16B16A16", "r16g16b16a16" },
            { GRL_PIXELFORMAT_COMPRESSED_DXT1_RGB, "GRL_PIXELFORMAT_COMPRESSED_DXT1_RGB", "dxt1-rgb" },
            { GRL_PIXELFORMAT_COMPRESSED_DXT1_RGBA, "GRL_PIXELFORMAT_COMPRESSED_DXT1_RGBA", "dxt1-rgba" },
            { GRL_PIXELFORMAT_COMPRESSED_DXT3_RGBA, "GRL_PIXELFORMAT_COMPRESSED_DXT3_RGBA", "dxt3-rgba" },
            { GRL_PIXELFORMAT_COMPRESSED_DXT5_RGBA, "GRL_PIXELFORMAT_COMPRESSED_DXT5_RGBA", "dxt5-rgba" },
            { GRL_PIXELFORMAT_COMPRESSED_ETC1_RGB, "GRL_PIXELFORMAT_COMPRESSED_ETC1_RGB", "etc1-rgb" },
            { GRL_PIXELFORMAT_COMPRESSED_ETC2_RGB, "GRL_PIXELFORMAT_COMPRESSED_ETC2_RGB", "etc2-rgb" },
            { GRL_PIXELFORMAT_COMPRESSED_ETC2_EAC_RGBA, "GRL_PIXELFORMAT_COMPRESSED_ETC2_EAC_RGBA", "etc2-eac-rgba" },
            { GRL_PIXELFORMAT_COMPRESSED_PVRT_RGB, "GRL_PIXELFORMAT_COMPRESSED_PVRT_RGB", "pvrt-rgb" },
            { GRL_PIXELFORMAT_COMPRESSED_PVRT_RGBA, "GRL_PIXELFORMAT_COMPRESSED_PVRT_RGBA", "pvrt-rgba" },
            { GRL_PIXELFORMAT_COMPRESSED_ASTC_4x4_RGBA, "GRL_PIXELFORMAT_COMPRESSED_ASTC_4x4_RGBA", "astc-4x4-rgba" },
            { GRL_PIXELFORMAT_COMPRESSED_ASTC_8x8_RGBA, "GRL_PIXELFORMAT_COMPRESSED_ASTC_8x8_RGBA", "astc-8x8-rgba" },
            { 0, NULL, NULL }
        };
        GType g_define_type_id = g_enum_register_static ("GrlPixelFormat", values);
        g_once_init_leave (&g_define_type_id__volatile, g_define_type_id);
    }

    return g_define_type_id__volatile;
}

/*
 * GrlCameraMode
 */
GType
grl_camera_mode_get_type (void)
{
    static volatile gsize g_define_type_id__volatile = 0;

    if (g_once_init_enter (&g_define_type_id__volatile))
    {
        static const GEnumValue values[] = {
            { GRL_CAMERA_CUSTOM, "GRL_CAMERA_CUSTOM", "custom" },
            { GRL_CAMERA_FREE, "GRL_CAMERA_FREE", "free" },
            { GRL_CAMERA_ORBITAL, "GRL_CAMERA_ORBITAL", "orbital" },
            { GRL_CAMERA_FIRST_PERSON, "GRL_CAMERA_FIRST_PERSON", "first-person" },
            { GRL_CAMERA_THIRD_PERSON, "GRL_CAMERA_THIRD_PERSON", "third-person" },
            { 0, NULL, NULL }
        };
        GType g_define_type_id = g_enum_register_static ("GrlCameraMode", values);
        g_once_init_leave (&g_define_type_id__volatile, g_define_type_id);
    }

    return g_define_type_id__volatile;
}

/*
 * GrlCameraProjection
 */
GType
grl_camera_projection_get_type (void)
{
    static volatile gsize g_define_type_id__volatile = 0;

    if (g_once_init_enter (&g_define_type_id__volatile))
    {
        static const GEnumValue values[] = {
            { GRL_CAMERA_PERSPECTIVE, "GRL_CAMERA_PERSPECTIVE", "perspective" },
            { GRL_CAMERA_ORTHOGRAPHIC, "GRL_CAMERA_ORTHOGRAPHIC", "orthographic" },
            { 0, NULL, NULL }
        };
        GType g_define_type_id = g_enum_register_static ("GrlCameraProjection", values);
        g_once_init_leave (&g_define_type_id__volatile, g_define_type_id);
    }

    return g_define_type_id__volatile;
}

/*
 * GrlGesture
 */
GType
grl_gesture_get_type (void)
{
    static volatile gsize g_define_type_id__volatile = 0;

    if (g_once_init_enter (&g_define_type_id__volatile))
    {
        static const GFlagsValue values[] = {
            { GRL_GESTURE_NONE, "GRL_GESTURE_NONE", "none" },
            { GRL_GESTURE_TAP, "GRL_GESTURE_TAP", "tap" },
            { GRL_GESTURE_DOUBLETAP, "GRL_GESTURE_DOUBLETAP", "doubletap" },
            { GRL_GESTURE_HOLD, "GRL_GESTURE_HOLD", "hold" },
            { GRL_GESTURE_DRAG, "GRL_GESTURE_DRAG", "drag" },
            { GRL_GESTURE_SWIPE_RIGHT, "GRL_GESTURE_SWIPE_RIGHT", "swipe-right" },
            { GRL_GESTURE_SWIPE_LEFT, "GRL_GESTURE_SWIPE_LEFT", "swipe-left" },
            { GRL_GESTURE_SWIPE_UP, "GRL_GESTURE_SWIPE_UP", "swipe-up" },
            { GRL_GESTURE_SWIPE_DOWN, "GRL_GESTURE_SWIPE_DOWN", "swipe-down" },
            { GRL_GESTURE_PINCH_IN, "GRL_GESTURE_PINCH_IN", "pinch-in" },
            { GRL_GESTURE_PINCH_OUT, "GRL_GESTURE_PINCH_OUT", "pinch-out" },
            { 0, NULL, NULL }
        };
        GType g_define_type_id = g_flags_register_static ("GrlGesture", values);
        g_once_init_leave (&g_define_type_id__volatile, g_define_type_id);
    }

    return g_define_type_id__volatile;
}

/*
 * =============================================================================
 * RLGL Enums
 * =============================================================================
 */

/*
 * GrlRlglDrawMode
 */
GType
grl_rlgl_draw_mode_get_type (void)
{
    static volatile gsize g_define_type_id__volatile = 0;

    if (g_once_init_enter (&g_define_type_id__volatile))
    {
        static const GEnumValue values[] = {
            { GRL_RLGL_LINES, "GRL_RLGL_LINES", "lines" },
            { GRL_RLGL_TRIANGLES, "GRL_RLGL_TRIANGLES", "triangles" },
            { GRL_RLGL_QUADS, "GRL_RLGL_QUADS", "quads" },
            { 0, NULL, NULL }
        };
        GType g_define_type_id = g_enum_register_static ("GrlRlglDrawMode", values);
        g_once_init_leave (&g_define_type_id__volatile, g_define_type_id);
    }

    return g_define_type_id__volatile;
}

/*
 * GrlRlglMatrixMode
 */
GType
grl_rlgl_matrix_mode_get_type (void)
{
    static volatile gsize g_define_type_id__volatile = 0;

    if (g_once_init_enter (&g_define_type_id__volatile))
    {
        static const GEnumValue values[] = {
            { GRL_RLGL_MODELVIEW, "GRL_RLGL_MODELVIEW", "modelview" },
            { GRL_RLGL_PROJECTION, "GRL_RLGL_PROJECTION", "projection" },
            { GRL_RLGL_TEXTURE, "GRL_RLGL_TEXTURE", "texture" },
            { 0, NULL, NULL }
        };
        GType g_define_type_id = g_enum_register_static ("GrlRlglMatrixMode", values);
        g_once_init_leave (&g_define_type_id__volatile, g_define_type_id);
    }

    return g_define_type_id__volatile;
}

/*
 * GrlRlglCullMode
 */
GType
grl_rlgl_cull_mode_get_type (void)
{
    static volatile gsize g_define_type_id__volatile = 0;

    if (g_once_init_enter (&g_define_type_id__volatile))
    {
        static const GEnumValue values[] = {
            { GRL_RLGL_CULL_FRONT, "GRL_RLGL_CULL_FRONT", "front" },
            { GRL_RLGL_CULL_BACK, "GRL_RLGL_CULL_BACK", "back" },
            { 0, NULL, NULL }
        };
        GType g_define_type_id = g_enum_register_static ("GrlRlglCullMode", values);
        g_once_init_leave (&g_define_type_id__volatile, g_define_type_id);
    }

    return g_define_type_id__volatile;
}

/*
 * GrlRlglGlVersion
 */
GType
grl_rlgl_gl_version_get_type (void)
{
    static volatile gsize g_define_type_id__volatile = 0;

    if (g_once_init_enter (&g_define_type_id__volatile))
    {
        static const GEnumValue values[] = {
            { GRL_RLGL_OPENGL_11, "GRL_RLGL_OPENGL_11", "opengl-11" },
            { GRL_RLGL_OPENGL_21, "GRL_RLGL_OPENGL_21", "opengl-21" },
            { GRL_RLGL_OPENGL_33, "GRL_RLGL_OPENGL_33", "opengl-33" },
            { GRL_RLGL_OPENGL_43, "GRL_RLGL_OPENGL_43", "opengl-43" },
            { GRL_RLGL_OPENGL_ES_20, "GRL_RLGL_OPENGL_ES_20", "opengl-es-20" },
            { GRL_RLGL_OPENGL_ES_30, "GRL_RLGL_OPENGL_ES_30", "opengl-es-30" },
            { 0, NULL, NULL }
        };
        GType g_define_type_id = g_enum_register_static ("GrlRlglGlVersion", values);
        g_once_init_leave (&g_define_type_id__volatile, g_define_type_id);
    }

    return g_define_type_id__volatile;
}

/*
 * GrlRlglFramebufferAttachType
 */
GType
grl_rlgl_framebuffer_attach_type_get_type (void)
{
    static volatile gsize g_define_type_id__volatile = 0;

    if (g_once_init_enter (&g_define_type_id__volatile))
    {
        static const GEnumValue values[] = {
            { GRL_RLGL_ATTACHMENT_COLOR_CHANNEL0, "GRL_RLGL_ATTACHMENT_COLOR_CHANNEL0", "color-channel0" },
            { GRL_RLGL_ATTACHMENT_COLOR_CHANNEL1, "GRL_RLGL_ATTACHMENT_COLOR_CHANNEL1", "color-channel1" },
            { GRL_RLGL_ATTACHMENT_COLOR_CHANNEL2, "GRL_RLGL_ATTACHMENT_COLOR_CHANNEL2", "color-channel2" },
            { GRL_RLGL_ATTACHMENT_COLOR_CHANNEL3, "GRL_RLGL_ATTACHMENT_COLOR_CHANNEL3", "color-channel3" },
            { GRL_RLGL_ATTACHMENT_COLOR_CHANNEL4, "GRL_RLGL_ATTACHMENT_COLOR_CHANNEL4", "color-channel4" },
            { GRL_RLGL_ATTACHMENT_COLOR_CHANNEL5, "GRL_RLGL_ATTACHMENT_COLOR_CHANNEL5", "color-channel5" },
            { GRL_RLGL_ATTACHMENT_COLOR_CHANNEL6, "GRL_RLGL_ATTACHMENT_COLOR_CHANNEL6", "color-channel6" },
            { GRL_RLGL_ATTACHMENT_COLOR_CHANNEL7, "GRL_RLGL_ATTACHMENT_COLOR_CHANNEL7", "color-channel7" },
            { GRL_RLGL_ATTACHMENT_DEPTH, "GRL_RLGL_ATTACHMENT_DEPTH", "depth" },
            { GRL_RLGL_ATTACHMENT_STENCIL, "GRL_RLGL_ATTACHMENT_STENCIL", "stencil" },
            { 0, NULL, NULL }
        };
        GType g_define_type_id = g_enum_register_static ("GrlRlglFramebufferAttachType", values);
        g_once_init_leave (&g_define_type_id__volatile, g_define_type_id);
    }

    return g_define_type_id__volatile;
}

/*
 * GrlRlglFramebufferTexType
 */
GType
grl_rlgl_framebuffer_tex_type_get_type (void)
{
    static volatile gsize g_define_type_id__volatile = 0;

    if (g_once_init_enter (&g_define_type_id__volatile))
    {
        static const GEnumValue values[] = {
            { GRL_RLGL_ATTACHMENT_CUBEMAP_POSITIVE_X, "GRL_RLGL_ATTACHMENT_CUBEMAP_POSITIVE_X", "cubemap-positive-x" },
            { GRL_RLGL_ATTACHMENT_CUBEMAP_NEGATIVE_X, "GRL_RLGL_ATTACHMENT_CUBEMAP_NEGATIVE_X", "cubemap-negative-x" },
            { GRL_RLGL_ATTACHMENT_CUBEMAP_POSITIVE_Y, "GRL_RLGL_ATTACHMENT_CUBEMAP_POSITIVE_Y", "cubemap-positive-y" },
            { GRL_RLGL_ATTACHMENT_CUBEMAP_NEGATIVE_Y, "GRL_RLGL_ATTACHMENT_CUBEMAP_NEGATIVE_Y", "cubemap-negative-y" },
            { GRL_RLGL_ATTACHMENT_CUBEMAP_POSITIVE_Z, "GRL_RLGL_ATTACHMENT_CUBEMAP_POSITIVE_Z", "cubemap-positive-z" },
            { GRL_RLGL_ATTACHMENT_CUBEMAP_NEGATIVE_Z, "GRL_RLGL_ATTACHMENT_CUBEMAP_NEGATIVE_Z", "cubemap-negative-z" },
            { GRL_RLGL_ATTACHMENT_TEXTURE2D, "GRL_RLGL_ATTACHMENT_TEXTURE2D", "texture2d" },
            { GRL_RLGL_ATTACHMENT_RENDERBUFFER, "GRL_RLGL_ATTACHMENT_RENDERBUFFER", "renderbuffer" },
            { 0, NULL, NULL }
        };
        GType g_define_type_id = g_enum_register_static ("GrlRlglFramebufferTexType", values);
        g_once_init_leave (&g_define_type_id__volatile, g_define_type_id);
    }

    return g_define_type_id__volatile;
}

/*
 * GrlRlglShaderType
 */
GType
grl_rlgl_shader_type_get_type (void)
{
    static volatile gsize g_define_type_id__volatile = 0;

    if (g_once_init_enter (&g_define_type_id__volatile))
    {
        static const GEnumValue values[] = {
            { GRL_RLGL_SHADER_FRAGMENT, "GRL_RLGL_SHADER_FRAGMENT", "fragment" },
            { GRL_RLGL_SHADER_VERTEX, "GRL_RLGL_SHADER_VERTEX", "vertex" },
            { GRL_RLGL_SHADER_COMPUTE, "GRL_RLGL_SHADER_COMPUTE", "compute" },
            { 0, NULL, NULL }
        };
        GType g_define_type_id = g_enum_register_static ("GrlRlglShaderType", values);
        g_once_init_leave (&g_define_type_id__volatile, g_define_type_id);
    }

    return g_define_type_id__volatile;
}

/*
 * GrlPorterDuffOp
 */
GType
grl_porter_duff_op_get_type (void)
{
    static volatile gsize g_define_type_id__volatile = 0;

    if (g_once_init_enter (&g_define_type_id__volatile))
    {
        static const GEnumValue values[] = {
            { GRL_PORTER_DUFF_CLEAR,    "GRL_PORTER_DUFF_CLEAR",    "clear"    },
            { GRL_PORTER_DUFF_SRC,      "GRL_PORTER_DUFF_SRC",      "src"      },
            { GRL_PORTER_DUFF_DST,      "GRL_PORTER_DUFF_DST",      "dst"      },
            { GRL_PORTER_DUFF_SRC_OVER, "GRL_PORTER_DUFF_SRC_OVER", "src-over" },
            { GRL_PORTER_DUFF_DST_OVER, "GRL_PORTER_DUFF_DST_OVER", "dst-over" },
            { GRL_PORTER_DUFF_SRC_IN,   "GRL_PORTER_DUFF_SRC_IN",   "src-in"   },
            { GRL_PORTER_DUFF_DST_IN,   "GRL_PORTER_DUFF_DST_IN",   "dst-in"   },
            { GRL_PORTER_DUFF_SRC_OUT,  "GRL_PORTER_DUFF_SRC_OUT",  "src-out"  },
            { GRL_PORTER_DUFF_DST_OUT,  "GRL_PORTER_DUFF_DST_OUT",  "dst-out"  },
            { GRL_PORTER_DUFF_SRC_ATOP, "GRL_PORTER_DUFF_SRC_ATOP", "src-atop" },
            { GRL_PORTER_DUFF_DST_ATOP, "GRL_PORTER_DUFF_DST_ATOP", "dst-atop" },
            { GRL_PORTER_DUFF_XOR,      "GRL_PORTER_DUFF_XOR",      "xor"      },
            { 0, NULL, NULL }
        };
        GType g_define_type_id = g_enum_register_static ("GrlPorterDuffOp", values);
        g_once_init_leave (&g_define_type_id__volatile, g_define_type_id);
    }

    return g_define_type_id__volatile;
}

/*
 * =============================================================================
 * GIF Writer Enums
 * =============================================================================
 */

/*
 * GrlGifQuantizer
 */
GType
grl_gif_quantizer_get_type (void)
{
    static volatile gsize g_define_type_id__volatile = 0;

    if (g_once_init_enter (&g_define_type_id__volatile))
    {
        static const GEnumValue values[] = {
            { GRL_GIF_QUANTIZER_WEB_SAFE,   "GRL_GIF_QUANTIZER_WEB_SAFE",   "web-safe"   },
            { GRL_GIF_QUANTIZER_MEDIAN_CUT, "GRL_GIF_QUANTIZER_MEDIAN_CUT", "median-cut" },
            { 0, NULL, NULL }
        };
        GType g_define_type_id = g_enum_register_static ("GrlGifQuantizer", values);
        g_once_init_leave (&g_define_type_id__volatile, g_define_type_id);
    }

    return g_define_type_id__volatile;
}

/*
 * GrlGifDither
 */
GType
grl_gif_dither_get_type (void)
{
    static volatile gsize g_define_type_id__volatile = 0;

    if (g_once_init_enter (&g_define_type_id__volatile))
    {
        static const GEnumValue values[] = {
            { GRL_GIF_DITHER_NONE,            "GRL_GIF_DITHER_NONE",            "none"            },
            { GRL_GIF_DITHER_FLOYD_STEINBERG, "GRL_GIF_DITHER_FLOYD_STEINBERG", "floyd-steinberg" },
            { 0, NULL, NULL }
        };
        GType g_define_type_id = g_enum_register_static ("GrlGifDither", values);
        g_once_init_leave (&g_define_type_id__volatile, g_define_type_id);
    }

    return g_define_type_id__volatile;
}

/*
 * GrlGifPaletteScope
 */
GType
grl_gif_palette_scope_get_type (void)
{
    static volatile gsize g_define_type_id__volatile = 0;

    if (g_once_init_enter (&g_define_type_id__volatile))
    {
        static const GEnumValue values[] = {
            { GRL_GIF_PALETTE_SCOPE_GLOBAL,    "GRL_GIF_PALETTE_SCOPE_GLOBAL",    "global"    },
            { GRL_GIF_PALETTE_SCOPE_PER_FRAME, "GRL_GIF_PALETTE_SCOPE_PER_FRAME", "per-frame" },
            { 0, NULL, NULL }
        };
        GType g_define_type_id = g_enum_register_static ("GrlGifPaletteScope", values);
        g_once_init_leave (&g_define_type_id__volatile, g_define_type_id);
    }

    return g_define_type_id__volatile;
}
