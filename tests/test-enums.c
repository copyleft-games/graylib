/* test-enums.c
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Unit tests for graylib enum GTypes.
 * Tests that enum types are properly registered with GLib.
 */

#include <glib.h>
#include <glib-object.h>
#include "src/grl-enums.h"

/*
 * Test that enum types are properly registered
 */

static void
test_config_flags_type (void)
{
    GType type = grl_config_flags_get_type ();

    g_assert_true (G_TYPE_IS_FLAGS (type));
    g_assert_cmpstr (g_type_name (type), ==, "GrlConfigFlags");
}

static void
test_config_flags_values (void)
{
    GFlagsClass *klass = g_type_class_ref (GRL_TYPE_CONFIG_FLAGS);

    g_assert_nonnull (klass);

    /* Check that some flags exist */
    g_assert_nonnull (g_flags_get_value_by_nick (klass, "vsync-hint"));
    g_assert_nonnull (g_flags_get_value_by_nick (klass, "fullscreen-mode"));
    g_assert_nonnull (g_flags_get_value_by_nick (klass, "window-resizable"));

    g_type_class_unref (klass);
}

static void
test_log_level_type (void)
{
    GType type = grl_log_level_get_type ();

    g_assert_true (G_TYPE_IS_ENUM (type));
    g_assert_cmpstr (g_type_name (type), ==, "GrlLogLevel");
}

static void
test_log_level_values (void)
{
    GEnumClass *klass = g_type_class_ref (GRL_TYPE_LOG_LEVEL);

    g_assert_nonnull (klass);

    /* Check that values exist */
    g_assert_nonnull (g_enum_get_value (klass, GRL_LOG_ALL));
    g_assert_nonnull (g_enum_get_value (klass, GRL_LOG_DEBUG));
    g_assert_nonnull (g_enum_get_value (klass, GRL_LOG_INFO));
    g_assert_nonnull (g_enum_get_value (klass, GRL_LOG_WARNING));
    g_assert_nonnull (g_enum_get_value (klass, GRL_LOG_ERROR));
    g_assert_nonnull (g_enum_get_value (klass, GRL_LOG_NONE));

    g_type_class_unref (klass);
}

static void
test_key_type (void)
{
    GType type = grl_key_get_type ();

    g_assert_true (G_TYPE_IS_ENUM (type));
    g_assert_cmpstr (g_type_name (type), ==, "GrlKey");
}

static void
test_key_values (void)
{
    GEnumClass *klass = g_type_class_ref (GRL_TYPE_KEY);

    g_assert_nonnull (klass);

    /* Check some common keys */
    g_assert_nonnull (g_enum_get_value (klass, GRL_KEY_SPACE));
    g_assert_nonnull (g_enum_get_value (klass, GRL_KEY_ESCAPE));
    g_assert_nonnull (g_enum_get_value (klass, GRL_KEY_ENTER));
    g_assert_nonnull (g_enum_get_value (klass, GRL_KEY_A));
    g_assert_nonnull (g_enum_get_value (klass, GRL_KEY_Z));

    g_type_class_unref (klass);
}

static void
test_mouse_button_type (void)
{
    GType type = grl_mouse_button_get_type ();

    g_assert_true (G_TYPE_IS_ENUM (type));
    g_assert_cmpstr (g_type_name (type), ==, "GrlMouseButton");
}

static void
test_mouse_button_values (void)
{
    GEnumClass *klass = g_type_class_ref (GRL_TYPE_MOUSE_BUTTON);

    g_assert_nonnull (klass);

    g_assert_nonnull (g_enum_get_value (klass, GRL_MOUSE_BUTTON_LEFT));
    g_assert_nonnull (g_enum_get_value (klass, GRL_MOUSE_BUTTON_RIGHT));
    g_assert_nonnull (g_enum_get_value (klass, GRL_MOUSE_BUTTON_MIDDLE));

    g_type_class_unref (klass);
}

static void
test_mouse_cursor_type (void)
{
    GType type = grl_mouse_cursor_get_type ();

    g_assert_true (G_TYPE_IS_ENUM (type));
    g_assert_cmpstr (g_type_name (type), ==, "GrlMouseCursor");
}

static void
test_gamepad_button_type (void)
{
    GType type = grl_gamepad_button_get_type ();

    g_assert_true (G_TYPE_IS_ENUM (type));
    g_assert_cmpstr (g_type_name (type), ==, "GrlGamepadButton");
}

static void
test_gamepad_axis_type (void)
{
    GType type = grl_gamepad_axis_get_type ();

    g_assert_true (G_TYPE_IS_ENUM (type));
    g_assert_cmpstr (g_type_name (type), ==, "GrlGamepadAxis");
}

static void
test_blend_mode_type (void)
{
    GType type = grl_blend_mode_get_type ();

    g_assert_true (G_TYPE_IS_ENUM (type));
    g_assert_cmpstr (g_type_name (type), ==, "GrlBlendMode");
}

static void
test_blend_mode_values (void)
{
    GEnumClass *klass = g_type_class_ref (GRL_TYPE_BLEND_MODE);

    g_assert_nonnull (klass);

    g_assert_nonnull (g_enum_get_value (klass, GRL_BLEND_ALPHA));
    g_assert_nonnull (g_enum_get_value (klass, GRL_BLEND_ADDITIVE));
    g_assert_nonnull (g_enum_get_value (klass, GRL_BLEND_MULTIPLIED));

    g_type_class_unref (klass);
}

static void
test_texture_filter_type (void)
{
    GType type = grl_texture_filter_get_type ();

    g_assert_true (G_TYPE_IS_ENUM (type));
    g_assert_cmpstr (g_type_name (type), ==, "GrlTextureFilter");
}

static void
test_texture_wrap_type (void)
{
    GType type = grl_texture_wrap_get_type ();

    g_assert_true (G_TYPE_IS_ENUM (type));
    g_assert_cmpstr (g_type_name (type), ==, "GrlTextureWrap");
}

static void
test_pixel_format_type (void)
{
    GType type = grl_pixel_format_get_type ();

    g_assert_true (G_TYPE_IS_ENUM (type));
    g_assert_cmpstr (g_type_name (type), ==, "GrlPixelFormat");
}

static void
test_camera_mode_type (void)
{
    GType type = grl_camera_mode_get_type ();

    g_assert_true (G_TYPE_IS_ENUM (type));
    g_assert_cmpstr (g_type_name (type), ==, "GrlCameraMode");
}

static void
test_camera_projection_type (void)
{
    GType type = grl_camera_projection_get_type ();

    g_assert_true (G_TYPE_IS_ENUM (type));
    g_assert_cmpstr (g_type_name (type), ==, "GrlCameraProjection");
}

static void
test_camera_projection_values (void)
{
    GEnumClass *klass = g_type_class_ref (GRL_TYPE_CAMERA_PROJECTION);

    g_assert_nonnull (klass);

    g_assert_nonnull (g_enum_get_value (klass, GRL_CAMERA_PERSPECTIVE));
    g_assert_nonnull (g_enum_get_value (klass, GRL_CAMERA_ORTHOGRAPHIC));

    g_type_class_unref (klass);
}

static void
test_gesture_type (void)
{
    GType type = grl_gesture_get_type ();

    g_assert_true (G_TYPE_IS_FLAGS (type));
    g_assert_cmpstr (g_type_name (type), ==, "GrlGesture");
}

static void
test_gesture_values (void)
{
    GFlagsClass *klass = g_type_class_ref (GRL_TYPE_GESTURE);

    g_assert_nonnull (klass);

    g_assert_nonnull (g_flags_get_value_by_nick (klass, "tap"));
    g_assert_nonnull (g_flags_get_value_by_nick (klass, "doubletap"));
    g_assert_nonnull (g_flags_get_value_by_nick (klass, "hold"));
    g_assert_nonnull (g_flags_get_value_by_nick (klass, "drag"));

    g_type_class_unref (klass);
}

/*
 * Test registration
 */

int
main (int   argc,
      char *argv[])
{
    g_test_init (&argc, &argv, NULL);

    /* Config flags */
    g_test_add_func ("/enums/config-flags-type", test_config_flags_type);
    g_test_add_func ("/enums/config-flags-values", test_config_flags_values);

    /* Log level */
    g_test_add_func ("/enums/log-level-type", test_log_level_type);
    g_test_add_func ("/enums/log-level-values", test_log_level_values);

    /* Keyboard */
    g_test_add_func ("/enums/key-type", test_key_type);
    g_test_add_func ("/enums/key-values", test_key_values);

    /* Mouse */
    g_test_add_func ("/enums/mouse-button-type", test_mouse_button_type);
    g_test_add_func ("/enums/mouse-button-values", test_mouse_button_values);
    g_test_add_func ("/enums/mouse-cursor-type", test_mouse_cursor_type);

    /* Gamepad */
    g_test_add_func ("/enums/gamepad-button-type", test_gamepad_button_type);
    g_test_add_func ("/enums/gamepad-axis-type", test_gamepad_axis_type);

    /* Graphics */
    g_test_add_func ("/enums/blend-mode-type", test_blend_mode_type);
    g_test_add_func ("/enums/blend-mode-values", test_blend_mode_values);
    g_test_add_func ("/enums/texture-filter-type", test_texture_filter_type);
    g_test_add_func ("/enums/texture-wrap-type", test_texture_wrap_type);
    g_test_add_func ("/enums/pixel-format-type", test_pixel_format_type);

    /* Camera */
    g_test_add_func ("/enums/camera-mode-type", test_camera_mode_type);
    g_test_add_func ("/enums/camera-projection-type", test_camera_projection_type);
    g_test_add_func ("/enums/camera-projection-values", test_camera_projection_values);

    /* Gestures */
    g_test_add_func ("/enums/gesture-type", test_gesture_type);
    g_test_add_func ("/enums/gesture-values", test_gesture_values);

    return g_test_run ();
}
