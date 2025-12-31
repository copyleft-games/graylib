/* grl-input.h
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Input manager for keyboard, mouse, gamepad, and touch input.
 */

#pragma once

#if !defined(GRAYLIB_INSIDE) && !defined(GRAYLIB_COMPILATION)
#error "Only <graylib.h> can be included directly."
#endif

#include <glib-object.h>
#include "../grl-version.h"
#include "../grl-enums.h"
#include "../math/grl-vector2.h"

G_BEGIN_DECLS

/*
 * =============================================================================
 * Keyboard Input
 * =============================================================================
 */

/**
 * grl_input_is_key_pressed:
 * @key: The #GrlKey to check
 *
 * Checks if a key has been pressed once (not held).
 * Returns %TRUE only on the first frame the key is pressed.
 *
 * Returns: %TRUE if the key was just pressed
 */
GRL_AVAILABLE_IN_ALL
gboolean        grl_input_is_key_pressed        (GrlKey              key);

/**
 * grl_input_is_key_pressed_repeat:
 * @key: The #GrlKey to check
 *
 * Checks if a key has been pressed again (key repeat).
 * Returns %TRUE when keyboard repeat triggers.
 *
 * Returns: %TRUE if key repeat triggered
 */
GRL_AVAILABLE_IN_ALL
gboolean        grl_input_is_key_pressed_repeat (GrlKey              key);

/**
 * grl_input_is_key_down:
 * @key: The #GrlKey to check
 *
 * Checks if a key is currently being held down.
 *
 * Returns: %TRUE if the key is down
 */
GRL_AVAILABLE_IN_ALL
gboolean        grl_input_is_key_down           (GrlKey              key);

/**
 * grl_input_is_key_released:
 * @key: The #GrlKey to check
 *
 * Checks if a key has been released once.
 * Returns %TRUE only on the first frame the key is released.
 *
 * Returns: %TRUE if the key was just released
 */
GRL_AVAILABLE_IN_ALL
gboolean        grl_input_is_key_released       (GrlKey              key);

/**
 * grl_input_is_key_up:
 * @key: The #GrlKey to check
 *
 * Checks if a key is NOT being pressed.
 *
 * Returns: %TRUE if the key is up
 */
GRL_AVAILABLE_IN_ALL
gboolean        grl_input_is_key_up             (GrlKey              key);

/**
 * grl_input_get_key_pressed:
 *
 * Gets the last key pressed. Call repeatedly to get all keys
 * from the input queue. Returns %GRL_KEY_NULL when queue is empty.
 *
 * Returns: The last #GrlKey pressed, or %GRL_KEY_NULL
 */
GRL_AVAILABLE_IN_ALL
GrlKey          grl_input_get_key_pressed       (void);

/**
 * grl_input_get_char_pressed:
 *
 * Gets the last Unicode character pressed. Useful for text input.
 * Call repeatedly to get all characters from the input queue.
 *
 * Returns: Unicode codepoint, or 0 when queue is empty
 */
GRL_AVAILABLE_IN_ALL
gint            grl_input_get_char_pressed      (void);

/**
 * grl_input_set_exit_key:
 * @key: The #GrlKey to use for exiting, or %GRL_KEY_NULL to disable
 *
 * Sets a custom key to trigger window close. The default exit key is
 * %GRL_KEY_ESCAPE. Pass %GRL_KEY_NULL (0) to disable the exit key entirely,
 * which is useful for games that want to use ESC for their pause menu.
 *
 * Example:
 * |[<!-- language="C" -->
 * // Disable the default ESC exit behavior
 * grl_input_set_exit_key (GRL_KEY_NULL);
 *
 * // Or use a different key to exit
 * grl_input_set_exit_key (GRL_KEY_F10);
 * ]|
 */
GRL_AVAILABLE_IN_ALL
void            grl_input_set_exit_key          (GrlKey              key);

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
GRL_AVAILABLE_IN_ALL
gboolean        grl_input_is_mouse_button_pressed   (GrlMouseButton  button);

/**
 * grl_input_is_mouse_button_down:
 * @button: The #GrlMouseButton to check
 *
 * Checks if a mouse button is currently held down.
 *
 * Returns: %TRUE if the button is down
 */
GRL_AVAILABLE_IN_ALL
gboolean        grl_input_is_mouse_button_down      (GrlMouseButton  button);

/**
 * grl_input_is_mouse_button_released:
 * @button: The #GrlMouseButton to check
 *
 * Checks if a mouse button has been released once.
 *
 * Returns: %TRUE if the button was just released
 */
GRL_AVAILABLE_IN_ALL
gboolean        grl_input_is_mouse_button_released  (GrlMouseButton  button);

/**
 * grl_input_is_mouse_button_up:
 * @button: The #GrlMouseButton to check
 *
 * Checks if a mouse button is NOT being pressed.
 *
 * Returns: %TRUE if the button is up
 */
GRL_AVAILABLE_IN_ALL
gboolean        grl_input_is_mouse_button_up        (GrlMouseButton  button);

/**
 * grl_input_get_mouse_x:
 *
 * Gets the current mouse X position.
 *
 * Returns: Mouse X coordinate
 */
GRL_AVAILABLE_IN_ALL
gint            grl_input_get_mouse_x               (void);

/**
 * grl_input_get_mouse_y:
 *
 * Gets the current mouse Y position.
 *
 * Returns: Mouse Y coordinate
 */
GRL_AVAILABLE_IN_ALL
gint            grl_input_get_mouse_y               (void);

/**
 * grl_input_get_mouse_position:
 *
 * Gets the current mouse position as a vector.
 *
 * Returns: (transfer full): A #GrlVector2 with mouse position
 */
GRL_AVAILABLE_IN_ALL
GrlVector2 *    grl_input_get_mouse_position        (void);

/**
 * grl_input_get_mouse_delta:
 *
 * Gets the mouse movement since last frame.
 *
 * Returns: (transfer full): A #GrlVector2 with mouse delta
 */
GRL_AVAILABLE_IN_ALL
GrlVector2 *    grl_input_get_mouse_delta           (void);

/**
 * grl_input_set_mouse_position:
 * @x: New X position
 * @y: New Y position
 *
 * Sets the mouse cursor position.
 */
GRL_AVAILABLE_IN_ALL
void            grl_input_set_mouse_position        (gint            x,
                                                     gint            y);

/**
 * grl_input_set_mouse_offset:
 * @offset_x: X offset
 * @offset_y: Y offset
 *
 * Sets mouse offset for scaled/transformed views.
 */
GRL_AVAILABLE_IN_ALL
void            grl_input_set_mouse_offset          (gint            offset_x,
                                                     gint            offset_y);

/**
 * grl_input_set_mouse_scale:
 * @scale_x: X scale factor
 * @scale_y: Y scale factor
 *
 * Sets mouse scaling for scaled/transformed views.
 */
GRL_AVAILABLE_IN_ALL
void            grl_input_set_mouse_scale           (gfloat          scale_x,
                                                     gfloat          scale_y);

/**
 * grl_input_get_mouse_wheel_move:
 *
 * Gets mouse wheel movement (vertical, or horizontal if larger).
 *
 * Returns: Wheel movement value
 */
GRL_AVAILABLE_IN_ALL
gfloat          grl_input_get_mouse_wheel_move      (void);

/**
 * grl_input_get_mouse_wheel_move_v:
 *
 * Gets mouse wheel movement for both axes.
 *
 * Returns: (transfer full): A #GrlVector2 with wheel movement
 */
GRL_AVAILABLE_IN_ALL
GrlVector2 *    grl_input_get_mouse_wheel_move_v    (void);

/**
 * grl_input_set_mouse_cursor:
 * @cursor: The #GrlMouseCursor shape to use
 *
 * Sets the mouse cursor shape.
 */
GRL_AVAILABLE_IN_ALL
void            grl_input_set_mouse_cursor          (GrlMouseCursor  cursor);

/*
 * =============================================================================
 * Gamepad Input
 * =============================================================================
 */

/**
 * grl_input_is_gamepad_available:
 * @gamepad: Gamepad index (0-3)
 *
 * Checks if a gamepad is connected and available.
 *
 * Returns: %TRUE if gamepad is available
 */
GRL_AVAILABLE_IN_ALL
gboolean        grl_input_is_gamepad_available      (gint            gamepad);

/**
 * grl_input_get_gamepad_name:
 * @gamepad: Gamepad index (0-3)
 *
 * Gets the internal name of a gamepad.
 *
 * Returns: (transfer none): Gamepad name string
 */
GRL_AVAILABLE_IN_ALL
const gchar *   grl_input_get_gamepad_name          (gint            gamepad);

/**
 * grl_input_is_gamepad_button_pressed:
 * @gamepad: Gamepad index (0-3)
 * @button: The #GrlGamepadButton to check
 *
 * Checks if a gamepad button has been pressed once.
 *
 * Returns: %TRUE if button was just pressed
 */
GRL_AVAILABLE_IN_ALL
gboolean        grl_input_is_gamepad_button_pressed (gint            gamepad,
                                                     GrlGamepadButton button);

/**
 * grl_input_is_gamepad_button_down:
 * @gamepad: Gamepad index (0-3)
 * @button: The #GrlGamepadButton to check
 *
 * Checks if a gamepad button is being held down.
 *
 * Returns: %TRUE if button is down
 */
GRL_AVAILABLE_IN_ALL
gboolean        grl_input_is_gamepad_button_down    (gint            gamepad,
                                                     GrlGamepadButton button);

/**
 * grl_input_is_gamepad_button_released:
 * @gamepad: Gamepad index (0-3)
 * @button: The #GrlGamepadButton to check
 *
 * Checks if a gamepad button has been released once.
 *
 * Returns: %TRUE if button was just released
 */
GRL_AVAILABLE_IN_ALL
gboolean        grl_input_is_gamepad_button_released (gint           gamepad,
                                                      GrlGamepadButton button);

/**
 * grl_input_is_gamepad_button_up:
 * @gamepad: Gamepad index (0-3)
 * @button: The #GrlGamepadButton to check
 *
 * Checks if a gamepad button is NOT being pressed.
 *
 * Returns: %TRUE if button is up
 */
GRL_AVAILABLE_IN_ALL
gboolean        grl_input_is_gamepad_button_up      (gint            gamepad,
                                                     GrlGamepadButton button);

/**
 * grl_input_get_gamepad_button_pressed:
 *
 * Gets the last gamepad button pressed.
 *
 * Returns: The last button pressed, or 0 if none
 */
GRL_AVAILABLE_IN_ALL
gint            grl_input_get_gamepad_button_pressed (void);

/**
 * grl_input_get_gamepad_axis_count:
 * @gamepad: Gamepad index (0-3)
 *
 * Gets the number of axes on a gamepad.
 *
 * Returns: Number of axes
 */
GRL_AVAILABLE_IN_ALL
gint            grl_input_get_gamepad_axis_count    (gint            gamepad);

/**
 * grl_input_get_gamepad_axis_movement:
 * @gamepad: Gamepad index (0-3)
 * @axis: The #GrlGamepadAxis to query
 *
 * Gets the movement value for a gamepad axis (-1.0 to 1.0).
 *
 * Returns: Axis value
 */
GRL_AVAILABLE_IN_ALL
gfloat          grl_input_get_gamepad_axis_movement (gint            gamepad,
                                                     GrlGamepadAxis  axis);

/**
 * grl_input_set_gamepad_mappings:
 * @mappings: SDL_GameControllerDB format mappings string
 *
 * Sets internal gamepad mappings using SDL format.
 *
 * Returns: Number of mappings added
 */
GRL_AVAILABLE_IN_ALL
gint            grl_input_set_gamepad_mappings      (const gchar    *mappings);

/**
 * grl_input_set_gamepad_vibration:
 * @gamepad: Gamepad index (0-3)
 * @left_motor: Left motor intensity (0.0 to 1.0)
 * @right_motor: Right motor intensity (0.0 to 1.0)
 * @duration: Vibration duration in seconds
 *
 * Sets gamepad vibration for haptic feedback.
 */
GRL_AVAILABLE_IN_ALL
void            grl_input_set_gamepad_vibration     (gint            gamepad,
                                                     gfloat          left_motor,
                                                     gfloat          right_motor,
                                                     gfloat          duration);

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
GRL_AVAILABLE_IN_ALL
gint            grl_input_get_touch_x               (void);

/**
 * grl_input_get_touch_y:
 *
 * Gets touch position Y for touch point 0.
 *
 * Returns: Touch Y coordinate
 */
GRL_AVAILABLE_IN_ALL
gint            grl_input_get_touch_y               (void);

/**
 * grl_input_get_touch_position:
 * @index: Touch point index
 *
 * Gets touch position for a specific touch point.
 *
 * Returns: (transfer full): A #GrlVector2 with touch position
 */
GRL_AVAILABLE_IN_ALL
GrlVector2 *    grl_input_get_touch_position        (gint            index);

/**
 * grl_input_get_touch_point_id:
 * @index: Touch point index
 *
 * Gets the identifier for a touch point.
 *
 * Returns: Touch point ID
 */
GRL_AVAILABLE_IN_ALL
gint            grl_input_get_touch_point_id        (gint            index);

/**
 * grl_input_get_touch_point_count:
 *
 * Gets the number of active touch points.
 *
 * Returns: Number of touch points
 */
GRL_AVAILABLE_IN_ALL
gint            grl_input_get_touch_point_count     (void);

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
GRL_AVAILABLE_IN_ALL
void            grl_input_set_gestures_enabled      (GrlGesture      flags);

/**
 * grl_input_is_gesture_detected:
 * @gesture: The #GrlGesture to check
 *
 * Checks if a specific gesture has been detected.
 *
 * Returns: %TRUE if gesture was detected
 */
GRL_AVAILABLE_IN_ALL
gboolean        grl_input_is_gesture_detected       (GrlGesture      gesture);

/**
 * grl_input_get_gesture_detected:
 *
 * Gets the latest detected gesture.
 *
 * Returns: The detected #GrlGesture
 */
GRL_AVAILABLE_IN_ALL
GrlGesture      grl_input_get_gesture_detected      (void);

/**
 * grl_input_get_gesture_hold_duration:
 *
 * Gets how long a hold gesture has been held.
 *
 * Returns: Hold duration in seconds
 */
GRL_AVAILABLE_IN_ALL
gfloat          grl_input_get_gesture_hold_duration (void);

/**
 * grl_input_get_gesture_drag_vector:
 *
 * Gets the drag vector for drag gestures.
 *
 * Returns: (transfer full): A #GrlVector2 with drag vector
 */
GRL_AVAILABLE_IN_ALL
GrlVector2 *    grl_input_get_gesture_drag_vector   (void);

/**
 * grl_input_get_gesture_drag_angle:
 *
 * Gets the drag angle for drag gestures.
 *
 * Returns: Drag angle in radians
 */
GRL_AVAILABLE_IN_ALL
gfloat          grl_input_get_gesture_drag_angle    (void);

/**
 * grl_input_get_gesture_pinch_vector:
 *
 * Gets the pinch delta vector for pinch gestures.
 *
 * Returns: (transfer full): A #GrlVector2 with pinch vector
 */
GRL_AVAILABLE_IN_ALL
GrlVector2 *    grl_input_get_gesture_pinch_vector  (void);

/**
 * grl_input_get_gesture_pinch_angle:
 *
 * Gets the pinch angle for pinch gestures.
 *
 * Returns: Pinch angle in radians
 */
GRL_AVAILABLE_IN_ALL
gfloat          grl_input_get_gesture_pinch_angle   (void);

G_END_DECLS
