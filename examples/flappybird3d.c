/* flappybird3d.c
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Flappy Bird clone in 3D demonstrating physics-based movement,
 * procedural obstacle generation, and collision detection.
 */

#include <graylib.h>
#include <stdlib.h>
#include <time.h>

/* Window dimensions */
#define WINDOW_WIDTH      1280
#define WINDOW_HEIGHT     900

/* Physics */
#define GRAVITY           25.0f
#define FLAP_VELOCITY     10.0f
#define BIRD_RADIUS       0.5f

/* World dimensions */
#define GROUND_Y          0.0f
#define CEILING_Y         15.0f
#define BIRD_X            0.0f

/* Pipes */
#define PIPE_WIDTH        2.0f
#define PIPE_DEPTH        4.0f
#define GAP_HEIGHT        5.0f
#define PIPE_SPACING      10.0f
#define PIPE_SPEED        6.0f
#define MAX_PIPES         5
#define SPAWN_X           25.0f
#define DESPAWN_X         -15.0f

/* Gap position constraints */
#define GAP_MIN_Y         (GROUND_Y + GAP_HEIGHT / 2.0f + 1.0f)
#define GAP_MAX_Y         (CEILING_Y - GAP_HEIGHT / 2.0f - 1.0f)

/* Camera position */
#define CAM_POS_X         -12.0f
#define CAM_POS_Y         10.0f
#define CAM_POS_Z         20.0f

/* Pipe obstacle */
typedef struct
{
    gfloat x;
    gfloat gap_y;
    gboolean passed;
    gboolean active;
} Pipe;

/* Game state */
typedef struct
{
    /* Bird */
    gfloat bird_y;
    gfloat bird_velocity;

    /* Pipes */
    Pipe pipes[MAX_PIPES];
    gfloat spawn_timer;

    /* Game state */
    gint score;
    gboolean paused;
    gboolean game_over;
    gboolean started;
} FlappyGame;

/*
 * Get random gap Y position within valid range.
 */
static gfloat
random_gap_y (void)
{
    gfloat range;

    range = GAP_MAX_Y - GAP_MIN_Y;
    return GAP_MIN_Y + ((gfloat)rand () / (gfloat)RAND_MAX) * range;
}

/*
 * Spawn a new pipe at the spawn position.
 */
static void
flappy_spawn_pipe (FlappyGame *game)
{
    gint i;

    /* Find inactive pipe slot */
    for (i = 0; i < MAX_PIPES; i++)
    {
        if (!game->pipes[i].active)
        {
            game->pipes[i].x = SPAWN_X;
            game->pipes[i].gap_y = random_gap_y ();
            game->pipes[i].passed = FALSE;
            game->pipes[i].active = TRUE;
            return;
        }
    }
}

/*
 * Initialize game state.
 */
static void
flappy_init (FlappyGame *game)
{
    gint i;

    srand ((unsigned int)time (NULL));

    /* Reset bird to center */
    game->bird_y = (GROUND_Y + CEILING_Y) / 2.0f;
    game->bird_velocity = 0.0f;

    /* Clear all pipes */
    for (i = 0; i < MAX_PIPES; i++)
    {
        game->pipes[i].active = FALSE;
    }

    game->spawn_timer = 0.0f;
    game->score = 0;
    game->paused = FALSE;
    game->game_over = FALSE;
    game->started = FALSE;
}

/*
 * Check collision between bird and a pipe.
 */
static gboolean
check_pipe_collision (FlappyGame *game,
                      Pipe       *pipe)
{
    gfloat bird_left;
    gfloat bird_right;
    gfloat bird_top;
    gfloat bird_bottom;
    gfloat pipe_left;
    gfloat pipe_right;
    gfloat gap_bottom;
    gfloat gap_top;

    bird_left = BIRD_X - BIRD_RADIUS;
    bird_right = BIRD_X + BIRD_RADIUS;
    bird_top = game->bird_y + BIRD_RADIUS;
    bird_bottom = game->bird_y - BIRD_RADIUS;

    pipe_left = pipe->x - PIPE_WIDTH / 2.0f;
    pipe_right = pipe->x + PIPE_WIDTH / 2.0f;

    /* Check horizontal overlap first */
    if (bird_right < pipe_left || bird_left > pipe_right)
        return FALSE;

    /* Check if bird is in the gap (safe) */
    gap_bottom = pipe->gap_y - GAP_HEIGHT / 2.0f;
    gap_top = pipe->gap_y + GAP_HEIGHT / 2.0f;

    if (bird_bottom >= gap_bottom && bird_top <= gap_top)
        return FALSE;

    /* Bird hit the pipe */
    return TRUE;
}

/*
 * Update game logic.
 */
static void
flappy_update (FlappyGame *game,
               gfloat      dt)
{
    gint i;

    if (game->paused || game->game_over)
        return;

    /* Wait for first flap to start */
    if (!game->started)
    {
        if (grl_input_is_key_pressed (GRL_KEY_SPACE) ||
            grl_input_is_key_pressed (GRL_KEY_W))
        {
            game->started = TRUE;
            game->bird_velocity = FLAP_VELOCITY;
        }
        return;
    }

    /* Handle flap input */
    if (grl_input_is_key_pressed (GRL_KEY_SPACE) ||
        grl_input_is_key_pressed (GRL_KEY_W))
    {
        game->bird_velocity = FLAP_VELOCITY;
    }

    /* Apply gravity */
    game->bird_velocity -= GRAVITY * dt;

    /* Update bird position */
    game->bird_y += game->bird_velocity * dt;

    /* Check ground/ceiling collision */
    if (game->bird_y - BIRD_RADIUS <= GROUND_Y)
    {
        game->bird_y = GROUND_Y + BIRD_RADIUS;
        game->game_over = TRUE;
        return;
    }
    if (game->bird_y + BIRD_RADIUS >= CEILING_Y)
    {
        game->bird_y = CEILING_Y - BIRD_RADIUS;
        game->game_over = TRUE;
        return;
    }

    /* Update pipes */
    for (i = 0; i < MAX_PIPES; i++)
    {
        if (!game->pipes[i].active)
            continue;

        /* Move pipe left */
        game->pipes[i].x -= PIPE_SPEED * dt;

        /* Check if bird passed pipe */
        if (!game->pipes[i].passed && game->pipes[i].x < BIRD_X)
        {
            game->pipes[i].passed = TRUE;
            game->score++;
        }

        /* Check collision */
        if (check_pipe_collision (game, &game->pipes[i]))
        {
            game->game_over = TRUE;
            return;
        }

        /* Despawn if off screen */
        if (game->pipes[i].x < DESPAWN_X)
        {
            game->pipes[i].active = FALSE;
        }
    }

    /* Spawn new pipes */
    game->spawn_timer += dt;
    if (game->spawn_timer >= PIPE_SPACING / PIPE_SPEED)
    {
        flappy_spawn_pipe (game);
        game->spawn_timer = 0.0f;
    }
}

/*
 * Draw 3D scene.
 */
static void
flappy_draw_3d (FlappyGame  *game,
                GrlCamera3D *camera)
{
    g_autoptr(GrlColor) ground_color = grl_color_new (80, 160, 80, 255);
    g_autoptr(GrlColor) bird_color = grl_color_new (255, 220, 50, 255);
    g_autoptr(GrlColor) pipe_color = grl_color_new (50, 180, 50, 255);
    g_autoptr(GrlColor) pipe_dark = grl_color_new (30, 120, 30, 255);
    g_autoptr(GrlColor) ceiling_color = grl_color_new (150, 200, 255, 200);
    g_autoptr(GrlVector3) pos = NULL;
    g_autoptr(GrlVector2) size = NULL;
    gint i;

    grl_camera3d_begin (camera);

    /* Ground plane */
    pos = grl_vector3_new (0.0f, GROUND_Y - 0.1f, 0.0f);
    size = grl_vector2_new (80.0f, 15.0f);
    grl_draw_plane (pos, size, ground_color);
    g_clear_pointer (&pos, grl_vector3_free);
    g_clear_pointer (&size, grl_vector2_free);

    /* Ceiling indicator (semi-transparent) */
    pos = grl_vector3_new (0.0f, CEILING_Y, 0.0f);
    size = grl_vector2_new (80.0f, 15.0f);
    grl_draw_plane (pos, size, ceiling_color);
    g_clear_pointer (&pos, grl_vector3_free);
    g_clear_pointer (&size, grl_vector2_free);

    /* Draw pipes */
    for (i = 0; i < MAX_PIPES; i++)
    {
        Pipe *pipe;
        gfloat gap_bottom;
        gfloat gap_top;
        gfloat lower_height;
        gfloat upper_height;

        if (!game->pipes[i].active)
            continue;

        pipe = &game->pipes[i];
        gap_bottom = pipe->gap_y - GAP_HEIGHT / 2.0f;
        gap_top = pipe->gap_y + GAP_HEIGHT / 2.0f;

        /* Lower pipe */
        lower_height = gap_bottom - GROUND_Y;
        if (lower_height > 0.0f)
        {
            pos = grl_vector3_new (pipe->x, GROUND_Y + lower_height / 2.0f, 0.0f);
            grl_draw_cube (pos, PIPE_WIDTH, lower_height, PIPE_DEPTH, pipe_color);
            grl_draw_cube_wires (pos, PIPE_WIDTH, lower_height, PIPE_DEPTH, pipe_dark);
            g_clear_pointer (&pos, grl_vector3_free);
        }

        /* Upper pipe */
        upper_height = CEILING_Y - gap_top;
        if (upper_height > 0.0f)
        {
            pos = grl_vector3_new (pipe->x, gap_top + upper_height / 2.0f, 0.0f);
            grl_draw_cube (pos, PIPE_WIDTH, upper_height, PIPE_DEPTH, pipe_color);
            grl_draw_cube_wires (pos, PIPE_WIDTH, upper_height, PIPE_DEPTH, pipe_dark);
            g_clear_pointer (&pos, grl_vector3_free);
        }
    }

    /* Bird (sphere) */
    pos = grl_vector3_new (BIRD_X, game->bird_y, 0.0f);
    grl_draw_sphere (pos, BIRD_RADIUS, bird_color);

    grl_camera3d_end (camera);
}

/*
 * Draw 2D UI overlay.
 */
static void
flappy_draw_ui (FlappyGame *game)
{
    g_autoptr(GrlColor) white = grl_color_new (255, 255, 255, 255);
    g_autoptr(GrlColor) gray = grl_color_new (150, 150, 150, 255);
    g_autoptr(GrlColor) yellow = grl_color_new (255, 220, 50, 255);
    g_autoptr(GrlColor) shadow = grl_color_new (0, 0, 0, 150);
    gchar text[64];
    gint text_width;

    /* Score display with shadow */
    g_snprintf (text, sizeof (text), "%d", game->score);
    text_width = grl_measure_text (text, 80);
    grl_draw_text (text, WINDOW_WIDTH / 2 - text_width / 2 + 3, 33, 80, shadow);
    grl_draw_text (text, WINDOW_WIDTH / 2 - text_width / 2, 30, 80, white);

    /* Start prompt */
    if (!game->started && !game->game_over)
    {
        grl_draw_text ("Press SPACE or W to start", WINDOW_WIDTH / 2 - 120, WINDOW_HEIGHT / 2, 20, white);
    }

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

        g_snprintf (text, sizeof (text), "Score: %d", game->score);
        text_width = grl_measure_text (text, 30);
        grl_draw_text (text, WINDOW_WIDTH / 2 - text_width / 2, WINDOW_HEIGHT / 2 - 20, 30, white);

        grl_draw_text ("Press SPACE to restart", WINDOW_WIDTH / 2 - 100, WINDOW_HEIGHT / 2 + 30, 16, gray);
    }

    /* Controls help */
    grl_draw_text ("SPACE/W: Flap", 20, WINDOW_HEIGHT - 50, 14, gray);
    grl_draw_text ("P: Pause", 20, WINDOW_HEIGHT - 30, 14, gray);
}

int
main (int    argc,
      char **argv)
{
    g_autoptr(GrlApplication) app = NULL;
    g_autoptr(GrlWindow) window = NULL;
    g_autoptr(GrlCamera3D) camera = NULL;
    g_autoptr(GrlColor) bg_color = NULL;
    FlappyGame game;
    gfloat dt;

    app = grl_application_new ("com.example.flappybird3d");
    window = grl_window_new (WINDOW_WIDTH, WINDOW_HEIGHT, "3D Flappy Bird");
    bg_color = grl_color_new (100, 180, 255, 255);

    /* Setup 3D camera - side view */
    camera = grl_camera3d_new ();
    grl_camera3d_set_position_xyz (camera, CAM_POS_X, CAM_POS_Y, CAM_POS_Z);
    grl_camera3d_set_target_xyz (camera, 5.0f, (CEILING_Y + GROUND_Y) / 2.0f, 0.0f);
    grl_camera3d_set_fovy (camera, 45.0f);

    grl_window_set_target_fps (window, 60);

    flappy_init (&game);

    while (!grl_window_should_close (window))
    {
        dt = grl_window_get_frame_time (window);

        /* Poll input */
        grl_window_poll_input (window);

        /* Pause toggle */
        if (grl_input_is_key_pressed (GRL_KEY_P))
        {
            if (!game.game_over && game.started)
            {
                game.paused = !game.paused;
            }
        }

        /* Restart on SPACE when game over */
        if (game.game_over && grl_input_is_key_pressed (GRL_KEY_SPACE))
        {
            flappy_init (&game);
        }

        /* Update */
        flappy_update (&game, dt);

        /* Draw */
        grl_window_begin_drawing (window);
        grl_draw_clear_background (bg_color);

        flappy_draw_3d (&game, camera);
        flappy_draw_ui (&game);

        grl_window_end_drawing (window);
    }

    return 0;
}
