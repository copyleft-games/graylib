/* test-image-draw.c
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Unit tests for the GrlImage drawing additions: blend modes, clipping,
 * new primitives, headless bitmap text, foot-gun fixes, resize-to-new, and the
 * animated GIF writer. These all run headless (no window / GL context).
 */

#include <glib.h>
#include <glib/gstdio.h>
#include <string.h>
#include "src/graphics/grl-image.h"
#include "src/graphics/grl-gif-writer.h"
#include "src/grl-enums.h"
#include "src/math/grl-color.h"
#include "src/math/grl-vector2.h"
#include "src/math/grl-rectangle.h"

static void
assert_pixel (GrlImage *img,
              gint      x,
              gint      y,
              gint      r,
              gint      g,
              gint      b,
              gint      a,
              gint      tol)
{
    g_autoptr(GrlColor) c = grl_image_get_pixel (img, x, y);

    g_assert_nonnull (c);
    g_assert_cmpint (ABS ((gint)c->r - r), <=, tol);
    g_assert_cmpint (ABS ((gint)c->g - g), <=, tol);
    g_assert_cmpint (ABS ((gint)c->b - b), <=, tol);
    g_assert_cmpint (ABS ((gint)c->a - a), <=, tol);
}

/*
 * Drawing state
 */

static void
test_state_defaults (void)
{
    g_autoptr(GrlColor) bg = grl_color_new (0, 0, 0, 255);
    g_autoptr(GrlImage) img = grl_image_new_color (8, 8, bg);
    GrlRectangle clip;

    g_assert_cmpint (grl_image_get_blend_mode (img), ==, GRL_IMAGE_BLEND_REPLACE);
    g_assert_false (grl_image_get_antialias (img));
    g_assert_false (grl_image_get_clip_rect (img, &clip));
}

static void
test_state_roundtrip (void)
{
    g_autoptr(GrlColor) bg = grl_color_new (0, 0, 0, 255);
    g_autoptr(GrlImage) img = grl_image_new_color (16, 16, bg);
    g_autoptr(GrlRectangle) r = grl_rectangle_new (2.0f, 3.0f, 4.0f, 5.0f);
    GrlRectangle out;

    grl_image_set_blend_mode (img, GRL_IMAGE_BLEND_ADD);
    g_assert_cmpint (grl_image_get_blend_mode (img), ==, GRL_IMAGE_BLEND_ADD);

    grl_image_set_antialias (img, TRUE);
    g_assert_true (grl_image_get_antialias (img));

    grl_image_set_clip_rect (img, r);
    g_assert_true (grl_image_get_clip_rect (img, &out));
    g_assert_cmpint ((gint)out.x, ==, 2);
    g_assert_cmpint ((gint)out.width, ==, 4);

    grl_image_set_clip_rect (img, NULL);
    g_assert_false (grl_image_get_clip_rect (img, &out));
}

/*
 * Blend math
 */

static void
test_blend_replace_default (void)
{
    g_autoptr(GrlColor) bg = grl_color_new (255, 255, 255, 255);
    g_autoptr(GrlColor) c = grl_color_new (10, 20, 30, 128);
    g_autoptr(GrlImage) img = grl_image_new_color (8, 8, bg);
    g_autoptr(GrlRectangle) rect = grl_rectangle_new (0.0f, 0.0f, 8.0f, 8.0f);

    /* Default REPLACE: a non-new primitive overwrites exactly (raylib path). */
    grl_image_draw_rectangle (img, rect, c);
    assert_pixel (img, 4, 4, 10, 20, 30, 128, 0);
}

static void
test_blend_over (void)
{
    g_autoptr(GrlColor) bg = grl_color_new (255, 255, 255, 255);
    g_autoptr(GrlColor) red = grl_color_new (255, 0, 0, 128);
    g_autoptr(GrlImage) img = grl_image_new_color (8, 8, bg);

    grl_image_set_blend_mode (img, GRL_IMAGE_BLEND_OVER);
    grl_image_draw_ellipse (img, 4, 4, 4, 4, red);

    /* Red(128) over white -> (255, ~127, ~127, 255). */
    assert_pixel (img, 4, 4, 255, 127, 127, 255, 2);
}

static void
test_blend_add_saturates (void)
{
    g_autoptr(GrlColor) bg = grl_color_new (200, 200, 200, 255);
    g_autoptr(GrlColor) add = grl_color_new (100, 100, 100, 255);
    g_autoptr(GrlImage) img = grl_image_new_color (8, 8, bg);

    grl_image_set_blend_mode (img, GRL_IMAGE_BLEND_ADD);
    grl_image_draw_ellipse (img, 4, 4, 4, 4, add);

    /* 200 + 100 = 300 -> clamps to 255. */
    assert_pixel (img, 4, 4, 255, 255, 255, 255, 1);
}

static void
test_blend_requires_rgba (void)
{
    g_autoptr(GrlColor) bg = grl_color_new (200, 200, 200, 255);
    g_autoptr(GrlColor) add = grl_color_new (100, 100, 100, 255);
    g_autoptr(GrlImage) img = grl_image_new_color (8, 8, bg);

    /* On a non-RGBA8 image, blending falls back to REPLACE (no crash). */
    grl_image_set_format (img, GRL_PIXELFORMAT_UNCOMPRESSED_R8G8B8);
    grl_image_set_blend_mode (img, GRL_IMAGE_BLEND_ADD);
    grl_image_draw_ellipse (img, 4, 4, 4, 4, add);

    assert_pixel (img, 4, 4, 100, 100, 100, 255, 1);
}

/*
 * Clipping
 */

static void
test_clip_rect (void)
{
    g_autoptr(GrlColor) bg = grl_color_new (0, 0, 0, 255);
    g_autoptr(GrlColor) white = grl_color_new (255, 255, 255, 255);
    g_autoptr(GrlImage) img = grl_image_new_color (32, 32, bg);
    g_autoptr(GrlRectangle) clip = grl_rectangle_new (10.0f, 10.0f, 4.0f, 4.0f);

    grl_image_set_clip_rect (img, clip);
    /* A big ellipse, but only the clip box may change. */
    grl_image_draw_ellipse (img, 16, 16, 15, 15, white);

    assert_pixel (img, 11, 11, 255, 255, 255, 255, 0);   /* inside clip */
    assert_pixel (img, 16, 16, 0, 0, 0, 255, 0);         /* outside clip */
    assert_pixel (img, 2, 2, 0, 0, 0, 255, 0);           /* far outside */
}

/*
 * Primitives smoke tests (must not crash, must mark the canvas)
 */

static void
test_primitives_smoke (void)
{
    g_autoptr(GrlColor) bg = grl_color_new (0, 0, 0, 255);
    g_autoptr(GrlColor) col = grl_color_new (255, 255, 255, 255);
    g_autoptr(GrlImage) img = grl_image_new_color (64, 64, bg);
    g_autoptr(GrlVector2) a = grl_vector2_new (4.0f, 4.0f);
    g_autoptr(GrlVector2) b = grl_vector2_new (60.0f, 60.0f);
    g_autoptr(GrlVector2) v1 = grl_vector2_new (10.0f, 50.0f);
    g_autoptr(GrlVector2) v2 = grl_vector2_new (32.0f, 8.0f);
    g_autoptr(GrlVector2) v3 = grl_vector2_new (54.0f, 50.0f);
    g_autoptr(GrlRectangle) grad = grl_rectangle_new (0.0f, 0.0f, 64.0f, 8.0f);
    GrlVector2 poly[4];

    grl_image_set_antialias (img, TRUE);

    grl_image_draw_line_ex (img, a, b, 3, col);
    grl_image_draw_circle_lines (img, 32, 32, 20, 2, col);
    grl_image_draw_ellipse_lines (img, 32, 32, 24, 12, 2, col);
    grl_image_draw_triangle (img, v1, v2, v3, col);
    grl_image_draw_triangle_lines (img, v1, v2, v3, 2, col);
    grl_image_draw_bezier (img, a, v2, v3, b, 2, col);

    poly[0] = (GrlVector2){ 5.0f, 5.0f };
    poly[1] = (GrlVector2){ 30.0f, 8.0f };
    poly[2] = (GrlVector2){ 25.0f, 28.0f };
    poly[3] = (GrlVector2){ 8.0f, 24.0f };
    grl_image_draw_polygon (img, poly, 4, col);
    grl_image_draw_polyline (img, poly, 4, TRUE, 2, col);

    grl_image_draw_gradient_rect (img, grad, bg, col, GRL_GRADIENT_AXIS_HORIZONTAL);

    /* Nothing to assert precisely; reaching here without a crash is the test. */
    g_assert_true (grl_image_is_valid (img));
}

static void
test_gradient_radial (void)
{
    g_autoptr(GrlColor) bg = grl_color_new (0, 0, 0, 255);
    g_autoptr(GrlColor) inner = grl_color_new (255, 255, 255, 255);
    g_autoptr(GrlColor) outer = grl_color_new (0, 0, 0, 255);
    g_autoptr(GrlImage) img = grl_image_new_color (32, 32, bg);

    grl_image_draw_gradient_radial (img, 16, 16, 12, inner, outer);

    assert_pixel (img, 16, 16, 255, 255, 255, 255, 8);   /* bright center */
    assert_pixel (img, 0, 0, 0, 0, 0, 255, 2);           /* untouched corner */
}

static void
test_flood_fill (void)
{
    g_autoptr(GrlColor) bg = grl_color_new (0, 0, 0, 255);
    g_autoptr(GrlColor) red = grl_color_new (255, 0, 0, 255);
    g_autoptr(GrlImage) img = grl_image_new_color (16, 16, bg);

    grl_image_flood_fill (img, 0, 0, red, 0);

    assert_pixel (img, 0, 0, 255, 0, 0, 255, 0);
    assert_pixel (img, 15, 15, 255, 0, 0, 255, 0);
}

/*
 * Foot-gun fix: NULL tint on draw_image
 */

static void
test_draw_image_null_tint (void)
{
    g_autoptr(GrlColor) bg = grl_color_new (0, 0, 0, 255);
    g_autoptr(GrlColor) src_col = grl_color_new (40, 80, 120, 255);
    g_autoptr(GrlImage) dst = grl_image_new_color (16, 16, bg);
    g_autoptr(GrlImage) src = grl_image_new_color (8, 8, src_col);
    g_autoptr(GrlRectangle) dst_rect = grl_rectangle_new (0.0f, 0.0f, 8.0f, 8.0f);

    /* NULL tint must behave as opaque white (no CRITICAL, no crash). */
    grl_image_draw_image (dst, src, NULL, dst_rect, NULL);
    assert_pixel (dst, 2, 2, 40, 80, 120, 255, 0);
}

/*
 * Headless bitmap text
 */

static void
test_text_bitmap_headless (void)
{
    g_autoptr(GrlColor) bg = grl_color_new (0, 0, 0, 255);
    g_autoptr(GrlColor) white = grl_color_new (255, 255, 255, 255);
    g_autoptr(GrlImage) img = grl_image_new_color (128, 32, bg);
    g_autoptr(GrlVector2) size = NULL;

    /* Must not crash with no window / GL context. */
    grl_image_draw_text_bitmap (img, "Hello, World!", 2, 2, 20, white);
    /* draw_text falls back to the bitmap path when headless. */
    grl_image_draw_text (img, "Fallback", 2, 16, 10, white);

    size = grl_image_measure_text_bitmap ("Hello", 20);
    g_assert_nonnull (size);
    g_assert_cmpfloat (size->x, >, 0.0f);
    g_assert_cmpfloat (size->y, >, 0.0f);

    g_assert_true (grl_image_is_valid (img));
}

/*
 * Resize-to-new variants
 */

static void
test_resized (void)
{
    g_autoptr(GrlColor) bg = grl_color_new (10, 20, 30, 255);
    g_autoptr(GrlImage) img = grl_image_new_color (16, 16, bg);
    g_autoptr(GrlImage) bigger = grl_image_resized (img, 64, 48);
    g_autoptr(GrlImage) smaller = grl_image_scaled_nearest (img, 8, 8);

    /* Original is untouched. */
    g_assert_cmpint (grl_image_get_width (img), ==, 16);
    g_assert_cmpint (grl_image_get_height (img), ==, 16);

    g_assert_cmpint (grl_image_get_width (bigger), ==, 64);
    g_assert_cmpint (grl_image_get_height (bigger), ==, 48);

    g_assert_cmpint (grl_image_get_width (smaller), ==, 8);
    g_assert_cmpint (grl_image_get_height (smaller), ==, 8);
}

/*
 * Animated GIF writer
 */

static void
test_gif_writer (void)
{
    g_autoptr(GError) error = NULL;
    g_autofree gchar *path = g_build_filename (g_get_tmp_dir (),
                                               "grl-test-anim.gif", NULL);
    GrlGifWriter *writer;
    g_autoptr(GrlColor) red = grl_color_new (255, 0, 0, 255);
    g_autoptr(GrlColor) green = grl_color_new (0, 255, 0, 255);
    g_autoptr(GrlColor) blue = grl_color_new (0, 0, 255, 255);
    g_autoptr(GrlImage) f1 = grl_image_new_color (16, 16, red);
    g_autoptr(GrlImage) f2 = grl_image_new_color (16, 16, green);
    g_autoptr(GrlImage) f3 = grl_image_new_color (16, 16, blue);
    g_autofree gchar *data = NULL;
    gsize len = 0;
    gsize i, frames = 0;

    writer = grl_gif_writer_new (path, 16, 16, 0, &error);
    g_assert_no_error (error);
    g_assert_nonnull (writer);

    g_assert_true (grl_gif_writer_add_frame (writer, f1, 10, &error));
    g_assert_no_error (error);
    g_assert_true (grl_gif_writer_add_frame (writer, f2, 10, &error));
    g_assert_no_error (error);
    g_assert_true (grl_gif_writer_add_frame (writer, f3, 10, &error));
    g_assert_no_error (error);

    g_assert_true (grl_gif_writer_close (writer, &error));
    g_assert_no_error (error);
    g_object_unref (writer);

    /* Validate the file: GIF89a header, three Graphic Control Extensions,
     * and a trailer byte. */
    g_assert_true (g_file_get_contents (path, &data, &len, &error));
    g_assert_no_error (error);
    g_assert_cmpuint (len, >, 14);
    g_assert_cmpint (memcmp (data, "GIF89a", 6), ==, 0);

    for (i = 0; i + 2 < len; i++)
    {
        if ((guchar)data[i] == 0x21 && (guchar)data[i + 1] == 0xf9 &&
            (guchar)data[i + 2] == 0x04)
            frames++;
    }
    g_assert_cmpuint (frames, ==, 3);
    g_assert_cmpint ((guchar)data[len - 1], ==, 0x3b);

    g_unlink (path);
}

/*
 * Blend modes (the remaining two, plus alpha edge cases)
 */

static void
test_blend_multiply (void)
{
    g_autoptr(GrlColor) bg = grl_color_new (200, 200, 200, 255);
    g_autoptr(GrlColor) half = grl_color_new (128, 128, 128, 255);
    g_autoptr(GrlImage) img = grl_image_new_color (8, 8, bg);

    grl_image_set_blend_mode (img, GRL_IMAGE_BLEND_MULTIPLY);
    grl_image_draw_ellipse (img, 4, 4, 4, 4, half);

    /* 200 * 128/255 ~= 100; alpha unchanged. */
    assert_pixel (img, 4, 4, 100, 100, 100, 255, 2);
}

static void
test_blend_subtract (void)
{
    g_autoptr(GrlColor) bg = grl_color_new (200, 200, 200, 255);
    g_autoptr(GrlColor) sub = grl_color_new (50, 50, 50, 255);
    g_autoptr(GrlImage) img = grl_image_new_color (8, 8, bg);

    grl_image_set_blend_mode (img, GRL_IMAGE_BLEND_SUBTRACT);
    grl_image_draw_ellipse (img, 4, 4, 4, 4, sub);

    /* 200 - 50 = 150; alpha unchanged. */
    assert_pixel (img, 4, 4, 150, 150, 150, 255, 2);
}

static void
test_blend_transparent_noop (void)
{
    g_autoptr(GrlColor) bg = grl_color_new (100, 100, 100, 255);
    g_autoptr(GrlColor) clear = grl_color_new (255, 0, 0, 0);
    g_autoptr(GrlImage) img = grl_image_new_color (8, 8, bg);

    grl_image_set_blend_mode (img, GRL_IMAGE_BLEND_OVER);
    grl_image_draw_ellipse (img, 4, 4, 4, 4, clear);

    /* Fully transparent source leaves the destination untouched. */
    assert_pixel (img, 4, 4, 100, 100, 100, 255, 0);
}

static void
test_blend_over_onto_transparent (void)
{
    g_autoptr(GrlColor) bg = grl_color_new (0, 0, 0, 0);
    g_autoptr(GrlColor) red = grl_color_new (255, 0, 0, 128);
    g_autoptr(GrlImage) img = grl_image_new_color (8, 8, bg);

    grl_image_set_blend_mode (img, GRL_IMAGE_BLEND_OVER);
    grl_image_draw_ellipse (img, 4, 4, 4, 4, red);

    /* Over an empty buffer: color preserved, alpha = source alpha. */
    assert_pixel (img, 4, 4, 255, 0, 0, 128, 2);
}

/*
 * Clip clearing
 */

static void
test_clip_cleared (void)
{
    g_autoptr(GrlColor) bg = grl_color_new (0, 0, 0, 255);
    g_autoptr(GrlColor) white = grl_color_new (255, 255, 255, 255);
    g_autoptr(GrlImage) img = grl_image_new_color (32, 32, bg);
    g_autoptr(GrlRectangle) clip = grl_rectangle_new (10.0f, 10.0f, 4.0f, 4.0f);

    grl_image_set_clip_rect (img, clip);
    grl_image_set_clip_rect (img, NULL);   /* clear it again */
    grl_image_draw_ellipse (img, 16, 16, 15, 15, white);

    /* With the clip cleared, the center (outside the old clip) is drawn. */
    assert_pixel (img, 16, 16, 255, 255, 255, 255, 0);
}

/*
 * Primitive pixel correctness (REPLACE, AA off => crisp)
 */

static void
test_triangle_fill_coverage (void)
{
    g_autoptr(GrlColor) bg = grl_color_new (0, 0, 0, 255);
    g_autoptr(GrlColor) white = grl_color_new (255, 255, 255, 255);
    g_autoptr(GrlImage) img = grl_image_new_color (32, 32, bg);
    g_autoptr(GrlVector2) v1 = grl_vector2_new (4.0f, 4.0f);
    g_autoptr(GrlVector2) v2 = grl_vector2_new (28.0f, 4.0f);
    g_autoptr(GrlVector2) v3 = grl_vector2_new (16.0f, 28.0f);

    grl_image_draw_triangle (img, v1, v2, v3, white);

    assert_pixel (img, 16, 12, 255, 255, 255, 255, 0);   /* interior */
    assert_pixel (img, 1, 1, 0, 0, 0, 255, 0);           /* exterior */
}

static void
test_polygon_fill_coverage (void)
{
    g_autoptr(GrlColor) bg = grl_color_new (0, 0, 0, 255);
    g_autoptr(GrlColor) white = grl_color_new (255, 255, 255, 255);
    g_autoptr(GrlImage) img = grl_image_new_color (32, 32, bg);
    GrlVector2 poly[4];

    poly[0] = (GrlVector2){ 4.0f, 4.0f };
    poly[1] = (GrlVector2){ 28.0f, 4.0f };
    poly[2] = (GrlVector2){ 28.0f, 28.0f };
    poly[3] = (GrlVector2){ 4.0f, 28.0f };
    grl_image_draw_polygon (img, poly, 4, white);

    assert_pixel (img, 16, 16, 255, 255, 255, 255, 0);   /* interior */
    assert_pixel (img, 1, 1, 0, 0, 0, 255, 0);           /* exterior */
}

static void
test_circle_lines_ring (void)
{
    g_autoptr(GrlColor) bg = grl_color_new (0, 0, 0, 255);
    g_autoptr(GrlColor) white = grl_color_new (255, 255, 255, 255);
    g_autoptr(GrlImage) img = grl_image_new_color (32, 32, bg);

    grl_image_draw_circle_lines (img, 16, 16, 10, 2, white);

    assert_pixel (img, 16, 6, 255, 255, 255, 255, 0);    /* on the ring */
    assert_pixel (img, 16, 16, 0, 0, 0, 255, 0);         /* center is hollow */
}

static void
test_line_ex_covers_midpoint (void)
{
    g_autoptr(GrlColor) bg = grl_color_new (0, 0, 0, 255);
    g_autoptr(GrlColor) white = grl_color_new (255, 255, 255, 255);
    g_autoptr(GrlImage) img = grl_image_new_color (32, 32, bg);
    g_autoptr(GrlVector2) a = grl_vector2_new (0.0f, 0.0f);
    g_autoptr(GrlVector2) b = grl_vector2_new (31.0f, 31.0f);

    grl_image_draw_line_ex (img, a, b, 1, white);

    assert_pixel (img, 16, 16, 255, 255, 255, 255, 0);   /* on the diagonal */
    assert_pixel (img, 2, 28, 0, 0, 0, 255, 0);          /* well off the line */
}

static void
test_gradient_rect_axes (void)
{
    g_autoptr(GrlColor) bg = grl_color_new (0, 0, 0, 255);
    g_autoptr(GrlColor) red = grl_color_new (255, 0, 0, 255);
    g_autoptr(GrlColor) blue = grl_color_new (0, 0, 255, 255);
    g_autoptr(GrlImage) himg = grl_image_new_color (16, 8, bg);
    g_autoptr(GrlImage) vimg = grl_image_new_color (8, 16, bg);
    g_autoptr(GrlRectangle) hrect = grl_rectangle_new (0.0f, 0.0f, 16.0f, 8.0f);
    g_autoptr(GrlRectangle) vrect = grl_rectangle_new (0.0f, 0.0f, 8.0f, 16.0f);

    grl_image_draw_gradient_rect (himg, hrect, red, blue, GRL_GRADIENT_AXIS_HORIZONTAL);
    assert_pixel (himg, 0, 4, 255, 0, 0, 255, 2);        /* left = a */
    assert_pixel (himg, 15, 4, 0, 0, 255, 255, 2);       /* right = b */

    grl_image_draw_gradient_rect (vimg, vrect, red, blue, GRL_GRADIENT_AXIS_VERTICAL);
    assert_pixel (vimg, 4, 0, 255, 0, 0, 255, 2);        /* top = a */
    assert_pixel (vimg, 4, 15, 0, 0, 255, 255, 2);       /* bottom = b */
}

/*
 * Flood fill variants
 */

static void
test_flood_fill_bounded (void)
{
    g_autoptr(GrlColor) bg = grl_color_new (0, 0, 0, 255);
    g_autoptr(GrlColor) white = grl_color_new (255, 255, 255, 255);
    g_autoptr(GrlColor) red = grl_color_new (255, 0, 0, 255);
    g_autoptr(GrlImage) img = grl_image_new_color (16, 16, bg);
    g_autoptr(GrlRectangle) barrier = grl_rectangle_new (8.0f, 0.0f, 1.0f, 16.0f);

    /* A full-height white barrier splits the canvas at x=8. (A drawn line would
     * leave a 1px gap at its endpoint; a filled rect seals the column.) */
    grl_image_draw_rectangle (img, barrier, white);
    grl_image_flood_fill (img, 0, 0, red, 0);

    assert_pixel (img, 7, 7, 255, 0, 0, 255, 0);         /* left of barrier */
    assert_pixel (img, 8, 5, 255, 255, 255, 255, 0);     /* barrier intact */
    assert_pixel (img, 12, 12, 0, 0, 0, 255, 0);         /* right side untouched */
}

static void
test_flood_fill_respects_clip (void)
{
    g_autoptr(GrlColor) bg = grl_color_new (0, 0, 0, 255);
    g_autoptr(GrlColor) red = grl_color_new (255, 0, 0, 255);
    g_autoptr(GrlImage) img = grl_image_new_color (16, 16, bg);
    g_autoptr(GrlRectangle) clip = grl_rectangle_new (0.0f, 0.0f, 8.0f, 16.0f);

    grl_image_set_clip_rect (img, clip);
    grl_image_flood_fill (img, 0, 0, red, 0);

    assert_pixel (img, 7, 7, 255, 0, 0, 255, 0);         /* inside clip */
    assert_pixel (img, 9, 7, 0, 0, 0, 255, 0);           /* outside clip untouched */
}

static void
test_flood_fill_noop (void)
{
    g_autoptr(GrlColor) bg = grl_color_new (0, 0, 0, 255);
    g_autoptr(GrlColor) same = grl_color_new (0, 0, 0, 255);
    g_autoptr(GrlImage) img = grl_image_new_color (16, 16, bg);

    /* Seed already equals the fill: must return immediately (no infinite loop). */
    grl_image_flood_fill (img, 0, 0, same, 0);
    assert_pixel (img, 8, 8, 0, 0, 0, 255, 0);
}

/*
 * Off-canvas primitives must clip, not crash
 */

static void
test_offcanvas_no_crash (void)
{
    g_autoptr(GrlColor) bg = grl_color_new (0, 0, 0, 255);
    g_autoptr(GrlColor) white = grl_color_new (255, 255, 255, 255);
    g_autoptr(GrlImage) img = grl_image_new_color (16, 16, bg);
    g_autoptr(GrlVector2) a = grl_vector2_new (-20.0f, -20.0f);
    g_autoptr(GrlVector2) b = grl_vector2_new (40.0f, 40.0f);

    grl_image_draw_circle_lines (img, -5, -5, 12, 3, white);
    grl_image_draw_ellipse (img, 20, 20, 10, 10, white);
    grl_image_draw_line_ex (img, a, b, 4, white);
    grl_image_flood_fill (img, -1, -1, white, 0);   /* seed out of bounds */

    g_assert_true (grl_image_is_valid (img));
}

/*
 * Multi-line text
 */

static void
test_text_multiline (void)
{
    g_autoptr(GrlColor) bg = grl_color_new (0, 0, 0, 255);
    g_autoptr(GrlColor) white = grl_color_new (255, 255, 255, 255);
    g_autoptr(GrlImage) img = grl_image_new_color (64, 64, bg);
    g_autoptr(GrlVector2) two = grl_image_measure_text_bitmap ("ab\ncd", 10);
    g_autoptr(GrlVector2) one = grl_image_measure_text_bitmap ("ab", 10);

    /* Two lines are twice as tall; width is the widest line. */
    g_assert_cmpfloat_with_epsilon (two->y, 20.0f, 0.01f);
    g_assert_cmpfloat (two->x, >=, one->x);

    grl_image_draw_text_bitmap (img, "Line1\nLine2", 2, 2, 10, white);
    g_assert_true (grl_image_is_valid (img));
}

/*
 * Arbitrary-angle rotation
 */

static void
test_rotate (void)
{
    g_autoptr(GrlColor) bg = grl_color_new (10, 20, 30, 255);
    g_autoptr(GrlImage) square = grl_image_new_color (64, 32, bg);
    g_autoptr(GrlImage) arbitrary = grl_image_new_color (32, 32, bg);

    /* 90 degrees swaps dimensions. */
    grl_image_rotate (square, 90);
    g_assert_cmpint (grl_image_get_width (square), ==, 32);
    g_assert_cmpint (grl_image_get_height (square), ==, 64);

    /* Arbitrary angle must not crash and keeps a valid image. */
    grl_image_rotate (arbitrary, 45);
    g_assert_true (grl_image_is_valid (arbitrary));
}

/*
 * GIF writer edge cases
 */

static void
test_gif_after_close_errors (void)
{
    g_autoptr(GError) error = NULL;
    g_autofree gchar *path = g_build_filename (g_get_tmp_dir (),
                                               "grl-test-closed.gif", NULL);
    GrlGifWriter *writer = grl_gif_writer_new (path, 8, 8, 0, &error);
    g_autoptr(GrlColor) red = grl_color_new (255, 0, 0, 255);
    g_autoptr(GrlImage) f = grl_image_new_color (8, 8, red);

    g_assert_nonnull (writer);
    g_assert_true (grl_gif_writer_add_frame (writer, f, 10, &error));
    g_assert_true (grl_gif_writer_close (writer, &error));

    /* Adding after close fails with GRL_GIF_ERROR_CLOSED. */
    g_assert_false (grl_gif_writer_add_frame (writer, f, 10, &error));
    g_assert_error (error, GRL_GIF_ERROR, GRL_GIF_ERROR_CLOSED);

    g_object_unref (writer);
    g_unlink (path);
}

static void
test_gif_frame_resize_and_count (void)
{
    g_autoptr(GError) error = NULL;
    g_autofree gchar *path = g_build_filename (g_get_tmp_dir (),
                                               "grl-test-resize.gif", NULL);
    GrlGifWriter *writer = grl_gif_writer_new (path, 16, 16, 0, &error);
    g_autoptr(GrlColor) red = grl_color_new (255, 0, 0, 255);
    g_autoptr(GrlImage) odd = grl_image_new_color (5, 9, red);   /* mismatched size */
    g_autofree gchar *data = NULL;
    gsize len = 0, i, frames = 0;

    g_assert_nonnull (writer);
    /* A differently-sized frame is scaled to the canvas, not rejected. */
    g_assert_true (grl_gif_writer_add_frame (writer, odd, 10, &error));
    g_assert_no_error (error);
    g_assert_true (grl_gif_writer_close (writer, &error));
    g_object_unref (writer);

    g_assert_true (g_file_get_contents (path, &data, &len, &error));
    g_assert_cmpint (memcmp (data, "GIF89a", 6), ==, 0);
    for (i = 0; i + 2 < len; i++)
        if ((guchar)data[i] == 0x21 && (guchar)data[i + 1] == 0xf9 &&
            (guchar)data[i + 2] == 0x04)
            frames++;
    g_assert_cmpuint (frames, ==, 1);   /* exactly one frame */

    g_unlink (path);
}

int
main (int   argc,
      char *argv[])
{
    g_test_init (&argc, &argv, NULL);

    g_test_add_func ("/image-draw/state/defaults", test_state_defaults);
    g_test_add_func ("/image-draw/state/roundtrip", test_state_roundtrip);

    g_test_add_func ("/image-draw/blend/replace-default", test_blend_replace_default);
    g_test_add_func ("/image-draw/blend/over", test_blend_over);
    g_test_add_func ("/image-draw/blend/add-saturates", test_blend_add_saturates);
    g_test_add_func ("/image-draw/blend/multiply", test_blend_multiply);
    g_test_add_func ("/image-draw/blend/subtract", test_blend_subtract);
    g_test_add_func ("/image-draw/blend/transparent-noop", test_blend_transparent_noop);
    g_test_add_func ("/image-draw/blend/over-onto-transparent", test_blend_over_onto_transparent);
    g_test_add_func ("/image-draw/blend/requires-rgba", test_blend_requires_rgba);

    g_test_add_func ("/image-draw/clip-rect", test_clip_rect);
    g_test_add_func ("/image-draw/clip-cleared", test_clip_cleared);

    g_test_add_func ("/image-draw/primitives/smoke", test_primitives_smoke);
    g_test_add_func ("/image-draw/primitives/triangle-fill", test_triangle_fill_coverage);
    g_test_add_func ("/image-draw/primitives/polygon-fill", test_polygon_fill_coverage);
    g_test_add_func ("/image-draw/primitives/circle-lines-ring", test_circle_lines_ring);
    g_test_add_func ("/image-draw/primitives/line-ex-midpoint", test_line_ex_covers_midpoint);
    g_test_add_func ("/image-draw/gradient-rect-axes", test_gradient_rect_axes);
    g_test_add_func ("/image-draw/gradient-radial", test_gradient_radial);

    g_test_add_func ("/image-draw/flood-fill", test_flood_fill);
    g_test_add_func ("/image-draw/flood-fill/bounded", test_flood_fill_bounded);
    g_test_add_func ("/image-draw/flood-fill/respects-clip", test_flood_fill_respects_clip);
    g_test_add_func ("/image-draw/flood-fill/noop", test_flood_fill_noop);

    g_test_add_func ("/image-draw/offcanvas-no-crash", test_offcanvas_no_crash);

    g_test_add_func ("/image-draw/draw-image-null-tint", test_draw_image_null_tint);

    g_test_add_func ("/image-draw/text-bitmap-headless", test_text_bitmap_headless);
    g_test_add_func ("/image-draw/text-multiline", test_text_multiline);

    g_test_add_func ("/image-draw/resized", test_resized);
    g_test_add_func ("/image-draw/rotate", test_rotate);

    g_test_add_func ("/image-draw/gif-writer", test_gif_writer);
    g_test_add_func ("/image-draw/gif/after-close-errors", test_gif_after_close_errors);
    g_test_add_func ("/image-draw/gif/frame-resize", test_gif_frame_resize_and_count);

    return g_test_run ();
}
