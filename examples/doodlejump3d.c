/* doodlejump3d.c
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Doodle Jump clone in 3D demonstrating auto-bounce physics,
 * one-way platform collision, vertical scrolling camera,
 * and procedural platform generation.
 */

#include <graylib.h>
#include <stdlib.h>
#include <time.h>

/* Window dimensions */
#define WINDOW_WIDTH      1280
#define WINDOW_HEIGHT     900

/* Physics */
#define GRAVITY           30.0f
#define BOUNCE_VELOCITY   18.0f
#define MOVE_SPEED        15.0f
#define PLAYER_RADIUS     0.5f

/* World dimensions */
#define WORLD_WIDTH       16.0f
#define WORLD_HALF_WIDTH  8.0f

/* Platforms */
#define PLATFORM_WIDTH    2.5f
#define PLATFORM_HEIGHT   0.4f
#define PLATFORM_DEPTH    1.5f
#define MAX_PLATFORMS     25
#define PLATFORM_SPACING  2.5f
#define SPAWN_AHEAD       30.0f
#define DESPAWN_BEHIND    15.0f

/* Camera */
#define CAM_OFFSET_Y      3.0f
#define CAM_DISTANCE_Z    25.0f
#define DEATH_DISTANCE    12.0f

/* Platform */
typedef struct
{
    gfloat x;
    gfloat y;
    gboolean active;
} Platform;

/* Game state */
typedef struct
{
    /* Player */
    gfloat player_x;
    gfloat player_y;
    gfloat prev_player_y;  /* Previous Y for swept collision */
    gfloat velocity_y;

    /* Camera */
    gfloat camera_y;
    gfloat max_height;

    /* Platforms */
    Platform platforms[MAX_PLATFORMS];
    gfloat highest_platform_y;

    /* Game state */
    gint score;
    gboolean paused;
    gboolean game_over;
} DoodleGame;

/*
 * Spawn a platform at given Y with random X position.
 */
static void
doodle_spawn_platform (DoodleGame *game,
                       gfloat      y)
{
    gint i;
    gfloat x;
    gfloat range;

    /* Find inactive slot */
    for (i = 0; i < MAX_PLATFORMS; i++)
    {
        if (!game->platforms[i].active)
        {
            /* Random X position within bounds */
            range = WORLD_WIDTH - PLATFORM_WIDTH;
            x = ((gfloat)rand () / (gfloat)RAND_MAX) * range
                - WORLD_HALF_WIDTH + PLATFORM_WIDTH / 2.0f;

            game->platforms[i].x = x;
            game->platforms[i].y = y;
            game->platforms[i].active = TRUE;

            if (y > game->highest_platform_y)
                game->highest_platform_y = y;

            return;
        }
    }
}

/*
 * Spawn initial platforms to fill the screen.
 */
static void
doodle_spawn_initial_platforms (DoodleGame *game)
{
    gfloat y;

    /* Starting platform directly under player */
    game->platforms[0].x = 0.0f;
    game->platforms[0].y = 0.0f;
    game->platforms[0].active = TRUE;
    game->highest_platform_y = 0.0f;

    /* Fill screen with platforms */
    for (y = PLATFORM_SPACING; y < SPAWN_AHEAD; y += PLATFORM_SPACING)
    {
        doodle_spawn_platform (game, y);
    }
}

/*
 * Initialize game state.
 */
static void
doodle_init (DoodleGame *game)
{
    gint i;

    srand ((unsigned int)time (NULL));

    /* Reset player */
    game->player_x = 0.0f;
    game->player_y = PLAYER_RADIUS + PLATFORM_HEIGHT / 2.0f;
    game->prev_player_y = game->player_y;
    game->velocity_y = BOUNCE_VELOCITY;

    /* Reset camera */
    game->camera_y = CAM_OFFSET_Y;
    game->max_height = 0.0f;

    /* Clear all platforms */
    for (i = 0; i < MAX_PLATFORMS; i++)
    {
        game->platforms[i].active = FALSE;
    }
    game->highest_platform_y = 0.0f;

    /* Spawn initial platforms */
    doodle_spawn_initial_platforms (game);

    /* Reset state */
    game->score = 0;
    game->paused = FALSE;
    game->game_over = FALSE;
}

/*
 * Check one-way platform collision using swept detection.
 * This prevents tunneling through platforms at high velocities.
 */
static void
doodle_check_platform_collision (DoodleGame *game)
{
    gint i;
    Platform *plat;
    gfloat player_bottom;
    gfloat prev_bottom;
    gfloat player_left;
    gfloat player_right;
    gfloat plat_top;
    gfloat plat_left;
    gfloat plat_right;

    /* Only check when falling */
    if (game->velocity_y >= 0)
        return;

    player_bottom = game->player_y - PLAYER_RADIUS;
    prev_bottom = game->prev_player_y - PLAYER_RADIUS;
    player_left = game->player_x - PLAYER_RADIUS;
    player_right = game->player_x + PLAYER_RADIUS;

    for (i = 0; i < MAX_PLATFORMS; i++)
    {
        if (!game->platforms[i].active)
            continue;

        plat = &game->platforms[i];
        plat_top = plat->y + PLATFORM_HEIGHT / 2.0f;
        plat_left = plat->x - PLATFORM_WIDTH / 2.0f;
        plat_right = plat->x + PLATFORM_WIDTH / 2.0f;

        /* Check horizontal overlap */
        if (player_right < plat_left || player_left > plat_right)
            continue;

        /* Swept collision: was above platform last frame, now at or below? */
        if (prev_bottom >= plat_top && player_bottom <= plat_top)
        {
            /* Bounce! */
            game->player_y = plat_top + PLAYER_RADIUS;
            game->velocity_y = BOUNCE_VELOCITY;
            return;
        }
    }
}

/*
 * Update game logic.
 */
static void
doodle_update (DoodleGame *game,
               gfloat      dt)
{
    gint i;
    gfloat target_cam_y;

    if (game->paused || game->game_over)
        return;

    /* Save previous position for swept collision detection */
    game->prev_player_y = game->player_y;

    /* Handle horizontal input */
    if (grl_input_is_key_down (GRL_KEY_A) ||
        grl_input_is_key_down (GRL_KEY_LEFT))
    {
        game->player_x -= MOVE_SPEED * dt;
    }
    else if (grl_input_is_key_down (GRL_KEY_D) ||
             grl_input_is_key_down (GRL_KEY_RIGHT))
    {
        game->player_x += MOVE_SPEED * dt;
    }

    /* Wrap around screen edges */
    if (game->player_x < -WORLD_HALF_WIDTH - PLAYER_RADIUS)
        game->player_x = WORLD_HALF_WIDTH + PLAYER_RADIUS;
    else if (game->player_x > WORLD_HALF_WIDTH + PLAYER_RADIUS)
        game->player_x = -WORLD_HALF_WIDTH - PLAYER_RADIUS;

    /* Apply gravity */
    game->velocity_y -= GRAVITY * dt;

    /* Update player position */
    game->player_y += game->velocity_y * dt;

    /* Check platform collision */
    doodle_check_platform_collision (game);

    /* Floor collision - bounce off bottom */
    if (game->player_y - PLAYER_RADIUS <= 0.0f)
    {
        game->player_y = PLAYER_RADIUS;
        game->velocity_y = BOUNCE_VELOCITY;
    }

    /* Update camera (only moves up, never down) */
    target_cam_y = game->player_y + CAM_OFFSET_Y;
    if (target_cam_y > game->camera_y)
    {
        game->camera_y = target_cam_y;
    }

    /* Update max height and score */
    if (game->player_y > game->max_height)
    {
        game->max_height = game->player_y;
        game->score = (gint)(game->max_height * 10.0f);
    }

    /* Check death (fell below camera view) */
    if (game->player_y < game->camera_y - DEATH_DISTANCE)
    {
        game->game_over = TRUE;
        return;
    }

    /* Spawn platforms above viewport */
    while (game->highest_platform_y < game->camera_y + SPAWN_AHEAD)
    {
        /* Add some randomness to spacing */
        gfloat spacing = PLATFORM_SPACING + ((gfloat)rand () / (gfloat)RAND_MAX) * 1.0f;
        doodle_spawn_platform (game, game->highest_platform_y + spacing);
    }

    /* Despawn platforms below viewport */
    for (i = 0; i < MAX_PLATFORMS; i++)
    {
        if (game->platforms[i].active &&
            game->platforms[i].y < game->camera_y - DESPAWN_BEHIND)
        {
            game->platforms[i].active = FALSE;
        }
    }
}

/*
 * Draw 3D scene.
 */
static void
doodle_draw_3d (DoodleGame  *game,
                GrlCamera3D *camera)
{
    g_autoptr(GrlColor) platform_color = grl_color_new (139, 90, 43, 255);
    g_autoptr(GrlColor) platform_dark = grl_color_new (100, 65, 30, 255);
    g_autoptr(GrlColor) player_color = grl_color_new (80, 200, 80, 255);
    g_autoptr(GrlColor) player_dark = grl_color_new (50, 150, 50, 255);
    g_autoptr(GrlColor) wall_color = grl_color_new (200, 190, 170, 100);
    g_autoptr(GrlVector3) pos = NULL;
    gint i;

    /* Update camera position */
    grl_camera3d_set_position_xyz (camera, 0.0f, game->camera_y, CAM_DISTANCE_Z);
    grl_camera3d_set_target_xyz (camera, 0.0f, game->camera_y, 0.0f);

    grl_camera3d_begin (camera);

    /* Draw boundary walls (visual guides) */
    pos = grl_vector3_new (-WORLD_HALF_WIDTH - 0.2f, game->camera_y, 0.0f);
    grl_draw_cube (pos, 0.3f, 30.0f, 2.0f, wall_color);
    g_clear_pointer (&pos, grl_vector3_free);

    pos = grl_vector3_new (WORLD_HALF_WIDTH + 0.2f, game->camera_y, 0.0f);
    grl_draw_cube (pos, 0.3f, 30.0f, 2.0f, wall_color);
    g_clear_pointer (&pos, grl_vector3_free);

    /* Draw platforms */
    for (i = 0; i < MAX_PLATFORMS; i++)
    {
        Platform *plat;

        if (!game->platforms[i].active)
            continue;

        plat = &game->platforms[i];
        pos = grl_vector3_new (plat->x, plat->y, 0.0f);
        grl_draw_cube (pos, PLATFORM_WIDTH, PLATFORM_HEIGHT, PLATFORM_DEPTH, platform_color);
        grl_draw_cube_wires (pos, PLATFORM_WIDTH, PLATFORM_HEIGHT, PLATFORM_DEPTH, platform_dark);
        g_clear_pointer (&pos, grl_vector3_free);
    }

    /* Draw player (sphere) */
    pos = grl_vector3_new (game->player_x, game->player_y, 0.0f);
    grl_draw_sphere (pos, PLAYER_RADIUS, player_color);
    grl_draw_sphere_wires (pos, PLAYER_RADIUS, 8, 8, player_dark);

    grl_camera3d_end (camera);
}

/*
 * Draw 2D UI overlay.
 */
static void
doodle_draw_ui (DoodleGame *game)
{
    g_autoptr(GrlColor) white = grl_color_new (255, 255, 255, 255);
    g_autoptr(GrlColor) gray = grl_color_new (100, 100, 100, 255);
    g_autoptr(GrlColor) dark = grl_color_new (60, 50, 40, 255);
    g_autoptr(GrlColor) yellow = grl_color_new (255, 200, 50, 255);
    g_autoptr(GrlColor) shadow = grl_color_new (0, 0, 0, 100);
    gchar text[64];
    gint text_width;

    /* Score display with shadow */
    g_snprintf (text, sizeof (text), "Score: %d", game->score);
    grl_draw_text (text, 22, 22, 30, shadow);
    grl_draw_text (text, 20, 20, 30, dark);

    /* Height display */
    g_snprintf (text, sizeof (text), "Height: %.0f", game->max_height);
    grl_draw_text (text, 20, 55, 20, gray);

    /* Controls help */
    grl_draw_text ("A/D: Move", 20, WINDOW_HEIGHT - 50, 14, gray);
    grl_draw_text ("P: Pause", 20, WINDOW_HEIGHT - 30, 14, gray);

    /* Pause indicator */
    if (game->paused && !game->game_over)
    {
        text_width = grl_measure_text ("PAUSED", 40);
        grl_draw_text ("PAUSED", WINDOW_WIDTH / 2 - text_width / 2, WINDOW_HEIGHT / 2 - 50, 40, yellow);
        grl_draw_text ("Press P to resume", WINDOW_WIDTH / 2 - 80, WINDOW_HEIGHT / 2, 16, gray);
    }

    /* Game over */
    if (game->game_over)
    {
        text_width = grl_measure_text ("GAME OVER", 50);
        grl_draw_text ("GAME OVER", WINDOW_WIDTH / 2 - text_width / 2, WINDOW_HEIGHT / 2 - 80, 50, yellow);

        g_snprintf (text, sizeof (text), "Final Score: %d", game->score);
        text_width = grl_measure_text (text, 30);
        grl_draw_text (text, WINDOW_WIDTH / 2 - text_width / 2, WINDOW_HEIGHT / 2 - 20, 30, dark);

        g_snprintf (text, sizeof (text), "Max Height: %.0f", game->max_height);
        text_width = grl_measure_text (text, 20);
        grl_draw_text (text, WINDOW_WIDTH / 2 - text_width / 2, WINDOW_HEIGHT / 2 + 20, 20, gray);

        grl_draw_text ("Press SPACE to restart", WINDOW_WIDTH / 2 - 100, WINDOW_HEIGHT / 2 + 60, 16, gray);
    }
}

int
main (int    argc,
      char **argv)
{
    g_autoptr(GrlApplication) app = NULL;
    g_autoptr(GrlWindow) window = NULL;
    g_autoptr(GrlCamera3D) camera = NULL;
    g_autoptr(GrlColor) bg_color = NULL;
    DoodleGame game;
    gfloat dt;

    app = grl_application_new ("com.example.doodlejump3d");
    window = grl_window_new (WINDOW_WIDTH, WINDOW_HEIGHT, "3D Doodle Jump");
    bg_color = grl_color_new (230, 220, 190, 255);

    /* Setup 3D camera */
    camera = grl_camera3d_new ();
    grl_camera3d_set_position_xyz (camera, 0.0f, CAM_OFFSET_Y, CAM_DISTANCE_Z);
    grl_camera3d_set_target_xyz (camera, 0.0f, CAM_OFFSET_Y, 0.0f);
    grl_camera3d_set_fovy (camera, 45.0f);

    grl_window_set_target_fps (window, 60);

    doodle_init (&game);

    while (!grl_window_should_close (window))
    {
        dt = grl_window_get_frame_time (window);

        /* Poll input */
        grl_window_poll_input (window);

        /* Pause toggle */
        if (grl_input_is_key_pressed (GRL_KEY_P))
        {
            if (!game.game_over)
            {
                game.paused = !game.paused;
            }
        }

        /* Restart on SPACE when game over */
        if (game.game_over && grl_input_is_key_pressed (GRL_KEY_SPACE))
        {
            doodle_init (&game);
        }

        /* Update */
        doodle_update (&game, dt);

        /* Draw */
        grl_window_begin_drawing (window);
        grl_draw_clear_background (bg_color);

        doodle_draw_3d (&game, camera);
        doodle_draw_ui (&game);

        grl_window_end_drawing (window);
    }

    return 0;
}
