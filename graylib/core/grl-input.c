/* grl-input.c
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Input manager implementation.
 */

#include "config.h"
#include "grl-input.h"
#include <raylib.h>

/**
 * SECTION:grl-input
 * @title: Input Functions
 * @short_description: Keyboard, mouse, gamepad, and touch input
 *
 * The input module provides functions to query the state of input devices.
 * These are stateless functions that query raylib's internal input state.
 *
 * For event-driven input handling, use the signals on #GrlWindow:
 * - #GrlWindow::key-pressed
 * - #GrlWindow::key-released
 * - #GrlWindow::mouse-button-pressed
 * - #GrlWindow::mouse-button-released
 *
 * Example - Polling keyboard input:
 * |[<!-- language="C" -->
 * if (grl_input_is_key_pressed (GRL_KEY_SPACE))
 * {
 *     // Space was just pressed this frame
 *     player_jump ();
 * }
 *
 * if (grl_input_is_key_down (GRL_KEY_RIGHT))
 * {
 *     // Right arrow is being held
 *     player_move_right ();
 * }
 * ]|
 *
 * Example - Mouse input:
 * |[<!-- language="C" -->
 * if (grl_input_is_mouse_button_pressed (GRL_MOUSE_BUTTON_LEFT))
 * {
 *     g_autoptr(GrlVector2) pos = grl_input_get_mouse_position ();
 *     handle_click (pos->x, pos->y);
 * }
 * ]|
 *
 * Example - Gamepad input:
 * |[<!-- language="C" -->
 * if (grl_input_is_gamepad_available (0))
 * {
 *     gfloat x = grl_input_get_gamepad_axis_movement (0, GRL_GAMEPAD_AXIS_LEFT_X);
 *     gfloat y = grl_input_get_gamepad_axis_movement (0, GRL_GAMEPAD_AXIS_LEFT_Y);
 *
 *     if (grl_input_is_gamepad_button_pressed (0, GRL_GAMEPAD_BUTTON_RIGHT_FACE_DOWN))
 *     {
 *         player_jump ();
 *     }
 * }
 * ]|
 */

/*
 * =============================================================================
 * Keyboard Input
 * =============================================================================
 */

/**
 * grl_input_is_key_pressed:
 * @key: The #GrlKey to check
 *
 * Checks if a key has been pressed once.
 *
 * Returns: %TRUE if the key was just pressed
 */
gboolean
grl_input_is_key_pressed (GrlKey key)
{
    /* Explicit conversion from bool (1 byte) to gboolean (4 bytes) to prevent
     * tail-call optimization from passing through garbage in upper bytes */
    return IsKeyPressed ((int)key) ? TRUE : FALSE;
}

/**
 * grl_input_is_key_pressed_repeat:
 * @key: The #GrlKey to check
 *
 * Checks if a key has been pressed again (key repeat).
 *
 * Returns: %TRUE if key repeat triggered
 */
gboolean
grl_input_is_key_pressed_repeat (GrlKey key)
{
    return IsKeyPressedRepeat ((int)key) ? TRUE : FALSE;
}

/**
 * grl_input_is_key_down:
 * @key: The #GrlKey to check
 *
 * Checks if a key is currently being held down.
 *
 * Returns: %TRUE if the key is down
 */
gboolean
grl_input_is_key_down (GrlKey key)
{
    return IsKeyDown ((int)key) ? TRUE : FALSE;
}

/**
 * grl_input_is_key_released:
 * @key: The #GrlKey to check
 *
 * Checks if a key has been released once.
 *
 * Returns: %TRUE if the key was just released
 */
gboolean
grl_input_is_key_released (GrlKey key)
{
    return IsKeyReleased ((int)key) ? TRUE : FALSE;
}

/**
 * grl_input_is_key_up:
 * @key: The #GrlKey to check
 *
 * Checks if a key is NOT being pressed.
 *
 * Returns: %TRUE if the key is up
 */
gboolean
grl_input_is_key_up (GrlKey key)
{
    return IsKeyUp ((int)key) ? TRUE : FALSE;
}

/**
 * grl_input_get_key_pressed:
 *
 * Gets the last key pressed from the input queue.
 *
 * Returns: The last #GrlKey pressed, or %GRL_KEY_NULL
 */
GrlKey
grl_input_get_key_pressed (void)
{
    return (GrlKey)GetKeyPressed ();
}

/**
 * grl_input_get_char_pressed:
 *
 * Gets the last Unicode character pressed.
 *
 * Returns: Unicode codepoint, or 0 when queue is empty
 */
gint
grl_input_get_char_pressed (void)
{
    return GetCharPressed ();
}

/*
 * =============================================================================
 * Mouse Input
 * =============================================================================
 */

/**
 * grl_input_is_mouse_button_pressed:
 * @button: The #GrlMouseButton to check
 *
 * Checks if a mouse button has been pressed once.
 *
 * Returns: %TRUE if the button was just pressed
 */
gboolean
grl_input_is_mouse_button_pressed (GrlMouseButton button)
{
    return IsMouseButtonPressed ((int)button) ? TRUE : FALSE;
}

/**
 * grl_input_is_mouse_button_down:
 * @button: The #GrlMouseButton to check
 *
 * Checks if a mouse button is currently held down.
 *
 * Returns: %TRUE if the button is down
 */
gboolean
grl_input_is_mouse_button_down (GrlMouseButton button)
{
    return IsMouseButtonDown ((int)button) ? TRUE : FALSE;
}

/**
 * grl_input_is_mouse_button_released:
 * @button: The #GrlMouseButton to check
 *
 * Checks if a mouse button has been released once.
 *
 * Returns: %TRUE if the button was just released
 */
gboolean
grl_input_is_mouse_button_released (GrlMouseButton button)
{
    return IsMouseButtonReleased ((int)button) ? TRUE : FALSE;
}

/**
 * grl_input_is_mouse_button_up:
 * @button: The #GrlMouseButton to check
 *
 * Checks if a mouse button is NOT being pressed.
 *
 * Returns: %TRUE if the button is up
 */
gboolean
grl_input_is_mouse_button_up (GrlMouseButton button)
{
    return IsMouseButtonUp ((int)button) ? TRUE : FALSE;
}

/**
 * grl_input_get_mouse_x:
 *
 * Gets the current mouse X position.
 *
 * Returns: Mouse X coordinate
 */
gint
grl_input_get_mouse_x (void)
{
    return GetMouseX ();
}

/**
 * grl_input_get_mouse_y:
 *
 * Gets the current mouse Y position.
 *
 * Returns: Mouse Y coordinate
 */
gint
grl_input_get_mouse_y (void)
{
    return GetMouseY ();
}

/**
 * grl_input_get_mouse_position:
 *
 * Gets the current mouse position as a vector.
 *
 * Returns: (transfer full): A #GrlVector2 with mouse position
 */
GrlVector2 *
grl_input_get_mouse_position (void)
{
    Vector2 pos = GetMousePosition ();
    return grl_vector2_new (pos.x, pos.y);
}

/**
 * grl_input_get_mouse_delta:
 *
 * Gets the mouse movement since last frame.
 *
 * Returns: (transfer full): A #GrlVector2 with mouse delta
 */
GrlVector2 *
grl_input_get_mouse_delta (void)
{
    Vector2 delta = GetMouseDelta ();
    return grl_vector2_new (delta.x, delta.y);
}

/**
 * grl_input_set_mouse_position:
 * @x: New X position
 * @y: New Y position
 *
 * Sets the mouse cursor position.
 */
void
grl_input_set_mouse_position (gint x,
                              gint y)
{
    SetMousePosition (x, y);
}

/**
 * grl_input_set_mouse_offset:
 * @offset_x: X offset
 * @offset_y: Y offset
 *
 * Sets mouse offset for scaled/transformed views.
 */
void
grl_input_set_mouse_offset (gint offset_x,
                            gint offset_y)
{
    SetMouseOffset (offset_x, offset_y);
}

/**
 * grl_input_set_mouse_scale:
 * @scale_x: X scale factor
 * @scale_y: Y scale factor
 *
 * Sets mouse scaling for scaled/transformed views.
 */
void
grl_input_set_mouse_scale (gfloat scale_x,
                           gfloat scale_y)
{
    SetMouseScale (scale_x, scale_y);
}

/**
 * grl_input_get_mouse_wheel_move:
 *
 * Gets mouse wheel movement.
 *
 * Returns: Wheel movement value
 */
gfloat
grl_input_get_mouse_wheel_move (void)
{
    return GetMouseWheelMove ();
}

/**
 * grl_input_get_mouse_wheel_move_v:
 *
 * Gets mouse wheel movement for both axes.
 *
 * Returns: (transfer full): A #GrlVector2 with wheel movement
 */
GrlVector2 *
grl_input_get_mouse_wheel_move_v (void)
{
    Vector2 move = GetMouseWheelMoveV ();
    return grl_vector2_new (move.x, move.y);
}

/**
 * grl_input_set_mouse_cursor:
 * @cursor: The #GrlMouseCursor shape to use
 *
 * Sets the mouse cursor shape.
 */
void
grl_input_set_mouse_cursor (GrlMouseCursor cursor)
{
    SetMouseCursor ((int)cursor);
}

/*
 * =============================================================================
 * Gamepad Input
 * =============================================================================
 */

/**
 * grl_input_is_gamepad_available:
 * @gamepad: Gamepad index (0-3)
 *
 * Checks if a gamepad is connected.
 *
 * Returns: %TRUE if gamepad is available
 */
gboolean
grl_input_is_gamepad_available (gint gamepad)
{
    return IsGamepadAvailable (gamepad) ? TRUE : FALSE;
}

/**
 * grl_input_get_gamepad_name:
 * @gamepad: Gamepad index (0-3)
 *
 * Gets the internal name of a gamepad.
 *
 * Returns: (transfer none): Gamepad name string
 */
const gchar *
grl_input_get_gamepad_name (gint gamepad)
{
    return GetGamepadName (gamepad);
}

/**
 * grl_input_is_gamepad_button_pressed:
 * @gamepad: Gamepad index (0-3)
 * @button: The #GrlGamepadButton to check
 *
 * Checks if a gamepad button has been pressed once.
 *
 * Returns: %TRUE if button was just pressed
 */
gboolean
grl_input_is_gamepad_button_pressed (gint             gamepad,
                                     GrlGamepadButton button)
{
    return IsGamepadButtonPressed (gamepad, (int)button) ? TRUE : FALSE;
}

/**
 * grl_input_is_gamepad_button_down:
 * @gamepad: Gamepad index (0-3)
 * @button: The #GrlGamepadButton to check
 *
 * Checks if a gamepad button is being held.
 *
 * Returns: %TRUE if button is down
 */
gboolean
grl_input_is_gamepad_button_down (gint             gamepad,
                                  GrlGamepadButton button)
{
    return IsGamepadButtonDown (gamepad, (int)button) ? TRUE : FALSE;
}

/**
 * grl_input_is_gamepad_button_released:
 * @gamepad: Gamepad index (0-3)
 * @button: The #GrlGamepadButton to check
 *
 * Checks if a gamepad button has been released once.
 *
 * Returns: %TRUE if button was just released
 */
gboolean
grl_input_is_gamepad_button_released (gint             gamepad,
                                      GrlGamepadButton button)
{
    return IsGamepadButtonReleased (gamepad, (int)button) ? TRUE : FALSE;
}

/**
 * grl_input_is_gamepad_button_up:
 * @gamepad: Gamepad index (0-3)
 * @button: The #GrlGamepadButton to check
 *
 * Checks if a gamepad button is NOT being pressed.
 *
 * Returns: %TRUE if button is up
 */
gboolean
grl_input_is_gamepad_button_up (gint             gamepad,
                                GrlGamepadButton button)
{
    return IsGamepadButtonUp (gamepad, (int)button) ? TRUE : FALSE;
}

/**
 * grl_input_get_gamepad_button_pressed:
 *
 * Gets the last gamepad button pressed.
 *
 * Returns: The last button pressed, or 0 if none
 */
gint
grl_input_get_gamepad_button_pressed (void)
{
    return GetGamepadButtonPressed ();
}

/**
 * grl_input_get_gamepad_axis_count:
 * @gamepad: Gamepad index (0-3)
 *
 * Gets the number of axes on a gamepad.
 *
 * Returns: Number of axes
 */
gint
grl_input_get_gamepad_axis_count (gint gamepad)
{
    return GetGamepadAxisCount (gamepad);
}

/**
 * grl_input_get_gamepad_axis_movement:
 * @gamepad: Gamepad index (0-3)
 * @axis: The #GrlGamepadAxis to query
 *
 * Gets the movement value for a gamepad axis.
 *
 * Returns: Axis value (-1.0 to 1.0)
 */
gfloat
grl_input_get_gamepad_axis_movement (gint           gamepad,
                                     GrlGamepadAxis axis)
{
    return GetGamepadAxisMovement (gamepad, (int)axis);
}

/**
 * grl_input_set_gamepad_mappings:
 * @mappings: SDL_GameControllerDB format mappings string
 *
 * Sets internal gamepad mappings.
 *
 * Returns: Number of mappings added
 */
gint
grl_input_set_gamepad_mappings (const gchar *mappings)
{
    g_return_val_if_fail (mappings != NULL, 0);

    return SetGamepadMappings (mappings);
}

/**
 * grl_input_set_gamepad_vibration:
 * @gamepad: Gamepad index (0-3)
 * @left_motor: Left motor intensity (0.0 to 1.0)
 * @right_motor: Right motor intensity (0.0 to 1.0)
 * @duration: Vibration duration in seconds
 *
 * Sets gamepad vibration for haptic feedback.
 */
void
grl_input_set_gamepad_vibration (gint   gamepad,
                                 gfloat left_motor,
                                 gfloat right_motor,
                                 gfloat duration)
{
    SetGamepadVibration (gamepad, left_motor, right_motor, duration);
}

/*
 * =============================================================================
 * Touch Input
 * =============================================================================
 */

/**
 * grl_input_get_touch_x:
 *
 * Gets touch position X for touch point 0.
 *
 * Returns: Touch X coordinate
 */
gint
grl_input_get_touch_x (void)
{
    return GetTouchX ();
}

/**
 * grl_input_get_touch_y:
 *
 * Gets touch position Y for touch point 0.
 *
 * Returns: Touch Y coordinate
 */
gint
grl_input_get_touch_y (void)
{
    return GetTouchY ();
}

/**
 * grl_input_get_touch_position:
 * @index: Touch point index
 *
 * Gets touch position for a specific touch point.
 *
 * Returns: (transfer full): A #GrlVector2 with touch position
 */
GrlVector2 *
grl_input_get_touch_position (gint index)
{
    Vector2 pos = GetTouchPosition (index);
    return grl_vector2_new (pos.x, pos.y);
}

/**
 * grl_input_get_touch_point_id:
 * @index: Touch point index
 *
 * Gets the identifier for a touch point.
 *
 * Returns: Touch point ID
 */
gint
grl_input_get_touch_point_id (gint index)
{
    return GetTouchPointId (index);
}

/**
 * grl_input_get_touch_point_count:
 *
 * Gets the number of active touch points.
 *
 * Returns: Number of touch points
 */
gint
grl_input_get_touch_point_count (void)
{
    return GetTouchPointCount ();
}

/*
 * =============================================================================
 * Gesture Detection
 * =============================================================================
 */

/**
 * grl_input_set_gestures_enabled:
 * @flags: Combination of #GrlGesture flags to enable
 *
 * Enables specific gesture detection.
 */
void
grl_input_set_gestures_enabled (GrlGesture flags)
{
    SetGesturesEnabled ((unsigned int)flags);
}

/**
 * grl_input_is_gesture_detected:
 * @gesture: The #GrlGesture to check
 *
 * Checks if a specific gesture has been detected.
 *
 * Returns: %TRUE if gesture was detected
 */
gboolean
grl_input_is_gesture_detected (GrlGesture gesture)
{
    return IsGestureDetected ((unsigned int)gesture) ? TRUE : FALSE;
}

/**
 * grl_input_get_gesture_detected:
 *
 * Gets the latest detected gesture.
 *
 * Returns: The detected #GrlGesture
 */
GrlGesture
grl_input_get_gesture_detected (void)
{
    return (GrlGesture)GetGestureDetected ();
}

/**
 * grl_input_get_gesture_hold_duration:
 *
 * Gets how long a hold gesture has been held.
 *
 * Returns: Hold duration in seconds
 */
gfloat
grl_input_get_gesture_hold_duration (void)
{
    return GetGestureHoldDuration ();
}

/**
 * grl_input_get_gesture_drag_vector:
 *
 * Gets the drag vector for drag gestures.
 *
 * Returns: (transfer full): A #GrlVector2 with drag vector
 */
GrlVector2 *
grl_input_get_gesture_drag_vector (void)
{
    Vector2 vec = GetGestureDragVector ();
    return grl_vector2_new (vec.x, vec.y);
}

/**
 * grl_input_get_gesture_drag_angle:
 *
 * Gets the drag angle for drag gestures.
 *
 * Returns: Drag angle in radians
 */
gfloat
grl_input_get_gesture_drag_angle (void)
{
    return GetGestureDragAngle ();
}

/**
 * grl_input_get_gesture_pinch_vector:
 *
 * Gets the pinch delta vector for pinch gestures.
 *
 * Returns: (transfer full): A #GrlVector2 with pinch vector
 */
GrlVector2 *
grl_input_get_gesture_pinch_vector (void)
{
    Vector2 vec = GetGesturePinchVector ();
    return grl_vector2_new (vec.x, vec.y);
}

/**
 * grl_input_get_gesture_pinch_angle:
 *
 * Gets the pinch angle for pinch gestures.
 *
 * Returns: Pinch angle in radians
 */
gfloat
grl_input_get_gesture_pinch_angle (void)
{
    return GetGesturePinchAngle ();
}
