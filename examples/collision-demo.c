/* collision-demo.c
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Demonstrates 2D collision detection functions.
 * Shows point vs shape, shape vs shape collisions with visual feedback.
 */

#include <graylib.h>
#include <math.h>

/*
 * Collision demo - 2D collision detection visualization
 *
 * Move the mouse to test point collisions.
 * Shows rectangle, circle, and triangle collision tests.
 */

int
main (int    argc,
      char **argv)
{
    g_autoptr(GrlApplication) app = NULL;
    g_autoptr(GrlWindow) window = NULL;
    g_autoptr(GrlColor) bg_color = NULL;
    g_autoptr(GrlColor) white = NULL;
    g_autoptr(GrlColor) green = NULL;
    g_autoptr(GrlColor) red = NULL;
    g_autoptr(GrlColor) blue = NULL;
    g_autoptr(GrlColor) yellow = NULL;
    g_autoptr(GrlColor) gray = NULL;
    g_autoptr(GrlColor) dark_gray = NULL;

    /* Test shapes */
    g_autoptr(GrlRectangle) rect1 = NULL;
    g_autoptr(GrlRectangle) rect2 = NULL;
    g_autoptr(GrlVector2) circle1_center = NULL;
    g_autoptr(GrlVector2) circle2_center = NULL;
    g_autoptr(GrlVector2) tri1 = NULL;
    g_autoptr(GrlVector2) tri2 = NULL;
    g_autoptr(GrlVector2) tri3 = NULL;

    gfloat circle1_radius;
    gfloat circle2_radius;
    gfloat rect2_vx;
    gfloat circle2_vy;
    gfloat time;

    /* Initialize application and window */
    app = grl_application_new ("com.example.collision-demo");
    window = grl_window_new (1024, 768, "Collision Demo - 2D Collision Detection");
    grl_window_set_target_fps (window, 60);

    /* Create colors */
    bg_color = grl_color_new (30, 30, 40, 255);
    white = grl_color_new (255, 255, 255, 255);
    green = grl_color_new (0, 228, 48, 255);
    red = grl_color_new (230, 41, 55, 255);
    blue = grl_color_new (0, 121, 241, 255);
    yellow = grl_color_new (253, 249, 0, 255);
    gray = grl_color_new (130, 130, 130, 255);
    dark_gray = grl_color_new (80, 80, 80, 255);

    /* Initialize test shapes */
    rect1 = grl_rectangle_new (100.0f, 200.0f, 200.0f, 150.0f);
    rect2 = grl_rectangle_new (400.0f, 200.0f, 120.0f, 80.0f);

    circle1_center = grl_vector2_new (600.0f, 400.0f);
    circle1_radius = 80.0f;

    circle2_center = grl_vector2_new (800.0f, 400.0f);
    circle2_radius = 50.0f;

    tri1 = grl_vector2_new (150.0f, 500.0f);
    tri2 = grl_vector2_new (50.0f, 650.0f);
    tri3 = grl_vector2_new (250.0f, 650.0f);

    /* Animation state */
    rect2_vx = 100.0f;
    circle2_vy = 80.0f;
    time = 0.0f;

    /* Main loop */
    while (!grl_window_should_close (window))
    {
        g_autoptr(GrlVector2) mouse_pos = NULL;
        g_autoptr(GrlRectangle) overlap = NULL;
        GrlColor *rect1_color;
        GrlColor *rect2_color;
        GrlColor *circle1_color;
        GrlColor *circle2_color;
        GrlColor *tri_color;
        gfloat dt;
        gboolean mouse_in_rect1;
        gboolean mouse_in_circle1;
        gboolean mouse_in_triangle;
        gboolean rects_collide;
        gboolean circles_collide;
        gboolean circle_rect_collide;

        dt = grl_window_get_frame_time (window);
        time += dt;

        /* Get mouse position */
        mouse_pos = grl_input_get_mouse_position ();

        /* Animate rect2 horizontally */
        rect2->x += rect2_vx * dt;
        if (rect2->x > 700.0f || rect2->x < 350.0f)
            rect2_vx = -rect2_vx;

        /* Animate circle2 vertically */
        circle2_center->y += circle2_vy * dt;
        if (circle2_center->y > 550.0f || circle2_center->y < 250.0f)
            circle2_vy = -circle2_vy;

        /* Test point vs rectangle collision */
        mouse_in_rect1 = grl_collision_point_rect (mouse_pos, rect1);
        rect1_color = mouse_in_rect1 ? green : blue;

        /* Test point vs circle collision */
        mouse_in_circle1 = grl_collision_point_circle (mouse_pos, circle1_center, circle1_radius);
        circle1_color = mouse_in_circle1 ? green : yellow;

        /* Test point vs triangle collision */
        mouse_in_triangle = grl_collision_point_triangle (mouse_pos, tri1, tri2, tri3);
        tri_color = mouse_in_triangle ? green : red;

        /* Test rectangle vs rectangle collision */
        rects_collide = grl_collision_rects (rect1, rect2);
        rect2_color = rects_collide ? red : gray;

        /* Test circle vs circle collision */
        circles_collide = grl_collision_circles (circle1_center, circle1_radius,
                                                  circle2_center, circle2_radius);
        circle2_color = circles_collide ? red : gray;

        /* Test circle vs rectangle collision */
        circle_rect_collide = grl_collision_circle_rect (circle2_center, circle2_radius, rect1);

        /* Get rectangle overlap if they collide */
        if (rects_collide)
            overlap = grl_collision_get_rect_overlap (rect1, rect2);

        /* Begin drawing */
        grl_window_begin_drawing (window);
        grl_draw_clear_background (bg_color);

        /* Draw static rectangle (rect1) */
        grl_draw_rectangle ((gint)rect1->x, (gint)rect1->y,
                            (gint)rect1->width, (gint)rect1->height, rect1_color);
        grl_draw_rectangle_lines ((gint)rect1->x, (gint)rect1->y,
                                  (gint)rect1->width, (gint)rect1->height, white);

        /* Draw moving rectangle (rect2) */
        grl_draw_rectangle ((gint)rect2->x, (gint)rect2->y,
                            (gint)rect2->width, (gint)rect2->height, rect2_color);
        grl_draw_rectangle_lines ((gint)rect2->x, (gint)rect2->y,
                                  (gint)rect2->width, (gint)rect2->height, white);

        /* Draw overlap rectangle if exists */
        if (overlap != NULL)
        {
            grl_draw_rectangle ((gint)overlap->x, (gint)overlap->y,
                                (gint)overlap->width, (gint)overlap->height, yellow);
        }

        /* Draw static circle (circle1) */
        grl_draw_circle ((gint)circle1_center->x, (gint)circle1_center->y,
                         circle1_radius, circle1_color);
        grl_draw_circle_lines ((gint)circle1_center->x, (gint)circle1_center->y,
                               circle1_radius, white);

        /* Draw moving circle (circle2) */
        {
            GrlColor *c2_color = circle_rect_collide ? yellow : circle2_color;
            grl_draw_circle ((gint)circle2_center->x, (gint)circle2_center->y,
                             circle2_radius, c2_color);
            grl_draw_circle_lines ((gint)circle2_center->x, (gint)circle2_center->y,
                                   circle2_radius, white);
        }

        /* Draw triangle */
        {
            grl_draw_triangle (tri1, tri2, tri3, tri_color);
            grl_draw_line (tri1->x, tri1->y, tri2->x, tri2->y, white);
            grl_draw_line (tri2->x, tri2->y, tri3->x, tri3->y, white);
            grl_draw_line (tri3->x, tri3->y, tri1->x, tri1->y, white);
        }

        /* Draw mouse cursor indicator */
        grl_draw_circle ((gint)mouse_pos->x, (gint)mouse_pos->y, 5.0f, white);

        /* Draw UI text */
        grl_draw_text ("Collision Demo - 2D Collision Detection", 10, 10, 20, white);
        grl_draw_text ("Move mouse to test point collisions", 10, 35, 16, gray);
        grl_draw_text ("Shapes turn GREEN when mouse is inside", 10, 55, 16, gray);

        /* Status panel */
        grl_draw_rectangle (700, 10, 310, 180, dark_gray);
        grl_draw_text ("Collision Status:", 710, 20, 18, white);

        grl_draw_text (mouse_in_rect1 ? "Mouse in Rectangle: YES" : "Mouse in Rectangle: NO",
                       710, 50, 14, mouse_in_rect1 ? green : gray);
        grl_draw_text (mouse_in_circle1 ? "Mouse in Circle: YES" : "Mouse in Circle: NO",
                       710, 70, 14, mouse_in_circle1 ? green : gray);
        grl_draw_text (mouse_in_triangle ? "Mouse in Triangle: YES" : "Mouse in Triangle: NO",
                       710, 90, 14, mouse_in_triangle ? green : gray);
        grl_draw_text (rects_collide ? "Rect vs Rect: COLLIDING" : "Rect vs Rect: NO",
                       710, 120, 14, rects_collide ? red : gray);
        grl_draw_text (circles_collide ? "Circle vs Circle: COLLIDING" : "Circle vs Circle: NO",
                       710, 140, 14, circles_collide ? red : gray);
        grl_draw_text (circle_rect_collide ? "Circle vs Rect: COLLIDING" : "Circle vs Rect: NO",
                       710, 160, 14, circle_rect_collide ? yellow : gray);

        grl_draw_fps (10, 740);

        grl_window_end_drawing (window);
    }

    return 0;
}
