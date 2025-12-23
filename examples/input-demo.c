/* input-demo.c
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Demonstrates Graylib's input system:
 * - Keyboard input via signals and polling
 * - Mouse input via signals and polling
 * - Gamepad input via signals and polling
 *
 * This example shows both the signal-based event handling approach
 * and the direct polling approach for input handling.
 */

#include <graylib.h>
#include <stdio.h>

/* Input state structure for display */
typedef struct
{
    gchar   last_key_name[32];
    gchar   mouse_button_name[32];
    gint    mouse_x;
    gint    mouse_y;
    gint    mouse_delta_x;
    gint    mouse_delta_y;
    gfloat  wheel_x;
    gfloat  wheel_y;
    gint    gamepad_index;
    gchar   gamepad_button_name[32];
    gfloat  left_stick_x;
    gfloat  left_stick_y;
} InputState;

static InputState state = { 0 };

/* Helper to get key name */
static const gchar *
get_key_name (GrlKey key)
{
    switch (key)
    {
    case GRL_KEY_SPACE: return "SPACE";
    case GRL_KEY_ESCAPE: return "ESCAPE";
    case GRL_KEY_ENTER: return "ENTER";
    case GRL_KEY_UP: return "UP";
    case GRL_KEY_DOWN: return "DOWN";
    case GRL_KEY_LEFT: return "LEFT";
    case GRL_KEY_RIGHT: return "RIGHT";
    case GRL_KEY_A: return "A";
    case GRL_KEY_D: return "D";
    case GRL_KEY_S: return "S";
    case GRL_KEY_W: return "W";
    case GRL_KEY_LEFT_SHIFT: return "L-SHIFT";
    case GRL_KEY_LEFT_CONTROL: return "L-CTRL";
    case GRL_KEY_TAB: return "TAB";
    default:
        if (key >= GRL_KEY_A && key <= GRL_KEY_Z)
        {
            return "LETTER";
        }
        if (key >= GRL_KEY_ZERO && key <= GRL_KEY_NINE)
        {
            return "NUMBER";
        }
        return "OTHER";
    }
}

/* Helper to get mouse button name */
static const gchar *
get_mouse_button_name (GrlMouseButton button)
{
    switch (button)
    {
    case GRL_MOUSE_BUTTON_LEFT: return "LEFT";
    case GRL_MOUSE_BUTTON_RIGHT: return "RIGHT";
    case GRL_MOUSE_BUTTON_MIDDLE: return "MIDDLE";
    case GRL_MOUSE_BUTTON_SIDE: return "SIDE";
    case GRL_MOUSE_BUTTON_EXTRA: return "EXTRA";
    case GRL_MOUSE_BUTTON_FORWARD: return "FORWARD";
    case GRL_MOUSE_BUTTON_BACK: return "BACK";
    default: return "UNKNOWN";
    }
}

/* Helper to get gamepad button name */
static const gchar *
get_gamepad_button_name (GrlGamepadButton button)
{
    switch (button)
    {
    case GRL_GAMEPAD_BUTTON_LEFT_FACE_UP: return "D-PAD UP";
    case GRL_GAMEPAD_BUTTON_LEFT_FACE_DOWN: return "D-PAD DOWN";
    case GRL_GAMEPAD_BUTTON_LEFT_FACE_LEFT: return "D-PAD LEFT";
    case GRL_GAMEPAD_BUTTON_LEFT_FACE_RIGHT: return "D-PAD RIGHT";
    case GRL_GAMEPAD_BUTTON_RIGHT_FACE_UP: return "Y/TRIANGLE";
    case GRL_GAMEPAD_BUTTON_RIGHT_FACE_DOWN: return "A/CROSS";
    case GRL_GAMEPAD_BUTTON_RIGHT_FACE_LEFT: return "X/SQUARE";
    case GRL_GAMEPAD_BUTTON_RIGHT_FACE_RIGHT: return "B/CIRCLE";
    case GRL_GAMEPAD_BUTTON_LEFT_TRIGGER_1: return "L1/LB";
    case GRL_GAMEPAD_BUTTON_LEFT_TRIGGER_2: return "L2/LT";
    case GRL_GAMEPAD_BUTTON_RIGHT_TRIGGER_1: return "R1/RB";
    case GRL_GAMEPAD_BUTTON_RIGHT_TRIGGER_2: return "R2/RT";
    case GRL_GAMEPAD_BUTTON_MIDDLE_LEFT: return "SELECT";
    case GRL_GAMEPAD_BUTTON_MIDDLE: return "HOME";
    case GRL_GAMEPAD_BUTTON_MIDDLE_RIGHT: return "START";
    case GRL_GAMEPAD_BUTTON_LEFT_THUMB: return "L3";
    case GRL_GAMEPAD_BUTTON_RIGHT_THUMB: return "R3";
    default: return "UNKNOWN";
    }
}

/*
 * Signal handlers for input events
 */

static void
on_key_pressed (GrlWindow *window,
                GrlKey     key,
                gpointer   user_data)
{
    g_snprintf (state.last_key_name, sizeof (state.last_key_name),
                "%s PRESSED", get_key_name (key));
}

static void
on_key_released (GrlWindow *window,
                 GrlKey     key,
                 gpointer   user_data)
{
    g_snprintf (state.last_key_name, sizeof (state.last_key_name),
                "%s RELEASED", get_key_name (key));
}

static void
on_mouse_button_pressed (GrlWindow      *window,
                          GrlMouseButton  button,
                          gint            x,
                          gint            y,
                          gpointer        user_data)
{
    g_snprintf (state.mouse_button_name, sizeof (state.mouse_button_name),
                "%s PRESSED at (%d, %d)", get_mouse_button_name (button), x, y);
}

static void
on_mouse_button_released (GrlWindow      *window,
                           GrlMouseButton  button,
                           gint            x,
                           gint            y,
                           gpointer        user_data)
{
    g_snprintf (state.mouse_button_name, sizeof (state.mouse_button_name),
                "%s RELEASED", get_mouse_button_name (button));
}

static void
on_mouse_moved (GrlWindow *window,
                gint       x,
                gint       y,
                gint       delta_x,
                gint       delta_y,
                gpointer   user_data)
{
    state.mouse_x = x;
    state.mouse_y = y;
    state.mouse_delta_x = delta_x;
    state.mouse_delta_y = delta_y;
}

static void
on_mouse_wheel (GrlWindow *window,
                gfloat     x,
                gfloat     y,
                gpointer   user_data)
{
    state.wheel_x = x;
    state.wheel_y = y;
}

static void
on_gamepad_button_pressed (GrlWindow        *window,
                            gint              gamepad,
                            GrlGamepadButton  button,
                            gpointer          user_data)
{
    state.gamepad_index = gamepad;
    g_snprintf (state.gamepad_button_name, sizeof (state.gamepad_button_name),
                "GP%d: %s PRESSED", gamepad, get_gamepad_button_name (button));
}

static void
on_gamepad_button_released (GrlWindow        *window,
                             gint              gamepad,
                             GrlGamepadButton  button,
                             gpointer          user_data)
{
    g_snprintf (state.gamepad_button_name, sizeof (state.gamepad_button_name),
                "GP%d: %s RELEASED", gamepad, get_gamepad_button_name (button));
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
        state.left_stick_x = value;
    }
    else if (axis == GRL_GAMEPAD_AXIS_LEFT_Y)
    {
        state.left_stick_y = value;
    }
}

int
main (int   argc,
      char *argv[])
{
    g_autoptr(GrlWindow) window = NULL;
    g_autoptr(GrlColor) bg_color = NULL;
    g_autoptr(GrlColor) text_color = NULL;
    g_autoptr(GrlColor) highlight = NULL;
    g_autoptr(GrlColor) box_color = NULL;
    g_autoptr(GrlVector2) box_pos = NULL;
    gfloat box_x, box_y;
    gchar info_text[256];

    /* Create window */
    window = grl_window_new (800, 600, "Graylib Input Demo");
    grl_window_set_target_fps (window, 60);

    /* Create colors */
    bg_color = grl_color_new (30, 30, 40, 255);
    text_color = grl_color_new_white ();
    highlight = grl_color_new (100, 200, 100, 255);
    box_color = grl_color_new (200, 100, 100, 255);

    /* Connect signal handlers for event-based input */
    g_signal_connect (window, "key-pressed",
                      G_CALLBACK (on_key_pressed), NULL);
    g_signal_connect (window, "key-released",
                      G_CALLBACK (on_key_released), NULL);
    g_signal_connect (window, "mouse-button-pressed",
                      G_CALLBACK (on_mouse_button_pressed), NULL);
    g_signal_connect (window, "mouse-button-released",
                      G_CALLBACK (on_mouse_button_released), NULL);
    g_signal_connect (window, "mouse-moved",
                      G_CALLBACK (on_mouse_moved), NULL);
    g_signal_connect (window, "mouse-wheel",
                      G_CALLBACK (on_mouse_wheel), NULL);
    g_signal_connect (window, "gamepad-button-pressed",
                      G_CALLBACK (on_gamepad_button_pressed), NULL);
    g_signal_connect (window, "gamepad-button-released",
                      G_CALLBACK (on_gamepad_button_released), NULL);
    g_signal_connect (window, "gamepad-axis-moved",
                      G_CALLBACK (on_gamepad_axis_moved), NULL);

    /* Initialize box position */
    box_x = 400.0f;
    box_y = 400.0f;

    /* Initialize state */
    g_strlcpy (state.last_key_name, "(none)", sizeof (state.last_key_name));
    g_strlcpy (state.mouse_button_name, "(none)", sizeof (state.mouse_button_name));
    g_strlcpy (state.gamepad_button_name, "(none)", sizeof (state.gamepad_button_name));

    /* Main loop */
    while (!grl_window_should_close (window))
    {
        /*
         * Polling-based input for game logic
         * Use this for continuous movement (e.g., player control)
         */

        /* WASD or Arrow keys move the box */
        if (grl_input_is_key_down (GRL_KEY_W) || grl_input_is_key_down (GRL_KEY_UP))
        {
            box_y -= 5.0f;
        }
        if (grl_input_is_key_down (GRL_KEY_S) || grl_input_is_key_down (GRL_KEY_DOWN))
        {
            box_y += 5.0f;
        }
        if (grl_input_is_key_down (GRL_KEY_A) || grl_input_is_key_down (GRL_KEY_LEFT))
        {
            box_x -= 5.0f;
        }
        if (grl_input_is_key_down (GRL_KEY_D) || grl_input_is_key_down (GRL_KEY_RIGHT))
        {
            box_x += 5.0f;
        }

        /* Gamepad left stick also moves the box */
        box_x += state.left_stick_x * 5.0f;
        box_y += state.left_stick_y * 5.0f;

        /* Keep box within window bounds */
        if (box_x < 25.0f) box_x = 25.0f;
        if (box_x > 775.0f) box_x = 775.0f;
        if (box_y < 250.0f) box_y = 250.0f;
        if (box_y > 575.0f) box_y = 575.0f;

        /* Poll input events (required for signals to fire) */
        grl_window_poll_input (window);

        /* Drawing */
        grl_window_begin_drawing (window);
        grl_draw_clear_background (bg_color);

        /* Title */
        grl_draw_text ("GRAYLIB INPUT DEMO", 260, 20, 24, highlight);
        grl_draw_text ("Press keys, click mouse, or use gamepad", 200, 50, 16, text_color);

        /* Input state display */
        grl_draw_text ("--- SIGNAL-BASED INPUT (Events) ---", 20, 90, 18, highlight);

        g_snprintf (info_text, sizeof (info_text), "Last Key: %s", state.last_key_name);
        grl_draw_text (info_text, 40, 115, 16, text_color);

        g_snprintf (info_text, sizeof (info_text), "Mouse Button: %s", state.mouse_button_name);
        grl_draw_text (info_text, 40, 135, 16, text_color);

        g_snprintf (info_text, sizeof (info_text), "Mouse Position: (%d, %d) Delta: (%d, %d)",
                    state.mouse_x, state.mouse_y, state.mouse_delta_x, state.mouse_delta_y);
        grl_draw_text (info_text, 40, 155, 16, text_color);

        g_snprintf (info_text, sizeof (info_text), "Mouse Wheel: (%.1f, %.1f)",
                    state.wheel_x, state.wheel_y);
        grl_draw_text (info_text, 40, 175, 16, text_color);

        g_snprintf (info_text, sizeof (info_text), "Gamepad: %s", state.gamepad_button_name);
        grl_draw_text (info_text, 40, 195, 16, text_color);

        /* Polling-based input display */
        grl_draw_text ("--- POLLING-BASED INPUT (Continuous) ---", 400, 90, 18, highlight);

        g_snprintf (info_text, sizeof (info_text), "Left Stick: (%.2f, %.2f)",
                    state.left_stick_x, state.left_stick_y);
        grl_draw_text (info_text, 420, 115, 16, text_color);

        grl_draw_text ("Held Keys:", 420, 135, 16, text_color);
        if (grl_input_is_key_down (GRL_KEY_W)) grl_draw_text ("W", 520, 135, 16, highlight);
        if (grl_input_is_key_down (GRL_KEY_A)) grl_draw_text ("A", 540, 135, 16, highlight);
        if (grl_input_is_key_down (GRL_KEY_S)) grl_draw_text ("S", 560, 135, 16, highlight);
        if (grl_input_is_key_down (GRL_KEY_D)) grl_draw_text ("D", 580, 135, 16, highlight);
        if (grl_input_is_key_down (GRL_KEY_LEFT_SHIFT)) grl_draw_text ("SHIFT", 600, 135, 16, highlight);
        if (grl_input_is_key_down (GRL_KEY_SPACE)) grl_draw_text ("SPACE", 660, 135, 16, highlight);

        grl_draw_text ("Mouse Buttons:", 420, 155, 16, text_color);
        if (grl_input_is_mouse_button_down (GRL_MOUSE_BUTTON_LEFT)) grl_draw_text ("L", 560, 155, 16, highlight);
        if (grl_input_is_mouse_button_down (GRL_MOUSE_BUTTON_MIDDLE)) grl_draw_text ("M", 580, 155, 16, highlight);
        if (grl_input_is_mouse_button_down (GRL_MOUSE_BUTTON_RIGHT)) grl_draw_text ("R", 600, 155, 16, highlight);

        /* Movement instructions */
        grl_draw_text ("--- MOVEMENT DEMO ---", 20, 220, 18, highlight);
        grl_draw_text ("Use WASD/Arrows or Left Stick to move the box", 40, 245, 16, text_color);

        /* Draw movable box */
        box_pos = grl_vector2_new (box_x, box_y);
        grl_draw_circle_v (box_pos, 25.0f, box_color);

        grl_draw_fps (10, 10);

        grl_window_end_drawing (window);
    }

    return 0;
}
