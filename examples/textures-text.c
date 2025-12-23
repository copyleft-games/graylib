/* textures-text.c
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Example demonstrating texture loading and text rendering.
 */

#include <graylib.h>

int
main (int argc, char *argv[])
{
    g_autoptr(GrlApplication) app = NULL;
    g_autoptr(GrlWindow) window = NULL;
    g_autoptr(GrlColor) bg = NULL;
    g_autoptr(GrlColor) white = NULL;
    g_autoptr(GrlColor) yellow = NULL;
    g_autoptr(GrlImage) img = NULL;
    g_autoptr(GrlTexture) texture = NULL;
    g_autoptr(GrlVector2) tex_pos = NULL;
    g_autoptr(GrlVector2) text_pos = NULL;

    /*
     * Initialize the application and create a window.
     */
    app = grl_application_new ("com.example.TexturesText");
    window = grl_window_new (800, 600, "Graylib - Textures and Text");
    grl_window_set_target_fps (window, 60);

    /*
     * Create colors for drawing.
     */
    bg = grl_color_new (40, 40, 60, 255);
    white = grl_color_new_white ();
    yellow = grl_color_new (255, 200, 0, 255);

    /*
     * Create a procedural image (checkerboard pattern).
     * This demonstrates CPU-side image generation.
     */
    {
        g_autoptr(GrlColor) c1 = grl_color_new (100, 100, 150, 255);
        g_autoptr(GrlColor) c2 = grl_color_new (50, 50, 80, 255);

        img = grl_image_new_checked (128, 128, 8, 8, c1, c2);
    }

    /*
     * Upload the image to GPU as a texture.
     * We set point filtering for a pixelated look.
     */
    texture = grl_texture_new_from_image (img);
    grl_texture_set_filter (texture, GRL_TEXTURE_FILTER_POINT);

    /*
     * Positions for drawing.
     */
    tex_pos = grl_vector2_new (336.0f, 200.0f);
    text_pos = grl_vector2_new (100.0f, 400.0f);

    /*
     * Main game loop.
     */
    while (!grl_window_should_close (window))
    {
        grl_window_begin_drawing (window);
        grl_draw_clear_background (bg);

        /*
         * Draw the texture.
         */
        grl_draw_texture_v (texture, tex_pos, white);

        /*
         * Draw text using the default font.
         */
        grl_draw_text ("Graylib Phase 2: Graphics Resources", 200, 50, 24, white);
        grl_draw_text ("Procedurally generated checkerboard texture:", 220, 150, 18, yellow);

        /*
         * Draw FPS counter.
         */
        grl_draw_fps (10, 10);

        /*
         * Draw texture information.
         */
        {
            gint tex_width = grl_texture_get_width (texture);
            gint tex_height = grl_texture_get_height (texture);
            g_autofree gchar *info = g_strdup_printf (
                "Texture: %dx%d pixels", tex_width, tex_height);

            grl_draw_text (info, 320, 340, 16, white);
        }

        /*
         * Draw usage instructions.
         */
        grl_draw_text ("Press ESC to exit", 320, 550, 16, yellow);

        grl_window_end_drawing (window);
    }

    return 0;
}
