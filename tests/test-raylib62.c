/* test-raylib62.c
 *
 * Copyright 2026 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Tests for the API graylib wraps from raylib 6.2 and raygui 5: the path
 * predicates and fixed file semantics, the image blits and gradients, the
 * TABBAR control, binary styles from memory and, with a display, the new
 * outline/gradient draws and render textures with a pixel format.
 */

#include <glib.h>
#include <glib/gstdio.h>
#include <gio/gio.h>
#include <string.h>
#include "src/system/grl-fs.h"
#include "src/graphics/grl-image.h"
#include "src/graphics/grl-texture.h"
#include "src/graphics/grl-render-texture.h"
#include "src/core/grl-window.h"
#include "src/drawing/grl-draw.h"
#include "src/ui/grl-ui-enums.h"
#include "src/ui/grl-ui-style.h"
#include "src/ui/grl-ui-tabbar.h"
#include "src/ui/grl-ui-dropdownbox.h"
#include "src/ui/grl-ui-progressbar.h"
#include "src/grl-enums.h"
#include "src/math/grl-color.h"
#include "src/math/grl-vector2.h"
#include "src/math/grl-rectangle.h"

/* Skip if no display available (CI/headless). */
#define SKIP_IF_NO_DISPLAY() \
    do { \
        if (g_getenv ("DISPLAY") == NULL && g_getenv ("WAYLAND_DISPLAY") == NULL) \
        { \
            g_test_skip ("No display available (headless environment)"); \
            return; \
        } \
    } while (0)

static void
assert_pixel (GrlImage *img,
              gint      x,
              gint      y,
              gint      r,
              gint      g,
              gint      b,
              gint      tol)
{
    g_autoptr(GrlColor) c = grl_image_get_pixel (img, x, y);

    g_assert_nonnull (c);
    g_assert_cmpint (ABS ((gint)c->r - r), <=, tol);
    g_assert_cmpint (ABS ((gint)c->g - g), <=, tol);
    g_assert_cmpint (ABS ((gint)c->b - b), <=, tol);
}

/*
 * Filesystem
 */

static void
test_fs_path_predicates (void)
{
    g_autofree gchar *dir = NULL;
    g_autofree gchar *file = NULL;
    g_autofree gchar *hidden = NULL;
    g_autofree gchar *missing = NULL;
    g_autoptr(GError) error = NULL;

    dir = g_dir_make_tmp ("grl-raylib62-XXXXXX", &error);
    g_assert_no_error (error);
    file = g_build_filename (dir, "plain.txt", NULL);
    hidden = g_build_filename (dir, ".hidden", NULL);
    missing = g_build_filename (dir, "missing", NULL);
    g_assert_true (g_file_set_contents (file, "x", 1, NULL));
    g_assert_true (g_file_set_contents (hidden, "x", 1, NULL));

    /* a directory is a directory, a file and a missing path are not */
    g_assert_true (grl_fs_is_path_directory (dir));
    g_assert_false (grl_fs_is_path_directory (file));
    g_assert_false (grl_fs_is_path_directory (missing));

    g_assert_true (grl_fs_is_path_absolute (dir));
    g_assert_true (grl_fs_is_path_absolute ("/"));
    g_assert_false (grl_fs_is_path_absolute ("relative/path"));
    g_assert_false (grl_fs_is_path_absolute (""));

    g_assert_true (grl_fs_is_file_hidden (hidden));
    g_assert_false (grl_fs_is_file_hidden (file));
    g_assert_false (grl_fs_is_file_hidden ("."));
    g_assert_false (grl_fs_is_file_hidden (".."));

    g_remove (file);
    g_remove (hidden);
    g_rmdir (dir);
}

static void
test_fs_copy_semantics (void)
{
    g_autofree gchar *dir = NULL;
    g_autofree gchar *src = NULL;
    g_autofree gchar *dst = NULL;
    g_autofree gchar *nested = NULL;
    g_autofree gchar *missing = NULL;
    g_autofree gchar *contents = NULL;
    g_autofree gchar *nested_a = NULL;
    g_autofree gchar *nested_b = NULL;
    g_autoptr(GError) error = NULL;

    dir = g_dir_make_tmp ("grl-raylib62-XXXXXX", &error);
    g_assert_no_error (error);
    nested_a = g_build_filename (dir, "a", NULL);
    nested_b = g_build_filename (dir, "a", "b", NULL);
    src = g_build_filename (dir, "src.txt", NULL);
    dst = g_build_filename (dir, "dst.txt", NULL);
    nested = g_build_filename (dir, "a", "b", "copy.txt", NULL);
    missing = g_build_filename (dir, "missing.txt", NULL);
    g_assert_true (g_file_set_contents (src, "hello", -1, NULL));

    /* 6.2 returns 0 on success: graylib must report TRUE */
    g_assert_true (grl_fs_file_copy (src, dst));
    g_assert_true (g_file_get_contents (dst, &contents, NULL, NULL));
    g_assert_cmpstr (contents, ==, "hello");

    /* intermediate directories are created */
    g_assert_true (grl_fs_file_copy (src, nested));
    g_assert_true (g_file_test (nested, G_FILE_TEST_IS_REGULAR));

    /* 6.2's FileCopy() reports success for a missing source; graylib must not */
    g_assert_false (grl_fs_file_copy (missing, dst));

    /* text replace returns 0 on success in 6.2 */
    g_assert_true (grl_fs_file_text_replace (dst, "hello", "world"));
    g_clear_pointer (&contents, g_free);
    g_assert_true (g_file_get_contents (dst, &contents, NULL, NULL));
    g_assert_cmpstr (contents, ==, "world");

    g_remove (nested);
    g_rmdir (nested_b);
    g_rmdir (nested_a);
    g_remove (src);
    g_remove (dst);
    g_rmdir (dir);
}

static void
test_fs_change_directory (void)
{
    g_autofree gchar *before = g_get_current_dir ();
    g_autofree gchar *tmp = NULL;
    g_autofree gchar *after = NULL;
    g_autoptr(GError) error = NULL;

    tmp = g_dir_make_tmp ("grl-raylib62-XXXXXX", &error);
    g_assert_no_error (error);

    /* 6.2 returns chdir()'s 0 on success: graylib must report TRUE */
    g_assert_true (grl_fs_change_directory (tmp));
    after = g_get_current_dir ();
    g_assert_true (g_str_has_suffix (after, strrchr (tmp, '/')));

    g_assert_true (grl_fs_change_directory (before));
    g_assert_false (grl_fs_change_directory ("/nonexistent/grl-raylib62"));
    g_rmdir (tmp);
}

/*
 * Images
 */

static void
test_image_blits (void)
{
    g_autoptr(GrlColor) black = grl_color_new (0, 0, 0, 255);
    g_autoptr(GrlColor) red = grl_color_new (255, 0, 0, 255);
    g_autoptr(GrlColor) blue = grl_color_new (0, 0, 255, 255);
    g_autoptr(GrlImage) dst = grl_image_new_color (32, 32, black);
    g_autoptr(GrlImage) src = grl_image_new_color (4, 2, red);
    g_autoptr(GrlRectangle) left = grl_rectangle_new (0, 0, 2, 2);
    g_autoptr(GrlRectangle) dst_rect = grl_rectangle_new (20, 20, 8, 4);
    g_autoptr(GrlVector2) pos = grl_vector2_new (10.0f, 0.0f);
    g_autoptr(GrlVector2) origin = grl_vector2_new (2.0f, 1.0f);
    g_autoptr(GrlRectangle) rot_dst = grl_rectangle_new (24.0f, 8.0f, 4.0f, 2.0f);
    g_autoptr(GrlRectangle) rect = grl_rectangle_new (2, 26, 4, 4);

    /* at: unscaled at (1, 1) */
    grl_image_draw_image_at (dst, src, 1, 1, NULL);
    assert_pixel (dst, 1, 1, 255, 0, 0, 0);
    assert_pixel (dst, 4, 2, 255, 0, 0, 0);
    assert_pixel (dst, 5, 1, 0, 0, 0, 0);

    /* rec: only the 2x2 region lands at (10, 0) */
    grl_image_draw_image_rec (dst, src, left, pos, NULL);
    assert_pixel (dst, 10, 0, 255, 0, 0, 0);
    assert_pixel (dst, 11, 1, 255, 0, 0, 0);
    assert_pixel (dst, 12, 0, 0, 0, 0, 0);

    /* a tint multiplies the source: red tinted blue is black */
    grl_image_draw_image_at (dst, src, 14, 4, blue);
    assert_pixel (dst, 14, 4, 0, 0, 0, 0);

    /* pro without rotation scales the source into the destination */
    grl_image_draw_image_pro (dst, src, NULL, dst_rect, NULL, 0.0f, NULL);
    assert_pixel (dst, 20, 20, 255, 0, 0, 0);
    assert_pixel (dst, 27, 23, 255, 0, 0, 0);
    assert_pixel (dst, 28, 23, 0, 0, 0, 0);

    /* pro with 90 degrees around the centre of a 4x2 region placed at
     * (24, 8): it becomes 2 wide and 4 tall around (24, 8) */
    grl_image_draw_image_pro (dst, src, NULL, rot_dst, origin, 90.0f, NULL);
    assert_pixel (dst, 24, 7, 255, 0, 0, 0);
    assert_pixel (dst, 23, 9, 255, 0, 0, 0);
    assert_pixel (dst, 21, 8, 0, 0, 0, 0);
    assert_pixel (dst, 26, 8, 0, 0, 0, 0);

    /* rectangle pro without origin or rotation is a plain fill */
    grl_image_draw_rectangle_pro (dst, rect, NULL, 0.0f, red);
    assert_pixel (dst, 2, 26, 255, 0, 0, 0);
    assert_pixel (dst, 5, 29, 255, 0, 0, 0);
}

static void
test_image_gradients (void)
{
    g_autoptr(GrlColor) black = grl_color_new (0, 0, 0, 255);
    g_autoptr(GrlColor) red = grl_color_new (255, 0, 0, 255);
    g_autoptr(GrlColor) green = grl_color_new (0, 255, 0, 255);
    g_autoptr(GrlColor) blue = grl_color_new (0, 0, 255, 255);
    g_autoptr(GrlColor) white = grl_color_new (255, 255, 255, 255);
    g_autoptr(GrlImage) img = grl_image_new_color (64, 64, black);
    g_autoptr(GrlRectangle) rect = grl_rectangle_new (0, 0, 32, 32);
    g_autoptr(GrlVector2) center = grl_vector2_new (48.0f, 48.0f);
    g_autoptr(GrlVector2) v1 = grl_vector2_new (34.0f, 2.0f);
    g_autoptr(GrlVector2) v2 = grl_vector2_new (34.0f, 30.0f);
    g_autoptr(GrlVector2) v3 = grl_vector2_new (62.0f, 30.0f);
    GrlVector2 strip[3];

    /* corners: top-left red, bottom-left green, top-right blue,
     * bottom-right white (same order as grl_draw_rectangle_gradient_ex) */
    grl_image_draw_rectangle_gradient_ex (img, rect, red, green, blue, white);
    assert_pixel (img, 0, 0, 255, 0, 0, 12);
    assert_pixel (img, 0, 31, 0, 255, 0, 12);
    assert_pixel (img, 31, 0, 0, 0, 255, 12);
    assert_pixel (img, 31, 31, 255, 255, 255, 20);

    grl_image_draw_circle_gradient (img, center, 10.0f, white, black);
    assert_pixel (img, 48, 48, 255, 255, 255, 40);
    assert_pixel (img, 48, 63, 0, 0, 0, 0);

    grl_image_draw_triangle_gradient (img, v1, v2, v3, red, green, blue);
    assert_pixel (img, 35, 5, 255, 0, 0, 60);

    strip[0].x = 2.0f;  strip[0].y = 40.0f;
    strip[1].x = 20.0f; strip[1].y = 40.0f;
    strip[2].x = 20.0f; strip[2].y = 60.0f;
    grl_image_draw_line_strip (img, strip, 3, red);
    assert_pixel (img, 10, 40, 255, 0, 0, 0);
    assert_pixel (img, 20, 50, 255, 0, 0, 0);
}

/*
 * raygui 5
 */

static void
test_ui_tabbar_properties (void)
{
    g_autoptr(GrlUiTabBar) bar = grl_ui_tabbar_new_with_bounds (10, 20, 400, 24, "One;Two;Three");
    g_autofree gchar *text = NULL;

    g_assert_cmpint (GRL_UI_CONTROL_TABBAR, ==, 11);
    g_assert_cmpint (GRL_UI_CONTROL_LISTVIEW, ==, 12);

    g_assert_cmpint (grl_ui_tabbar_get_item_count (bar), ==, 3);
    grl_ui_tabbar_set_active (bar, 7);
    g_assert_cmpint (grl_ui_tabbar_get_active (bar), ==, 2);
    text = grl_ui_tabbar_get_active_text (bar);
    g_assert_cmpstr (text, ==, "Three");

    /* shrinking the list clamps the active tab */
    grl_ui_tabbar_set_items (bar, "Only");
    g_assert_cmpint (grl_ui_tabbar_get_active (bar), ==, 0);
    grl_ui_tabbar_set_items (bar, "One;Two;Three");

    /* hit-testing follows raygui's layout: width + 4 px gap per tab */
    grl_ui_tabbar_set_tab_width (bar, 100);
    g_assert_cmpint (grl_ui_tabbar_tab_at (bar, 15.0f, 30.0f), ==, 0);
    g_assert_cmpint (grl_ui_tabbar_tab_at (bar, 112.0f, 30.0f), ==, -1);
    g_assert_cmpint (grl_ui_tabbar_tab_at (bar, 115.0f, 30.0f), ==, 1);
    g_assert_cmpint (grl_ui_tabbar_tab_at (bar, 230.0f, 30.0f), ==, 2);
    g_assert_cmpint (grl_ui_tabbar_tab_at (bar, 330.0f, 30.0f), ==, -1);
    g_assert_cmpint (grl_ui_tabbar_tab_at (bar, 15.0f, 50.0f), ==, -1);

    grl_ui_tabbar_set_show_close_buttons (bar, TRUE);
    grl_ui_tabbar_set_line_on_top (bar, TRUE);
    g_assert_true (grl_ui_tabbar_get_show_close_buttons (bar));
    g_assert_true (grl_ui_tabbar_get_line_on_top (bar));
}

static void
test_ui_new_style_properties (void)
{
    g_autoptr(GrlUiDropdownBox) dropdown = grl_ui_dropdownbox_new ("A;B");
    g_autoptr(GrlUiProgressBar) bar = grl_ui_progressbar_new (0.0f, 100.0f);
    gboolean value;

    g_assert_false (grl_ui_dropdownbox_get_roll_up (dropdown));
    g_object_set (dropdown, "roll-up", TRUE, NULL);
    g_object_get (dropdown, "roll-up", &value, NULL);
    g_assert_true (value);

    g_assert_false (grl_ui_progressbar_get_right_to_left (bar));
    grl_ui_progressbar_set_right_to_left (bar, TRUE);
    g_assert_true (grl_ui_progressbar_get_right_to_left (bar));
}

static void
test_ui_style_from_memory_rejects (void)
{
    GrlUiStyle *style = grl_ui_style_get_default ();
    static const guchar not_rgs[] = "#\n# text style\np 00 00 0x2f7486ff\n";
    guchar truncated[16];
    gint32 count = 1000;
    g_autoptr(GBytes) text = g_bytes_new_static (not_rgs, sizeof not_rgs);
    g_autoptr(GBytes) trunc = NULL;
    g_autoptr(GError) error = NULL;

    /* a text style is not a binary one */
    g_assert_false (grl_ui_style_load_from_memory (style, text, &error));
    g_assert_error (error, G_IO_ERROR, G_IO_ERROR_INVALID_DATA);
    g_clear_error (&error);

    /* 1000 properties cannot fit in 16 bytes */
    memset (truncated, 0, sizeof truncated);
    memcpy (truncated, "rGS ", 4);
    memcpy (truncated + 8, &count, sizeof count);
    trunc = g_bytes_new (truncated, sizeof truncated);
    g_assert_false (grl_ui_style_load_from_memory (style, trunc, &error));
    g_assert_error (error, G_IO_ERROR, G_IO_ERROR_INVALID_DATA);
}

/*
 * Drawing (needs a GL context)
 */

static GrlImage *
read_back (GrlRenderTexture *target)
{
    GrlTexture *texture = grl_render_texture_get_texture (target);
    GrlImage   *img = grl_texture_to_image (texture);

    /* render textures are stored bottom-up */
    grl_image_flip_vertical (img);
    return img;
}

static void
test_draw_new_shapes (void)
{
    g_autoptr(GrlWindow) window = NULL;
    g_autoptr(GrlRenderTexture) target = NULL;
    g_autoptr(GrlRenderTexture) hdr = NULL;
    g_autoptr(GrlImage) img = NULL;
    g_autoptr(GrlColor) black = grl_color_new (0, 0, 0, 255);
    g_autoptr(GrlColor) red = grl_color_new (255, 0, 0, 255);
    g_autoptr(GrlColor) green = grl_color_new (0, 255, 0, 255);
    g_autoptr(GrlColor) blue = grl_color_new (0, 0, 255, 255);
    g_autoptr(GrlColor) white = grl_color_new (255, 255, 255, 255);
    g_autoptr(GrlRectangle) rect = grl_rectangle_new (0, 0, 32, 32);
    g_autoptr(GrlVector2) center = grl_vector2_new (48.0f, 16.0f);
    g_autoptr(GrlVector2) v1 = grl_vector2_new (4.0f, 36.0f);
    g_autoptr(GrlVector2) v2 = grl_vector2_new (4.0f, 60.0f);
    g_autoptr(GrlVector2) v3 = grl_vector2_new (28.0f, 60.0f);

    SKIP_IF_NO_DISPLAY ();

    window = grl_window_new (64, 64, "test-raylib62");
    if (window == NULL || !grl_window_is_ready (window))
    {
        g_test_skip ("Window/GL context not ready in this environment");
        return;
    }

    target = grl_render_texture_new (64, 64);
    g_assert_true (grl_render_texture_is_valid (target));
    g_assert_cmpint (grl_render_texture_get_format (target), ==,
                     GRL_PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);

    grl_render_texture_begin (target);
    grl_draw_clear_background (black);
    grl_draw_rectangle_gradient_ex (rect, red, green, blue, white);
    grl_draw_circle_lines_ex (center, 12.0f, 3.0f, white);
    grl_draw_triangle_gradient (v1, v2, v3, red, green, blue);
    grl_render_texture_end (target);

    img = read_back (target);

    /* the corners land where their names say (graylib used to swap the
     * right-hand pair) */
    assert_pixel (img, 0, 0, 255, 0, 0, 24);
    assert_pixel (img, 0, 31, 0, 255, 0, 24);
    assert_pixel (img, 31, 0, 0, 0, 255, 24);
    assert_pixel (img, 31, 31, 255, 255, 255, 24);

    /* a 3 px outline inside radius 12: lit at r=10.5, dark at the centre
     * and outside */
    assert_pixel (img, 48 + 10, 16, 255, 255, 255, 40);
    assert_pixel (img, 48, 16, 0, 0, 0, 0);
    assert_pixel (img, 48 + 14, 16, 0, 0, 0, 0);

    /* the gradient triangle is lit near its first vertex */
    assert_pixel (img, 6, 40, 255, 0, 0, 90);

    /* a float colour attachment */
    hdr = grl_render_texture_new_with_format (32, 32,
                                              GRL_PIXELFORMAT_UNCOMPRESSED_R32G32B32A32);
    g_assert_true (grl_render_texture_is_valid (hdr));
    g_assert_cmpint (grl_render_texture_get_format (hdr), ==,
                     GRL_PIXELFORMAT_UNCOMPRESSED_R32G32B32A32);
}

int
main (int   argc,
      char *argv[])
{
    g_test_init (&argc, &argv, NULL);

    g_test_add_func ("/raylib62/fs/path-predicates", test_fs_path_predicates);
    g_test_add_func ("/raylib62/fs/copy-semantics", test_fs_copy_semantics);
    g_test_add_func ("/raylib62/fs/change-directory", test_fs_change_directory);
    g_test_add_func ("/raylib62/image/blits", test_image_blits);
    g_test_add_func ("/raylib62/image/gradients", test_image_gradients);
    g_test_add_func ("/raylib62/ui/tabbar-properties", test_ui_tabbar_properties);
    g_test_add_func ("/raylib62/ui/new-style-properties", test_ui_new_style_properties);
    g_test_add_func ("/raylib62/ui/style-from-memory-rejects", test_ui_style_from_memory_rejects);
    g_test_add_func ("/raylib62/draw/new-shapes", test_draw_new_shapes);

    return g_test_run ();
}
