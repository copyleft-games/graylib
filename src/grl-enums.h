/* grl-enums.h
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Enumeration types for Graylib.
 * These wrap raylib's enumerations with GLib type registration for
 * GObject Introspection support.
 */

#pragma once

#if !defined(GRAYLIB_INSIDE) && !defined(GRAYLIB_COMPILATION)
#error "Only <graylib.h> can be included directly."
#endif

#include <glib-object.h>
#include "grl-version.h"

G_BEGIN_DECLS

/*
 * =============================================================================
 * Window Configuration Flags
 * =============================================================================
 */

/**
 * GrlConfigFlags:
 * @GRL_FLAG_VSYNC_HINT: Set to try enabling V-Sync on GPU
 * @GRL_FLAG_FULLSCREEN_MODE: Set to run program in fullscreen
 * @GRL_FLAG_WINDOW_RESIZABLE: Set to allow resizable window
 * @GRL_FLAG_WINDOW_UNDECORATED: Set to disable window decoration
 * @GRL_FLAG_WINDOW_HIDDEN: Set to hide window
 * @GRL_FLAG_WINDOW_MINIMIZED: Set to minimize window
 * @GRL_FLAG_WINDOW_MAXIMIZED: Set to maximize window
 * @GRL_FLAG_WINDOW_UNFOCUSED: Set to window non focused
 * @GRL_FLAG_WINDOW_TOPMOST: Set to window always on top
 * @GRL_FLAG_WINDOW_ALWAYS_RUN: Set to allow windows running while minimized
 * @GRL_FLAG_WINDOW_TRANSPARENT: Set to allow transparent framebuffer
 * @GRL_FLAG_WINDOW_HIGHDPI: Set to support HighDPI
 * @GRL_FLAG_WINDOW_MOUSE_PASSTHROUGH: Set to support mouse passthrough
 * @GRL_FLAG_BORDERLESS_WINDOWED_MODE: Set to run in borderless windowed mode
 * @GRL_FLAG_MSAA_4X_HINT: Set to try enabling MSAA 4X
 * @GRL_FLAG_INTERLACED_HINT: Set to try enabling interlaced video format
 *
 * Window configuration flags. Can be combined with bitwise OR.
 */
typedef enum /*< flags >*/
{
    GRL_FLAG_VSYNC_HINT               = 0x00000040,
    GRL_FLAG_FULLSCREEN_MODE          = 0x00000002,
    GRL_FLAG_WINDOW_RESIZABLE         = 0x00000004,
    GRL_FLAG_WINDOW_UNDECORATED       = 0x00000008,
    GRL_FLAG_WINDOW_HIDDEN            = 0x00000080,
    GRL_FLAG_WINDOW_MINIMIZED         = 0x00000200,
    GRL_FLAG_WINDOW_MAXIMIZED         = 0x00000400,
    GRL_FLAG_WINDOW_UNFOCUSED         = 0x00000800,
    GRL_FLAG_WINDOW_TOPMOST           = 0x00001000,
    GRL_FLAG_WINDOW_ALWAYS_RUN        = 0x00000100,
    GRL_FLAG_WINDOW_TRANSPARENT       = 0x00000010,
    GRL_FLAG_WINDOW_HIGHDPI           = 0x00002000,
    GRL_FLAG_WINDOW_MOUSE_PASSTHROUGH = 0x00004000,
    GRL_FLAG_BORDERLESS_WINDOWED_MODE = 0x00008000,
    GRL_FLAG_MSAA_4X_HINT             = 0x00000020,
    GRL_FLAG_INTERLACED_HINT          = 0x00010000
} GrlConfigFlags;

GRL_AVAILABLE_IN_ALL
GType grl_config_flags_get_type (void) G_GNUC_CONST;
#define GRL_TYPE_CONFIG_FLAGS (grl_config_flags_get_type ())

/*
 * =============================================================================
 * Trace Log Level
 * =============================================================================
 */

/**
 * GrlLogLevel:
 * @GRL_LOG_ALL: Display all logs
 * @GRL_LOG_TRACE: Trace logging, intended for internal use only
 * @GRL_LOG_DEBUG: Debug logging, used for internal debugging
 * @GRL_LOG_INFO: Info logging, used for program execution info
 * @GRL_LOG_WARNING: Warning logging, used on recoverable failures
 * @GRL_LOG_ERROR: Error logging, used on unrecoverable failures
 * @GRL_LOG_FATAL: Fatal logging, used to abort program
 * @GRL_LOG_NONE: Disable logging
 *
 * Trace log level for controlling log output verbosity.
 */
typedef enum
{
    GRL_LOG_ALL = 0,
    GRL_LOG_TRACE,
    GRL_LOG_DEBUG,
    GRL_LOG_INFO,
    GRL_LOG_WARNING,
    GRL_LOG_ERROR,
    GRL_LOG_FATAL,
    GRL_LOG_NONE
} GrlLogLevel;

GRL_AVAILABLE_IN_ALL
GType grl_log_level_get_type (void) G_GNUC_CONST;
#define GRL_TYPE_LOG_LEVEL (grl_log_level_get_type ())

/*
 * =============================================================================
 * Keyboard Keys
 * =============================================================================
 */

/**
 * GrlKey:
 * @GRL_KEY_NULL: No key pressed
 * @GRL_KEY_APOSTROPHE: Key: '
 * @GRL_KEY_COMMA: Key: ,
 * @GRL_KEY_MINUS: Key: -
 * @GRL_KEY_PERIOD: Key: .
 * @GRL_KEY_SLASH: Key: /
 * @GRL_KEY_ZERO: Key: 0
 * @GRL_KEY_ONE: Key: 1
 * @GRL_KEY_TWO: Key: 2
 * @GRL_KEY_THREE: Key: 3
 * @GRL_KEY_FOUR: Key: 4
 * @GRL_KEY_FIVE: Key: 5
 * @GRL_KEY_SIX: Key: 6
 * @GRL_KEY_SEVEN: Key: 7
 * @GRL_KEY_EIGHT: Key: 8
 * @GRL_KEY_NINE: Key: 9
 * @GRL_KEY_SEMICOLON: Key: ;
 * @GRL_KEY_EQUAL: Key: =
 * @GRL_KEY_A: Key: A
 * @GRL_KEY_B: Key: B
 * @GRL_KEY_C: Key: C
 * @GRL_KEY_D: Key: D
 * @GRL_KEY_E: Key: E
 * @GRL_KEY_F: Key: F
 * @GRL_KEY_G: Key: G
 * @GRL_KEY_H: Key: H
 * @GRL_KEY_I: Key: I
 * @GRL_KEY_J: Key: J
 * @GRL_KEY_K: Key: K
 * @GRL_KEY_L: Key: L
 * @GRL_KEY_M: Key: M
 * @GRL_KEY_N: Key: N
 * @GRL_KEY_O: Key: O
 * @GRL_KEY_P: Key: P
 * @GRL_KEY_Q: Key: Q
 * @GRL_KEY_R: Key: R
 * @GRL_KEY_S: Key: S
 * @GRL_KEY_T: Key: T
 * @GRL_KEY_U: Key: U
 * @GRL_KEY_V: Key: V
 * @GRL_KEY_W: Key: W
 * @GRL_KEY_X: Key: X
 * @GRL_KEY_Y: Key: Y
 * @GRL_KEY_Z: Key: Z
 * @GRL_KEY_LEFT_BRACKET: Key: [
 * @GRL_KEY_BACKSLASH: Key: \
 * @GRL_KEY_RIGHT_BRACKET: Key: ]
 * @GRL_KEY_GRAVE: Key: `
 * @GRL_KEY_SPACE: Key: Space
 * @GRL_KEY_ESCAPE: Key: Escape
 * @GRL_KEY_ENTER: Key: Enter
 * @GRL_KEY_TAB: Key: Tab
 * @GRL_KEY_BACKSPACE: Key: Backspace
 * @GRL_KEY_INSERT: Key: Insert
 * @GRL_KEY_DELETE: Key: Delete
 * @GRL_KEY_RIGHT: Key: Right Arrow
 * @GRL_KEY_LEFT: Key: Left Arrow
 * @GRL_KEY_DOWN: Key: Down Arrow
 * @GRL_KEY_UP: Key: Up Arrow
 * @GRL_KEY_PAGE_UP: Key: Page Up
 * @GRL_KEY_PAGE_DOWN: Key: Page Down
 * @GRL_KEY_HOME: Key: Home
 * @GRL_KEY_END: Key: End
 * @GRL_KEY_CAPS_LOCK: Key: Caps Lock
 * @GRL_KEY_SCROLL_LOCK: Key: Scroll Lock
 * @GRL_KEY_NUM_LOCK: Key: Num Lock
 * @GRL_KEY_PRINT_SCREEN: Key: Print Screen
 * @GRL_KEY_PAUSE: Key: Pause
 * @GRL_KEY_F1: Key: F1
 * @GRL_KEY_F2: Key: F2
 * @GRL_KEY_F3: Key: F3
 * @GRL_KEY_F4: Key: F4
 * @GRL_KEY_F5: Key: F5
 * @GRL_KEY_F6: Key: F6
 * @GRL_KEY_F7: Key: F7
 * @GRL_KEY_F8: Key: F8
 * @GRL_KEY_F9: Key: F9
 * @GRL_KEY_F10: Key: F10
 * @GRL_KEY_F11: Key: F11
 * @GRL_KEY_F12: Key: F12
 * @GRL_KEY_LEFT_SHIFT: Key: Left Shift
 * @GRL_KEY_LEFT_CONTROL: Key: Left Control
 * @GRL_KEY_LEFT_ALT: Key: Left Alt
 * @GRL_KEY_LEFT_SUPER: Key: Left Super (Windows key)
 * @GRL_KEY_RIGHT_SHIFT: Key: Right Shift
 * @GRL_KEY_RIGHT_CONTROL: Key: Right Control
 * @GRL_KEY_RIGHT_ALT: Key: Right Alt
 * @GRL_KEY_RIGHT_SUPER: Key: Right Super (Windows key)
 * @GRL_KEY_KB_MENU: Key: Keyboard Menu
 * @GRL_KEY_KP_0: Key: Keypad 0
 * @GRL_KEY_KP_1: Key: Keypad 1
 * @GRL_KEY_KP_2: Key: Keypad 2
 * @GRL_KEY_KP_3: Key: Keypad 3
 * @GRL_KEY_KP_4: Key: Keypad 4
 * @GRL_KEY_KP_5: Key: Keypad 5
 * @GRL_KEY_KP_6: Key: Keypad 6
 * @GRL_KEY_KP_7: Key: Keypad 7
 * @GRL_KEY_KP_8: Key: Keypad 8
 * @GRL_KEY_KP_9: Key: Keypad 9
 * @GRL_KEY_KP_DECIMAL: Key: Keypad Decimal
 * @GRL_KEY_KP_DIVIDE: Key: Keypad Divide
 * @GRL_KEY_KP_MULTIPLY: Key: Keypad Multiply
 * @GRL_KEY_KP_SUBTRACT: Key: Keypad Subtract
 * @GRL_KEY_KP_ADD: Key: Keypad Add
 * @GRL_KEY_KP_ENTER: Key: Keypad Enter
 * @GRL_KEY_KP_EQUAL: Key: Keypad Equal
 * @GRL_KEY_BACK: Key: Android Back button
 * @GRL_KEY_MENU: Key: Android Menu button
 * @GRL_KEY_VOLUME_UP: Key: Android Volume Up
 * @GRL_KEY_VOLUME_DOWN: Key: Android Volume Down
 *
 * Keyboard keys (US keyboard layout).
 */
typedef enum
{
    GRL_KEY_NULL            = 0,
    /* Alphanumeric keys */
    GRL_KEY_APOSTROPHE      = 39,
    GRL_KEY_COMMA           = 44,
    GRL_KEY_MINUS           = 45,
    GRL_KEY_PERIOD          = 46,
    GRL_KEY_SLASH           = 47,
    GRL_KEY_ZERO            = 48,
    GRL_KEY_ONE             = 49,
    GRL_KEY_TWO             = 50,
    GRL_KEY_THREE           = 51,
    GRL_KEY_FOUR            = 52,
    GRL_KEY_FIVE            = 53,
    GRL_KEY_SIX             = 54,
    GRL_KEY_SEVEN           = 55,
    GRL_KEY_EIGHT           = 56,
    GRL_KEY_NINE            = 57,
    GRL_KEY_SEMICOLON       = 59,
    GRL_KEY_EQUAL           = 61,
    GRL_KEY_A               = 65,
    GRL_KEY_B               = 66,
    GRL_KEY_C               = 67,
    GRL_KEY_D               = 68,
    GRL_KEY_E               = 69,
    GRL_KEY_F               = 70,
    GRL_KEY_G               = 71,
    GRL_KEY_H               = 72,
    GRL_KEY_I               = 73,
    GRL_KEY_J               = 74,
    GRL_KEY_K               = 75,
    GRL_KEY_L               = 76,
    GRL_KEY_M               = 77,
    GRL_KEY_N               = 78,
    GRL_KEY_O               = 79,
    GRL_KEY_P               = 80,
    GRL_KEY_Q               = 81,
    GRL_KEY_R               = 82,
    GRL_KEY_S               = 83,
    GRL_KEY_T               = 84,
    GRL_KEY_U               = 85,
    GRL_KEY_V               = 86,
    GRL_KEY_W               = 87,
    GRL_KEY_X               = 88,
    GRL_KEY_Y               = 89,
    GRL_KEY_Z               = 90,
    GRL_KEY_LEFT_BRACKET    = 91,
    GRL_KEY_BACKSLASH       = 92,
    GRL_KEY_RIGHT_BRACKET   = 93,
    GRL_KEY_GRAVE           = 96,
    /* Function keys */
    GRL_KEY_SPACE           = 32,
    GRL_KEY_ESCAPE          = 256,
    GRL_KEY_ENTER           = 257,
    GRL_KEY_TAB             = 258,
    GRL_KEY_BACKSPACE       = 259,
    GRL_KEY_INSERT          = 260,
    GRL_KEY_DELETE          = 261,
    GRL_KEY_RIGHT           = 262,
    GRL_KEY_LEFT            = 263,
    GRL_KEY_DOWN            = 264,
    GRL_KEY_UP              = 265,
    GRL_KEY_PAGE_UP         = 266,
    GRL_KEY_PAGE_DOWN       = 267,
    GRL_KEY_HOME            = 268,
    GRL_KEY_END             = 269,
    GRL_KEY_CAPS_LOCK       = 280,
    GRL_KEY_SCROLL_LOCK     = 281,
    GRL_KEY_NUM_LOCK        = 282,
    GRL_KEY_PRINT_SCREEN    = 283,
    GRL_KEY_PAUSE           = 284,
    GRL_KEY_F1              = 290,
    GRL_KEY_F2              = 291,
    GRL_KEY_F3              = 292,
    GRL_KEY_F4              = 293,
    GRL_KEY_F5              = 294,
    GRL_KEY_F6              = 295,
    GRL_KEY_F7              = 296,
    GRL_KEY_F8              = 297,
    GRL_KEY_F9              = 298,
    GRL_KEY_F10             = 299,
    GRL_KEY_F11             = 300,
    GRL_KEY_F12             = 301,
    GRL_KEY_LEFT_SHIFT      = 340,
    GRL_KEY_LEFT_CONTROL    = 341,
    GRL_KEY_LEFT_ALT        = 342,
    GRL_KEY_LEFT_SUPER      = 343,
    GRL_KEY_RIGHT_SHIFT     = 344,
    GRL_KEY_RIGHT_CONTROL   = 345,
    GRL_KEY_RIGHT_ALT       = 346,
    GRL_KEY_RIGHT_SUPER     = 347,
    GRL_KEY_KB_MENU         = 348,
    /* Keypad keys */
    GRL_KEY_KP_0            = 320,
    GRL_KEY_KP_1            = 321,
    GRL_KEY_KP_2            = 322,
    GRL_KEY_KP_3            = 323,
    GRL_KEY_KP_4            = 324,
    GRL_KEY_KP_5            = 325,
    GRL_KEY_KP_6            = 326,
    GRL_KEY_KP_7            = 327,
    GRL_KEY_KP_8            = 328,
    GRL_KEY_KP_9            = 329,
    GRL_KEY_KP_DECIMAL      = 330,
    GRL_KEY_KP_DIVIDE       = 331,
    GRL_KEY_KP_MULTIPLY     = 332,
    GRL_KEY_KP_SUBTRACT     = 333,
    GRL_KEY_KP_ADD          = 334,
    GRL_KEY_KP_ENTER        = 335,
    GRL_KEY_KP_EQUAL        = 336,
    /* Android keys */
    GRL_KEY_BACK            = 4,
    GRL_KEY_MENU            = 5,
    GRL_KEY_VOLUME_UP       = 24,
    GRL_KEY_VOLUME_DOWN     = 25
} GrlKey;

GRL_AVAILABLE_IN_ALL
GType grl_key_get_type (void) G_GNUC_CONST;
#define GRL_TYPE_KEY (grl_key_get_type ())

/*
 * =============================================================================
 * Mouse Buttons
 * =============================================================================
 */

/**
 * GrlMouseButton:
 * @GRL_MOUSE_BUTTON_LEFT: Left mouse button
 * @GRL_MOUSE_BUTTON_RIGHT: Right mouse button
 * @GRL_MOUSE_BUTTON_MIDDLE: Middle mouse button (scroll wheel)
 * @GRL_MOUSE_BUTTON_SIDE: Side mouse button
 * @GRL_MOUSE_BUTTON_EXTRA: Extra mouse button
 * @GRL_MOUSE_BUTTON_FORWARD: Forward mouse button
 * @GRL_MOUSE_BUTTON_BACK: Back mouse button
 *
 * Mouse buttons.
 */
typedef enum
{
    GRL_MOUSE_BUTTON_LEFT    = 0,
    GRL_MOUSE_BUTTON_RIGHT   = 1,
    GRL_MOUSE_BUTTON_MIDDLE  = 2,
    GRL_MOUSE_BUTTON_SIDE    = 3,
    GRL_MOUSE_BUTTON_EXTRA   = 4,
    GRL_MOUSE_BUTTON_FORWARD = 5,
    GRL_MOUSE_BUTTON_BACK    = 6
} GrlMouseButton;

GRL_AVAILABLE_IN_ALL
GType grl_mouse_button_get_type (void) G_GNUC_CONST;
#define GRL_TYPE_MOUSE_BUTTON (grl_mouse_button_get_type ())

/**
 * GrlMouseCursor:
 * @GRL_MOUSE_CURSOR_DEFAULT: Default pointer shape
 * @GRL_MOUSE_CURSOR_ARROW: Arrow shape
 * @GRL_MOUSE_CURSOR_IBEAM: Text writing cursor shape
 * @GRL_MOUSE_CURSOR_CROSSHAIR: Cross shape
 * @GRL_MOUSE_CURSOR_POINTING_HAND: Pointing hand cursor
 * @GRL_MOUSE_CURSOR_RESIZE_EW: Horizontal resize arrow
 * @GRL_MOUSE_CURSOR_RESIZE_NS: Vertical resize arrow
 * @GRL_MOUSE_CURSOR_RESIZE_NWSE: Diagonal resize (NW-SE)
 * @GRL_MOUSE_CURSOR_RESIZE_NESW: Diagonal resize (NE-SW)
 * @GRL_MOUSE_CURSOR_RESIZE_ALL: Omnidirectional resize
 * @GRL_MOUSE_CURSOR_NOT_ALLOWED: Operation not allowed
 *
 * Mouse cursor shapes.
 */
typedef enum
{
    GRL_MOUSE_CURSOR_DEFAULT       = 0,
    GRL_MOUSE_CURSOR_ARROW         = 1,
    GRL_MOUSE_CURSOR_IBEAM         = 2,
    GRL_MOUSE_CURSOR_CROSSHAIR     = 3,
    GRL_MOUSE_CURSOR_POINTING_HAND = 4,
    GRL_MOUSE_CURSOR_RESIZE_EW     = 5,
    GRL_MOUSE_CURSOR_RESIZE_NS     = 6,
    GRL_MOUSE_CURSOR_RESIZE_NWSE   = 7,
    GRL_MOUSE_CURSOR_RESIZE_NESW   = 8,
    GRL_MOUSE_CURSOR_RESIZE_ALL    = 9,
    GRL_MOUSE_CURSOR_NOT_ALLOWED   = 10
} GrlMouseCursor;

GRL_AVAILABLE_IN_ALL
GType grl_mouse_cursor_get_type (void) G_GNUC_CONST;
#define GRL_TYPE_MOUSE_CURSOR (grl_mouse_cursor_get_type ())

/*
 * =============================================================================
 * Gamepad
 * =============================================================================
 */

/**
 * GrlGamepadButton:
 * @GRL_GAMEPAD_BUTTON_UNKNOWN: Unknown button
 * @GRL_GAMEPAD_BUTTON_LEFT_FACE_UP: D-pad up
 * @GRL_GAMEPAD_BUTTON_LEFT_FACE_RIGHT: D-pad right
 * @GRL_GAMEPAD_BUTTON_LEFT_FACE_DOWN: D-pad down
 * @GRL_GAMEPAD_BUTTON_LEFT_FACE_LEFT: D-pad left
 * @GRL_GAMEPAD_BUTTON_RIGHT_FACE_UP: Face button up (Y/Triangle)
 * @GRL_GAMEPAD_BUTTON_RIGHT_FACE_RIGHT: Face button right (B/Circle)
 * @GRL_GAMEPAD_BUTTON_RIGHT_FACE_DOWN: Face button down (A/Cross)
 * @GRL_GAMEPAD_BUTTON_RIGHT_FACE_LEFT: Face button left (X/Square)
 * @GRL_GAMEPAD_BUTTON_LEFT_TRIGGER_1: Left bumper (L1/LB)
 * @GRL_GAMEPAD_BUTTON_LEFT_TRIGGER_2: Left trigger (L2/LT)
 * @GRL_GAMEPAD_BUTTON_RIGHT_TRIGGER_1: Right bumper (R1/RB)
 * @GRL_GAMEPAD_BUTTON_RIGHT_TRIGGER_2: Right trigger (R2/RT)
 * @GRL_GAMEPAD_BUTTON_MIDDLE_LEFT: Select/Back
 * @GRL_GAMEPAD_BUTTON_MIDDLE: Guide/Home
 * @GRL_GAMEPAD_BUTTON_MIDDLE_RIGHT: Start
 * @GRL_GAMEPAD_BUTTON_LEFT_THUMB: Left stick press (L3)
 * @GRL_GAMEPAD_BUTTON_RIGHT_THUMB: Right stick press (R3)
 *
 * Gamepad buttons.
 */
typedef enum
{
    GRL_GAMEPAD_BUTTON_UNKNOWN = 0,
    GRL_GAMEPAD_BUTTON_LEFT_FACE_UP,
    GRL_GAMEPAD_BUTTON_LEFT_FACE_RIGHT,
    GRL_GAMEPAD_BUTTON_LEFT_FACE_DOWN,
    GRL_GAMEPAD_BUTTON_LEFT_FACE_LEFT,
    GRL_GAMEPAD_BUTTON_RIGHT_FACE_UP,
    GRL_GAMEPAD_BUTTON_RIGHT_FACE_RIGHT,
    GRL_GAMEPAD_BUTTON_RIGHT_FACE_DOWN,
    GRL_GAMEPAD_BUTTON_RIGHT_FACE_LEFT,
    GRL_GAMEPAD_BUTTON_LEFT_TRIGGER_1,
    GRL_GAMEPAD_BUTTON_LEFT_TRIGGER_2,
    GRL_GAMEPAD_BUTTON_RIGHT_TRIGGER_1,
    GRL_GAMEPAD_BUTTON_RIGHT_TRIGGER_2,
    GRL_GAMEPAD_BUTTON_MIDDLE_LEFT,
    GRL_GAMEPAD_BUTTON_MIDDLE,
    GRL_GAMEPAD_BUTTON_MIDDLE_RIGHT,
    GRL_GAMEPAD_BUTTON_LEFT_THUMB,
    GRL_GAMEPAD_BUTTON_RIGHT_THUMB
} GrlGamepadButton;

GRL_AVAILABLE_IN_ALL
GType grl_gamepad_button_get_type (void) G_GNUC_CONST;
#define GRL_TYPE_GAMEPAD_BUTTON (grl_gamepad_button_get_type ())

/**
 * GrlGamepadAxis:
 * @GRL_GAMEPAD_AXIS_LEFT_X: Left stick X axis
 * @GRL_GAMEPAD_AXIS_LEFT_Y: Left stick Y axis
 * @GRL_GAMEPAD_AXIS_RIGHT_X: Right stick X axis
 * @GRL_GAMEPAD_AXIS_RIGHT_Y: Right stick Y axis
 * @GRL_GAMEPAD_AXIS_LEFT_TRIGGER: Left trigger pressure
 * @GRL_GAMEPAD_AXIS_RIGHT_TRIGGER: Right trigger pressure
 *
 * Gamepad axes.
 */
typedef enum
{
    GRL_GAMEPAD_AXIS_LEFT_X        = 0,
    GRL_GAMEPAD_AXIS_LEFT_Y        = 1,
    GRL_GAMEPAD_AXIS_RIGHT_X       = 2,
    GRL_GAMEPAD_AXIS_RIGHT_Y       = 3,
    GRL_GAMEPAD_AXIS_LEFT_TRIGGER  = 4,
    GRL_GAMEPAD_AXIS_RIGHT_TRIGGER = 5
} GrlGamepadAxis;

GRL_AVAILABLE_IN_ALL
GType grl_gamepad_axis_get_type (void) G_GNUC_CONST;
#define GRL_TYPE_GAMEPAD_AXIS (grl_gamepad_axis_get_type ())

/*
 * =============================================================================
 * Graphics Enums
 * =============================================================================
 */

/**
 * GrlBlendMode:
 * @GRL_BLEND_ALPHA: Blend textures considering alpha (default)
 * @GRL_BLEND_ADDITIVE: Blend textures adding colors
 * @GRL_BLEND_MULTIPLIED: Blend textures multiplying colors
 * @GRL_BLEND_ADD_COLORS: Blend textures adding colors (alternative)
 * @GRL_BLEND_SUBTRACT_COLORS: Blend textures subtracting colors
 * @GRL_BLEND_ALPHA_PREMULTIPLY: Blend premultiplied textures
 * @GRL_BLEND_CUSTOM: Custom blend mode
 * @GRL_BLEND_CUSTOM_SEPARATE: Custom blend with separate RGB/alpha
 *
 * Color blending modes.
 */
typedef enum
{
    GRL_BLEND_ALPHA = 0,
    GRL_BLEND_ADDITIVE,
    GRL_BLEND_MULTIPLIED,
    GRL_BLEND_ADD_COLORS,
    GRL_BLEND_SUBTRACT_COLORS,
    GRL_BLEND_ALPHA_PREMULTIPLY,
    GRL_BLEND_CUSTOM,
    GRL_BLEND_CUSTOM_SEPARATE
} GrlBlendMode;

GRL_AVAILABLE_IN_ALL
GType grl_blend_mode_get_type (void) G_GNUC_CONST;
#define GRL_TYPE_BLEND_MODE (grl_blend_mode_get_type ())

/**
 * GrlTextureFilter:
 * @GRL_TEXTURE_FILTER_POINT: No filter, pixel approximation
 * @GRL_TEXTURE_FILTER_BILINEAR: Linear filtering
 * @GRL_TEXTURE_FILTER_TRILINEAR: Trilinear filtering (with mipmaps)
 * @GRL_TEXTURE_FILTER_ANISOTROPIC_4X: Anisotropic 4x
 * @GRL_TEXTURE_FILTER_ANISOTROPIC_8X: Anisotropic 8x
 * @GRL_TEXTURE_FILTER_ANISOTROPIC_16X: Anisotropic 16x
 *
 * Texture filtering modes.
 */
typedef enum
{
    GRL_TEXTURE_FILTER_POINT = 0,
    GRL_TEXTURE_FILTER_BILINEAR,
    GRL_TEXTURE_FILTER_TRILINEAR,
    GRL_TEXTURE_FILTER_ANISOTROPIC_4X,
    GRL_TEXTURE_FILTER_ANISOTROPIC_8X,
    GRL_TEXTURE_FILTER_ANISOTROPIC_16X
} GrlTextureFilter;

GRL_AVAILABLE_IN_ALL
GType grl_texture_filter_get_type (void) G_GNUC_CONST;
#define GRL_TYPE_TEXTURE_FILTER (grl_texture_filter_get_type ())

/**
 * GrlTextureWrap:
 * @GRL_TEXTURE_WRAP_REPEAT: Repeat texture in tiled mode
 * @GRL_TEXTURE_WRAP_CLAMP: Clamp texture to edge pixel
 * @GRL_TEXTURE_WRAP_MIRROR_REPEAT: Mirror and repeat texture
 * @GRL_TEXTURE_WRAP_MIRROR_CLAMP: Mirror and clamp to border
 *
 * Texture wrapping modes.
 */
typedef enum
{
    GRL_TEXTURE_WRAP_REPEAT = 0,
    GRL_TEXTURE_WRAP_CLAMP,
    GRL_TEXTURE_WRAP_MIRROR_REPEAT,
    GRL_TEXTURE_WRAP_MIRROR_CLAMP
} GrlTextureWrap;

GRL_AVAILABLE_IN_ALL
GType grl_texture_wrap_get_type (void) G_GNUC_CONST;
#define GRL_TYPE_TEXTURE_WRAP (grl_texture_wrap_get_type ())

/**
 * GrlPixelFormat:
 * @GRL_PIXELFORMAT_UNCOMPRESSED_GRAYSCALE: 8 bpp grayscale
 * @GRL_PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA: 16 bpp grayscale + alpha
 * @GRL_PIXELFORMAT_UNCOMPRESSED_R5G6B5: 16 bpp RGB
 * @GRL_PIXELFORMAT_UNCOMPRESSED_R8G8B8: 24 bpp RGB
 * @GRL_PIXELFORMAT_UNCOMPRESSED_R5G5B5A1: 16 bpp RGBA (1-bit alpha)
 * @GRL_PIXELFORMAT_UNCOMPRESSED_R4G4B4A4: 16 bpp RGBA (4-bit alpha)
 * @GRL_PIXELFORMAT_UNCOMPRESSED_R8G8B8A8: 32 bpp RGBA
 * @GRL_PIXELFORMAT_UNCOMPRESSED_R32: 32 bpp single float
 * @GRL_PIXELFORMAT_UNCOMPRESSED_R32G32B32: 96 bpp RGB float
 * @GRL_PIXELFORMAT_UNCOMPRESSED_R32G32B32A32: 128 bpp RGBA float
 * @GRL_PIXELFORMAT_UNCOMPRESSED_R16: 16 bpp half float
 * @GRL_PIXELFORMAT_UNCOMPRESSED_R16G16B16: 48 bpp RGB half float
 * @GRL_PIXELFORMAT_UNCOMPRESSED_R16G16B16A16: 64 bpp RGBA half float
 * @GRL_PIXELFORMAT_COMPRESSED_DXT1_RGB: DXT1 RGB (4 bpp)
 * @GRL_PIXELFORMAT_COMPRESSED_DXT1_RGBA: DXT1 RGBA (4 bpp)
 * @GRL_PIXELFORMAT_COMPRESSED_DXT3_RGBA: DXT3 RGBA (8 bpp)
 * @GRL_PIXELFORMAT_COMPRESSED_DXT5_RGBA: DXT5 RGBA (8 bpp)
 * @GRL_PIXELFORMAT_COMPRESSED_ETC1_RGB: ETC1 RGB (4 bpp)
 * @GRL_PIXELFORMAT_COMPRESSED_ETC2_RGB: ETC2 RGB (4 bpp)
 * @GRL_PIXELFORMAT_COMPRESSED_ETC2_EAC_RGBA: ETC2 RGBA (8 bpp)
 * @GRL_PIXELFORMAT_COMPRESSED_PVRT_RGB: PVRT RGB (4 bpp)
 * @GRL_PIXELFORMAT_COMPRESSED_PVRT_RGBA: PVRT RGBA (4 bpp)
 * @GRL_PIXELFORMAT_COMPRESSED_ASTC_4x4_RGBA: ASTC 4x4 RGBA (8 bpp)
 * @GRL_PIXELFORMAT_COMPRESSED_ASTC_8x8_RGBA: ASTC 8x8 RGBA (2 bpp)
 *
 * Pixel formats for images and textures.
 */
typedef enum
{
    GRL_PIXELFORMAT_UNCOMPRESSED_GRAYSCALE = 1,
    GRL_PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA,
    GRL_PIXELFORMAT_UNCOMPRESSED_R5G6B5,
    GRL_PIXELFORMAT_UNCOMPRESSED_R8G8B8,
    GRL_PIXELFORMAT_UNCOMPRESSED_R5G5B5A1,
    GRL_PIXELFORMAT_UNCOMPRESSED_R4G4B4A4,
    GRL_PIXELFORMAT_UNCOMPRESSED_R8G8B8A8,
    GRL_PIXELFORMAT_UNCOMPRESSED_R32,
    GRL_PIXELFORMAT_UNCOMPRESSED_R32G32B32,
    GRL_PIXELFORMAT_UNCOMPRESSED_R32G32B32A32,
    GRL_PIXELFORMAT_UNCOMPRESSED_R16,
    GRL_PIXELFORMAT_UNCOMPRESSED_R16G16B16,
    GRL_PIXELFORMAT_UNCOMPRESSED_R16G16B16A16,
    GRL_PIXELFORMAT_COMPRESSED_DXT1_RGB,
    GRL_PIXELFORMAT_COMPRESSED_DXT1_RGBA,
    GRL_PIXELFORMAT_COMPRESSED_DXT3_RGBA,
    GRL_PIXELFORMAT_COMPRESSED_DXT5_RGBA,
    GRL_PIXELFORMAT_COMPRESSED_ETC1_RGB,
    GRL_PIXELFORMAT_COMPRESSED_ETC2_RGB,
    GRL_PIXELFORMAT_COMPRESSED_ETC2_EAC_RGBA,
    GRL_PIXELFORMAT_COMPRESSED_PVRT_RGB,
    GRL_PIXELFORMAT_COMPRESSED_PVRT_RGBA,
    GRL_PIXELFORMAT_COMPRESSED_ASTC_4x4_RGBA,
    GRL_PIXELFORMAT_COMPRESSED_ASTC_8x8_RGBA
} GrlPixelFormat;

GRL_AVAILABLE_IN_ALL
GType grl_pixel_format_get_type (void) G_GNUC_CONST;
#define GRL_TYPE_PIXEL_FORMAT (grl_pixel_format_get_type ())

/*
 * =============================================================================
 * Camera Enums
 * =============================================================================
 */

/**
 * GrlCameraMode:
 * @GRL_CAMERA_CUSTOM: Custom camera, controlled by user
 * @GRL_CAMERA_FREE: Free camera mode
 * @GRL_CAMERA_ORBITAL: Orbital camera around target
 * @GRL_CAMERA_FIRST_PERSON: First person camera
 * @GRL_CAMERA_THIRD_PERSON: Third person camera
 *
 * Camera system modes for 3D cameras.
 */
typedef enum
{
    GRL_CAMERA_CUSTOM = 0,
    GRL_CAMERA_FREE,
    GRL_CAMERA_ORBITAL,
    GRL_CAMERA_FIRST_PERSON,
    GRL_CAMERA_THIRD_PERSON
} GrlCameraMode;

GRL_AVAILABLE_IN_ALL
GType grl_camera_mode_get_type (void) G_GNUC_CONST;
#define GRL_TYPE_CAMERA_MODE (grl_camera_mode_get_type ())

/**
 * GrlCameraProjection:
 * @GRL_CAMERA_PERSPECTIVE: Perspective projection
 * @GRL_CAMERA_ORTHOGRAPHIC: Orthographic projection
 *
 * Camera projection types.
 */
typedef enum
{
    GRL_CAMERA_PERSPECTIVE = 0,
    GRL_CAMERA_ORTHOGRAPHIC
} GrlCameraProjection;

GRL_AVAILABLE_IN_ALL
GType grl_camera_projection_get_type (void) G_GNUC_CONST;
#define GRL_TYPE_CAMERA_PROJECTION (grl_camera_projection_get_type ())

/*
 * =============================================================================
 * Gesture Flags
 * =============================================================================
 */

/**
 * GrlGesture:
 * @GRL_GESTURE_NONE: No gesture
 * @GRL_GESTURE_TAP: Tap gesture
 * @GRL_GESTURE_DOUBLETAP: Double tap gesture
 * @GRL_GESTURE_HOLD: Hold gesture
 * @GRL_GESTURE_DRAG: Drag gesture
 * @GRL_GESTURE_SWIPE_RIGHT: Swipe right gesture
 * @GRL_GESTURE_SWIPE_LEFT: Swipe left gesture
 * @GRL_GESTURE_SWIPE_UP: Swipe up gesture
 * @GRL_GESTURE_SWIPE_DOWN: Swipe down gesture
 * @GRL_GESTURE_PINCH_IN: Pinch in gesture
 * @GRL_GESTURE_PINCH_OUT: Pinch out gesture
 *
 * Touch gestures. Can be combined with bitwise OR for multi-gesture detection.
 */
typedef enum /*< flags >*/
{
    GRL_GESTURE_NONE        = 0,
    GRL_GESTURE_TAP         = 1,
    GRL_GESTURE_DOUBLETAP   = 2,
    GRL_GESTURE_HOLD        = 4,
    GRL_GESTURE_DRAG        = 8,
    GRL_GESTURE_SWIPE_RIGHT = 16,
    GRL_GESTURE_SWIPE_LEFT  = 32,
    GRL_GESTURE_SWIPE_UP    = 64,
    GRL_GESTURE_SWIPE_DOWN  = 128,
    GRL_GESTURE_PINCH_IN    = 256,
    GRL_GESTURE_PINCH_OUT   = 512
} GrlGesture;

GRL_AVAILABLE_IN_ALL
GType grl_gesture_get_type (void) G_GNUC_CONST;
#define GRL_TYPE_GESTURE (grl_gesture_get_type ())

/*
 * =============================================================================
 * RLGL Enums (Low-level OpenGL Abstraction)
 * =============================================================================
 */

/**
 * GrlRlglDrawMode:
 * @GRL_RLGL_LINES: Draw lines
 * @GRL_RLGL_TRIANGLES: Draw triangles
 * @GRL_RLGL_QUADS: Draw quads
 *
 * Primitive drawing modes for immediate mode rendering.
 */
typedef enum
{
    GRL_RLGL_LINES     = 0x0001,
    GRL_RLGL_TRIANGLES = 0x0004,
    GRL_RLGL_QUADS     = 0x0007
} GrlRlglDrawMode;

GRL_AVAILABLE_IN_ALL
GType grl_rlgl_draw_mode_get_type (void) G_GNUC_CONST;
#define GRL_TYPE_RLGL_DRAW_MODE (grl_rlgl_draw_mode_get_type ())

/**
 * GrlRlglMatrixMode:
 * @GRL_RLGL_MODELVIEW: Modelview matrix
 * @GRL_RLGL_PROJECTION: Projection matrix
 * @GRL_RLGL_TEXTURE: Texture matrix
 *
 * Matrix modes for rlgl matrix operations.
 */
typedef enum
{
    GRL_RLGL_MODELVIEW  = 0x1700,
    GRL_RLGL_PROJECTION = 0x1701,
    GRL_RLGL_TEXTURE    = 0x1702
} GrlRlglMatrixMode;

GRL_AVAILABLE_IN_ALL
GType grl_rlgl_matrix_mode_get_type (void) G_GNUC_CONST;
#define GRL_TYPE_RLGL_MATRIX_MODE (grl_rlgl_matrix_mode_get_type ())

/**
 * GrlRlglCullMode:
 * @GRL_RLGL_CULL_FRONT: Cull front faces
 * @GRL_RLGL_CULL_BACK: Cull back faces
 *
 * Face culling modes.
 */
typedef enum
{
    GRL_RLGL_CULL_FRONT = 0,
    GRL_RLGL_CULL_BACK  = 1
} GrlRlglCullMode;

GRL_AVAILABLE_IN_ALL
GType grl_rlgl_cull_mode_get_type (void) G_GNUC_CONST;
#define GRL_TYPE_RLGL_CULL_MODE (grl_rlgl_cull_mode_get_type ())

/**
 * GrlRlglGlVersion:
 * @GRL_RLGL_OPENGL_11: OpenGL 1.1
 * @GRL_RLGL_OPENGL_21: OpenGL 2.1
 * @GRL_RLGL_OPENGL_33: OpenGL 3.3
 * @GRL_RLGL_OPENGL_43: OpenGL 4.3
 * @GRL_RLGL_OPENGL_ES_20: OpenGL ES 2.0
 * @GRL_RLGL_OPENGL_ES_30: OpenGL ES 3.0
 *
 * OpenGL version identifiers.
 */
typedef enum
{
    GRL_RLGL_OPENGL_11    = 1,
    GRL_RLGL_OPENGL_21    = 2,
    GRL_RLGL_OPENGL_33    = 3,
    GRL_RLGL_OPENGL_43    = 4,
    GRL_RLGL_OPENGL_ES_20 = 5,
    GRL_RLGL_OPENGL_ES_30 = 6
} GrlRlglGlVersion;

GRL_AVAILABLE_IN_ALL
GType grl_rlgl_gl_version_get_type (void) G_GNUC_CONST;
#define GRL_TYPE_RLGL_GL_VERSION (grl_rlgl_gl_version_get_type ())

/**
 * GrlRlglFramebufferAttachType:
 * @GRL_RLGL_ATTACHMENT_COLOR_CHANNEL0: Color attachment 0
 * @GRL_RLGL_ATTACHMENT_COLOR_CHANNEL1: Color attachment 1
 * @GRL_RLGL_ATTACHMENT_COLOR_CHANNEL2: Color attachment 2
 * @GRL_RLGL_ATTACHMENT_COLOR_CHANNEL3: Color attachment 3
 * @GRL_RLGL_ATTACHMENT_COLOR_CHANNEL4: Color attachment 4
 * @GRL_RLGL_ATTACHMENT_COLOR_CHANNEL5: Color attachment 5
 * @GRL_RLGL_ATTACHMENT_COLOR_CHANNEL6: Color attachment 6
 * @GRL_RLGL_ATTACHMENT_COLOR_CHANNEL7: Color attachment 7
 * @GRL_RLGL_ATTACHMENT_DEPTH: Depth attachment
 * @GRL_RLGL_ATTACHMENT_STENCIL: Stencil attachment
 *
 * Framebuffer attachment types.
 */
typedef enum
{
    GRL_RLGL_ATTACHMENT_COLOR_CHANNEL0 = 0,
    GRL_RLGL_ATTACHMENT_COLOR_CHANNEL1 = 1,
    GRL_RLGL_ATTACHMENT_COLOR_CHANNEL2 = 2,
    GRL_RLGL_ATTACHMENT_COLOR_CHANNEL3 = 3,
    GRL_RLGL_ATTACHMENT_COLOR_CHANNEL4 = 4,
    GRL_RLGL_ATTACHMENT_COLOR_CHANNEL5 = 5,
    GRL_RLGL_ATTACHMENT_COLOR_CHANNEL6 = 6,
    GRL_RLGL_ATTACHMENT_COLOR_CHANNEL7 = 7,
    GRL_RLGL_ATTACHMENT_DEPTH          = 100,
    GRL_RLGL_ATTACHMENT_STENCIL        = 200
} GrlRlglFramebufferAttachType;

GRL_AVAILABLE_IN_ALL
GType grl_rlgl_framebuffer_attach_type_get_type (void) G_GNUC_CONST;
#define GRL_TYPE_RLGL_FRAMEBUFFER_ATTACH_TYPE (grl_rlgl_framebuffer_attach_type_get_type ())

/**
 * GrlRlglFramebufferTexType:
 * @GRL_RLGL_ATTACHMENT_CUBEMAP_POSITIVE_X: Cubemap +X face
 * @GRL_RLGL_ATTACHMENT_CUBEMAP_NEGATIVE_X: Cubemap -X face
 * @GRL_RLGL_ATTACHMENT_CUBEMAP_POSITIVE_Y: Cubemap +Y face
 * @GRL_RLGL_ATTACHMENT_CUBEMAP_NEGATIVE_Y: Cubemap -Y face
 * @GRL_RLGL_ATTACHMENT_CUBEMAP_POSITIVE_Z: Cubemap +Z face
 * @GRL_RLGL_ATTACHMENT_CUBEMAP_NEGATIVE_Z: Cubemap -Z face
 * @GRL_RLGL_ATTACHMENT_TEXTURE2D: 2D texture
 * @GRL_RLGL_ATTACHMENT_RENDERBUFFER: Renderbuffer
 *
 * Framebuffer texture types for attachment.
 */
typedef enum
{
    GRL_RLGL_ATTACHMENT_CUBEMAP_POSITIVE_X = 0,
    GRL_RLGL_ATTACHMENT_CUBEMAP_NEGATIVE_X = 1,
    GRL_RLGL_ATTACHMENT_CUBEMAP_POSITIVE_Y = 2,
    GRL_RLGL_ATTACHMENT_CUBEMAP_NEGATIVE_Y = 3,
    GRL_RLGL_ATTACHMENT_CUBEMAP_POSITIVE_Z = 4,
    GRL_RLGL_ATTACHMENT_CUBEMAP_NEGATIVE_Z = 5,
    GRL_RLGL_ATTACHMENT_TEXTURE2D          = 100,
    GRL_RLGL_ATTACHMENT_RENDERBUFFER       = 200
} GrlRlglFramebufferTexType;

GRL_AVAILABLE_IN_ALL
GType grl_rlgl_framebuffer_tex_type_get_type (void) G_GNUC_CONST;
#define GRL_TYPE_RLGL_FRAMEBUFFER_TEX_TYPE (grl_rlgl_framebuffer_tex_type_get_type ())

/**
 * GrlRlglShaderType:
 * @GRL_RLGL_SHADER_FRAGMENT: Fragment shader
 * @GRL_RLGL_SHADER_VERTEX: Vertex shader
 * @GRL_RLGL_SHADER_COMPUTE: Compute shader
 *
 * Shader types for compilation.
 */
typedef enum
{
    GRL_RLGL_SHADER_FRAGMENT = 0x8B30,
    GRL_RLGL_SHADER_VERTEX   = 0x8B31,
    GRL_RLGL_SHADER_COMPUTE  = 0x91B9
} GrlRlglShaderType;

GRL_AVAILABLE_IN_ALL
GType grl_rlgl_shader_type_get_type (void) G_GNUC_CONST;
#define GRL_TYPE_RLGL_SHADER_TYPE (grl_rlgl_shader_type_get_type ())

G_END_DECLS
