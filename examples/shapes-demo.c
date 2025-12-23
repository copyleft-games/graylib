/* shapes-demo.c
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Shapes demonstration example.
 *
 * This example demonstrates various shape drawing functions
 * available in Graylib.
 */

#include <graylib.h>
#include <math.h>

int
main (int   argc,
      char *argv[])
{
    g_autoptr(GrlWindow) window = NULL;
    g_autoptr(GrlColor) bg_color = NULL;
    gint frame_count;

    /* Create the window */
    window = grl_window_new (1024, 768, "Graylib - Shapes Demo");

    if (!grl_window_is_ready (window))
    {
        g_printerr ("Failed to create window\n");
        return 1;
    }

    grl_window_set_target_fps (window, 60);

    /* Background color */
    bg_color = grl_color_new (30, 30, 40, 255);

    frame_count = 0;

    /* Main loop */
    while (!grl_window_should_close (window))
    {
        gfloat time;
        gint row_y;

        time = (gfloat)grl_window_get_time (window);
        frame_count++;

        grl_window_begin_drawing (window);
        grl_window_clear_background (window, bg_color);

        row_y = 50;

        /* Row 1: Basic shapes */
        {
            g_autoptr(GrlColor) white = grl_color_new_white ();
            g_autoptr(GrlColor) red = grl_color_new_red ();
            g_autoptr(GrlColor) green = grl_color_new_green ();
            g_autoptr(GrlColor) blue = grl_color_new_blue ();
            g_autoptr(GrlColor) yellow = grl_color_new_yellow ();

            /* Rectangle filled */
            grl_draw_rectangle (50, row_y, 80, 60, red);

            /* Rectangle outline */
            grl_draw_rectangle_lines (150, row_y, 80, 60, green);

            /* Circle filled */
            grl_draw_circle (290, row_y + 30, 30.0f, blue);

            /* Circle outline */
            grl_draw_circle_lines (370, row_y + 30, 30.0f, yellow);

            /* Line */
            {
                g_autoptr(GrlVector2) start = grl_vector2_new (420.0f, (gfloat)(row_y + 60));
                g_autoptr(GrlVector2) end = grl_vector2_new (500.0f, (gfloat)row_y);

                grl_draw_line_ex (start, end, 3.0f, white);
            }
        }

        row_y += 100;

        /* Row 2: Rounded rectangles */
        {
            g_autoptr(GrlColor) c1 = grl_color_new (255, 100, 100, 255);
            g_autoptr(GrlColor) c2 = grl_color_new (100, 255, 100, 255);
            g_autoptr(GrlColor) c3 = grl_color_new (100, 100, 255, 255);

            g_autoptr(GrlRectangle) r1 = grl_rectangle_new (50.0f, (gfloat)row_y, 100.0f, 60.0f);
            g_autoptr(GrlRectangle) r2 = grl_rectangle_new (170.0f, (gfloat)row_y, 100.0f, 60.0f);
            g_autoptr(GrlRectangle) r3 = grl_rectangle_new (290.0f, (gfloat)row_y, 100.0f, 60.0f);

            grl_draw_rectangle_rounded (r1, 0.2f, 4, c1);
            grl_draw_rectangle_rounded (r2, 0.5f, 8, c2);
            grl_draw_rectangle_rounded_lines (r3, 0.8f, 12, c3);
        }

        row_y += 100;

        /* Row 3: Gradients */
        {
            g_autoptr(GrlColor) top = grl_color_new (255, 0, 0, 255);
            g_autoptr(GrlColor) bottom = grl_color_new (0, 0, 255, 255);
            g_autoptr(GrlColor) left = grl_color_new (255, 255, 0, 255);
            g_autoptr(GrlColor) right = grl_color_new (0, 255, 255, 255);
            g_autoptr(GrlColor) inner = grl_color_new (255, 255, 255, 255);
            g_autoptr(GrlColor) outer = grl_color_new (0, 0, 0, 255);

            /* Vertical gradient */
            grl_draw_rectangle_gradient_v (50, row_y, 100, 60, top, bottom);

            /* Horizontal gradient */
            grl_draw_rectangle_gradient_h (170, row_y, 100, 60, left, right);

            /* Circle gradient */
            grl_draw_circle_gradient (340, row_y + 30, 40.0f, inner, outer);
        }

        row_y += 100;

        /* Row 4: Triangles and polygons */
        {
            g_autoptr(GrlColor) magenta = grl_color_new_magenta ();
            g_autoptr(GrlColor) cyan = grl_color_new_cyan ();
            g_autoptr(GrlColor) orange = grl_color_new (255, 165, 0, 255);

            /* Triangle */
            {
                g_autoptr(GrlVector2) v1 = grl_vector2_new (100.0f, (gfloat)(row_y + 60));
                g_autoptr(GrlVector2) v2 = grl_vector2_new (50.0f, (gfloat)row_y);
                g_autoptr(GrlVector2) v3 = grl_vector2_new (150.0f, (gfloat)row_y);

                grl_draw_triangle (v1, v2, v3, magenta);
            }

            /* Pentagon */
            {
                g_autoptr(GrlVector2) center = grl_vector2_new (230.0f, (gfloat)(row_y + 30));
                gfloat rotation = time * 30.0f;

                grl_draw_poly (center, 5, 35.0f, rotation, cyan);
            }

            /* Hexagon outline */
            {
                g_autoptr(GrlVector2) center = grl_vector2_new (330.0f, (gfloat)(row_y + 30));
                gfloat rotation = -time * 20.0f;

                grl_draw_poly_lines_ex (center, 6, 35.0f, rotation, 2.0f, orange);
            }
        }

        row_y += 100;

        /* Row 5: Ellipses and rings */
        {
            g_autoptr(GrlColor) purple = grl_color_new (180, 80, 220, 255);
            g_autoptr(GrlColor) teal = grl_color_new (0, 180, 180, 255);
            g_autoptr(GrlColor) gold = grl_color_new (255, 215, 0, 255);

            /* Ellipse */
            grl_draw_ellipse (100, row_y + 30, 60.0f, 25.0f, purple);

            /* Ellipse outline */
            grl_draw_ellipse_lines (230, row_y + 30, 50.0f, 30.0f, teal);

            /* Ring */
            {
                g_autoptr(GrlVector2) center = grl_vector2_new (360.0f, (gfloat)(row_y + 30));
                gfloat start_angle = time * 50.0f;
                gfloat end_angle = start_angle + 270.0f;

                grl_draw_ring (center, 20.0f, 40.0f, start_angle, end_angle, 16, gold);
            }
        }

        row_y += 100;

        /* Row 6: Sectors */
        {
            g_autoptr(GrlColor) pink = grl_color_new (255, 105, 180, 255);
            g_autoptr(GrlColor) lime = grl_color_new (50, 205, 50, 255);

            /* Circle sector (pie slice) */
            {
                g_autoptr(GrlVector2) center = grl_vector2_new (100.0f, (gfloat)(row_y + 35));
                gfloat start = 0.0f;
                gfloat end = 135.0f + sinf (time) * 45.0f;

                grl_draw_circle_sector (center, 40.0f, start, end, 16, pink);
            }

            /* Circle sector outline */
            {
                g_autoptr(GrlVector2) center = grl_vector2_new (200.0f, (gfloat)(row_y + 35));
                gfloat start = time * 40.0f;
                gfloat end = start + 90.0f;

                grl_draw_circle_sector_lines (center, 40.0f, start, end, 12, lime);
            }
        }

        grl_window_end_drawing (window);
    }

    return 0;
}
