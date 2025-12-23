/* model-demo.c
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Demonstrates 3D model rendering with meshes and materials.
 * Shows primitive meshes, camera controls, and wireframe rendering.
 */

#include <graylib.h>
#include <math.h>

/*
 * Model demo application
 *
 * Creates various primitive meshes (cube, sphere, cylinder, torus)
 * and renders them in a 3D scene with camera controls.
 */

int
main (int    argc,
      char **argv)
{
    g_autoptr(GrlApplication) app = NULL;
    g_autoptr(GrlWindow) window = NULL;
    g_autoptr(GrlCamera3D) camera = NULL;
    g_autoptr(GrlColor) white = NULL;
    g_autoptr(GrlColor) red = NULL;
    g_autoptr(GrlColor) green = NULL;
    g_autoptr(GrlColor) blue = NULL;
    g_autoptr(GrlColor) yellow = NULL;
    g_autoptr(GrlColor) gray = NULL;
    g_autoptr(GrlColor) dark_gray = NULL;
    g_autoptr(GrlMesh) cube_mesh = NULL;
    g_autoptr(GrlMesh) sphere_mesh = NULL;
    g_autoptr(GrlMesh) cylinder_mesh = NULL;
    g_autoptr(GrlMesh) torus_mesh = NULL;
    g_autoptr(GrlMesh) plane_mesh = NULL;
    g_autoptr(GrlModel) cube_model = NULL;
    g_autoptr(GrlModel) sphere_model = NULL;
    g_autoptr(GrlModel) cylinder_model = NULL;
    g_autoptr(GrlModel) torus_model = NULL;
    g_autoptr(GrlModel) plane_model = NULL;
    g_autoptr(GrlVector3) cube_pos = NULL;
    g_autoptr(GrlVector3) sphere_pos = NULL;
    g_autoptr(GrlVector3) cylinder_pos = NULL;
    g_autoptr(GrlVector3) torus_pos = NULL;
    g_autoptr(GrlVector3) plane_pos = NULL;
    gfloat camera_x, camera_y, camera_z;
    gboolean show_wireframe;

    /* Initialize application and window */
    app = grl_application_new ("com.example.model-demo");
    window = grl_window_new (1024, 768, "Model Demo - 3D Primitives");
    grl_window_set_target_fps (window, 60);

    /* Create colors */
    white = grl_color_new (255, 255, 255, 255);
    red = grl_color_new (230, 41, 55, 255);
    green = grl_color_new (0, 228, 48, 255);
    blue = grl_color_new (0, 121, 241, 255);
    yellow = grl_color_new (253, 249, 0, 255);
    gray = grl_color_new (130, 130, 130, 255);
    dark_gray = grl_color_new (80, 80, 80, 255);

    /* Setup camera */
    camera_x = 10.0f;
    camera_y = 8.0f;
    camera_z = 10.0f;
    camera = grl_camera3d_new ();
    grl_camera3d_set_position_xyz (camera, camera_x, camera_y, camera_z);
    grl_camera3d_set_target_xyz (camera, 0.0f, 0.0f, 0.0f);
    grl_camera3d_set_fovy (camera, 45.0f);

    /* Create primitive meshes */
    cube_mesh = grl_mesh_new_cube (2.0f, 2.0f, 2.0f);
    sphere_mesh = grl_mesh_new_sphere (16, 16, 1.5f);
    cylinder_mesh = grl_mesh_new_cylinder (16, 1.0f, 2.5f);
    torus_mesh = grl_mesh_new_torus (16, 32, 0.5f, 1.5f);
    plane_mesh = grl_mesh_new_plane (20.0f, 20.0f, 1, 1);

    /* Upload meshes to GPU */
    grl_mesh_upload (cube_mesh, FALSE);
    grl_mesh_upload (sphere_mesh, FALSE);
    grl_mesh_upload (cylinder_mesh, FALSE);
    grl_mesh_upload (torus_mesh, FALSE);
    grl_mesh_upload (plane_mesh, FALSE);

    /* Create models from meshes */
    cube_model = grl_model_new_from_mesh (cube_mesh);
    sphere_model = grl_model_new_from_mesh (sphere_mesh);
    cylinder_model = grl_model_new_from_mesh (cylinder_mesh);
    torus_model = grl_model_new_from_mesh (torus_mesh);
    plane_model = grl_model_new_from_mesh (plane_mesh);

    /* Set model positions */
    cube_pos = grl_vector3_new (-4.0f, 1.0f, 0.0f);
    sphere_pos = grl_vector3_new (0.0f, 1.5f, 0.0f);
    cylinder_pos = grl_vector3_new (4.0f, 1.25f, 0.0f);
    torus_pos = grl_vector3_new (0.0f, 1.0f, -4.0f);
    plane_pos = grl_vector3_new (0.0f, 0.0f, 0.0f);

    show_wireframe = FALSE;

    /* Main loop */
    while (!grl_window_should_close (window))
    {
        gfloat dt = grl_window_get_frame_time (window);

        /* Camera controls - WASD for horizontal, arrow keys for height */
        if (grl_input_is_key_down (GRL_KEY_W))
            camera_z -= 8.0f * dt;
        if (grl_input_is_key_down (GRL_KEY_S))
            camera_z += 8.0f * dt;
        if (grl_input_is_key_down (GRL_KEY_A))
            camera_x -= 8.0f * dt;
        if (grl_input_is_key_down (GRL_KEY_D))
            camera_x += 8.0f * dt;
        if (grl_input_is_key_down (GRL_KEY_UP))
            camera_y += 5.0f * dt;
        if (grl_input_is_key_down (GRL_KEY_DOWN))
            camera_y -= 5.0f * dt;

        /* Clamp camera height */
        if (camera_y < 1.0f)
            camera_y = 1.0f;
        if (camera_y > 20.0f)
            camera_y = 20.0f;

        grl_camera3d_set_position_xyz (camera, camera_x, camera_y, camera_z);

        /* Toggle wireframe mode with space */
        if (grl_input_is_key_pressed (GRL_KEY_SPACE))
            show_wireframe = !show_wireframe;

        /* Begin drawing */
        grl_window_begin_drawing (window);
        grl_draw_clear_background (dark_gray);

        /* Begin 3D mode */
        grl_camera3d_begin (camera);

        /* Draw floor plane */
        grl_model_draw (plane_model, plane_pos, 1.0f, gray);

        /* Draw models */
        if (show_wireframe)
        {
            grl_model_draw_wires (cube_model, cube_pos, 1.0f, red);
            grl_model_draw_wires (sphere_model, sphere_pos, 1.0f, green);
            grl_model_draw_wires (cylinder_model, cylinder_pos, 1.0f, blue);
            grl_model_draw_wires (torus_model, torus_pos, 1.0f, yellow);
        }
        else
        {
            grl_model_draw (cube_model, cube_pos, 1.0f, red);
            grl_model_draw (sphere_model, sphere_pos, 1.0f, green);
            grl_model_draw (cylinder_model, cylinder_pos, 1.0f, blue);
            grl_model_draw (torus_model, torus_pos, 1.0f, yellow);
        }

        /* Draw grid for reference */
        grl_draw_grid (20, 1.0f);

        /* End 3D mode */
        grl_camera3d_end (camera);

        /* Draw UI text */
        grl_draw_text ("Model Demo - 3D Primitives", 10, 10, 20, white);
        grl_draw_text ("WASD - Move camera horizontally", 10, 35, 16, gray);
        grl_draw_text ("Arrow Up/Down - Move camera vertically", 10, 55, 16, gray);
        grl_draw_text ("Space - Toggle wireframe", 10, 75, 16, gray);

        if (show_wireframe)
            grl_draw_text ("Mode: WIREFRAME", 10, 100, 16, yellow);
        else
            grl_draw_text ("Mode: SOLID", 10, 100, 16, green);

        grl_draw_fps (10, 740);

        grl_window_end_drawing (window);
    }

    return 0;
}
