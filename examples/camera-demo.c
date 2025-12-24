/* camera-demo.c
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Demonstrates 2D/3D cameras, render textures, and 3D drawing.
 */

#include <graylib.h>
#include <math.h>

/* Demo modes */
typedef enum
{
    MODE_2D_CAMERA,
    MODE_3D_CAMERA,
    MODE_RENDER_TEXTURE,
    MODE_COUNT
} DemoMode;

static const char *mode_names[] = {
    "2D Camera",
    "3D Camera",
    "Render Texture"
};

/*
 * 2D Camera demo - shows zoom, rotation, and movement
 */
static void
draw_2d_camera_demo (GrlCamera2D *camera,
                     GrlColor    *bg_color)
{
    g_autoptr(GrlColor) red = grl_color_new (230, 41, 55, 255);
    g_autoptr(GrlColor) green = grl_color_new (0, 228, 48, 255);
    g_autoptr(GrlColor) blue = grl_color_new (0, 121, 241, 255);
    g_autoptr(GrlColor) white = grl_color_new (255, 255, 255, 255);
    g_autoptr(GrlColor) gray = grl_color_new (130, 130, 130, 255);
    g_autoptr(GrlColor) dark_gray = grl_color_fade (gray, 0.5f);
    g_autoptr(GrlVector2) target = NULL;
    gint i;

    grl_draw_clear_background (bg_color);

    /* Begin 2D camera mode */
    grl_camera2d_begin (camera);

    /* Draw a grid of lines */
    for (i = -10; i <= 10; i++)
    {
        grl_draw_line ((gfloat)(i * 50), -500.0f, (gfloat)(i * 50), 500.0f, gray);
        grl_draw_line (-500.0f, (gfloat)(i * 50), 500.0f, (gfloat)(i * 50), gray);
    }

    /* Draw some shapes around the origin */
    grl_draw_rectangle (-50, -50, 100, 100, red);
    grl_draw_circle (100, 0, 40.0f, green);

    /* Draw a triangle using vectors */
    {
        g_autoptr(GrlVector2) v1 = grl_vector2_new (-100.0f, 50.0f);
        g_autoptr(GrlVector2) v2 = grl_vector2_new (-150.0f, -50.0f);
        g_autoptr(GrlVector2) v3 = grl_vector2_new (-50.0f, -50.0f);
        grl_draw_triangle (v1, v2, v3, blue);
    }

    /* Draw camera target marker */
    target = grl_camera2d_get_target (camera);
    grl_draw_line (target->x - 10.0f, target->y, target->x + 10.0f, target->y, white);
    grl_draw_line (target->x, target->y - 10.0f, target->x, target->y + 10.0f, white);

    /* End 2D camera mode */
    grl_camera2d_end (camera);

    /* Draw UI (not affected by camera) */
    grl_draw_rectangle (10, 10, 280, 80, dark_gray);
    grl_draw_text ("WASD - Move camera", 20, 20, 14, white);
    grl_draw_text ("Mouse Wheel - Zoom", 20, 40, 14, white);
    grl_draw_text ("Q/E - Rotate", 20, 60, 14, white);
}

/*
 * 3D Camera demo - shows 3D primitives and camera movement
 */
static void
draw_3d_camera_demo (GrlCamera3D *camera,
                     GrlColor    *bg_color,
                     gfloat       time)
{
    g_autoptr(GrlColor) red = grl_color_new (230, 41, 55, 255);
    g_autoptr(GrlColor) green = grl_color_new (0, 228, 48, 255);
    g_autoptr(GrlColor) blue = grl_color_new (0, 121, 241, 255);
    g_autoptr(GrlColor) yellow = grl_color_new (253, 249, 0, 255);
    g_autoptr(GrlColor) white = grl_color_new (255, 255, 255, 255);
    g_autoptr(GrlColor) gray = grl_color_new (130, 130, 130, 255);
    g_autoptr(GrlColor) dark_gray = grl_color_fade (gray, 0.5f);
    g_autoptr(GrlVector3) cube_pos = grl_vector3_new (0.0f, 1.0f, 0.0f);
    g_autoptr(GrlVector3) sphere_pos = grl_vector3_new (4.0f, 1.5f, 0.0f);
    g_autoptr(GrlVector3) cylinder_pos = grl_vector3_new (-4.0f, 0.0f, 0.0f);
    g_autoptr(GrlVector3) circle_pos = grl_vector3_new (0.0f, 4.0f, 0.0f);
    g_autoptr(GrlVector3) rotation_axis = grl_vector3_new (0.0f, 1.0f, 0.0f);

    grl_draw_clear_background (bg_color);

    /* Begin 3D camera mode */
    grl_camera3d_begin (camera);

    /* Draw a grid on the floor */
    grl_draw_grid (20, 1.0f);

    /* Draw a cube */
    grl_draw_cube (cube_pos, 2.0f, 2.0f, 2.0f, red);
    grl_draw_cube_wires (cube_pos, 2.0f, 2.0f, 2.0f, white);

    /* Draw a sphere */
    grl_draw_sphere (sphere_pos, 1.5f, green);
    grl_draw_sphere_wires (sphere_pos, 1.5f, 16, 16, white);

    /* Draw a cylinder */
    grl_draw_cylinder (cylinder_pos, 0.5f, 1.0f, 3.0f, 16, blue);
    grl_draw_cylinder_wires (cylinder_pos, 0.5f, 1.0f, 3.0f, 16, white);

    /* Draw a rotating circle in 3D space */
    grl_draw_circle_3d (circle_pos, 2.0f, rotation_axis, time * 90.0f, yellow);

    /* End 3D camera mode */
    grl_camera3d_end (camera);

    /* Draw UI */
    grl_draw_rectangle (10, 10, 280, 60, dark_gray);
    grl_draw_text ("WASD - Move camera", 20, 20, 14, white);
    grl_draw_text ("Arrow keys - Look around", 20, 40, 14, white);
}

/*
 * Render texture demo - render to texture then display
 */
static void
draw_render_texture_demo (GrlRenderTexture *render_target,
                          GrlColor         *bg_color,
                          gfloat            time)
{
    g_autoptr(GrlColor) red = grl_color_new (230, 41, 55, 255);
    g_autoptr(GrlColor) green = grl_color_new (0, 228, 48, 255);
    g_autoptr(GrlColor) white = grl_color_new (255, 255, 255, 255);
    g_autoptr(GrlColor) gray = grl_color_new (130, 130, 130, 255);
    g_autoptr(GrlColor) dark_gray = grl_color_new (40, 40, 40, 255);
    g_autoptr(GrlTexture) tex = NULL;
    g_autoptr(GrlRectangle) src_rect = NULL;
    g_autoptr(GrlRectangle) dst_rect = NULL;
    g_autoptr(GrlVector2) origin = NULL;
    gint rt_width, rt_height;
    gint x, y;

    /* Render to texture */
    grl_render_texture_begin (render_target);
    grl_draw_clear_background (dark_gray);

    /* Draw animated content */
    x = (gint)(100.0f + sinf (time * 2.0f) * 50.0f);
    y = (gint)(100.0f + cosf (time * 2.0f) * 50.0f);
    grl_draw_circle (x, y, 30.0f, red);
    grl_draw_rectangle (150, 50, 100, 100, green);
    grl_draw_text ("Render Texture", 100, 170, 20, white);

    grl_render_texture_end (render_target);

    /* Draw to screen */
    grl_draw_clear_background (bg_color);

    /* Get the render texture as a texture */
    tex = grl_render_texture_get_texture (render_target);
    rt_width = grl_render_texture_get_width (render_target);
    rt_height = grl_render_texture_get_height (render_target);

    /* Note: Render textures in OpenGL are vertically flipped */
    src_rect = grl_rectangle_new (0.0f, 0.0f, (gfloat)rt_width, (gfloat)-rt_height);
    dst_rect = grl_rectangle_new (100.0f, 100.0f, 400.0f, 300.0f);
    origin = grl_vector2_new (0.0f, 0.0f);

    /* Draw the render texture scaled */
    grl_draw_texture_pro (tex, src_rect, dst_rect, origin, 0.0f, white);

    /* Draw border */
    grl_draw_rectangle_lines (100, 100, 400, 300, white);

    /* Draw UI */
    grl_draw_rectangle (10, 10, 350, 40, grl_color_fade (gray, 0.5f));
    grl_draw_text ("Render texture displayed with vertical flip", 20, 20, 14, white);
}

int
main (int    argc,
      char **argv)
{
    g_autoptr(GrlApplication) app = NULL;
    g_autoptr(GrlWindow) window = NULL;
    g_autoptr(GrlColor) bg_color = NULL;
    g_autoptr(GrlColor) white = NULL;
    g_autoptr(GrlCamera2D) camera_2d = NULL;
    g_autoptr(GrlCamera3D) camera_3d = NULL;
    g_autoptr(GrlRenderTexture) render_target = NULL;
    DemoMode current_mode = MODE_2D_CAMERA;
    gfloat camera_2d_target_x = 0.0f;
    gfloat camera_2d_target_y = 0.0f;
    gfloat camera_2d_rotation = 0.0f;
    gfloat camera_2d_zoom = 1.0f;
    gfloat camera_3d_x = 10.0f;
    gfloat camera_3d_y = 5.0f;
    gfloat camera_3d_z = 10.0f;
    gfloat time = 0.0f;

    app = grl_application_new ("com.example.camera-demo");
    window = grl_window_new (800, 600, "Camera & Render Texture Demo");
    bg_color = grl_color_new (40, 40, 60, 255);
    white = grl_color_new (255, 255, 255, 255);

    /* Create 2D camera */
    camera_2d = grl_camera2d_new ();
    grl_camera2d_set_offset_xy (camera_2d, 400.0f, 300.0f);
    grl_camera2d_set_target_xy (camera_2d, 0.0f, 0.0f);
    grl_camera2d_set_zoom (camera_2d, 1.0f);

    /* Create 3D camera */
    camera_3d = grl_camera3d_new ();
    grl_camera3d_set_position_xyz (camera_3d, camera_3d_x, camera_3d_y, camera_3d_z);
    grl_camera3d_set_target_xyz (camera_3d, 0.0f, 1.0f, 0.0f);
    grl_camera3d_set_fovy (camera_3d, 45.0f);

    /* Create render texture */
    render_target = grl_render_texture_new (400, 300);

    grl_window_set_target_fps (window, 60);

    while (!grl_window_should_close (window))
    {
        gfloat dt = grl_window_get_frame_time (window);
        time += dt;

        /* Poll input (required for input functions to work) */
        grl_window_poll_input (window);

        /* Switch demo mode with TAB */
        if (grl_input_is_key_pressed (GRL_KEY_TAB))
        {
            current_mode = (current_mode + 1) % MODE_COUNT;
        }

        /* 2D camera controls */
        if (current_mode == MODE_2D_CAMERA)
        {
            if (grl_input_is_key_down (GRL_KEY_W))
                camera_2d_target_y -= 200.0f * dt;
            if (grl_input_is_key_down (GRL_KEY_S))
                camera_2d_target_y += 200.0f * dt;
            if (grl_input_is_key_down (GRL_KEY_A))
                camera_2d_target_x -= 200.0f * dt;
            if (grl_input_is_key_down (GRL_KEY_D))
                camera_2d_target_x += 200.0f * dt;

            if (grl_input_is_key_down (GRL_KEY_Q))
                camera_2d_rotation -= 90.0f * dt;
            if (grl_input_is_key_down (GRL_KEY_E))
                camera_2d_rotation += 90.0f * dt;

            camera_2d_zoom += grl_input_get_mouse_wheel_move () * 0.1f;
            if (camera_2d_zoom < 0.1f)
                camera_2d_zoom = 0.1f;
            if (camera_2d_zoom > 3.0f)
                camera_2d_zoom = 3.0f;

            grl_camera2d_set_target_xy (camera_2d, camera_2d_target_x, camera_2d_target_y);
            grl_camera2d_set_rotation (camera_2d, camera_2d_rotation);
            grl_camera2d_set_zoom (camera_2d, camera_2d_zoom);
        }

        /* 3D camera controls */
        if (current_mode == MODE_3D_CAMERA)
        {
            if (grl_input_is_key_down (GRL_KEY_W))
                camera_3d_z -= 5.0f * dt;
            if (grl_input_is_key_down (GRL_KEY_S))
                camera_3d_z += 5.0f * dt;
            if (grl_input_is_key_down (GRL_KEY_A))
                camera_3d_x -= 5.0f * dt;
            if (grl_input_is_key_down (GRL_KEY_D))
                camera_3d_x += 5.0f * dt;
            if (grl_input_is_key_down (GRL_KEY_UP))
                camera_3d_y += 3.0f * dt;
            if (grl_input_is_key_down (GRL_KEY_DOWN))
                camera_3d_y -= 3.0f * dt;

            grl_camera3d_set_position_xyz (camera_3d, camera_3d_x, camera_3d_y, camera_3d_z);
        }

        grl_window_begin_drawing (window);

        switch (current_mode)
        {
            case MODE_2D_CAMERA:
                draw_2d_camera_demo (camera_2d, bg_color);
                break;
            case MODE_3D_CAMERA:
                draw_3d_camera_demo (camera_3d, bg_color, time);
                break;
            case MODE_RENDER_TEXTURE:
                draw_render_texture_demo (render_target, bg_color, time);
                break;
            default:
                break;
        }

        /* Draw mode indicator */
        grl_draw_text ("Press TAB to switch mode", 520, 10, 14, white);
        grl_draw_text (mode_names[current_mode], 520, 30, 20, white);

        grl_window_end_drawing (window);
    }

    return 0;
}
