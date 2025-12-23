# Input System

This document covers the input system added in Phase 3: keyboard, mouse, gamepad, and touch input.

## Overview

Graylib provides two approaches for handling input:

1. **Signal-based (Event-driven)**: Connect to GrlWindow signals for input events. Best for discrete actions (jump, shoot, menu selection).

2. **Polling-based (Continuous)**: Use `grl_input_*` functions to check input state each frame. Best for continuous actions (movement, camera control).

## Keyboard Input

### Polling Functions

```c
/* Check if a key was just pressed (single frame) */
if (grl_input_is_key_pressed (GRL_KEY_SPACE))
{
    /* Player jumps */
    player_jump ();
}

/* Check if a key is being held down (continuous) */
if (grl_input_is_key_down (GRL_KEY_W))
{
    /* Move forward continuously */
    player_move_forward (delta_time);
}

/* Check if a key was just released */
if (grl_input_is_key_released (GRL_KEY_LEFT_SHIFT))
{
    /* Stop sprinting */
    player_stop_sprint ();
}

/* Check if a key is NOT pressed */
if (grl_input_is_key_up (GRL_KEY_ESCAPE))
{
    /* Menu is closed */
}

/* Get last key pressed from queue */
GrlKey key = grl_input_get_key_pressed ();
if (key != GRL_KEY_NULL)
{
    g_print ("Key pressed: %d\n", key);
}

/* Get Unicode character for text input */
gint codepoint = grl_input_get_char_pressed ();
if (codepoint != 0)
{
    /* Process text input */
    text_buffer_append_char (codepoint);
}
```

### Signal-based Keyboard Input

```c
static void
on_key_pressed (GrlWindow *window,
                GrlKey     key,
                gpointer   user_data)
{
    switch (key)
    {
    case GRL_KEY_ESCAPE:
        toggle_pause_menu ();
        break;
    case GRL_KEY_F11:
        grl_window_toggle_fullscreen (window);
        break;
    }
}

static void
on_key_released (GrlWindow *window,
                 GrlKey     key,
                 gpointer   user_data)
{
    if (key == GRL_KEY_LEFT_SHIFT)
    {
        player_stop_sprint ();
    }
}

/* Connect signals */
g_signal_connect (window, "key-pressed",
                  G_CALLBACK (on_key_pressed), NULL);
g_signal_connect (window, "key-released",
                  G_CALLBACK (on_key_released), NULL);
```

### Key Constants

Common key constants (see `GrlKey` enum for full list):

| Constant | Description |
|----------|-------------|
| `GRL_KEY_SPACE` | Spacebar |
| `GRL_KEY_ESCAPE` | Escape key |
| `GRL_KEY_ENTER` | Enter/Return |
| `GRL_KEY_TAB` | Tab key |
| `GRL_KEY_A` - `GRL_KEY_Z` | Letter keys |
| `GRL_KEY_ZERO` - `GRL_KEY_NINE` | Number keys |
| `GRL_KEY_F1` - `GRL_KEY_F12` | Function keys |
| `GRL_KEY_UP/DOWN/LEFT/RIGHT` | Arrow keys |
| `GRL_KEY_LEFT_SHIFT` | Left Shift |
| `GRL_KEY_LEFT_CONTROL` | Left Control |
| `GRL_KEY_LEFT_ALT` | Left Alt |

## Mouse Input

### Polling Functions

```c
/* Check mouse button state */
if (grl_input_is_mouse_button_pressed (GRL_MOUSE_BUTTON_LEFT))
{
    /* Start selection or fire weapon */
    on_left_click ();
}

if (grl_input_is_mouse_button_down (GRL_MOUSE_BUTTON_LEFT))
{
    /* Drag selection or hold fire */
}

if (grl_input_is_mouse_button_released (GRL_MOUSE_BUTTON_LEFT))
{
    /* End selection or stop firing */
}

/* Get mouse position */
gint x = grl_input_get_mouse_x ();
gint y = grl_input_get_mouse_y ();

/* Or as a vector */
g_autoptr(GrlVector2) pos = grl_input_get_mouse_position ();

/* Get mouse movement since last frame */
g_autoptr(GrlVector2) delta = grl_input_get_mouse_delta ();
camera_rotate (delta->x, delta->y);

/* Get mouse wheel movement */
gfloat wheel = grl_input_get_mouse_wheel_move ();
if (wheel != 0.0f)
{
    camera_zoom (wheel);
}

/* Both axes for horizontal scrolling */
g_autoptr(GrlVector2) wheel_v = grl_input_get_mouse_wheel_move_v ();
```

### Signal-based Mouse Input

```c
static void
on_mouse_button_pressed (GrlWindow      *window,
                         GrlMouseButton  button,
                         gint            x,
                         gint            y,
                         gpointer        user_data)
{
    if (button == GRL_MOUSE_BUTTON_LEFT)
    {
        start_selection (x, y);
    }
    else if (button == GRL_MOUSE_BUTTON_RIGHT)
    {
        show_context_menu (x, y);
    }
}

static void
on_mouse_moved (GrlWindow *window,
                gint       x,
                gint       y,
                gint       delta_x,
                gint       delta_y,
                gpointer   user_data)
{
    /* First-person camera control */
    camera_rotate (delta_x * 0.1f, delta_y * 0.1f);
}

static void
on_mouse_wheel (GrlWindow *window,
                gfloat     x,
                gfloat     y,
                gpointer   user_data)
{
    camera_zoom (y);  /* y is vertical scroll */
}

/* Connect signals */
g_signal_connect (window, "mouse-button-pressed",
                  G_CALLBACK (on_mouse_button_pressed), NULL);
g_signal_connect (window, "mouse-moved",
                  G_CALLBACK (on_mouse_moved), NULL);
g_signal_connect (window, "mouse-wheel",
                  G_CALLBACK (on_mouse_wheel), NULL);
```

### Mouse Control Functions

```c
/* Set mouse position */
grl_input_set_mouse_position (screen_width / 2, screen_height / 2);

/* Set offset for scaled views */
grl_input_set_mouse_offset (offset_x, offset_y);
grl_input_set_mouse_scale (scale_x, scale_y);

/* Set cursor shape */
grl_input_set_mouse_cursor (GRL_MOUSE_CURSOR_CROSSHAIR);
```

## Gamepad Input

### Polling Functions

```c
/* Check if gamepad is connected (0-3) */
if (grl_input_is_gamepad_available (0))
{
    const gchar *name = grl_input_get_gamepad_name (0);
    g_print ("Gamepad 0: %s\n", name);

    /* Button input */
    if (grl_input_is_gamepad_button_pressed (0, GRL_GAMEPAD_BUTTON_RIGHT_FACE_DOWN))
    {
        /* A button pressed - jump */
        player_jump ();
    }

    if (grl_input_is_gamepad_button_down (0, GRL_GAMEPAD_BUTTON_RIGHT_TRIGGER_2))
    {
        /* R2 held - accelerate */
        car_accelerate ();
    }

    /* Analog stick input (-1.0 to 1.0) */
    gfloat left_x = grl_input_get_gamepad_axis_movement (0, GRL_GAMEPAD_AXIS_LEFT_X);
    gfloat left_y = grl_input_get_gamepad_axis_movement (0, GRL_GAMEPAD_AXIS_LEFT_Y);
    player_move (left_x, left_y);

    gfloat right_x = grl_input_get_gamepad_axis_movement (0, GRL_GAMEPAD_AXIS_RIGHT_X);
    gfloat right_y = grl_input_get_gamepad_axis_movement (0, GRL_GAMEPAD_AXIS_RIGHT_Y);
    camera_rotate (right_x, right_y);

    /* Trigger pressure (0.0 to 1.0) */
    gfloat lt = grl_input_get_gamepad_axis_movement (0, GRL_GAMEPAD_AXIS_LEFT_TRIGGER);
    gfloat rt = grl_input_get_gamepad_axis_movement (0, GRL_GAMEPAD_AXIS_RIGHT_TRIGGER);
}
```

### Signal-based Gamepad Input

```c
static void
on_gamepad_button_pressed (GrlWindow        *window,
                           gint              gamepad,
                           GrlGamepadButton  button,
                           gpointer          user_data)
{
    if (button == GRL_GAMEPAD_BUTTON_MIDDLE_RIGHT)
    {
        /* Start button - pause game */
        toggle_pause ();
    }
}

static void
on_gamepad_axis_moved (GrlWindow      *window,
                       gint            gamepad,
                       GrlGamepadAxis  axis,
                       gfloat          value,
                       gpointer        user_data)
{
    if (axis == GRL_GAMEPAD_AXIS_LEFT_X)
    {
        player_set_horizontal_input (value);
    }
}

/* Connect signals */
g_signal_connect (window, "gamepad-button-pressed",
                  G_CALLBACK (on_gamepad_button_pressed), NULL);
g_signal_connect (window, "gamepad-axis-moved",
                  G_CALLBACK (on_gamepad_axis_moved), NULL);
```

### Gamepad Vibration

```c
/* Rumble: left motor, right motor, duration in seconds */
grl_input_set_gamepad_vibration (0, 0.5f, 0.5f, 0.2f);

/* Asymmetric rumble */
grl_input_set_gamepad_vibration (0, 0.8f, 0.2f, 0.1f);
```

### Gamepad Mappings

```c
/* Load custom gamepad mappings (SDL_GameControllerDB format) */
const gchar *mappings =
    "03000000790000001100000010010000,Retro Controller,"
    "a:b1,b:b2,back:b8,dpdown:+a1,dpleft:-a0,dpright:+a0,dpup:-a1,"
    "leftshoulder:b4,lefttrigger:b6,rightshoulder:b5,righttrigger:b7,"
    "start:b9,x:b0,y:b3,platform:Linux,";

gint count = grl_input_set_gamepad_mappings (mappings);
g_print ("Loaded %d mappings\n", count);
```

## Touch Input

### Polling Functions

```c
/* Get number of touch points */
gint count = grl_input_get_touch_point_count ();

/* Get touch position for first touch */
gint x = grl_input_get_touch_x ();
gint y = grl_input_get_touch_y ();

/* Get specific touch point */
for (gint i = 0; i < count; i++)
{
    g_autoptr(GrlVector2) pos = grl_input_get_touch_position (i);
    gint id = grl_input_get_touch_point_id (i);
    /* Process touch */
}
```

## Gesture Detection

### Enabling Gestures

```c
/* Enable specific gestures */
grl_input_set_gestures_enabled (GRL_GESTURE_TAP |
                                 GRL_GESTURE_DOUBLETAP |
                                 GRL_GESTURE_DRAG |
                                 GRL_GESTURE_PINCH_IN |
                                 GRL_GESTURE_PINCH_OUT);
```

### Detecting Gestures

```c
/* Check for specific gesture */
if (grl_input_is_gesture_detected (GRL_GESTURE_TAP))
{
    on_tap ();
}

if (grl_input_is_gesture_detected (GRL_GESTURE_DOUBLETAP))
{
    toggle_fullscreen ();
}

/* Get last detected gesture */
GrlGesture gesture = grl_input_get_gesture_detected ();

/* Gesture-specific data */
if (gesture == GRL_GESTURE_HOLD)
{
    gfloat duration = grl_input_get_gesture_hold_duration ();
}

if (gesture == GRL_GESTURE_DRAG)
{
    g_autoptr(GrlVector2) drag = grl_input_get_gesture_drag_vector ();
    gfloat angle = grl_input_get_gesture_drag_angle ();
}

if (gesture == GRL_GESTURE_PINCH_IN || gesture == GRL_GESTURE_PINCH_OUT)
{
    g_autoptr(GrlVector2) pinch = grl_input_get_gesture_pinch_vector ();
    gfloat angle = grl_input_get_gesture_pinch_angle ();
}
```

## Complete Example

```c
#include <graylib.h>

typedef struct
{
    gfloat x, y;
    gfloat speed;
} Player;

static Player player = { 400, 300, 200.0f };

/* Event handlers for discrete actions */
static void
on_key_pressed (GrlWindow *window, GrlKey key, gpointer data)
{
    if (key == GRL_KEY_ESCAPE)
    {
        grl_window_set_should_close (window, TRUE);
    }
}

int
main (int argc, char *argv[])
{
    g_autoptr(GrlWindow) window = grl_window_new (800, 600, "Input Example");
    g_autoptr(GrlColor) bg = grl_color_new_raywhite ();
    g_autoptr(GrlColor) player_color = grl_color_new_red ();
    gfloat dt;

    grl_window_set_target_fps (window, 60);

    /* Connect event handlers */
    g_signal_connect (window, "key-pressed",
                      G_CALLBACK (on_key_pressed), NULL);

    while (!grl_window_should_close (window))
    {
        dt = grl_window_get_frame_time (window);

        /* Poll input for signals */
        grl_window_poll_input (window);

        /* Continuous movement with polling */
        if (grl_input_is_key_down (GRL_KEY_W))
            player.y -= player.speed * dt;
        if (grl_input_is_key_down (GRL_KEY_S))
            player.y += player.speed * dt;
        if (grl_input_is_key_down (GRL_KEY_A))
            player.x -= player.speed * dt;
        if (grl_input_is_key_down (GRL_KEY_D))
            player.x += player.speed * dt;

        /* Gamepad support */
        if (grl_input_is_gamepad_available (0))
        {
            gfloat lx = grl_input_get_gamepad_axis_movement (0, GRL_GAMEPAD_AXIS_LEFT_X);
            gfloat ly = grl_input_get_gamepad_axis_movement (0, GRL_GAMEPAD_AXIS_LEFT_Y);
            player.x += lx * player.speed * dt;
            player.y += ly * player.speed * dt;
        }

        /* Drawing */
        grl_window_begin_drawing (window);
        grl_draw_clear_background (bg);

        g_autoptr(GrlVector2) pos = grl_vector2_new (player.x, player.y);
        grl_draw_circle_v (pos, 20.0f, player_color);

        grl_draw_text ("WASD or Gamepad to move", 10, 10, 20, player_color);
        grl_draw_fps (10, 580);

        grl_window_end_drawing (window);
    }

    return 0;
}
```

## Window Signals Reference

| Signal | Parameters | Description |
|--------|------------|-------------|
| `key-pressed` | GrlKey key | Emitted when a key is pressed |
| `key-released` | GrlKey key | Emitted when a key is released |
| `mouse-button-pressed` | GrlMouseButton, int x, int y | Mouse button pressed |
| `mouse-button-released` | GrlMouseButton, int x, int y | Mouse button released |
| `mouse-moved` | int x, int y, int dx, int dy | Mouse position changed |
| `mouse-wheel` | float x, float y | Mouse wheel scrolled |
| `gamepad-button-pressed` | int gamepad, GrlGamepadButton | Gamepad button pressed |
| `gamepad-button-released` | int gamepad, GrlGamepadButton | Gamepad button released |
| `gamepad-axis-moved` | int gamepad, GrlGamepadAxis, float | Gamepad axis changed |
