/* firstperson-demo.c
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * First-person 3D demo demonstrating mouse look camera control,
 * WASD relative movement, stick swing combat, and NPC enemies.
 */

#include <graylib.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

/* Window dimensions */
#define WINDOW_WIDTH      1280
#define WINDOW_HEIGHT     900

/* Player settings */
#define PLAYER_HEIGHT     1.7f
#define MOVE_SPEED        8.0f
#define SPRINT_MULTIPLIER 1.8f
#define MOUSE_SENSITIVITY 0.003f
#define PLAYER_MAX_HEALTH 5
#define PLAYER_ATTACK_RANGE 2.5f
#define PLAYER_ATTACK_DAMAGE 1

/* World settings */
#define GROUND_SIZE       50.0f

/* Stick settings */
#define STICK_LENGTH      1.2f
#define STICK_THICKNESS   0.08f
#define SWING_SPEED       10.0f
#define MAX_SWING_ANGLE   (G_PI / 2.0f)
#define SWING_HIT_POINT   (G_PI / 2.0f)

/* NPC settings */
#define MAX_NPCS          10
#define NPC_HEALTH        3
#define NPC_SPEED         3.0f
#define NPC_ATTACK_RANGE  2.5f
#define NPC_ATTACK_DAMAGE 1
#define NPC_SWING_SPEED   8.0f
#define INITIAL_NPCS      5
#define NPC_SPAWN_MARGIN  8.0f

/* NPC entity */
typedef struct
{
    gfloat x, z;
    gfloat angle;
    gint health;
    gfloat swing_progress;
    gboolean swinging;
    gboolean hit_this_swing;
    gboolean active;
} NPC;

/* Game state */
typedef struct
{
    /* Camera angles (radians) */
    gfloat yaw;
    gfloat pitch;

    /* Position */
    gfloat pos_x;
    gfloat pos_y;
    gfloat pos_z;

    /* Player combat */
    gfloat swing_progress;
    gboolean swinging;
    gboolean hit_this_swing;
    gint health;

    /* NPCs */
    NPC npcs[MAX_NPCS];

    /* Stats */
    gint kills;

    /* State */
    gboolean cursor_locked;
    gboolean game_over;
} FPSGame;

/*
 * Spawn NPC at random position away from player.
 */
static void
spawn_npc (FPSGame *game)
{
    gint i;
    gfloat x, z;
    gfloat dx, dz, dist;
    gint attempts;

    for (i = 0; i < MAX_NPCS; i++)
    {
        if (!game->npcs[i].active)
        {
            attempts = 0;
            do
            {
                x = ((gfloat)rand () / (gfloat)RAND_MAX) * (GROUND_SIZE - 4.0f)
                    - (GROUND_SIZE - 4.0f) / 2.0f;
                z = ((gfloat)rand () / (gfloat)RAND_MAX) * (GROUND_SIZE - 4.0f)
                    - (GROUND_SIZE - 4.0f) / 2.0f;

                dx = x - game->pos_x;
                dz = z - game->pos_z;
                dist = sqrtf (dx * dx + dz * dz);
                attempts++;
            }
            while (dist < NPC_SPAWN_MARGIN && attempts < 50);

            game->npcs[i].x = x;
            game->npcs[i].z = z;
            game->npcs[i].angle = 0.0f;
            game->npcs[i].health = NPC_HEALTH;
            game->npcs[i].swing_progress = 0.0f;
            game->npcs[i].swinging = FALSE;
            game->npcs[i].hit_this_swing = FALSE;
            game->npcs[i].active = TRUE;
            return;
        }
    }
}

/*
 * Initialize game state.
 */
static void
fps_init (FPSGame *game)
{
    gint i;

    srand ((unsigned int)time (NULL));

    game->yaw = 0.0f;
    game->pitch = 0.0f;

    game->pos_x = 0.0f;
    game->pos_y = PLAYER_HEIGHT;
    game->pos_z = 10.0f;

    game->swing_progress = 0.0f;
    game->swinging = FALSE;
    game->hit_this_swing = FALSE;
    game->health = PLAYER_MAX_HEALTH;

    /* Clear NPCs */
    for (i = 0; i < MAX_NPCS; i++)
    {
        game->npcs[i].active = FALSE;
    }

    /* Spawn initial NPCs */
    for (i = 0; i < INITIAL_NPCS; i++)
    {
        spawn_npc (game);
    }

    game->kills = 0;
    game->cursor_locked = TRUE;
    game->game_over = FALSE;
}

/*
 * Update camera from mouse input.
 */
static void
fps_update_camera (FPSGame *game)
{
    g_autoptr(GrlVector2) delta = NULL;

    if (!game->cursor_locked || game->game_over)
        return;

    delta = grl_input_get_mouse_delta ();
    if (delta == NULL)
        return;

    game->yaw -= delta->x * MOUSE_SENSITIVITY;
    game->pitch -= delta->y * MOUSE_SENSITIVITY;

    /* Clamp pitch to prevent camera flip */
    if (game->pitch > G_PI / 2.0f - 0.1f)
        game->pitch = G_PI / 2.0f - 0.1f;
    if (game->pitch < -G_PI / 2.0f + 0.1f)
        game->pitch = -G_PI / 2.0f + 0.1f;
}

/*
 * Update movement from keyboard input.
 */
static void
fps_update_movement (FPSGame *game,
                     gfloat   dt)
{
    gfloat forward_x;
    gfloat forward_z;
    gfloat right_x;
    gfloat right_z;
    gfloat speed;
    gfloat move_x;
    gfloat move_z;

    if (game->game_over)
        return;

    /* Calculate direction vectors from yaw (horizontal only) */
    forward_x = sinf (game->yaw);
    forward_z = cosf (game->yaw);
    right_x = -cosf (game->yaw);  /* Fixed: flipped sign */
    right_z = sinf (game->yaw);   /* Fixed: flipped sign */

    /* Check sprint */
    speed = MOVE_SPEED;
    if (grl_input_is_key_down (GRL_KEY_LEFT_SHIFT))
        speed *= SPRINT_MULTIPLIER;

    move_x = 0.0f;
    move_z = 0.0f;

    /* WASD movement */
    if (grl_input_is_key_down (GRL_KEY_W))
    {
        move_x += forward_x;
        move_z += forward_z;
    }
    if (grl_input_is_key_down (GRL_KEY_S))
    {
        move_x -= forward_x;
        move_z -= forward_z;
    }
    if (grl_input_is_key_down (GRL_KEY_A))
    {
        move_x -= right_x;
        move_z -= right_z;
    }
    if (grl_input_is_key_down (GRL_KEY_D))
    {
        move_x += right_x;
        move_z += right_z;
    }

    /* Normalize diagonal movement */
    if (move_x != 0.0f || move_z != 0.0f)
    {
        gfloat len;

        len = sqrtf (move_x * move_x + move_z * move_z);
        move_x /= len;
        move_z /= len;

        game->pos_x += move_x * speed * dt;
        game->pos_z += move_z * speed * dt;
    }

    /* Keep player within bounds */
    if (game->pos_x < -GROUND_SIZE / 2.0f + 1.0f)
        game->pos_x = -GROUND_SIZE / 2.0f + 1.0f;
    if (game->pos_x > GROUND_SIZE / 2.0f - 1.0f)
        game->pos_x = GROUND_SIZE / 2.0f - 1.0f;
    if (game->pos_z < -GROUND_SIZE / 2.0f + 1.0f)
        game->pos_z = -GROUND_SIZE / 2.0f + 1.0f;
    if (game->pos_z > GROUND_SIZE / 2.0f - 1.0f)
        game->pos_z = GROUND_SIZE / 2.0f - 1.0f;
}

/*
 * Normalize angle to -PI to PI range.
 */
static gfloat
normalize_angle (gfloat angle)
{
    while (angle > G_PI)
        angle -= 2.0f * G_PI;
    while (angle < -G_PI)
        angle += 2.0f * G_PI;
    return angle;
}

/*
 * Check if player swing hit an NPC.
 */
static void
fps_check_player_hit (FPSGame *game)
{
    gint i;
    gfloat dx, dz, dist;
    gfloat dir_to_npc;
    gfloat angle_diff;

    if (!game->swinging || game->hit_this_swing)
        return;

    /* Check at swing peak */
    if (game->swing_progress < SWING_HIT_POINT - 0.3f ||
        game->swing_progress > SWING_HIT_POINT + 0.3f)
        return;

    for (i = 0; i < MAX_NPCS; i++)
    {
        if (!game->npcs[i].active)
            continue;

        dx = game->npcs[i].x - game->pos_x;
        dz = game->npcs[i].z - game->pos_z;
        dist = sqrtf (dx * dx + dz * dz);

        if (dist < PLAYER_ATTACK_RANGE)
        {
            /* Check if NPC is in front (within 60 degree cone) */
            dir_to_npc = atan2f (dx, dz);
            angle_diff = fabsf (normalize_angle (dir_to_npc - game->yaw));

            if (angle_diff < G_PI / 3.0f)
            {
                game->npcs[i].health -= PLAYER_ATTACK_DAMAGE;
                game->hit_this_swing = TRUE;

                if (game->npcs[i].health <= 0)
                {
                    game->npcs[i].active = FALSE;
                    game->kills++;

                    /* Spawn replacement */
                    spawn_npc (game);
                }
                return;
            }
        }
    }
}

/*
 * Update player stick swing.
 */
static void
fps_update_swing (FPSGame *game,
                  gfloat   dt)
{
    if (game->game_over)
        return;

    /* Start swing on left click */
    if (game->cursor_locked &&
        grl_input_is_mouse_button_pressed (GRL_MOUSE_BUTTON_LEFT) &&
        !game->swinging)
    {
        game->swinging = TRUE;
        game->swing_progress = 0.0f;
        game->hit_this_swing = FALSE;
    }

    /* Animate swing */
    if (game->swinging)
    {
        game->swing_progress += SWING_SPEED * dt;

        /* Check for hit */
        fps_check_player_hit (game);

        if (game->swing_progress >= G_PI)
        {
            game->swing_progress = 0.0f;
            game->swinging = FALSE;
        }
    }
}

/*
 * Update NPCs.
 */
static void
fps_update_npcs (FPSGame *game,
                 gfloat   dt)
{
    gint i;
    NPC *npc;
    gfloat dx, dz, dist;

    if (game->game_over)
        return;

    for (i = 0; i < MAX_NPCS; i++)
    {
        npc = &game->npcs[i];
        if (!npc->active)
            continue;

        dx = game->pos_x - npc->x;
        dz = game->pos_z - npc->z;
        dist = sqrtf (dx * dx + dz * dz);

        /* Face player */
        npc->angle = atan2f (dx, dz);

        if (dist < NPC_ATTACK_RANGE)
        {
            /* Attack */
            if (!npc->swinging)
            {
                npc->swinging = TRUE;
                npc->swing_progress = 0.0f;
                npc->hit_this_swing = FALSE;
            }
        }
        else
        {
            /* Move toward player */
            if (dist > 0.1f)
            {
                npc->x += (dx / dist) * NPC_SPEED * dt;
                npc->z += (dz / dist) * NPC_SPEED * dt;
            }
        }

        /* Update swing */
        if (npc->swinging)
        {
            npc->swing_progress += NPC_SWING_SPEED * dt;

            /* Check hit at peak */
            if (!npc->hit_this_swing &&
                npc->swing_progress >= SWING_HIT_POINT - 0.3f &&
                npc->swing_progress <= SWING_HIT_POINT + 0.3f)
            {
                if (dist < NPC_ATTACK_RANGE)
                {
                    game->health -= NPC_ATTACK_DAMAGE;
                    npc->hit_this_swing = TRUE;

                    if (game->health <= 0)
                    {
                        game->game_over = TRUE;
                    }
                }
            }

            if (npc->swing_progress >= G_PI)
            {
                npc->swing_progress = 0.0f;
                npc->swinging = FALSE;
            }
        }
    }
}

/*
 * Apply camera state to GrlCamera3D.
 */
static void
fps_apply_camera (FPSGame     *game,
                  GrlCamera3D *camera)
{
    gfloat target_x;
    gfloat target_y;
    gfloat target_z;

    /* Calculate target from spherical coordinates */
    target_x = game->pos_x + cosf (game->pitch) * sinf (game->yaw);
    target_y = game->pos_y + sinf (game->pitch);
    target_z = game->pos_z + cosf (game->pitch) * cosf (game->yaw);

    grl_camera3d_set_position_xyz (camera, game->pos_x, game->pos_y, game->pos_z);
    grl_camera3d_set_target_xyz (camera, target_x, target_y, target_z);
}

/*
 * Draw the map structures.
 */
static void
fps_draw_map (void)
{
    g_autoptr(GrlColor) ground_color = grl_color_new (60, 120, 60, 255);
    g_autoptr(GrlColor) pillar_color = grl_color_new (150, 100, 70, 255);
    g_autoptr(GrlColor) tower_color = grl_color_new (100, 100, 120, 255);
    g_autoptr(GrlColor) arch_color = grl_color_new (180, 160, 140, 255);
    g_autoptr(GrlColor) grid_color = grl_color_new (50, 100, 50, 255);
    g_autoptr(GrlVector3) pos = NULL;
    g_autoptr(GrlVector2) size = NULL;
    gint i;

    /* Ground plane */
    pos = grl_vector3_new (0.0f, 0.0f, 0.0f);
    size = grl_vector2_new (GROUND_SIZE, GROUND_SIZE);
    grl_draw_plane (pos, size, ground_color);
    g_clear_pointer (&pos, grl_vector3_free);
    g_clear_pointer (&size, grl_vector2_free);

    /* Ground grid */
    for (i = -25; i <= 25; i += 5)
    {
        g_autoptr(GrlVector3) start = NULL;
        g_autoptr(GrlVector3) end = NULL;

        start = grl_vector3_new ((gfloat)i, 0.01f, -25.0f);
        end = grl_vector3_new ((gfloat)i, 0.01f, 25.0f);
        grl_draw_line_3d (start, end, grid_color);

        g_clear_pointer (&start, grl_vector3_free);
        g_clear_pointer (&end, grl_vector3_free);

        start = grl_vector3_new (-25.0f, 0.01f, (gfloat)i);
        end = grl_vector3_new (25.0f, 0.01f, (gfloat)i);
        grl_draw_line_3d (start, end, grid_color);
    }

    /* Center pillar */
    pos = grl_vector3_new (0.0f, 3.0f, 0.0f);
    grl_draw_cube (pos, 2.0f, 6.0f, 2.0f, pillar_color);
    g_clear_pointer (&pos, grl_vector3_free);

    /* Corner towers */
    pos = grl_vector3_new (-15.0f, 2.5f, -15.0f);
    grl_draw_cube (pos, 3.0f, 5.0f, 3.0f, tower_color);
    g_clear_pointer (&pos, grl_vector3_free);

    pos = grl_vector3_new (15.0f, 2.5f, -15.0f);
    grl_draw_cube (pos, 3.0f, 5.0f, 3.0f, tower_color);
    g_clear_pointer (&pos, grl_vector3_free);

    pos = grl_vector3_new (-15.0f, 2.5f, 15.0f);
    grl_draw_cube (pos, 3.0f, 5.0f, 3.0f, tower_color);
    g_clear_pointer (&pos, grl_vector3_free);

    pos = grl_vector3_new (15.0f, 2.5f, 15.0f);
    grl_draw_cube (pos, 3.0f, 5.0f, 3.0f, tower_color);
    g_clear_pointer (&pos, grl_vector3_free);

    /* Archway */
    pos = grl_vector3_new (-8.0f, 2.0f, 0.0f);
    grl_draw_cube (pos, 1.5f, 4.0f, 1.5f, arch_color);
    g_clear_pointer (&pos, grl_vector3_free);

    pos = grl_vector3_new (8.0f, 2.0f, 0.0f);
    grl_draw_cube (pos, 1.5f, 4.0f, 1.5f, arch_color);
    g_clear_pointer (&pos, grl_vector3_free);

    pos = grl_vector3_new (0.0f, 4.5f, 0.0f);
    grl_draw_cube (pos, 18.0f, 1.0f, 1.5f, arch_color);
    g_clear_pointer (&pos, grl_vector3_free);

    /* Ramp */
    pos = grl_vector3_new (0.0f, 0.75f, -12.0f);
    grl_draw_cube (pos, 4.0f, 1.5f, 6.0f, pillar_color);
    g_clear_pointer (&pos, grl_vector3_free);
}

/*
 * Draw NPCs.
 */
static void
fps_draw_npcs (FPSGame *game)
{
    g_autoptr(GrlColor) body_color = grl_color_new (180, 80, 80, 255);
    g_autoptr(GrlColor) head_color = grl_color_new (220, 180, 160, 255);
    g_autoptr(GrlColor) stick_color = grl_color_new (100, 70, 40, 255);
    gint i;
    NPC *npc;

    for (i = 0; i < MAX_NPCS; i++)
    {
        g_autoptr(GrlVector3) body_pos = NULL;
        g_autoptr(GrlVector3) head_pos = NULL;
        g_autoptr(GrlVector3) stick_pos = NULL;
        gfloat swing_offset;
        gfloat stick_x, stick_z;

        npc = &game->npcs[i];
        if (!npc->active)
            continue;

        /* Body */
        body_pos = grl_vector3_new (npc->x, 0.8f, npc->z);
        grl_draw_cube (body_pos, 0.6f, 1.4f, 0.4f, body_color);

        /* Head */
        head_pos = grl_vector3_new (npc->x, 1.7f, npc->z);
        grl_draw_sphere (head_pos, 0.25f, head_color);

        /* Stick */
        swing_offset = 0.0f;
        if (npc->swinging)
        {
            swing_offset = sinf (npc->swing_progress) * 0.8f;
        }

        stick_x = npc->x + sinf (npc->angle) * 0.5f;
        stick_z = npc->z + cosf (npc->angle) * 0.5f;
        stick_pos = grl_vector3_new (stick_x, 1.0f - swing_offset * 0.5f, stick_z);
        grl_draw_cube (stick_pos, 0.1f, 1.0f, 0.1f, stick_color);
    }
}

/*
 * Draw the player's stick (first-person weapon view).
 */
static void
fps_draw_stick (FPSGame *game)
{
    g_autoptr(GrlColor) stick_color = grl_color_new (139, 90, 43, 255);
    g_autoptr(GrlColor) stick_dark = grl_color_new (100, 65, 30, 255);
    g_autoptr(GrlVector3) stick_pos = NULL;
    gfloat swing_angle;
    gfloat offset_x;
    gfloat offset_y;
    gfloat offset_z;
    gfloat forward_x;
    gfloat forward_y;
    gfloat forward_z;
    gfloat right_x;
    gfloat right_z;

    /* Calculate swing angle using sine for smooth motion */
    swing_angle = 0.0f;
    if (game->swinging)
    {
        swing_angle = sinf (game->swing_progress) * MAX_SWING_ANGLE;
    }

    /* Calculate camera forward and right vectors */
    forward_x = cosf (game->pitch) * sinf (game->yaw);
    forward_y = sinf (game->pitch);
    forward_z = cosf (game->pitch) * cosf (game->yaw);

    right_x = cosf (game->yaw);
    right_z = -sinf (game->yaw);

    /* Position stick to lower-right of view */
    offset_x = 0.3f;
    offset_y = -0.3f;
    offset_z = 0.8f;

    /* Apply swing rotation */
    offset_y -= sinf (swing_angle) * 0.5f;
    offset_z -= (1.0f - cosf (swing_angle)) * 0.3f;

    /* Calculate world position */
    stick_pos = grl_vector3_new (
        game->pos_x + right_x * offset_x + forward_x * offset_z,
        game->pos_y + offset_y + forward_y * offset_z,
        game->pos_z + right_z * offset_x + forward_z * offset_z
    );

    /* Draw stick */
    grl_draw_cube (stick_pos, STICK_THICKNESS, STICK_LENGTH, STICK_THICKNESS,
                   stick_color);
    grl_draw_cube_wires (stick_pos, STICK_THICKNESS, STICK_LENGTH, STICK_THICKNESS,
                         stick_dark);
}

/*
 * Draw 2D UI overlay.
 */
static void
fps_draw_ui (FPSGame *game)
{
    g_autoptr(GrlColor) white = grl_color_new (255, 255, 255, 255);
    g_autoptr(GrlColor) gray = grl_color_new (150, 150, 150, 255);
    g_autoptr(GrlColor) crosshair_color = grl_color_new (255, 255, 255, 180);
    g_autoptr(GrlColor) health_bg = grl_color_new (60, 20, 20, 255);
    g_autoptr(GrlColor) health_fg = grl_color_new (200, 50, 50, 255);
    g_autoptr(GrlColor) yellow = grl_color_new (255, 220, 50, 255);
    gchar text[64];
    gint health_width;
    gint text_width;

    /* Crosshair */
    grl_draw_line (WINDOW_WIDTH / 2 - 10, WINDOW_HEIGHT / 2,
                   WINDOW_WIDTH / 2 + 10, WINDOW_HEIGHT / 2, crosshair_color);
    grl_draw_line (WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2 - 10,
                   WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2 + 10, crosshair_color);

    /* Health label */
    grl_draw_text ("HEALTH", 20, 20, 16, gray);

    /* Health bar */
    grl_draw_rectangle (20, 40, 200, 20, health_bg);
    health_width = (gint)(200.0f * ((gfloat)game->health / (gfloat)PLAYER_MAX_HEALTH));
    if (health_width > 0)
        grl_draw_rectangle (20, 40, health_width, 20, health_fg);

    /* Kill counter */
    g_snprintf (text, sizeof (text), "Kills: %d", game->kills);
    grl_draw_text (text, 20, 70, 20, white);

    /* Controls help */
    grl_draw_text ("WASD: Move  Mouse: Look  LMB: Swing  ESC: Unlock",
                   20, WINDOW_HEIGHT - 30, 14, gray);

    /* Cursor lock indicator */
    if (!game->cursor_locked && !game->game_over)
    {
        grl_draw_text ("CURSOR UNLOCKED - Click to lock",
                       WINDOW_WIDTH / 2 - 120, 50, 18, white);
    }

    /* Game over overlay */
    if (game->game_over)
    {
        text_width = grl_measure_text ("GAME OVER", 60);
        grl_draw_text ("GAME OVER", WINDOW_WIDTH / 2 - text_width / 2,
                       WINDOW_HEIGHT / 2 - 100, 60, yellow);

        g_snprintf (text, sizeof (text), "You killed %d enemies!", game->kills);
        text_width = grl_measure_text (text, 30);
        grl_draw_text (text, WINDOW_WIDTH / 2 - text_width / 2,
                       WINDOW_HEIGHT / 2 - 20, 30, white);

        grl_draw_text ("Press SPACE to restart", WINDOW_WIDTH / 2 - 100,
                       WINDOW_HEIGHT / 2 + 40, 18, gray);
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
    FPSGame game;
    gfloat dt;

    app = grl_application_new ("com.example.firstperson-demo");
    window = grl_window_new (WINDOW_WIDTH, WINDOW_HEIGHT, "First-Person Combat Demo");
    bg_color = grl_color_new (135, 180, 230, 255);

    /* Setup 3D camera */
    camera = grl_camera3d_new ();
    grl_camera3d_set_fovy (camera, 70.0f);

    grl_window_set_target_fps (window, 60);

    fps_init (&game);

    /* Lock cursor on start */
    grl_window_disable_cursor (window);

    while (!grl_window_should_close (window))
    {
        dt = grl_window_get_frame_time (window);

        grl_window_poll_input (window);

        /* Restart on SPACE when game over */
        if (game.game_over && grl_input_is_key_pressed (GRL_KEY_SPACE))
        {
            fps_init (&game);
            grl_window_disable_cursor (window);
        }

        /* ESC toggles cursor lock */
        if (grl_input_is_key_pressed (GRL_KEY_ESCAPE))
        {
            game.cursor_locked = !game.cursor_locked;
            if (game.cursor_locked)
                grl_window_disable_cursor (window);
            else
                grl_window_enable_cursor (window);
        }

        /* Click to re-lock cursor */
        if (!game.cursor_locked && !game.game_over &&
            grl_input_is_mouse_button_pressed (GRL_MOUSE_BUTTON_LEFT))
        {
            game.cursor_locked = TRUE;
            grl_window_disable_cursor (window);
        }

        /* Update */
        fps_update_camera (&game);
        fps_update_movement (&game, dt);
        fps_update_swing (&game, dt);
        fps_update_npcs (&game, dt);
        fps_apply_camera (&game, camera);

        /* Draw */
        grl_window_begin_drawing (window);
        grl_draw_clear_background (bg_color);

        grl_camera3d_begin (camera);
        fps_draw_map ();
        fps_draw_npcs (&game);
        fps_draw_stick (&game);
        grl_camera3d_end (camera);

        fps_draw_ui (&game);

        grl_window_end_drawing (window);
    }

    return 0;
}
