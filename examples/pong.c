/* pong.c
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Classic Pong game demonstrating input handling, collision detection,
 * and game state management with Graylib.
 */

#include <graylib.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

/* Game constants */
#define WINDOW_WIDTH     800
#define WINDOW_HEIGHT    600
#define PADDLE_WIDTH     15
#define PADDLE_HEIGHT    80
#define PADDLE_MARGIN    30
#define PADDLE_SPEED     400.0f
#define BALL_RADIUS      10.0f
#define BALL_SPEED       350.0f
#define WINNING_SCORE    10

/* Game state */
typedef struct
{
    /* Ball state */
    gfloat ball_x;
    gfloat ball_y;
    gfloat ball_vx;
    gfloat ball_vy;

    /* Paddle positions (y-coordinate of top edge) */
    gfloat paddle1_y;
    gfloat paddle2_y;

    /* Scores */
    gint score1;
    gint score2;

    /* Game flags */
    gboolean paused;
    gboolean game_over;
    gint winner;
} PongGame;

/*
 * Reset the ball to center with a random direction.
 * The ball always starts moving toward the player who just scored against.
 */
static void
pong_reset_ball (PongGame *game,
                 gint      direction)
{
    gfloat angle;

    game->ball_x = WINDOW_WIDTH / 2.0f;
    game->ball_y = WINDOW_HEIGHT / 2.0f;

    /* Random angle between -45 and 45 degrees */
    angle = ((gfloat)(rand () % 90) - 45.0f) * (G_PI / 180.0f);

    game->ball_vx = BALL_SPEED * cosf (angle) * direction;
    game->ball_vy = BALL_SPEED * sinf (angle);
}

/*
 * Initialize the game state.
 */
static void
pong_init (PongGame *game)
{
    srand ((unsigned int)time (NULL));

    /* Center paddles vertically */
    game->paddle1_y = (WINDOW_HEIGHT - PADDLE_HEIGHT) / 2.0f;
    game->paddle2_y = (WINDOW_HEIGHT - PADDLE_HEIGHT) / 2.0f;

    /* Reset scores */
    game->score1 = 0;
    game->score2 = 0;

    /* Game state */
    game->paused = FALSE;
    game->game_over = FALSE;
    game->winner = 0;

    /* Start ball moving toward a random player */
    pong_reset_ball (game, (rand () % 2 == 0) ? 1 : -1);
}

/*
 * Update game logic.
 */
static void
pong_update (PongGame *game,
             gfloat    dt)
{
    gfloat paddle1_x;
    gfloat paddle2_x;
    gfloat hit_pos;
    gfloat normalized;
    gfloat bounce_angle;
    gfloat speed;

    if (game->paused || game->game_over)
        return;

    /* Player 1 input (W/S) */
    if (grl_input_is_key_down (GRL_KEY_W))
        game->paddle1_y -= PADDLE_SPEED * dt;
    if (grl_input_is_key_down (GRL_KEY_S))
        game->paddle1_y += PADDLE_SPEED * dt;

    /* Player 2 input (Up/Down arrows) */
    if (grl_input_is_key_down (GRL_KEY_UP))
        game->paddle2_y -= PADDLE_SPEED * dt;
    if (grl_input_is_key_down (GRL_KEY_DOWN))
        game->paddle2_y += PADDLE_SPEED * dt;

    /* Clamp paddles to screen bounds */
    if (game->paddle1_y < 0)
        game->paddle1_y = 0;
    if (game->paddle1_y > WINDOW_HEIGHT - PADDLE_HEIGHT)
        game->paddle1_y = WINDOW_HEIGHT - PADDLE_HEIGHT;
    if (game->paddle2_y < 0)
        game->paddle2_y = 0;
    if (game->paddle2_y > WINDOW_HEIGHT - PADDLE_HEIGHT)
        game->paddle2_y = WINDOW_HEIGHT - PADDLE_HEIGHT;

    /* Update ball position */
    game->ball_x += game->ball_vx * dt;
    game->ball_y += game->ball_vy * dt;

    /* Ball collision with top/bottom walls */
    if (game->ball_y - BALL_RADIUS <= 0)
    {
        game->ball_y = BALL_RADIUS;
        game->ball_vy = -game->ball_vy;
    }
    if (game->ball_y + BALL_RADIUS >= WINDOW_HEIGHT)
    {
        game->ball_y = WINDOW_HEIGHT - BALL_RADIUS;
        game->ball_vy = -game->ball_vy;
    }

    /* Paddle positions */
    paddle1_x = PADDLE_MARGIN;
    paddle2_x = WINDOW_WIDTH - PADDLE_MARGIN - PADDLE_WIDTH;

    /* Ball collision with paddle 1 (left) */
    if (game->ball_vx < 0 &&
        game->ball_x - BALL_RADIUS <= paddle1_x + PADDLE_WIDTH &&
        game->ball_x + BALL_RADIUS >= paddle1_x &&
        game->ball_y >= game->paddle1_y &&
        game->ball_y <= game->paddle1_y + PADDLE_HEIGHT)
    {
        game->ball_x = paddle1_x + PADDLE_WIDTH + BALL_RADIUS;

        /* Calculate bounce angle based on where ball hit paddle */
        hit_pos = game->ball_y - (game->paddle1_y + PADDLE_HEIGHT / 2.0f);
        normalized = hit_pos / (PADDLE_HEIGHT / 2.0f);
        bounce_angle = normalized * (G_PI / 4.0f);

        speed = sqrtf (game->ball_vx * game->ball_vx + game->ball_vy * game->ball_vy);
        game->ball_vx = speed * cosf (bounce_angle);
        game->ball_vy = speed * sinf (bounce_angle);
    }

    /* Ball collision with paddle 2 (right) */
    if (game->ball_vx > 0 &&
        game->ball_x + BALL_RADIUS >= paddle2_x &&
        game->ball_x - BALL_RADIUS <= paddle2_x + PADDLE_WIDTH &&
        game->ball_y >= game->paddle2_y &&
        game->ball_y <= game->paddle2_y + PADDLE_HEIGHT)
    {
        game->ball_x = paddle2_x - BALL_RADIUS;

        /* Calculate bounce angle based on where ball hit paddle */
        hit_pos = game->ball_y - (game->paddle2_y + PADDLE_HEIGHT / 2.0f);
        normalized = hit_pos / (PADDLE_HEIGHT / 2.0f);
        bounce_angle = normalized * (G_PI / 4.0f);

        speed = sqrtf (game->ball_vx * game->ball_vx + game->ball_vy * game->ball_vy);
        game->ball_vx = -speed * cosf (bounce_angle);
        game->ball_vy = speed * sinf (bounce_angle);
    }

    /* Scoring - ball passed paddle */
    if (game->ball_x < 0)
    {
        game->score2++;
        if (game->score2 >= WINNING_SCORE)
        {
            game->game_over = TRUE;
            game->winner = 2;
        }
        else
        {
            pong_reset_ball (game, -1);
        }
    }
    if (game->ball_x > WINDOW_WIDTH)
    {
        game->score1++;
        if (game->score1 >= WINNING_SCORE)
        {
            game->game_over = TRUE;
            game->winner = 1;
        }
        else
        {
            pong_reset_ball (game, 1);
        }
    }
}

/*
 * Draw the game.
 */
static void
pong_draw (PongGame *game)
{
    g_autoptr(GrlColor) bg_color = grl_color_new (20, 20, 30, 255);
    g_autoptr(GrlColor) white = grl_color_new (255, 255, 255, 255);
    g_autoptr(GrlColor) gray = grl_color_new (100, 100, 100, 255);
    g_autoptr(GrlColor) yellow = grl_color_new (255, 255, 0, 255);
    gchar score_text[32];
    gint text_width;
    gint i;

    grl_draw_clear_background (bg_color);

    /* Draw center line (dashed) */
    for (i = 0; i < WINDOW_HEIGHT; i += 30)
    {
        grl_draw_rectangle (WINDOW_WIDTH / 2 - 2, i, 4, 15, gray);
    }

    /* Draw paddles */
    grl_draw_rectangle ((gint)PADDLE_MARGIN,
                        (gint)game->paddle1_y,
                        PADDLE_WIDTH,
                        PADDLE_HEIGHT,
                        white);
    grl_draw_rectangle ((gint)(WINDOW_WIDTH - PADDLE_MARGIN - PADDLE_WIDTH),
                        (gint)game->paddle2_y,
                        PADDLE_WIDTH,
                        PADDLE_HEIGHT,
                        white);

    /* Draw ball */
    grl_draw_circle ((gint)game->ball_x, (gint)game->ball_y, BALL_RADIUS, white);

    /* Draw scores */
    g_snprintf (score_text, sizeof (score_text), "%d", game->score1);
    text_width = grl_measure_text (score_text, 60);
    grl_draw_text (score_text, WINDOW_WIDTH / 4 - text_width / 2, 30, 60, white);

    g_snprintf (score_text, sizeof (score_text), "%d", game->score2);
    text_width = grl_measure_text (score_text, 60);
    grl_draw_text (score_text, 3 * WINDOW_WIDTH / 4 - text_width / 2, 30, 60, white);

    /* Draw controls help */
    grl_draw_text ("W/S", 10, WINDOW_HEIGHT - 30, 16, gray);
    grl_draw_text ("UP/DOWN", WINDOW_WIDTH - 80, WINDOW_HEIGHT - 30, 16, gray);

    /* Draw pause indicator */
    if (game->paused && !game->game_over)
    {
        grl_draw_text ("PAUSED", WINDOW_WIDTH / 2 - 50, WINDOW_HEIGHT / 2, 30, yellow);
        grl_draw_text ("Press SPACE to resume", WINDOW_WIDTH / 2 - 100, WINDOW_HEIGHT / 2 + 40, 16, gray);
    }

    /* Draw game over screen */
    if (game->game_over)
    {
        g_snprintf (score_text, sizeof (score_text), "PLAYER %d WINS!", game->winner);
        text_width = grl_measure_text (score_text, 40);
        grl_draw_text (score_text, WINDOW_WIDTH / 2 - text_width / 2, WINDOW_HEIGHT / 2 - 20, 40, yellow);
        grl_draw_text ("Press SPACE to restart", WINDOW_WIDTH / 2 - 100, WINDOW_HEIGHT / 2 + 30, 16, gray);
    }
}

int
main (int    argc,
      char **argv)
{
    g_autoptr(GrlApplication) app = NULL;
    g_autoptr(GrlWindow) window = NULL;
    PongGame game;
    gfloat dt;

    app = grl_application_new ("com.example.pong");
    window = grl_window_new (WINDOW_WIDTH, WINDOW_HEIGHT, "Pong");

    grl_window_set_target_fps (window, 60);

    pong_init (&game);

    while (!grl_window_should_close (window))
    {
        dt = grl_window_get_frame_time (window);

        /* Poll input (required for input functions to work) */
        grl_window_poll_input (window);

        /* Handle pause toggle */
        if (grl_input_is_key_pressed (GRL_KEY_SPACE))
        {
            if (game.game_over)
            {
                pong_init (&game);
            }
            else
            {
                game.paused = !game.paused;
            }
        }

        /* Update game logic */
        pong_update (&game, dt);

        /* Draw */
        grl_window_begin_drawing (window);
        pong_draw (&game);
        grl_window_end_drawing (window);
    }

    return 0;
}
