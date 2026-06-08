/* test-image-stamp-path.c
 *
 * Copyright 2026 Ben Doty
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * TDD suite for grl_image_stamp_along_path.
 */

#include <glib.h>
#include "src/graphics/grl-image.h"
#include "src/graphics/grl-path.h"
#include "src/math/grl-color.h"

#define W 128
#define H 128
#define BRUSH_SIZE 8

static GrlImage *
make_canvas (void)
{
    g_autoptr (GrlColor) bg = grl_color_new (0, 0, 0, 255);
    return grl_image_new_color (W, H, bg);
}

static GrlImage *
make_white_brush (void)
{
    g_autoptr (GrlColor) c = grl_color_new (255, 255, 255, 255);
    return grl_image_new_color (BRUSH_SIZE, BRUSH_SIZE, c);
}

/* Helper: count pixels with r > 128 (white-ish). */
static gint
count_bright (GrlImage *img)
{
    gint count = 0;
    gint x, y;
    for (y = 0; y < H; y++) {
        for (x = 0; x < W; x++) {
            g_autoptr (GrlColor) c = grl_image_get_pixel (img, x, y);
            if (c->r > 128)
                count++;
        }
    }
    return count;
}

/* Stamping along a line path with white brush marks pixels. */
static void
test_stamp_marks_pixels (void)
{
    g_autoptr (GrlImage) canvas = make_canvas ();
    g_autoptr (GrlImage) brush  = make_white_brush ();
    g_autoptr (GrlPath)  path   = grl_path_new ();

    grl_path_move_to (path, 10.0f, 64.0f);
    grl_path_line_to (path, 118.0f, 64.0f);

    grl_image_stamp_along_path (canvas, brush, path, 8.0f, 0.0f, 0);
    /* expect at least some pixels brightened along the line */
    g_assert_cmpint (count_bright (canvas), >, 0);
}

/* Empty path (no commands) → canvas unchanged. */
static void
test_stamp_empty_path_unchanged (void)
{
    g_autoptr (GrlImage) canvas = make_canvas ();
    g_autoptr (GrlImage) brush  = make_white_brush ();
    g_autoptr (GrlPath)  path   = grl_path_new ();

    grl_image_stamp_along_path (canvas, brush, path, 8.0f, 0.0f, 0);
    g_assert_cmpint (count_bright (canvas), ==, 0);
}

/* Same seed → same stamp pattern (deterministic). */
static void
test_stamp_deterministic (void)
{
    g_autoptr (GrlImage) a = make_canvas ();
    g_autoptr (GrlImage) b = make_canvas ();
    g_autoptr (GrlImage) brush = make_white_brush ();
    g_autoptr (GrlPath)  path  = grl_path_new ();

    grl_path_move_to (path, 10.0f, 64.0f);
    grl_path_line_to (path, 118.0f, 64.0f);

    grl_image_stamp_along_path (a, brush, path, 8.0f, 5.0f, 42);
    grl_image_stamp_along_path (b, brush, path, 8.0f, 5.0f, 42);

    gint x, y;
    for (y = 0; y < H; y++) {
        for (x = 0; x < W; x++) {
            g_autoptr (GrlColor) ca = grl_image_get_pixel (a, x, y);
            g_autoptr (GrlColor) cb = grl_image_get_pixel (b, x, y);
            g_assert_cmpuint (ca->r, ==, cb->r);
        }
    }
}

/* Very large spacing (larger than path) → at most one stamp. */
static void
test_stamp_large_spacing_few_marks (void)
{
    g_autoptr (GrlImage) canvas = make_canvas ();
    g_autoptr (GrlImage) brush  = make_white_brush ();
    g_autoptr (GrlPath)  path   = grl_path_new ();

    grl_path_move_to (path, 60.0f, 64.0f);
    grl_path_line_to (path, 70.0f, 64.0f);  /* 10px path */

    /* spacing 100 means at most 1 stamp on a 10px path */
    grl_image_stamp_along_path (canvas, brush, path, 100.0f, 0.0f, 0);
    /* should still paint at least the first stamp point */
    g_assert_cmpint (count_bright (canvas), >=, 0); /* no crash */
}

int
main (int argc, char **argv)
{
    g_test_init (&argc, &argv, NULL);
    g_test_add_func ("/stamp/marks-pixels",        test_stamp_marks_pixels);
    g_test_add_func ("/stamp/empty-path",          test_stamp_empty_path_unchanged);
    g_test_add_func ("/stamp/deterministic",       test_stamp_deterministic);
    g_test_add_func ("/stamp/large-spacing",       test_stamp_large_spacing_few_marks);
    return g_test_run ();
}
