/* scene-demo.c
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Demonstrates the scene management system with multiple scenes
 * and transitions between them.
 */

#include <graylib.h>
#include <stdio.h>

/*
 * This demo shows:
 * - Creating multiple scenes
 * - Pushing/popping scenes on a stack
 * - Scene lifecycle (enter, exit, pause, resume)
 * - Adding entities to scenes
 * - Scene-specific update and draw logic
 */

/* Forward declarations */
static void setup_menu_scene (GrlScene *scene);
static void setup_game_scene (GrlScene *scene);
static void setup_pause_scene (GrlScene *scene);

/* Global scene manager and scenes */
static GrlSceneManager *manager = NULL;
static GrlScene *menu_scene = NULL;
static GrlScene *game_scene = NULL;
static GrlScene *pause_scene = NULL;

/* Game state */
static gfloat player_x = 400.0f;
static gfloat player_y = 300.0f;
static gfloat player_speed = 200.0f;
static gint score = 0;

/*
 * Menu Scene Callbacks
 */

static void
on_menu_entered (GrlScene *scene,
                 gpointer  user_data)
{
    g_print ("Menu scene entered\n");
    score = 0;
    player_x = 400.0f;
    player_y = 300.0f;
}

static void
on_menu_exited (GrlScene *scene,
                gpointer  user_data)
{
    g_print ("Menu scene exited\n");
}

/*
 * Game Scene Callbacks
 */

static void
on_game_entered (GrlScene *scene,
                 gpointer  user_data)
{
    g_print ("Game scene entered\n");
}

static void
on_game_exited (GrlScene *scene,
                gpointer  user_data)
{
    g_print ("Game scene exited\n");
}

static void
on_game_paused (GrlScene *scene,
                gpointer  user_data)
{
    g_print ("Game scene paused\n");
}

static void
on_game_resumed (GrlScene *scene,
                 gpointer  user_data)
{
    g_print ("Game scene resumed\n");
}

/*
 * Pause Scene Callbacks
 */

static void
on_pause_entered (GrlScene *scene,
                  gpointer  user_data)
{
    g_print ("Pause scene entered\n");
}

static void
on_pause_exited (GrlScene *scene,
                 gpointer  user_data)
{
    g_print ("Pause scene exited\n");
}

/*
 * Scene Manager Callbacks
 */

static void
on_scene_changed (GrlSceneManager *mgr,
                  GrlScene        *old_scene,
                  GrlScene        *new_scene,
                  gpointer         user_data)
{
    const gchar *old_name = old_scene ? grl_scene_get_name (old_scene) : "(none)";
    const gchar *new_name = new_scene ? grl_scene_get_name (new_scene) : "(none)";
    g_print ("Scene changed: %s -> %s\n", old_name, new_name);
}

/*
 * Custom update/draw for menu scene
 */

static void
menu_update (GrlScene *scene,
             gfloat    delta)
{
    /* Check for scene transition */
    if (grl_input_is_key_pressed (GRL_KEY_ENTER) ||
        grl_input_is_key_pressed (GRL_KEY_SPACE))
    {
        grl_scene_manager_push (manager, game_scene);
    }
}

static void
menu_draw (GrlScene *scene)
{
    g_autoptr(GrlColor) bg = grl_color_new (40, 40, 80, 255);
    g_autoptr(GrlColor) white = grl_color_new_white ();
    g_autoptr(GrlColor) yellow = grl_color_new_yellow ();

    grl_draw_clear_background (bg);

    grl_draw_text ("SCENE DEMO", 280, 150, 40, white);
    grl_draw_text ("A demonstration of Graylib's scene management", 180, 220, 20, white);

    grl_draw_text ("Press ENTER or SPACE to start", 270, 350, 20, yellow);
    grl_draw_text ("Press ESC to quit", 330, 400, 20, white);
}

/*
 * Custom update/draw for game scene
 */

static void
game_update (GrlScene *scene,
             gfloat    delta)
{
    /* Player movement */
    if (grl_input_is_key_down (GRL_KEY_RIGHT) ||
        grl_input_is_key_down (GRL_KEY_D))
        player_x += player_speed * delta;
    if (grl_input_is_key_down (GRL_KEY_LEFT) ||
        grl_input_is_key_down (GRL_KEY_A))
        player_x -= player_speed * delta;
    if (grl_input_is_key_down (GRL_KEY_DOWN) ||
        grl_input_is_key_down (GRL_KEY_S))
        player_y += player_speed * delta;
    if (grl_input_is_key_down (GRL_KEY_UP) ||
        grl_input_is_key_down (GRL_KEY_W))
        player_y -= player_speed * delta;

    /* Keep player in bounds */
    if (player_x < 25) player_x = 25;
    if (player_x > 775) player_x = 775;
    if (player_y < 25) player_y = 25;
    if (player_y > 575) player_y = 575;

    /* Score increases over time */
    score += (gint)(delta * 10);

    /* Check for pause */
    if (grl_input_is_key_pressed (GRL_KEY_ESCAPE) ||
        grl_input_is_key_pressed (GRL_KEY_P))
    {
        grl_scene_manager_push (manager, pause_scene);
    }

    /* Back to menu with M key */
    if (grl_input_is_key_pressed (GRL_KEY_M))
    {
        grl_scene_manager_pop_to_root (manager);
    }
}

static void
game_draw (GrlScene *scene)
{
    g_autoptr(GrlColor) bg = grl_color_new (30, 50, 30, 255);
    g_autoptr(GrlColor) white = grl_color_new_white ();
    g_autoptr(GrlColor) green = grl_color_new_green ();
    g_autoptr(GrlColor) yellow = grl_color_new_yellow ();
    g_autoptr(GrlVector2) player_pos = grl_vector2_new (player_x, player_y);
    gchar score_text[64];

    grl_draw_clear_background (bg);

    /* Draw player */
    grl_draw_circle_v (player_pos, 25.0f, green);

    /* Draw score */
    g_snprintf (score_text, sizeof (score_text), "Score: %d", score);
    grl_draw_text (score_text, 10, 10, 20, white);

    /* Draw controls hint */
    grl_draw_text ("WASD/Arrows: Move | P/ESC: Pause | M: Menu", 200, 570, 16, yellow);
}

/*
 * Custom update/draw for pause scene
 */

static void
pause_update (GrlScene *scene,
              gfloat    delta)
{
    /* Resume game */
    if (grl_input_is_key_pressed (GRL_KEY_ESCAPE) ||
        grl_input_is_key_pressed (GRL_KEY_P) ||
        grl_input_is_key_pressed (GRL_KEY_SPACE))
    {
        g_autoptr(GrlScene) popped = grl_scene_manager_pop (manager);
        /* popped is the pause scene, now game is on top */
    }

    /* Back to menu */
    if (grl_input_is_key_pressed (GRL_KEY_M))
    {
        grl_scene_manager_pop_to_root (manager);
    }

    /* Quit */
    if (grl_input_is_key_pressed (GRL_KEY_Q))
    {
        grl_scene_manager_clear (manager);
    }
}

static void
pause_draw (GrlScene *scene)
{
    g_autoptr(GrlColor) overlay = grl_color_new (0, 0, 0, 180);
    g_autoptr(GrlColor) white = grl_color_new_white ();
    g_autoptr(GrlColor) yellow = grl_color_new_yellow ();
    g_autoptr(GrlRectangle) screen_rect = grl_rectangle_new (0, 0, 800, 600);

    /* Draw semi-transparent overlay */
    grl_draw_rectangle_rec (screen_rect, overlay);

    /* Draw pause menu */
    grl_draw_text ("PAUSED", 340, 200, 40, white);

    grl_draw_text ("Press P/ESC/SPACE to resume", 270, 300, 20, yellow);
    grl_draw_text ("Press M to return to menu", 285, 340, 20, white);
    grl_draw_text ("Press Q to quit", 325, 380, 20, white);
}

/*
 * Scene Setup Functions
 */

static void
setup_menu_scene (GrlScene *scene)
{
    grl_scene_set_name (scene, "menu");

    /* Connect signals */
    g_signal_connect (scene, "entered", G_CALLBACK (on_menu_entered), NULL);
    g_signal_connect (scene, "exited", G_CALLBACK (on_menu_exited), NULL);
}

static void
setup_game_scene (GrlScene *scene)
{
    grl_scene_set_name (scene, "game");

    /* Connect signals */
    g_signal_connect (scene, "entered", G_CALLBACK (on_game_entered), NULL);
    g_signal_connect (scene, "exited", G_CALLBACK (on_game_exited), NULL);
    g_signal_connect (scene, "paused", G_CALLBACK (on_game_paused), NULL);
    g_signal_connect (scene, "resumed", G_CALLBACK (on_game_resumed), NULL);
}

static void
setup_pause_scene (GrlScene *scene)
{
    grl_scene_set_name (scene, "pause");

    /* Pause scene draws over the game scene */
    grl_scene_set_draw_when_paused (scene, TRUE);

    /* Connect signals */
    g_signal_connect (scene, "entered", G_CALLBACK (on_pause_entered), NULL);
    g_signal_connect (scene, "exited", G_CALLBACK (on_pause_exited), NULL);
}

/*
 * Custom scene update/draw routing
 * (In a real app, you'd subclass GrlScene and override virtual methods)
 */

static void
update_current_scene (gfloat delta)
{
    GrlScene *current = grl_scene_manager_get_current (manager);

    if (current == menu_scene)
        menu_update (current, delta);
    else if (current == game_scene)
        game_update (current, delta);
    else if (current == pause_scene)
        pause_update (current, delta);
}

static void
draw_current_scene (void)
{
    GrlScene *current = grl_scene_manager_get_current (manager);

    /* For pause scene, draw game scene first (background) */
    if (current == pause_scene)
    {
        game_draw (game_scene);
    }

    if (current == menu_scene)
        menu_draw (current);
    else if (current == game_scene)
        game_draw (current);
    else if (current == pause_scene)
        pause_draw (current);
}

int
main (int   argc,
      char *argv[])
{
    g_autoptr(GrlApplication) app = NULL;
    g_autoptr(GrlWindow) window = NULL;

    /* Initialize */
    app = grl_application_new ("com.example.SceneDemo");
    window = grl_window_new (800, 600, "Scene Management Demo");
    grl_window_set_target_fps (window, 60);

    /* Create scene manager */
    manager = grl_scene_manager_new ();
    g_signal_connect (manager, "scene-changed", G_CALLBACK (on_scene_changed), NULL);

    /* Create scenes */
    menu_scene = grl_scene_new ();
    game_scene = grl_scene_new ();
    pause_scene = grl_scene_new ();

    setup_menu_scene (menu_scene);
    setup_game_scene (game_scene);
    setup_pause_scene (pause_scene);

    /* Start with menu scene */
    grl_scene_manager_push (manager, menu_scene);

    /* Print controls */
    g_print ("\n=== Scene Management Demo ===\n");
    g_print ("This demo shows scene transitions:\n");
    g_print ("- Menu -> Game (ENTER/SPACE)\n");
    g_print ("- Game -> Pause (P/ESC)\n");
    g_print ("- Pause -> Game (P/ESC/SPACE)\n");
    g_print ("- Any -> Menu (M)\n");
    g_print ("- Quit (Q from pause, or close window)\n");
    g_print ("=============================\n\n");

    /* Main loop */
    while (!grl_window_should_close (window) && !grl_scene_manager_is_empty (manager))
    {
        gfloat delta = grl_window_get_frame_time (window);

        /* Update current scene */
        update_current_scene (delta);

        /* Draw */
        grl_window_begin_drawing (window);
        draw_current_scene ();
        grl_window_end_drawing (window);
    }

    /* Cleanup */
    grl_scene_manager_clear (manager);
    g_object_unref (manager);
    g_object_unref (menu_scene);
    g_object_unref (game_scene);
    g_object_unref (pause_scene);

    g_print ("Demo finished!\n");

    return 0;
}
