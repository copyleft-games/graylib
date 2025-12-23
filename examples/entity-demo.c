/* entity-demo.c
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Entity system demonstration for Graylib.
 *
 * This example shows how to:
 * - Create and manipulate sprites
 * - Use the entity base class properties
 * - Handle collision detection between entities
 * - Use animated textures with spritesheets
 * - Implement basic game object behavior
 *
 * Controls:
 * - Arrow keys: Move the player sprite
 * - A/D: Rotate the player
 * - W/S: Scale the player up/down
 * - SPACE: Toggle collision enabled
 * - ESC: Exit
 */

#include <graylib.h>
#include <stdio.h>
#include <math.h>

/* Constants */
#define PLAYER_SPEED    200.0f
#define ROTATION_SPEED  90.0f
#define SCALE_SPEED     0.5f
#define MIN_SCALE       0.5f
#define MAX_SCALE       3.0f

/* Simple box structure for demo (when no texture is available) */
typedef struct
{
    GrlEntity *entity;
    GrlColor  *color;
    gboolean   is_colliding;
} DemoBox;

static DemoBox *
demo_box_new (gfloat    x,
              gfloat    y,
              gfloat    width,
              gfloat    height,
              GrlColor *color)
{
    DemoBox *box;

    box = g_new0 (DemoBox, 1);
    box->entity = grl_entity_new_at (x, y);
    box->color = grl_color_copy (color);
    box->is_colliding = FALSE;

    grl_entity_set_width (box->entity, width);
    grl_entity_set_height (box->entity, height);

    return box;
}

static void
demo_box_free (DemoBox *box)
{
    if (box == NULL)
        return;

    g_clear_object (&box->entity);
    g_clear_object (&box->color);
    g_free (box);
}

static void
demo_box_draw (DemoBox *box)
{
    g_autoptr(GrlRectangle) bounds = NULL;
    g_autoptr(GrlColor) draw_color = NULL;
    gfloat x, y, width, height;

    if (box == NULL || box->entity == NULL)
        return;

    if (!grl_entity_get_visible (box->entity))
        return;

    bounds = grl_collidable_get_bounds (GRL_COLLIDABLE (box->entity));
    x = grl_rectangle_get_x (bounds);
    y = grl_rectangle_get_y (bounds);
    width = grl_rectangle_get_width (bounds);
    height = grl_rectangle_get_height (bounds);

    /* Draw with collision highlight */
    if (box->is_colliding)
    {
        draw_color = grl_color_new (255, 100, 100, 255);
    }
    else
    {
        draw_color = grl_color_copy (box->color);
    }

    grl_draw_rectangle ((gint)x, (gint)y, (gint)width, (gint)height, draw_color);

    /* Draw outline for collision-enabled objects */
    if (grl_collidable_get_collision_enabled (GRL_COLLIDABLE (box->entity)))
    {
        g_autoptr(GrlColor) outline = grl_color_new_raywhite ();

        grl_draw_rectangle_lines ((gint)x, (gint)y, (gint)width, (gint)height, outline);
    }
}

int
main (int   argc,
      char *argv[])
{
    g_autoptr(GrlWindow) window = NULL;
    g_autoptr(GrlColor) bg_color = NULL;
    g_autoptr(GrlColor) text_color = NULL;
    g_autoptr(GrlColor) player_color = NULL;
    g_autoptr(GrlColor) enemy_color = NULL;
    g_autoptr(GrlColor) obstacle_color = NULL;

    DemoBox *player = NULL;
    DemoBox *enemies[4] = { NULL };
    DemoBox *obstacles[3] = { NULL };
    gint num_enemies = 4;
    gint num_obstacles = 3;
    gint i;

    gfloat enemy_speeds[4] = { 80.0f, 100.0f, 60.0f, 120.0f };
    gfloat enemy_dirs[4] = { 1.0f, -1.0f, 1.0f, -1.0f };

    /* Create the window */
    window = grl_window_new (900, 600, "Graylib - Entity System Demo");

    if (!grl_window_is_ready (window))
    {
        g_printerr ("Failed to create window\n");
        return 1;
    }

    grl_window_set_target_fps (window, 60);

    /* Create colors */
    bg_color = grl_color_new (25, 25, 40, 255);
    text_color = grl_color_new_raywhite ();
    player_color = grl_color_new (100, 200, 100, 255);
    enemy_color = grl_color_new (200, 100, 100, 255);
    obstacle_color = grl_color_new (100, 100, 200, 255);

    /* Create player */
    player = demo_box_new (400.0f, 300.0f, 50.0f, 50.0f, player_color);
    grl_entity_set_tag (player->entity, "player");
    grl_entity_center_origin (player->entity);

    /* Create enemies (moving horizontally) */
    enemies[0] = demo_box_new (100.0f, 100.0f, 40.0f, 40.0f, enemy_color);
    enemies[1] = demo_box_new (700.0f, 200.0f, 45.0f, 45.0f, enemy_color);
    enemies[2] = demo_box_new (200.0f, 450.0f, 35.0f, 35.0f, enemy_color);
    enemies[3] = demo_box_new (600.0f, 500.0f, 50.0f, 30.0f, enemy_color);

    for (i = 0; i < num_enemies; i++)
    {
        grl_entity_set_tag (enemies[i]->entity, "enemy");
    }

    /* Create static obstacles */
    obstacles[0] = demo_box_new (300.0f, 150.0f, 80.0f, 80.0f, obstacle_color);
    obstacles[1] = demo_box_new (500.0f, 350.0f, 100.0f, 60.0f, obstacle_color);
    obstacles[2] = demo_box_new (150.0f, 300.0f, 60.0f, 100.0f, obstacle_color);

    for (i = 0; i < num_obstacles; i++)
    {
        grl_entity_set_tag (obstacles[i]->entity, "obstacle");
    }

    /* Print controls */
    g_print ("\n=== Entity Demo Controls ===\n");
    g_print ("Arrow keys  - Move player\n");
    g_print ("A/D         - Rotate player\n");
    g_print ("W/S         - Scale player\n");
    g_print ("SPACE       - Toggle collision\n");
    g_print ("ESC         - Exit\n");
    g_print ("============================\n\n");

    /* Main game loop */
    while (!grl_window_should_close (window))
    {
        gfloat delta;
        gfloat player_x, player_y;
        gfloat player_rotation, player_scale;
        gboolean collision_enabled;
        gint collision_count;
        gint width, height;

        delta = grl_window_get_frame_time (window);
        width = grl_window_get_width (window);
        height = grl_window_get_height (window);

        /* Reset collision states */
        player->is_colliding = FALSE;

        for (i = 0; i < num_enemies; i++)
        {
            enemies[i]->is_colliding = FALSE;
        }

        for (i = 0; i < num_obstacles; i++)
        {
            obstacles[i]->is_colliding = FALSE;
        }

        /*
         * Handle player input
         */

        player_x = grl_entity_get_x (player->entity);
        player_y = grl_entity_get_y (player->entity);
        player_rotation = grl_entity_get_rotation (player->entity);
        player_scale = grl_entity_get_scale (player->entity);

        /* Movement */
        if (grl_input_is_key_down (GRL_KEY_UP))
        {
            player_y -= PLAYER_SPEED * delta;
        }

        if (grl_input_is_key_down (GRL_KEY_DOWN))
        {
            player_y += PLAYER_SPEED * delta;
        }

        if (grl_input_is_key_down (GRL_KEY_LEFT))
        {
            player_x -= PLAYER_SPEED * delta;
        }

        if (grl_input_is_key_down (GRL_KEY_RIGHT))
        {
            player_x += PLAYER_SPEED * delta;
        }

        /* Rotation */
        if (grl_input_is_key_down (GRL_KEY_A))
        {
            player_rotation -= ROTATION_SPEED * delta;
        }

        if (grl_input_is_key_down (GRL_KEY_D))
        {
            player_rotation += ROTATION_SPEED * delta;
        }

        /* Scale */
        if (grl_input_is_key_down (GRL_KEY_W))
        {
            player_scale += SCALE_SPEED * delta;

            if (player_scale > MAX_SCALE)
                player_scale = MAX_SCALE;
        }

        if (grl_input_is_key_down (GRL_KEY_S))
        {
            player_scale -= SCALE_SPEED * delta;

            if (player_scale < MIN_SCALE)
                player_scale = MIN_SCALE;
        }

        /* Keep player in bounds */
        if (player_x < 0) player_x = 0;
        if (player_y < 0) player_y = 0;
        if (player_x > width) player_x = (gfloat)width;
        if (player_y > height) player_y = (gfloat)height;

        grl_entity_set_position_xy (player->entity, player_x, player_y);
        grl_entity_set_rotation (player->entity, player_rotation);
        grl_entity_set_scale (player->entity, player_scale);

        /* Toggle collision */
        if (grl_input_is_key_pressed (GRL_KEY_SPACE))
        {
            collision_enabled = grl_collidable_get_collision_enabled (
                GRL_COLLIDABLE (player->entity));
            grl_collidable_set_collision_enabled (
                GRL_COLLIDABLE (player->entity), !collision_enabled);
        }

        /*
         * Update enemies (simple horizontal movement)
         */
        for (i = 0; i < num_enemies; i++)
        {
            gfloat ex, ey, ew;

            ex = grl_entity_get_x (enemies[i]->entity);
            ey = grl_entity_get_y (enemies[i]->entity);
            ew = grl_entity_get_width (enemies[i]->entity);

            ex += enemy_speeds[i] * enemy_dirs[i] * delta;

            /* Bounce off screen edges */
            if (ex < 0)
            {
                ex = 0;
                enemy_dirs[i] = 1.0f;
            }
            else if (ex + ew > width)
            {
                ex = (gfloat)width - ew;
                enemy_dirs[i] = -1.0f;
            }

            grl_entity_set_x (enemies[i]->entity, ex);
        }

        /*
         * Check collisions
         */
        collision_count = 0;

        /* Player vs enemies */
        for (i = 0; i < num_enemies; i++)
        {
            if (grl_collidable_check_collision (
                    GRL_COLLIDABLE (player->entity),
                    GRL_COLLIDABLE (enemies[i]->entity)))
            {
                player->is_colliding = TRUE;
                enemies[i]->is_colliding = TRUE;
                collision_count++;
            }
        }

        /* Player vs obstacles */
        for (i = 0; i < num_obstacles; i++)
        {
            if (grl_collidable_check_collision (
                    GRL_COLLIDABLE (player->entity),
                    GRL_COLLIDABLE (obstacles[i]->entity)))
            {
                player->is_colliding = TRUE;
                obstacles[i]->is_colliding = TRUE;
                collision_count++;
            }
        }

        /*
         * Draw
         */
        grl_window_begin_drawing (window);
        grl_window_clear_background (window, bg_color);

        /* Draw obstacles first (lowest z-index) */
        for (i = 0; i < num_obstacles; i++)
        {
            demo_box_draw (obstacles[i]);
        }

        /* Draw enemies */
        for (i = 0; i < num_enemies; i++)
        {
            demo_box_draw (enemies[i]);
        }

        /* Draw player (on top) */
        demo_box_draw (player);

        /* Draw UI */
        {
            gchar info_text[256];
            gint y_pos = 10;

            grl_draw_text ("ENTITY SYSTEM DEMO", 10, y_pos, 24, text_color);
            y_pos += 35;

            collision_enabled = grl_collidable_get_collision_enabled (
                GRL_COLLIDABLE (player->entity));

            g_snprintf (info_text, sizeof (info_text),
                        "Player Position: (%.1f, %.1f)",
                        grl_entity_get_x (player->entity),
                        grl_entity_get_y (player->entity));
            grl_draw_text (info_text, 10, y_pos, 18, text_color);
            y_pos += 22;

            g_snprintf (info_text, sizeof (info_text),
                        "Rotation: %.1f deg  Scale: %.2f",
                        grl_entity_get_rotation (player->entity),
                        grl_entity_get_scale (player->entity));
            grl_draw_text (info_text, 10, y_pos, 18, text_color);
            y_pos += 22;

            g_snprintf (info_text, sizeof (info_text),
                        "Collision Enabled: %s",
                        collision_enabled ? "YES" : "NO");
            grl_draw_text (info_text, 10, y_pos, 18, text_color);
            y_pos += 22;

            g_snprintf (info_text, sizeof (info_text),
                        "Collisions: %d", collision_count);
            grl_draw_text (info_text, 10, y_pos, 18, text_color);
            y_pos += 35;

            /* Legend */
            {
                g_autoptr(GrlColor) dim_color = grl_color_new (150, 150, 150, 255);

                grl_draw_text ("Arrows=Move  A/D=Rotate  W/S=Scale  SPACE=Toggle Collision",
                               10, height - 25, 14, dim_color);
            }
        }

        grl_draw_fps (width - 80, 10);

        grl_window_end_drawing (window);
    }

    /* Cleanup */
    demo_box_free (player);

    for (i = 0; i < num_enemies; i++)
    {
        demo_box_free (enemies[i]);
    }

    for (i = 0; i < num_obstacles; i++)
    {
        demo_box_free (obstacles[i]);
    }

    g_print ("Entity demo finished\n");

    return 0;
}
