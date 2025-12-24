/* pong3d.c
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * 3D Pong game demonstrating 3D camera, primitive drawing, and 3D collision.
 */

#include <graylib.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

/* Window dimensions */
#define WINDOW_WIDTH      1280
#define WINDOW_HEIGHT     900

/* Field dimensions (XZ plane) */
#define FIELD_WIDTH       28.0f
#define FIELD_LENGTH      40.0f
#define WALL_HEIGHT       0.6f
#define WALL_THICKNESS    0.4f

/* Paddle dimensions */
#define PADDLE_WIDTH      1.0f
#define PADDLE_HEIGHT     2.0f
#define PADDLE_DEPTH      5.0f
#define PADDLE_X_OFFSET   12.0f
#define PADDLE_SPEED      16.0f

/* Ball */
#define BALL_SIZE         0.8f
#define BALL_SPEED        18.0f
#define BALL_Y            (BALL_SIZE / 2.0f + 0.01f)

/* Camera position */
#define CAM_POS_Y         22.0f
#define CAM_POS_Z         32.0f

/* Scoring */
#define WINNING_SCORE     10

/* Game state */
typedef struct
{
    /* Ball position and velocity */
    gfloat ball_x;
    gfloat ball_z;
    gfloat ball_vx;
    gfloat ball_vz;

    /* Paddle Z positions (X is fixed at +/- PADDLE_X_OFFSET) */
    gfloat paddle1_z;
    gfloat paddle2_z;

    /* Scores */
    gint score1;
    gint score2;

    /* State */
    gboolean paused;
    gboolean game_over;
    gint winner;
} Pong3DGame;

/*
 * Reset ball to center with random direction.
 */
static void
pong3d_reset_ball (Pong3DGame *game,
                   gint        direction)
{
    gfloat angle;

    game->ball_x = 0.0f;
    game->ball_z = 0.0f;

    /* Random angle between -45 and 45 degrees */
    angle = ((gfloat)(rand () % 90) - 45.0f) * (G_PI / 180.0f);

    game->ball_vx = BALL_SPEED * cosf (angle) * direction;
    game->ball_vz = BALL_SPEED * sinf (angle);
}

/*
 * Initialize game state.
 */
static void
pong3d_init (Pong3DGame *game)
{
    srand ((unsigned int)time (NULL));

    game->paddle1_z = 0.0f;
    game->paddle2_z = 0.0f;

    game->score1 = 0;
    game->score2 = 0;

    game->paused = FALSE;
    game->game_over = FALSE;
    game->winner = 0;

    pong3d_reset_ball (game, (rand () % 2 == 0) ? 1 : -1);
}

/*
 * Update game logic.
 */
static void
pong3d_update (Pong3DGame *game,
               gfloat      dt)
{
    gfloat z_limit;
    gfloat paddle1_x;
    gfloat paddle2_x;
    gfloat hit_pos;
    gfloat normalized;
    gfloat bounce_angle;
    gfloat speed;

    if (game->paused || game->game_over)
        return;

    /* Player 1 input (W/S) - paddle on left (-X) */
    if (grl_input_is_key_down (GRL_KEY_W))
        game->paddle1_z -= PADDLE_SPEED * dt;
    if (grl_input_is_key_down (GRL_KEY_S))
        game->paddle1_z += PADDLE_SPEED * dt;

    /* Player 2 input (Up/Down) - paddle on right (+X) */
    if (grl_input_is_key_down (GRL_KEY_UP))
        game->paddle2_z -= PADDLE_SPEED * dt;
    if (grl_input_is_key_down (GRL_KEY_DOWN))
        game->paddle2_z += PADDLE_SPEED * dt;

    /* Clamp paddles to field bounds */
    z_limit = (FIELD_LENGTH / 2.0f) - (PADDLE_DEPTH / 2.0f);
    if (game->paddle1_z < -z_limit)
        game->paddle1_z = -z_limit;
    if (game->paddle1_z > z_limit)
        game->paddle1_z = z_limit;
    if (game->paddle2_z < -z_limit)
        game->paddle2_z = -z_limit;
    if (game->paddle2_z > z_limit)
        game->paddle2_z = z_limit;

    /* Update ball position */
    game->ball_x += game->ball_vx * dt;
    game->ball_z += game->ball_vz * dt;

    /* Ball collision with Z walls (front/back of field) */
    if (game->ball_z - BALL_SIZE / 2.0f <= -FIELD_LENGTH / 2.0f)
    {
        game->ball_z = -FIELD_LENGTH / 2.0f + BALL_SIZE / 2.0f;
        game->ball_vz = -game->ball_vz;
    }
    if (game->ball_z + BALL_SIZE / 2.0f >= FIELD_LENGTH / 2.0f)
    {
        game->ball_z = FIELD_LENGTH / 2.0f - BALL_SIZE / 2.0f;
        game->ball_vz = -game->ball_vz;
    }

    /* Paddle X positions */
    paddle1_x = -PADDLE_X_OFFSET;
    paddle2_x = PADDLE_X_OFFSET;

    /* Ball collision with paddle 1 (left, -X) */
    if (game->ball_vx < 0 &&
        game->ball_x - BALL_SIZE / 2.0f <= paddle1_x + PADDLE_WIDTH / 2.0f &&
        game->ball_x + BALL_SIZE / 2.0f >= paddle1_x - PADDLE_WIDTH / 2.0f &&
        game->ball_z >= game->paddle1_z - PADDLE_DEPTH / 2.0f &&
        game->ball_z <= game->paddle1_z + PADDLE_DEPTH / 2.0f)
    {
        game->ball_x = paddle1_x + PADDLE_WIDTH / 2.0f + BALL_SIZE / 2.0f;

        /* Adjust angle based on hit position */
        hit_pos = game->ball_z - game->paddle1_z;
        normalized = hit_pos / (PADDLE_DEPTH / 2.0f);
        bounce_angle = normalized * (G_PI / 4.0f);

        speed = sqrtf (game->ball_vx * game->ball_vx + game->ball_vz * game->ball_vz);
        game->ball_vx = speed * cosf (bounce_angle);
        game->ball_vz = speed * sinf (bounce_angle);
    }

    /* Ball collision with paddle 2 (right, +X) */
    if (game->ball_vx > 0 &&
        game->ball_x + BALL_SIZE / 2.0f >= paddle2_x - PADDLE_WIDTH / 2.0f &&
        game->ball_x - BALL_SIZE / 2.0f <= paddle2_x + PADDLE_WIDTH / 2.0f &&
        game->ball_z >= game->paddle2_z - PADDLE_DEPTH / 2.0f &&
        game->ball_z <= game->paddle2_z + PADDLE_DEPTH / 2.0f)
    {
        game->ball_x = paddle2_x - PADDLE_WIDTH / 2.0f - BALL_SIZE / 2.0f;

        /* Adjust angle based on hit position */
        hit_pos = game->ball_z - game->paddle2_z;
        normalized = hit_pos / (PADDLE_DEPTH / 2.0f);
        bounce_angle = normalized * (G_PI / 4.0f);

        speed = sqrtf (game->ball_vx * game->ball_vx + game->ball_vz * game->ball_vz);
        game->ball_vx = -speed * cosf (bounce_angle);
        game->ball_vz = speed * sinf (bounce_angle);
    }

    /* Scoring - ball passed paddle */
    if (game->ball_x < -FIELD_WIDTH / 2.0f)
    {
        game->score2++;
        if (game->score2 >= WINNING_SCORE)
        {
            game->game_over = TRUE;
            game->winner = 2;
        }
        else
        {
            pong3d_reset_ball (game, -1);
        }
    }
    if (game->ball_x > FIELD_WIDTH / 2.0f)
    {
        game->score1++;
        if (game->score1 >= WINNING_SCORE)
        {
            game->game_over = TRUE;
            game->winner = 1;
        }
        else
        {
            pong3d_reset_ball (game, 1);
        }
    }
}

/*
 * Draw 3D scene.
 */
static void
pong3d_draw_3d (Pong3DGame  *game,
                GrlCamera3D *camera)
{
    g_autoptr(GrlColor) field_color = grl_color_new (40, 45, 60, 255);
    g_autoptr(GrlColor) wall_color = grl_color_new (80, 80, 100, 255);
    g_autoptr(GrlColor) line_color = grl_color_new (100, 100, 120, 255);
    g_autoptr(GrlColor) paddle1_color = grl_color_new (50, 120, 220, 255);
    g_autoptr(GrlColor) paddle2_color = grl_color_new (220, 80, 80, 255);
    g_autoptr(GrlColor) ball_color = grl_color_new (255, 220, 50, 255);
    g_autoptr(GrlColor) white = grl_color_new (255, 255, 255, 255);

    g_autoptr(GrlVector3) origin = grl_vector3_new (0.0f, 0.0f, 0.0f);
    g_autoptr(GrlVector2) field_size = grl_vector2_new (FIELD_WIDTH, FIELD_LENGTH);

    g_autoptr(GrlVector3) wall1_pos = grl_vector3_new (0.0f, WALL_HEIGHT / 2.0f, -FIELD_LENGTH / 2.0f - WALL_THICKNESS / 2.0f);
    g_autoptr(GrlVector3) wall2_pos = grl_vector3_new (0.0f, WALL_HEIGHT / 2.0f, FIELD_LENGTH / 2.0f + WALL_THICKNESS / 2.0f);

    g_autoptr(GrlVector3) paddle1_pos = grl_vector3_new (-PADDLE_X_OFFSET, PADDLE_HEIGHT / 2.0f, game->paddle1_z);
    g_autoptr(GrlVector3) paddle2_pos = grl_vector3_new (PADDLE_X_OFFSET, PADDLE_HEIGHT / 2.0f, game->paddle2_z);

    g_autoptr(GrlVector3) ball_pos = grl_vector3_new (game->ball_x, BALL_Y, game->ball_z);

    g_autoptr(GrlVector3) line_start = NULL;
    g_autoptr(GrlVector3) line_end = NULL;
    gint i;

    grl_camera3d_begin (camera);

    /* Playing field (plane on Y=0) */
    grl_draw_plane (origin, field_size, field_color);

    /* Walls at front and back (Z boundaries) */
    grl_draw_cube (wall1_pos, FIELD_WIDTH, WALL_HEIGHT, WALL_THICKNESS, wall_color);
    grl_draw_cube (wall2_pos, FIELD_WIDTH, WALL_HEIGHT, WALL_THICKNESS, wall_color);

    /* Center line (dashed) */
    for (i = -11; i <= 11; i += 2)
    {
        line_start = grl_vector3_new (0.0f, 0.02f, (gfloat)i);
        line_end = grl_vector3_new (0.0f, 0.02f, (gfloat)(i + 1));
        grl_draw_line_3d (line_start, line_end, line_color);
        g_clear_pointer (&line_start, grl_vector3_free);
        g_clear_pointer (&line_end, grl_vector3_free);
    }

    /* Paddles */
    grl_draw_cube (paddle1_pos, PADDLE_WIDTH, PADDLE_HEIGHT, PADDLE_DEPTH, paddle1_color);
    grl_draw_cube_wires (paddle1_pos, PADDLE_WIDTH, PADDLE_HEIGHT, PADDLE_DEPTH, white);

    grl_draw_cube (paddle2_pos, PADDLE_WIDTH, PADDLE_HEIGHT, PADDLE_DEPTH, paddle2_color);
    grl_draw_cube_wires (paddle2_pos, PADDLE_WIDTH, PADDLE_HEIGHT, PADDLE_DEPTH, white);

    /* Ball */
    grl_draw_cube (ball_pos, BALL_SIZE, BALL_SIZE, BALL_SIZE, ball_color);
    grl_draw_cube_wires (ball_pos, BALL_SIZE, BALL_SIZE, BALL_SIZE, white);

    grl_camera3d_end (camera);
}

/*
 * Draw 2D UI overlay.
 */
static void
pong3d_draw_ui (Pong3DGame *game)
{
    g_autoptr(GrlColor) white = grl_color_new (255, 255, 255, 255);
    g_autoptr(GrlColor) gray = grl_color_new (150, 150, 150, 255);
    g_autoptr(GrlColor) yellow = grl_color_new (255, 220, 50, 255);
    g_autoptr(GrlColor) blue = grl_color_new (50, 120, 220, 255);
    g_autoptr(GrlColor) red = grl_color_new (220, 80, 80, 255);
    gchar text[64];
    gint text_width;

    /* Score display */
    g_snprintf (text, sizeof (text), "%d", game->score1);
    text_width = grl_measure_text (text, 50);
    grl_draw_text (text, WINDOW_WIDTH / 4 - text_width / 2, 20, 50, blue);

    g_snprintf (text, sizeof (text), "%d", game->score2);
    text_width = grl_measure_text (text, 50);
    grl_draw_text (text, 3 * WINDOW_WIDTH / 4 - text_width / 2, 20, 50, red);

    /* Controls help */
    grl_draw_text ("P1: W/S", 10, WINDOW_HEIGHT - 30, 14, gray);
    grl_draw_text ("P2: UP/DOWN", WINDOW_WIDTH - 110, WINDOW_HEIGHT - 30, 14, gray);

    /* Pause indicator */
    if (game->paused && !game->game_over)
    {
        grl_draw_text ("PAUSED", WINDOW_WIDTH / 2 - 50, WINDOW_HEIGHT / 2 - 50, 30, yellow);
        grl_draw_text ("Press SPACE to resume", WINDOW_WIDTH / 2 - 100, WINDOW_HEIGHT / 2, 16, gray);
    }

    /* Game over */
    if (game->game_over)
    {
        g_snprintf (text, sizeof (text), "PLAYER %d WINS!", game->winner);
        text_width = grl_measure_text (text, 40);
        grl_draw_text (text, WINDOW_WIDTH / 2 - text_width / 2, WINDOW_HEIGHT / 2 - 50, 40, yellow);
        grl_draw_text ("Press SPACE to restart", WINDOW_WIDTH / 2 - 100, WINDOW_HEIGHT / 2, 16, gray);
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
    Pong3DGame game;
    gfloat dt;

    app = grl_application_new ("com.example.pong3d");
    window = grl_window_new (WINDOW_WIDTH, WINDOW_HEIGHT, "3D Pong");
    bg_color = grl_color_new (20, 20, 30, 255);

    /* Setup 3D camera */
    camera = grl_camera3d_new ();
    grl_camera3d_set_position_xyz (camera, 0.0f, CAM_POS_Y, CAM_POS_Z);
    grl_camera3d_set_target_xyz (camera, 0.0f, 0.0f, 0.0f);
    grl_camera3d_set_fovy (camera, 45.0f);

    grl_window_set_target_fps (window, 60);

    pong3d_init (&game);

    while (!grl_window_should_close (window))
    {
        dt = grl_window_get_frame_time (window);

        /* Poll input */
        grl_window_poll_input (window);

        /* Pause toggle */
        if (grl_input_is_key_pressed (GRL_KEY_SPACE))
        {
            if (game.game_over)
            {
                pong3d_init (&game);
            }
            else
            {
                game.paused = !game.paused;
            }
        }

        /* Update */
        pong3d_update (&game, dt);

        /* Draw */
        grl_window_begin_drawing (window);
        grl_draw_clear_background (bg_color);

        pong3d_draw_3d (&game, camera);
        pong3d_draw_ui (&game);

        grl_window_end_drawing (window);
    }

    return 0;
}
