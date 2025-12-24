/* snake3d.c
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Classic Snake game in 3D demonstrating grid-based movement,
 * dynamic body management, and collision detection.
 */

#include <graylib.h>
#include <stdlib.h>
#include <time.h>

/* Window dimensions */
#define WINDOW_WIDTH      1280
#define WINDOW_HEIGHT     900

/* Grid dimensions */
#define GRID_SIZE         20
#define CELL_SIZE         1.5f
#define FIELD_SIZE        (GRID_SIZE * CELL_SIZE)

/* Snake settings */
#define MAX_SNAKE_LENGTH  400
#define INITIAL_LENGTH    3
#define MOVE_INTERVAL     0.12f

/* Wall settings */
#define WALL_HEIGHT       0.8f
#define WALL_THICKNESS    0.3f

/* Camera position */
#define CAM_POS_Y         38.0f
#define CAM_POS_Z         28.0f

/* Direction enum */
typedef enum
{
    DIR_UP,
    DIR_DOWN,
    DIR_LEFT,
    DIR_RIGHT
} Direction;

/* Grid position */
typedef struct
{
    gint x;
    gint y;
} GridPos;

/* Game state */
typedef struct
{
    /* Snake body (head at index 0) */
    GridPos segments[MAX_SNAKE_LENGTH];
    gint length;
    Direction direction;
    Direction next_direction;

    /* Food */
    GridPos food;

    /* Timing */
    gfloat move_timer;

    /* State */
    gint score;
    gboolean paused;
    gboolean game_over;
} SnakeGame;

/*
 * Convert grid coordinate to world coordinate.
 */
static gfloat
grid_to_world (gint grid_coord)
{
    return ((gfloat)grid_coord - GRID_SIZE / 2.0f + 0.5f) * CELL_SIZE;
}

/*
 * Check if position is occupied by snake.
 */
static gboolean
is_on_snake (SnakeGame *game,
             gint       x,
             gint       y)
{
    gint i;

    for (i = 0; i < game->length; i++)
    {
        if (game->segments[i].x == x && game->segments[i].y == y)
            return TRUE;
    }
    return FALSE;
}

/*
 * Spawn food at random position not on snake.
 */
static void
snake_spawn_food (SnakeGame *game)
{
    gint attempts;

    attempts = 0;
    do
    {
        game->food.x = rand () % GRID_SIZE;
        game->food.y = rand () % GRID_SIZE;
        attempts++;
    }
    while (is_on_snake (game, game->food.x, game->food.y) && attempts < 1000);
}

/*
 * Initialize game state.
 */
static void
snake_init (SnakeGame *game)
{
    gint i;
    gint start_x;
    gint start_y;

    srand ((unsigned int)time (NULL));

    /* Start snake at center, facing right */
    start_x = GRID_SIZE / 2;
    start_y = GRID_SIZE / 2;

    game->length = INITIAL_LENGTH;
    for (i = 0; i < INITIAL_LENGTH; i++)
    {
        game->segments[i].x = start_x - i;
        game->segments[i].y = start_y;
    }

    game->direction = DIR_RIGHT;
    game->next_direction = DIR_RIGHT;

    game->move_timer = 0.0f;
    game->score = 0;
    game->paused = FALSE;
    game->game_over = FALSE;

    snake_spawn_food (game);
}

/*
 * Handle input for direction changes.
 */
static void
snake_handle_input (SnakeGame *game)
{
    /* Prevent 180-degree turns by checking current direction */
    if ((grl_input_is_key_pressed (GRL_KEY_W) ||
         grl_input_is_key_pressed (GRL_KEY_UP)) &&
        game->direction != DIR_DOWN)
    {
        game->next_direction = DIR_UP;
    }
    else if ((grl_input_is_key_pressed (GRL_KEY_S) ||
              grl_input_is_key_pressed (GRL_KEY_DOWN)) &&
             game->direction != DIR_UP)
    {
        game->next_direction = DIR_DOWN;
    }
    else if ((grl_input_is_key_pressed (GRL_KEY_A) ||
              grl_input_is_key_pressed (GRL_KEY_LEFT)) &&
             game->direction != DIR_RIGHT)
    {
        game->next_direction = DIR_LEFT;
    }
    else if ((grl_input_is_key_pressed (GRL_KEY_D) ||
              grl_input_is_key_pressed (GRL_KEY_RIGHT)) &&
             game->direction != DIR_LEFT)
    {
        game->next_direction = DIR_RIGHT;
    }
}

/*
 * Update game logic.
 */
static void
snake_update (SnakeGame *game,
              gfloat     dt)
{
    GridPos new_head;
    gint i;

    if (game->paused || game->game_over)
        return;

    snake_handle_input (game);

    /* Accumulate time */
    game->move_timer += dt;

    if (game->move_timer < MOVE_INTERVAL)
        return;

    game->move_timer = 0.0f;
    game->direction = game->next_direction;

    /* Calculate new head position */
    new_head = game->segments[0];
    switch (game->direction)
    {
        case DIR_UP:
            new_head.y--;
            break;
        case DIR_DOWN:
            new_head.y++;
            break;
        case DIR_LEFT:
            new_head.x--;
            break;
        case DIR_RIGHT:
            new_head.x++;
            break;
    }

    /* Check wall collision */
    if (new_head.x < 0 || new_head.x >= GRID_SIZE ||
        new_head.y < 0 || new_head.y >= GRID_SIZE)
    {
        game->game_over = TRUE;
        return;
    }

    /* Check self collision (skip tail since it will move) */
    for (i = 0; i < game->length - 1; i++)
    {
        if (new_head.x == game->segments[i].x &&
            new_head.y == game->segments[i].y)
        {
            game->game_over = TRUE;
            return;
        }
    }

    /* Check food collision before moving */
    if (new_head.x == game->food.x && new_head.y == game->food.y)
    {
        /* Grow snake - add segment at current tail position */
        if (game->length < MAX_SNAKE_LENGTH)
        {
            game->length++;
        }
        game->score += 10;
        snake_spawn_food (game);
    }

    /* Move body (shift segments from tail to head) */
    for (i = game->length - 1; i > 0; i--)
    {
        game->segments[i] = game->segments[i - 1];
    }
    game->segments[0] = new_head;
}

/*
 * Draw 3D scene.
 */
static void
snake_draw_3d (SnakeGame   *game,
               GrlCamera3D *camera)
{
    g_autoptr(GrlColor) field_color = grl_color_new (35, 40, 50, 255);
    g_autoptr(GrlColor) grid_color = grl_color_new (50, 55, 65, 255);
    g_autoptr(GrlColor) wall_color = grl_color_new (70, 75, 90, 255);
    g_autoptr(GrlColor) head_color = grl_color_new (50, 200, 80, 255);
    g_autoptr(GrlColor) body_color = grl_color_new (40, 160, 60, 255);
    g_autoptr(GrlColor) food_color = grl_color_new (220, 60, 60, 255);
    g_autoptr(GrlColor) white = grl_color_new (255, 255, 255, 255);

    g_autoptr(GrlVector3) origin = grl_vector3_new (0.0f, 0.0f, 0.0f);
    g_autoptr(GrlVector2) field_size = grl_vector2_new (FIELD_SIZE, FIELD_SIZE);

    g_autoptr(GrlVector3) pos = NULL;
    gfloat half_field;
    gfloat cube_size;
    gfloat x, z;
    gint i;

    half_field = FIELD_SIZE / 2.0f;

    grl_camera3d_begin (camera);

    /* Playing field */
    grl_draw_plane (origin, field_size, field_color);

    /* Grid lines */
    for (i = 0; i <= GRID_SIZE; i++)
    {
        g_autoptr(GrlVector3) start = NULL;
        g_autoptr(GrlVector3) end = NULL;
        gfloat line_pos;

        line_pos = -half_field + i * CELL_SIZE;

        /* Horizontal lines (along X) */
        start = grl_vector3_new (-half_field, 0.01f, line_pos);
        end = grl_vector3_new (half_field, 0.01f, line_pos);
        grl_draw_line_3d (start, end, grid_color);

        /* Vertical lines (along Z) */
        g_clear_pointer (&start, grl_vector3_free);
        g_clear_pointer (&end, grl_vector3_free);
        start = grl_vector3_new (line_pos, 0.01f, -half_field);
        end = grl_vector3_new (line_pos, 0.01f, half_field);
        grl_draw_line_3d (start, end, grid_color);
    }

    /* Walls (4 sides) */
    pos = grl_vector3_new (0.0f, WALL_HEIGHT / 2.0f, -half_field - WALL_THICKNESS / 2.0f);
    grl_draw_cube (pos, FIELD_SIZE + WALL_THICKNESS * 2, WALL_HEIGHT, WALL_THICKNESS, wall_color);
    g_clear_pointer (&pos, grl_vector3_free);

    pos = grl_vector3_new (0.0f, WALL_HEIGHT / 2.0f, half_field + WALL_THICKNESS / 2.0f);
    grl_draw_cube (pos, FIELD_SIZE + WALL_THICKNESS * 2, WALL_HEIGHT, WALL_THICKNESS, wall_color);
    g_clear_pointer (&pos, grl_vector3_free);

    pos = grl_vector3_new (-half_field - WALL_THICKNESS / 2.0f, WALL_HEIGHT / 2.0f, 0.0f);
    grl_draw_cube (pos, WALL_THICKNESS, WALL_HEIGHT, FIELD_SIZE, wall_color);
    g_clear_pointer (&pos, grl_vector3_free);

    pos = grl_vector3_new (half_field + WALL_THICKNESS / 2.0f, WALL_HEIGHT / 2.0f, 0.0f);
    grl_draw_cube (pos, WALL_THICKNESS, WALL_HEIGHT, FIELD_SIZE, wall_color);
    g_clear_pointer (&pos, grl_vector3_free);

    /* Snake segments */
    for (i = 0; i < game->length; i++)
    {
        x = grid_to_world (game->segments[i].x);
        z = grid_to_world (game->segments[i].y);

        if (i == 0)
        {
            /* Head - slightly larger */
            cube_size = CELL_SIZE * 0.9f;
            pos = grl_vector3_new (x, cube_size / 2.0f, z);
            grl_draw_cube (pos, cube_size, cube_size, cube_size, head_color);
            grl_draw_cube_wires (pos, cube_size, cube_size, cube_size, white);
        }
        else
        {
            /* Body */
            cube_size = CELL_SIZE * 0.8f;
            pos = grl_vector3_new (x, cube_size / 2.0f, z);
            grl_draw_cube (pos, cube_size, cube_size, cube_size, body_color);
        }
        g_clear_pointer (&pos, grl_vector3_free);
    }

    /* Food */
    x = grid_to_world (game->food.x);
    z = grid_to_world (game->food.y);
    cube_size = CELL_SIZE * 0.6f;
    pos = grl_vector3_new (x, cube_size / 2.0f + 0.1f, z);
    grl_draw_cube (pos, cube_size, cube_size, cube_size, food_color);
    grl_draw_cube_wires (pos, cube_size, cube_size, cube_size, white);

    grl_camera3d_end (camera);
}

/*
 * Draw 2D UI overlay.
 */
static void
snake_draw_ui (SnakeGame *game)
{
    g_autoptr(GrlColor) white = grl_color_new (255, 255, 255, 255);
    g_autoptr(GrlColor) gray = grl_color_new (150, 150, 150, 255);
    g_autoptr(GrlColor) yellow = grl_color_new (255, 220, 50, 255);
    g_autoptr(GrlColor) green = grl_color_new (50, 200, 80, 255);
    gchar text[64];
    gint text_width;

    /* Score display */
    g_snprintf (text, sizeof (text), "Score: %d", game->score);
    grl_draw_text (text, 20, 20, 30, green);

    /* Length display */
    g_snprintf (text, sizeof (text), "Length: %d", game->length);
    grl_draw_text (text, 20, 55, 20, white);

    /* Controls help */
    grl_draw_text ("WASD / Arrows: Move", WINDOW_WIDTH - 180, WINDOW_HEIGHT - 30, 14, gray);

    /* Pause indicator */
    if (game->paused && !game->game_over)
    {
        grl_draw_text ("PAUSED", WINDOW_WIDTH / 2 - 50, WINDOW_HEIGHT / 2 - 50, 30, yellow);
        grl_draw_text ("Press SPACE to resume", WINDOW_WIDTH / 2 - 100, WINDOW_HEIGHT / 2, 16, gray);
    }

    /* Game over */
    if (game->game_over)
    {
        grl_draw_text ("GAME OVER", WINDOW_WIDTH / 2 - 80, WINDOW_HEIGHT / 2 - 60, 40, yellow);

        g_snprintf (text, sizeof (text), "Final Score: %d", game->score);
        text_width = grl_measure_text (text, 24);
        grl_draw_text (text, WINDOW_WIDTH / 2 - text_width / 2, WINDOW_HEIGHT / 2 - 10, 24, white);

        grl_draw_text ("Press SPACE to restart", WINDOW_WIDTH / 2 - 100, WINDOW_HEIGHT / 2 + 30, 16, gray);
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
    SnakeGame game;
    gfloat dt;

    app = grl_application_new ("com.example.snake3d");
    window = grl_window_new (WINDOW_WIDTH, WINDOW_HEIGHT, "3D Snake");
    bg_color = grl_color_new (20, 20, 30, 255);

    /* Setup 3D camera - top-down view */
    camera = grl_camera3d_new ();
    grl_camera3d_set_position_xyz (camera, 0.0f, CAM_POS_Y, CAM_POS_Z);
    grl_camera3d_set_target_xyz (camera, 0.0f, 0.0f, 0.0f);
    grl_camera3d_set_fovy (camera, 45.0f);

    grl_window_set_target_fps (window, 60);

    snake_init (&game);

    while (!grl_window_should_close (window))
    {
        dt = grl_window_get_frame_time (window);

        /* Poll input */
        grl_window_poll_input (window);

        /* Pause/restart toggle */
        if (grl_input_is_key_pressed (GRL_KEY_SPACE))
        {
            if (game.game_over)
            {
                snake_init (&game);
            }
            else
            {
                game.paused = !game.paused;
            }
        }

        /* Update */
        snake_update (&game, dt);

        /* Draw */
        grl_window_begin_drawing (window);
        grl_draw_clear_background (bg_color);

        snake_draw_3d (&game, camera);
        snake_draw_ui (&game);

        grl_window_end_drawing (window);
    }

    return 0;
}
