/* basic-window.c
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Basic window example demonstrating Graylib usage.
 *
 * This example shows how to:
 * - Create a window
 * - Set up a basic game loop
 * - Clear the background with a color
 * - Draw simple text-like content (FPS counter)
 */

#include <graylib.h>
#include <stdio.h>

int
main (int   argc,
      char *argv[])
{
    g_autoptr(GrlWindow) window = NULL;
    g_autoptr(GrlColor) bg_color = NULL;
    g_autoptr(GrlColor) fg_color = NULL;
    g_autoptr(GrlVector2) center = NULL;
    gchar fps_text[32];
    gint frame_count;

    /* Create the window */
    window = grl_window_new (800, 600, "Graylib - Basic Window Example");

    if (!grl_window_is_ready (window))
    {
        g_printerr ("Failed to create window\n");
        return 1;
    }

    /* Set target FPS */
    grl_window_set_target_fps (window, 60);

    /* Create colors */
    bg_color = grl_color_new (40, 40, 60, 255);
    fg_color = grl_color_new_raywhite ();

    frame_count = 0;

    /* Main game loop */
    while (!grl_window_should_close (window))
    {
        gfloat delta_time;
        gint fps;
        gint width, height;

        /* Get timing info */
        delta_time = grl_window_get_frame_time (window);
        fps = grl_window_get_fps (window);
        frame_count++;

        /* Get window dimensions */
        width = grl_window_get_width (window);
        height = grl_window_get_height (window);

        /* Calculate center for drawing */
        if (center != NULL)
        {
            grl_vector2_free (center);
        }
        center = grl_vector2_new ((gfloat)width / 2.0f, (gfloat)height / 2.0f);

        /* Begin drawing */
        grl_window_begin_drawing (window);

        /* Clear background */
        grl_window_clear_background (window, bg_color);

        /* Draw a circle at the center */
        grl_draw_circle_v (center, 100.0f, fg_color);

        /* Draw some rectangles around the edges */
        {
            g_autoptr(GrlColor) red = grl_color_new_red ();
            g_autoptr(GrlColor) green = grl_color_new_green ();
            g_autoptr(GrlColor) blue = grl_color_new_blue ();
            g_autoptr(GrlColor) yellow = grl_color_new_yellow ();

            /* Corner rectangles */
            grl_draw_rectangle (10, 10, 50, 50, red);
            grl_draw_rectangle (width - 60, 10, 50, 50, green);
            grl_draw_rectangle (10, height - 60, 50, 50, blue);
            grl_draw_rectangle (width - 60, height - 60, 50, 50, yellow);
        }

        /* Draw a rotating polygon at center */
        {
            g_autoptr(GrlColor) poly_color = grl_color_new (200, 100, 255, 180);
            gfloat rotation = (gfloat)frame_count * 0.5f;

            grl_draw_poly (center, 6, 80.0f, rotation, poly_color);
        }

        /* End drawing */
        grl_window_end_drawing (window);

        /* Print FPS to terminal occasionally */
        if (frame_count % 60 == 0)
        {
            g_print ("FPS: %d, Delta: %.4f\n", fps, delta_time);
        }
    }

    g_print ("Window closed after %d frames\n", frame_count);

    return 0;
}
