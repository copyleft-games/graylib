/* tetris3d.c
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Classic Tetris game in 3D demonstrating tetromino mechanics,
 * rotation systems, and line clearing.
 */

#include <graylib.h>
#include <stdlib.h>
#include <time.h>

/* Window dimensions */
#define WINDOW_WIDTH      1280
#define WINDOW_HEIGHT     900

/* Playfield dimensions */
#define GRID_COLS         10
#define GRID_ROWS         20
#define CELL_SIZE         1.0f

/* Timing */
#define DROP_INTERVAL     0.5f
#define SOFT_DROP_MULT    0.1f
#define LOCK_DELAY        0.3f

/* Camera position */
#define CAM_POS_X         5.0f
#define CAM_POS_Y         12.0f
#define CAM_POS_Z         25.0f

/* Scoring */
#define SCORE_SINGLE      100
#define SCORE_DOUBLE      300
#define SCORE_TRIPLE      500
#define SCORE_TETRIS      800
#define SCORE_SOFT_DROP   1
#define SCORE_HARD_DROP   2

/* 7 tetromino types */
typedef enum
{
    PIECE_I,
    PIECE_O,
    PIECE_T,
    PIECE_S,
    PIECE_Z,
    PIECE_J,
    PIECE_L,
    PIECE_COUNT
} PieceType;

/* Block position within piece */
typedef struct
{
    gint x;
    gint y;
} BlockPos;

/* Single rotation state of a piece */
typedef struct
{
    BlockPos blocks[4];
} PieceRotation;

/*
 * All 7 tetrominoes with 4 rotation states each.
 * Coordinates are relative to piece origin.
 * Y increases upward in our coordinate system.
 */
static const PieceRotation PIECES[PIECE_COUNT][4] =
{
    /* I piece - cyan */
    {
        {{ {0, 1}, {1, 1}, {2, 1}, {3, 1} }},  /* Horizontal */
        {{ {2, 0}, {2, 1}, {2, 2}, {2, 3} }},  /* Vertical */
        {{ {0, 2}, {1, 2}, {2, 2}, {3, 2} }},  /* Horizontal */
        {{ {1, 0}, {1, 1}, {1, 2}, {1, 3} }}   /* Vertical */
    },
    /* O piece - yellow */
    {
        {{ {0, 0}, {1, 0}, {0, 1}, {1, 1} }},
        {{ {0, 0}, {1, 0}, {0, 1}, {1, 1} }},
        {{ {0, 0}, {1, 0}, {0, 1}, {1, 1} }},
        {{ {0, 0}, {1, 0}, {0, 1}, {1, 1} }}
    },
    /* T piece - purple */
    {
        {{ {0, 1}, {1, 1}, {2, 1}, {1, 2} }},  /* T up */
        {{ {1, 0}, {1, 1}, {1, 2}, {2, 1} }},  /* T right */
        {{ {0, 1}, {1, 1}, {2, 1}, {1, 0} }},  /* T down */
        {{ {1, 0}, {1, 1}, {1, 2}, {0, 1} }}   /* T left */
    },
    /* S piece - green */
    {
        {{ {1, 1}, {2, 1}, {0, 0}, {1, 0} }},
        {{ {1, 0}, {1, 1}, {2, 1}, {2, 2} }},
        {{ {1, 2}, {2, 2}, {0, 1}, {1, 1} }},
        {{ {0, 0}, {0, 1}, {1, 1}, {1, 2} }}
    },
    /* Z piece - red */
    {
        {{ {0, 1}, {1, 1}, {1, 0}, {2, 0} }},
        {{ {2, 0}, {2, 1}, {1, 1}, {1, 2} }},
        {{ {0, 2}, {1, 2}, {1, 1}, {2, 1} }},
        {{ {1, 0}, {1, 1}, {0, 1}, {0, 2} }}
    },
    /* J piece - blue */
    {
        {{ {0, 2}, {0, 1}, {1, 1}, {2, 1} }},
        {{ {1, 0}, {2, 0}, {1, 1}, {1, 2} }},
        {{ {0, 1}, {1, 1}, {2, 1}, {2, 0} }},
        {{ {1, 0}, {1, 1}, {0, 2}, {1, 2} }}
    },
    /* L piece - orange */
    {
        {{ {0, 1}, {1, 1}, {2, 1}, {2, 2} }},
        {{ {1, 0}, {1, 1}, {1, 2}, {2, 0} }},
        {{ {0, 0}, {0, 1}, {1, 1}, {2, 1} }},
        {{ {0, 2}, {1, 0}, {1, 1}, {1, 2} }}
    }
};

/* Game state */
typedef struct
{
    /* Playfield grid (0 = empty, 1-7 = piece type + 1) */
    gint grid[GRID_ROWS][GRID_COLS];

    /* Current falling piece */
    PieceType current_type;
    gint current_rotation;
    gint piece_x;
    gint piece_y;

    /* Next piece preview */
    PieceType next_type;

    /* Timing */
    gfloat drop_timer;
    gfloat lock_timer;
    gboolean locking;

    /* Scoring */
    gint score;
    gint lines_cleared;
    gint level;

    /* State */
    gboolean paused;
    gboolean game_over;
} TetrisGame;

/*
 * Get color for piece type.
 */
static GrlColor *
get_piece_color (PieceType type)
{
    switch (type)
    {
        case PIECE_I: return grl_color_new (0, 240, 240, 255);     /* Cyan */
        case PIECE_O: return grl_color_new (240, 240, 0, 255);     /* Yellow */
        case PIECE_T: return grl_color_new (160, 0, 240, 255);     /* Purple */
        case PIECE_S: return grl_color_new (0, 240, 0, 255);       /* Green */
        case PIECE_Z: return grl_color_new (240, 0, 0, 255);       /* Red */
        case PIECE_J: return grl_color_new (0, 0, 240, 255);       /* Blue */
        case PIECE_L: return grl_color_new (240, 160, 0, 255);     /* Orange */
        default:      return grl_color_new (128, 128, 128, 255);   /* Gray */
    }
}

/*
 * Check if piece can occupy given position with given rotation.
 */
static gboolean
tetris_can_move (TetrisGame *game,
                 gint        new_x,
                 gint        new_y,
                 gint        rotation)
{
    const PieceRotation *rot;
    gint i;
    gint bx;
    gint by;

    rot = &PIECES[game->current_type][rotation];

    for (i = 0; i < 4; i++)
    {
        bx = new_x + rot->blocks[i].x;
        by = new_y + rot->blocks[i].y;

        /* Check horizontal bounds */
        if (bx < 0 || bx >= GRID_COLS)
            return FALSE;

        /* Check vertical bounds (allow above grid for spawning) */
        if (by < 0)
            return FALSE;

        /* Check collision with placed blocks */
        if (by < GRID_ROWS && game->grid[by][bx] != 0)
            return FALSE;
    }

    return TRUE;
}

/*
 * Spawn a new piece at the top of the playfield.
 * Returns FALSE if piece cannot spawn (game over).
 */
static gboolean
tetris_spawn_piece (TetrisGame *game)
{
    game->current_type = game->next_type;
    game->next_type = rand () % PIECE_COUNT;
    game->current_rotation = 0;
    game->piece_x = GRID_COLS / 2 - 2;
    game->piece_y = GRID_ROWS - 2;
    game->locking = FALSE;
    game->lock_timer = 0.0f;

    /* Check if spawn position is valid */
    if (!tetris_can_move (game, game->piece_x, game->piece_y, game->current_rotation))
    {
        return FALSE;
    }

    return TRUE;
}

/*
 * Initialize game state.
 */
static void
tetris_init (TetrisGame *game)
{
    gint row;
    gint col;

    srand ((unsigned int)time (NULL));

    /* Clear grid */
    for (row = 0; row < GRID_ROWS; row++)
    {
        for (col = 0; col < GRID_COLS; col++)
        {
            game->grid[row][col] = 0;
        }
    }

    /* Initialize state */
    game->score = 0;
    game->lines_cleared = 0;
    game->level = 1;
    game->paused = FALSE;
    game->game_over = FALSE;
    game->drop_timer = 0.0f;
    game->lock_timer = 0.0f;
    game->locking = FALSE;

    /* Spawn first piece */
    game->next_type = rand () % PIECE_COUNT;
    tetris_spawn_piece (game);
}

/*
 * Lock the current piece into the grid.
 */
static void
tetris_lock_piece (TetrisGame *game)
{
    const PieceRotation *rot;
    gint i;
    gint bx;
    gint by;

    rot = &PIECES[game->current_type][game->current_rotation];

    for (i = 0; i < 4; i++)
    {
        bx = game->piece_x + rot->blocks[i].x;
        by = game->piece_y + rot->blocks[i].y;

        if (by >= 0 && by < GRID_ROWS && bx >= 0 && bx < GRID_COLS)
        {
            game->grid[by][bx] = game->current_type + 1;
        }
    }
}

/*
 * Clear completed lines and return count.
 */
static gint
tetris_clear_lines (TetrisGame *game)
{
    gint cleared;
    gint row;
    gint col;
    gint dest;
    gboolean full;

    cleared = 0;

    /* Check from bottom to top */
    for (row = 0; row < GRID_ROWS; row++)
    {
        full = TRUE;
        for (col = 0; col < GRID_COLS; col++)
        {
            if (game->grid[row][col] == 0)
            {
                full = FALSE;
                break;
            }
        }

        if (full)
        {
            cleared++;

            /* Shift all rows above down */
            for (dest = row; dest < GRID_ROWS - 1; dest++)
            {
                for (col = 0; col < GRID_COLS; col++)
                {
                    game->grid[dest][col] = game->grid[dest + 1][col];
                }
            }

            /* Clear top row */
            for (col = 0; col < GRID_COLS; col++)
            {
                game->grid[GRID_ROWS - 1][col] = 0;
            }

            /* Re-check this row since it now has new content */
            row--;
        }
    }

    return cleared;
}

/*
 * Award score for cleared lines.
 */
static void
tetris_award_score (TetrisGame *game,
                    gint        lines)
{
    gint base_score;

    if (lines <= 0)
        return;

    switch (lines)
    {
        case 1:  base_score = SCORE_SINGLE; break;
        case 2:  base_score = SCORE_DOUBLE; break;
        case 3:  base_score = SCORE_TRIPLE; break;
        default: base_score = SCORE_TETRIS; break;
    }

    game->score += base_score * game->level;
    game->lines_cleared += lines;

    /* Level up every 10 lines */
    game->level = (game->lines_cleared / 10) + 1;
}

/*
 * Get current drop interval based on level.
 */
static gfloat
tetris_get_drop_interval (TetrisGame *game)
{
    gfloat interval;

    /* Speed increases with level */
    interval = DROP_INTERVAL - (game->level - 1) * 0.05f;
    if (interval < 0.05f)
        interval = 0.05f;

    return interval;
}

/*
 * Try to rotate piece with wall kicks.
 */
static void
tetris_rotate (TetrisGame *game)
{
    gint new_rot;
    gint kicks[5][2] = {
        {0, 0},    /* No kick */
        {-1, 0},   /* Left */
        {1, 0},    /* Right */
        {0, 1},    /* Up */
        {-2, 0}    /* Far left (for I piece) */
    };
    gint i;

    new_rot = (game->current_rotation + 1) % 4;

    /* Try each wall kick offset */
    for (i = 0; i < 5; i++)
    {
        if (tetris_can_move (game,
                            game->piece_x + kicks[i][0],
                            game->piece_y + kicks[i][1],
                            new_rot))
        {
            game->piece_x += kicks[i][0];
            game->piece_y += kicks[i][1];
            game->current_rotation = new_rot;

            /* Reset lock timer on successful rotation */
            if (game->locking)
            {
                game->lock_timer = 0.0f;
            }
            return;
        }
    }
}

/*
 * Handle input.
 */
static void
tetris_handle_input (TetrisGame *game)
{
    /* Move left */
    if (grl_input_is_key_pressed (GRL_KEY_A) ||
        grl_input_is_key_pressed (GRL_KEY_LEFT))
    {
        if (tetris_can_move (game, game->piece_x - 1, game->piece_y, game->current_rotation))
        {
            game->piece_x--;
            if (game->locking)
                game->lock_timer = 0.0f;
        }
    }

    /* Move right */
    if (grl_input_is_key_pressed (GRL_KEY_D) ||
        grl_input_is_key_pressed (GRL_KEY_RIGHT))
    {
        if (tetris_can_move (game, game->piece_x + 1, game->piece_y, game->current_rotation))
        {
            game->piece_x++;
            if (game->locking)
                game->lock_timer = 0.0f;
        }
    }

    /* Rotate */
    if (grl_input_is_key_pressed (GRL_KEY_W) ||
        grl_input_is_key_pressed (GRL_KEY_UP))
    {
        tetris_rotate (game);
    }

    /* Hard drop */
    if (grl_input_is_key_pressed (GRL_KEY_SPACE))
    {
        gint drop_distance = 0;
        while (tetris_can_move (game, game->piece_x, game->piece_y - 1, game->current_rotation))
        {
            game->piece_y--;
            drop_distance++;
        }
        game->score += drop_distance * SCORE_HARD_DROP;

        /* Lock immediately */
        tetris_lock_piece (game);
        tetris_award_score (game, tetris_clear_lines (game));

        if (!tetris_spawn_piece (game))
        {
            game->game_over = TRUE;
        }
    }
}

/*
 * Update game logic.
 */
static void
tetris_update (TetrisGame *game,
               gfloat      dt)
{
    gfloat drop_interval;
    gboolean soft_drop;
    gint lines;

    if (game->paused || game->game_over)
        return;

    tetris_handle_input (game);

    /* Check for soft drop */
    soft_drop = grl_input_is_key_down (GRL_KEY_S) ||
                grl_input_is_key_down (GRL_KEY_DOWN);

    drop_interval = tetris_get_drop_interval (game);
    if (soft_drop)
        drop_interval *= SOFT_DROP_MULT;

    /* Accumulate drop timer */
    game->drop_timer += dt;

    if (game->drop_timer >= drop_interval)
    {
        game->drop_timer = 0.0f;

        /* Try to move down */
        if (tetris_can_move (game, game->piece_x, game->piece_y - 1, game->current_rotation))
        {
            game->piece_y--;
            game->locking = FALSE;
            game->lock_timer = 0.0f;

            if (soft_drop)
                game->score += SCORE_SOFT_DROP;
        }
        else
        {
            /* Start or continue lock delay */
            game->locking = TRUE;
        }
    }

    /* Handle lock delay */
    if (game->locking)
    {
        game->lock_timer += dt;

        if (game->lock_timer >= LOCK_DELAY)
        {
            tetris_lock_piece (game);
            lines = tetris_clear_lines (game);
            tetris_award_score (game, lines);

            if (!tetris_spawn_piece (game))
            {
                game->game_over = TRUE;
            }
        }
    }
}

/*
 * Draw a single block as a 3D cube.
 */
static void
draw_block (gfloat     grid_x,
            gfloat     grid_y,
            GrlColor  *color,
            GrlColor  *wire_color)
{
    g_autoptr(GrlVector3) pos = NULL;
    gfloat size;

    size = CELL_SIZE * 0.95f;
    pos = grl_vector3_new (grid_x * CELL_SIZE,
                           grid_y * CELL_SIZE + CELL_SIZE / 2.0f,
                           0.0f);

    grl_draw_cube (pos, size, size, size, color);
    grl_draw_cube_wires (pos, size, size, size, wire_color);
}

/*
 * Draw ghost piece (drop preview).
 */
static void
draw_ghost_piece (TetrisGame *game,
                  GrlColor   *ghost_color,
                  GrlColor   *wire_color)
{
    const PieceRotation *rot;
    gint ghost_y;
    gint i;
    gint bx;
    gint by;

    /* Find lowest valid position */
    ghost_y = game->piece_y;
    while (tetris_can_move (game, game->piece_x, ghost_y - 1, game->current_rotation))
    {
        ghost_y--;
    }

    /* Don't draw if ghost is at same position as piece */
    if (ghost_y == game->piece_y)
        return;

    rot = &PIECES[game->current_type][game->current_rotation];

    for (i = 0; i < 4; i++)
    {
        bx = game->piece_x + rot->blocks[i].x;
        by = ghost_y + rot->blocks[i].y;

        if (by >= 0 && by < GRID_ROWS)
        {
            draw_block ((gfloat)bx, (gfloat)by, ghost_color, wire_color);
        }
    }
}

/*
 * Draw next piece preview.
 */
static void
draw_next_piece (TetrisGame  *game,
                 gfloat       offset_x,
                 gfloat       offset_y,
                 GrlColor    *wire_color)
{
    const PieceRotation *rot;
    g_autoptr(GrlColor) color = NULL;
    gint i;

    rot = &PIECES[game->next_type][0];
    color = get_piece_color (game->next_type);

    for (i = 0; i < 4; i++)
    {
        g_autoptr(GrlVector3) pos = NULL;
        gfloat size;
        gfloat x;
        gfloat y;

        x = offset_x + rot->blocks[i].x * CELL_SIZE * 0.7f;
        y = offset_y + rot->blocks[i].y * CELL_SIZE * 0.7f;
        size = CELL_SIZE * 0.65f;

        pos = grl_vector3_new (x, y, 0.0f);
        grl_draw_cube (pos, size, size, size, color);
        grl_draw_cube_wires (pos, size, size, size, wire_color);
    }
}

/*
 * Draw 3D scene.
 */
static void
tetris_draw_3d (TetrisGame  *game,
                GrlCamera3D *camera)
{
    g_autoptr(GrlColor) grid_color = grl_color_new (40, 40, 50, 255);
    g_autoptr(GrlColor) border_color = grl_color_new (80, 80, 100, 255);
    g_autoptr(GrlColor) ghost_color = grl_color_new (100, 100, 100, 100);
    g_autoptr(GrlColor) white = grl_color_new (255, 255, 255, 255);
    g_autoptr(GrlColor) dark_wire = grl_color_new (30, 30, 40, 255);
    const PieceRotation *rot;
    gint row;
    gint col;
    gint i;
    gint bx;
    gint by;

    grl_camera3d_begin (camera);

    /* Draw grid background */
    for (row = 0; row < GRID_ROWS; row++)
    {
        for (col = 0; col < GRID_COLS; col++)
        {
            g_autoptr(GrlVector3) pos = NULL;
            pos = grl_vector3_new (col * CELL_SIZE, row * CELL_SIZE, -CELL_SIZE * 0.5f);
            grl_draw_cube (pos, CELL_SIZE * 0.98f, CELL_SIZE * 0.98f, CELL_SIZE * 0.1f, grid_color);
        }
    }

    /* Draw border */
    {
        g_autoptr(GrlVector3) left_pos = NULL;
        g_autoptr(GrlVector3) right_pos = NULL;
        g_autoptr(GrlVector3) bottom_pos = NULL;

        /* Left wall */
        left_pos = grl_vector3_new (-CELL_SIZE * 0.5f, GRID_ROWS * CELL_SIZE / 2.0f - CELL_SIZE / 2.0f, 0.0f);
        grl_draw_cube (left_pos, CELL_SIZE * 0.3f, GRID_ROWS * CELL_SIZE, CELL_SIZE, border_color);

        /* Right wall */
        right_pos = grl_vector3_new (GRID_COLS * CELL_SIZE - CELL_SIZE * 0.5f, GRID_ROWS * CELL_SIZE / 2.0f - CELL_SIZE / 2.0f, 0.0f);
        grl_draw_cube (right_pos, CELL_SIZE * 0.3f, GRID_ROWS * CELL_SIZE, CELL_SIZE, border_color);

        /* Bottom wall */
        bottom_pos = grl_vector3_new (GRID_COLS * CELL_SIZE / 2.0f - CELL_SIZE / 2.0f, -CELL_SIZE * 0.5f, 0.0f);
        grl_draw_cube (bottom_pos, GRID_COLS * CELL_SIZE + CELL_SIZE * 0.6f, CELL_SIZE * 0.3f, CELL_SIZE, border_color);
    }

    /* Draw placed blocks */
    for (row = 0; row < GRID_ROWS; row++)
    {
        for (col = 0; col < GRID_COLS; col++)
        {
            if (game->grid[row][col] != 0)
            {
                g_autoptr(GrlColor) color = NULL;
                color = get_piece_color (game->grid[row][col] - 1);
                draw_block ((gfloat)col, (gfloat)row, color, dark_wire);
            }
        }
    }

    /* Draw ghost piece */
    if (!game->game_over)
    {
        draw_ghost_piece (game, ghost_color, dark_wire);
    }

    /* Draw current falling piece */
    if (!game->game_over)
    {
        g_autoptr(GrlColor) piece_color = NULL;

        rot = &PIECES[game->current_type][game->current_rotation];
        piece_color = get_piece_color (game->current_type);

        for (i = 0; i < 4; i++)
        {
            bx = game->piece_x + rot->blocks[i].x;
            by = game->piece_y + rot->blocks[i].y;

            if (by >= 0 && by < GRID_ROWS + 4)
            {
                draw_block ((gfloat)bx, (gfloat)by, piece_color, white);
            }
        }
    }

    /* Draw next piece preview to the right of playfield */
    draw_next_piece (game, GRID_COLS * CELL_SIZE + 2.0f, GRID_ROWS * CELL_SIZE - 4.0f, white);

    grl_camera3d_end (camera);
}

/*
 * Draw 2D UI overlay.
 */
static void
tetris_draw_ui (TetrisGame *game)
{
    g_autoptr(GrlColor) white = grl_color_new (255, 255, 255, 255);
    g_autoptr(GrlColor) gray = grl_color_new (150, 150, 150, 255);
    g_autoptr(GrlColor) yellow = grl_color_new (255, 220, 50, 255);
    g_autoptr(GrlColor) cyan = grl_color_new (0, 200, 255, 255);
    gchar text[64];
    gint text_width;

    /* Score */
    g_snprintf (text, sizeof (text), "Score: %d", game->score);
    grl_draw_text (text, 20, 20, 30, cyan);

    /* Level */
    g_snprintf (text, sizeof (text), "Level: %d", game->level);
    grl_draw_text (text, 20, 55, 20, white);

    /* Lines */
    g_snprintf (text, sizeof (text), "Lines: %d", game->lines_cleared);
    grl_draw_text (text, 20, 80, 20, white);

    /* Next piece label */
    grl_draw_text ("Next:", WINDOW_WIDTH - 150, 20, 20, white);

    /* Controls */
    grl_draw_text ("A/D: Move", WINDOW_WIDTH - 150, WINDOW_HEIGHT - 100, 14, gray);
    grl_draw_text ("W: Rotate", WINDOW_WIDTH - 150, WINDOW_HEIGHT - 80, 14, gray);
    grl_draw_text ("S: Soft Drop", WINDOW_WIDTH - 150, WINDOW_HEIGHT - 60, 14, gray);
    grl_draw_text ("SPACE: Hard Drop", WINDOW_WIDTH - 150, WINDOW_HEIGHT - 40, 14, gray);
    grl_draw_text ("P: Pause", WINDOW_WIDTH - 150, WINDOW_HEIGHT - 20, 14, gray);

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
        grl_draw_text (text, WINDOW_WIDTH / 2 - text_width / 2, WINDOW_HEIGHT / 2 - 20, 30, white);

        g_snprintf (text, sizeof (text), "Lines Cleared: %d", game->lines_cleared);
        text_width = grl_measure_text (text, 20);
        grl_draw_text (text, WINDOW_WIDTH / 2 - text_width / 2, WINDOW_HEIGHT / 2 + 20, 20, white);

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
    TetrisGame game;
    gfloat dt;

    app = grl_application_new ("com.example.tetris3d");
    window = grl_window_new (WINDOW_WIDTH, WINDOW_HEIGHT, "3D Tetris");
    bg_color = grl_color_new (15, 15, 25, 255);

    /* Setup 3D camera - angled view of playfield */
    camera = grl_camera3d_new ();
    grl_camera3d_set_position_xyz (camera, CAM_POS_X, CAM_POS_Y, CAM_POS_Z);
    grl_camera3d_set_target_xyz (camera, GRID_COLS / 2.0f * CELL_SIZE, GRID_ROWS / 2.0f * CELL_SIZE, 0.0f);
    grl_camera3d_set_fovy (camera, 45.0f);

    grl_window_set_target_fps (window, 60);

    tetris_init (&game);

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
            tetris_init (&game);
        }

        /* Update */
        tetris_update (&game, dt);

        /* Draw */
        grl_window_begin_drawing (window);
        grl_draw_clear_background (bg_color);

        tetris_draw_3d (&game, camera);
        tetris_draw_ui (&game);

        grl_window_end_drawing (window);
    }

    return 0;
}
