/* test-gif-quantize.c
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Unit tests for GrlGifWriter quantization and dithering (median-cut,
 * Floyd-Steinberg, per-frame palettes, transparency).
 *
 * All tests run headless (no window / GL context required).
 */

#include <glib.h>
#include <glib/gstdio.h>
#include <string.h>
#include "src/graphics/grl-image.h"
#include "src/graphics/grl-gif-writer.h"
#include "src/grl-enums.h"
#include "src/math/grl-color.h"
#include "src/math/grl-rectangle.h"

/* -------------------------------------------------------------------------
 * Helpers
 * ------------------------------------------------------------------------- */

/* Build a solid-colour RGBA8 GrlImage. */
static GrlImage *
make_solid (gint width, gint height, guint8 r, guint8 g, guint8 b, guint8 a)
{
    g_autoptr(GrlColor) c = grl_color_new (r, g, b, a);

    return grl_image_new_color (width, height, c);
}

/* Build a horizontal-gradient image: left is @left, right is @right. */
static GrlImage *
make_gradient_h (gint width, gint height,
                 guint8 lr, guint8 lg, guint8 lb,
                 guint8 rr, guint8 rg, guint8 rb)
{
    g_autoptr(GrlColor) left  = grl_color_new (lr, lg, lb, 255);
    g_autoptr(GrlColor) right = grl_color_new (rr, rg, rb, 255);
    g_autoptr(GrlRectangle) rect = grl_rectangle_new (0.0f, 0.0f,
                                                       (gfloat)width,
                                                       (gfloat)height);
    GrlImage *img = grl_image_new_color (width, height, left);

    grl_image_draw_gradient_rect (img, rect, left, right,
                                  GRL_GRADIENT_AXIS_HORIZONTAL);
    return img;
}

/* Count occurrences of the 3-byte GCE marker (0x21 0xf9 0x04) in a buffer. */
static gsize
count_gce_markers (const gchar *data, gsize len)
{
    gsize count;
    gsize i;

    count = 0;
    for (i = 0; i + 2 < len; i++)
    {
        if ((guchar)data[i]     == 0x21 &&
            (guchar)data[i + 1] == 0xf9 &&
            (guchar)data[i + 2] == 0x04)
            count++;
    }
    return count;
}

/* Return a heap-allocated path in the system temp dir. Caller must g_free. */
static gchar *
tmp_path (const gchar *name)
{
    return g_build_filename (g_get_tmp_dir (), name, NULL);
}

/* -------------------------------------------------------------------------
 * Test: web-safe default GIF
 *   - Header begins "GIF89a"
 *   - GCT is the 216-colour web-safe table (golden bytes for 6x6x6)
 * ------------------------------------------------------------------------- */

static void
test_websafe_default_header (void)
{
    g_autoptr(GError)  error = NULL;
    g_autofree gchar  *path  = tmp_path ("grl-test-ws-default.gif");
    g_autoptr(GrlImage) frame = NULL;
    GrlGifWriter *writer;
    gchar *data = NULL;
    gsize  len  = 0;
    gint   i;

    frame  = make_solid (8, 8, 255, 0, 0, 255);
    writer = grl_gif_writer_new (path, 8, 8, 0, &error);
    g_assert_nonnull (writer);
    g_assert_no_error (error);

    g_assert_true (grl_gif_writer_add_frame (writer, frame, 10, &error));
    g_assert_no_error (error);
    g_assert_true (grl_gif_writer_close (writer, &error));
    g_assert_no_error (error);
    g_object_unref (writer);

    g_assert_true (g_file_get_contents (path, &data, &len, &error));
    g_assert_no_error (error);

    /* Must begin with GIF89a. */
    g_assert_cmpuint (len, >=, 6);
    g_assert_cmpint (memcmp (data, "GIF89a", 6), ==, 0);

    /* LSD packed byte at offset 10: 0xf7 (GCT present, 256 entries). */
    g_assert_cmpuint (len, >, 10);
    g_assert_cmpuint ((guchar)data[10], ==, 0xf7);

    /*
     * Verify the first 216 palette entries are the 6x6x6 web-safe colours.
     * GCT starts at offset 13.
     */
    g_assert_cmpuint (len, >, 13 + 768);
    for (i = 0; i < 216; i++)
    {
        guint8 er = (guint8)((i / 36) * 51);
        guint8 eg = (guint8)(((i / 6) % 6) * 51);
        guint8 eb = (guint8)((i % 6) * 51);

        g_assert_cmpuint ((guchar)data[13 + i * 3 + 0], ==, er);
        g_assert_cmpuint ((guchar)data[13 + i * 3 + 1], ==, eg);
        g_assert_cmpuint ((guchar)data[13 + i * 3 + 2], ==, eb);
    }

    g_free (data);
    g_unlink (path);
}

/* -------------------------------------------------------------------------
 * Test: single-frame median-cut GIF
 *   - File begins "GIF89a"
 *   - add_frame returns TRUE
 * ------------------------------------------------------------------------- */

static void
test_median_cut_single_frame (void)
{
    g_autoptr(GError)   error  = NULL;
    g_autofree gchar   *path   = tmp_path ("grl-test-mc-single.gif");
    g_autoptr(GrlImage) frame  = NULL;
    GrlGifWriter *writer;
    gchar *data = NULL;
    gsize  len  = 0;

    frame  = make_solid (16, 16, 100, 150, 200, 255);
    writer = grl_gif_writer_new (path, 16, 16, 0, &error);
    g_assert_nonnull (writer);
    g_assert_no_error (error);

    grl_gif_writer_set_quantizer (writer, GRL_GIF_QUANTIZER_MEDIAN_CUT);

    g_assert_true (grl_gif_writer_add_frame (writer, frame, 10, &error));
    g_assert_no_error (error);
    g_assert_true (grl_gif_writer_close (writer, &error));
    g_assert_no_error (error);
    g_object_unref (writer);

    g_assert_true (g_file_get_contents (path, &data, &len, &error));
    g_assert_no_error (error);

    g_assert_cmpuint (len, >=, 6);
    g_assert_cmpint (memcmp (data, "GIF89a", 6), ==, 0);

    g_free (data);
    g_unlink (path);
}

/* -------------------------------------------------------------------------
 * Test: many unique colours, quantised to max_colors=256
 *   add_frame must return TRUE.
 * ------------------------------------------------------------------------- */

static void
test_median_cut_many_colors (void)
{
    g_autoptr(GError)   error  = NULL;
    g_autofree gchar   *path   = tmp_path ("grl-test-mc-many.gif");
    g_autoptr(GrlImage) frame  = NULL;
    GrlGifWriter *writer;

    /* A gradient has many unique colours. */
    frame  = make_gradient_h (64, 64,
                               0, 0, 0,
                               255, 200, 100);
    writer = grl_gif_writer_new (path, 64, 64, 0, &error);
    g_assert_nonnull (writer);
    g_assert_no_error (error);

    grl_gif_writer_set_quantizer  (writer, GRL_GIF_QUANTIZER_MEDIAN_CUT);
    grl_gif_writer_set_max_colors (writer, 256);

    g_assert_true (grl_gif_writer_add_frame (writer, frame, 5, &error));
    g_assert_no_error (error);
    g_assert_true (grl_gif_writer_close (writer, &error));
    g_assert_no_error (error);
    g_object_unref (writer);

    g_unlink (path);
}

/* -------------------------------------------------------------------------
 * Test: fully-transparent frame with transparency enabled
 * ------------------------------------------------------------------------- */

static void
test_transparency_fully_transparent_frame (void)
{
    g_autoptr(GError)   error  = NULL;
    g_autofree gchar   *path   = tmp_path ("grl-test-mc-transp.gif");
    g_autoptr(GrlImage) frame  = NULL;
    GrlGifWriter *writer;
    gchar *data = NULL;
    gsize  len  = 0;

    /* All pixels fully transparent (alpha=0). */
    frame  = make_solid (8, 8, 0, 0, 0, 0);
    writer = grl_gif_writer_new (path, 8, 8, 0, &error);
    g_assert_nonnull (writer);
    g_assert_no_error (error);

    grl_gif_writer_set_quantizer    (writer, GRL_GIF_QUANTIZER_MEDIAN_CUT);
    grl_gif_writer_set_transparency (writer, TRUE, 128);

    g_assert_true (grl_gif_writer_add_frame (writer, frame, 10, &error));
    g_assert_no_error (error);
    g_assert_true (grl_gif_writer_close (writer, &error));
    g_assert_no_error (error);
    g_object_unref (writer);

    g_assert_true (g_file_get_contents (path, &data, &len, &error));
    g_assert_no_error (error);
    g_assert_cmpint (memcmp (data, "GIF89a", 6), ==, 0);

    g_free (data);
    g_unlink (path);
}

/* -------------------------------------------------------------------------
 * Test: max_colors clamping — values 1 and 1000 must not crash and
 * add_frame must still succeed.
 * ------------------------------------------------------------------------- */

static void
test_max_colors_clamped (void)
{
    g_autoptr(GError)   error  = NULL;
    g_autofree gchar   *path1  = tmp_path ("grl-test-mc-clamp1.gif");
    g_autofree gchar   *path2  = tmp_path ("grl-test-mc-clamp2.gif");
    g_autoptr(GrlImage) frame  = NULL;
    GrlGifWriter *writer;

    frame = make_solid (8, 8, 80, 160, 240, 255);

    /* max_colors = 1 (clamped to 2). */
    writer = grl_gif_writer_new (path1, 8, 8, 0, &error);
    g_assert_nonnull (writer);
    g_assert_no_error (error);
    grl_gif_writer_set_quantizer  (writer, GRL_GIF_QUANTIZER_MEDIAN_CUT);
    grl_gif_writer_set_max_colors (writer, 1);
    g_assert_true (grl_gif_writer_add_frame (writer, frame, 10, &error));
    g_assert_no_error (error);
    g_assert_true (grl_gif_writer_close (writer, &error));
    g_assert_no_error (error);
    g_object_unref (writer);
    g_unlink (path1);

    /* max_colors = 1000 (clamped to 256). */
    writer = grl_gif_writer_new (path2, 8, 8, 0, &error);
    g_assert_nonnull (writer);
    g_assert_no_error (error);
    grl_gif_writer_set_quantizer  (writer, GRL_GIF_QUANTIZER_MEDIAN_CUT);
    grl_gif_writer_set_max_colors (writer, 1000);
    g_assert_true (grl_gif_writer_add_frame (writer, frame, 10, &error));
    g_assert_no_error (error);
    g_assert_true (grl_gif_writer_close (writer, &error));
    g_assert_no_error (error);
    g_object_unref (writer);
    g_unlink (path2);
}

/* -------------------------------------------------------------------------
 * Test: Floyd-Steinberg dither on a gradient frame
 *   - add_frame succeeds
 *   - Running twice yields byte-identical output (deterministic)
 * ------------------------------------------------------------------------- */

static void
test_floyd_steinberg_deterministic (void)
{
    g_autoptr(GError)   error = NULL;
    g_autofree gchar   *path1 = tmp_path ("grl-test-fs-det1.gif");
    g_autofree gchar   *path2 = tmp_path ("grl-test-fs-det2.gif");
    g_autoptr(GrlImage) frame = NULL;
    GrlGifWriter *writer;
    gchar *data1 = NULL;
    gchar *data2 = NULL;
    gsize  len1  = 0;
    gsize  len2  = 0;
    gint   pass;

    frame = make_gradient_h (32, 32,
                              0,   0,   0,
                              255, 255, 255);

    for (pass = 0; pass < 2; pass++)
    {
        const gchar *p = (pass == 0) ? path1 : path2;
        g_autoptr(GrlImage) f = make_gradient_h (32, 32,
                                                  0,   0,   0,
                                                  255, 255, 255);

        writer = grl_gif_writer_new (p, 32, 32, 0, &error);
        g_assert_nonnull (writer);
        g_assert_no_error (error);

        grl_gif_writer_set_quantizer  (writer, GRL_GIF_QUANTIZER_MEDIAN_CUT);
        grl_gif_writer_set_dither     (writer, GRL_GIF_DITHER_FLOYD_STEINBERG);
        grl_gif_writer_set_max_colors (writer, 16);

        g_assert_true (grl_gif_writer_add_frame (writer, f, 10, &error));
        g_assert_no_error (error);
        g_assert_true (grl_gif_writer_close (writer, &error));
        g_assert_no_error (error);
        g_object_unref (writer);
    }

    g_assert_true (g_file_get_contents (path1, &data1, &len1, &error));
    g_assert_no_error (error);
    g_assert_true (g_file_get_contents (path2, &data2, &len2, &error));
    g_assert_no_error (error);

    /* Both runs must produce identical bytes. */
    g_assert_cmpuint (len1, ==, len2);
    g_assert_cmpint (memcmp (data1, data2, len1), ==, 0);

    g_free (data1);
    g_free (data2);
    g_unlink (path1);
    g_unlink (path2);

    /* Suppress unused variable warning. */
    (void)frame;
}

/* -------------------------------------------------------------------------
 * Test: per-frame palette scope — multi-frame file
 *   - Begins "GIF89a"
 *   - Contains exactly N Graphic Control Extension markers (0x21 0xf9 0x04)
 * ------------------------------------------------------------------------- */

static void
test_per_frame_palette_scope (void)
{
    g_autoptr(GError)   error = NULL;
    g_autofree gchar   *path  = tmp_path ("grl-test-mc-perframe.gif");
    g_autoptr(GrlImage) red   = NULL;
    g_autoptr(GrlImage) green = NULL;
    g_autoptr(GrlImage) blue  = NULL;
    GrlGifWriter *writer;
    gchar *data = NULL;
    gsize  len  = 0;
    gsize  n_frames;

    red   = make_solid (8, 8, 255,   0,   0, 255);
    green = make_solid (8, 8,   0, 255,   0, 255);
    blue  = make_solid (8, 8,   0,   0, 255, 255);

    writer = grl_gif_writer_new (path, 8, 8, 0, &error);
    g_assert_nonnull (writer);
    g_assert_no_error (error);

    grl_gif_writer_set_quantizer    (writer, GRL_GIF_QUANTIZER_MEDIAN_CUT);
    grl_gif_writer_set_palette_scope (writer, GRL_GIF_PALETTE_SCOPE_PER_FRAME);

    g_assert_true (grl_gif_writer_add_frame (writer, red,   10, &error));
    g_assert_no_error (error);
    g_assert_true (grl_gif_writer_add_frame (writer, green, 10, &error));
    g_assert_no_error (error);
    g_assert_true (grl_gif_writer_add_frame (writer, blue,  10, &error));
    g_assert_no_error (error);
    g_assert_true (grl_gif_writer_close (writer, &error));
    g_assert_no_error (error);
    g_object_unref (writer);

    g_assert_true (g_file_get_contents (path, &data, &len, &error));
    g_assert_no_error (error);

    g_assert_cmpuint (len, >=, 6);
    g_assert_cmpint (memcmp (data, "GIF89a", 6), ==, 0);

    /* There must be exactly 3 GCE markers — one per frame. */
    n_frames = count_gce_markers (data, len);
    g_assert_cmpuint (n_frames, ==, 3);

    g_free (data);
    g_unlink (path);
}

/* -------------------------------------------------------------------------
 * Test: global scope multi-frame GIF
 *   - Exactly 2 GCE markers for 2 frames
 *   - Begins "GIF89a"
 * ------------------------------------------------------------------------- */

static void
test_global_palette_multi_frame (void)
{
    g_autoptr(GError)   error  = NULL;
    g_autofree gchar   *path   = tmp_path ("grl-test-mc-global2.gif");
    g_autoptr(GrlImage) frame1 = NULL;
    g_autoptr(GrlImage) frame2 = NULL;
    GrlGifWriter *writer;
    gchar *data = NULL;
    gsize  len  = 0;

    frame1 = make_solid (8, 8, 200, 50, 80, 255);
    frame2 = make_solid (8, 8,  50, 200, 100, 255);

    writer = grl_gif_writer_new (path, 8, 8, 0, &error);
    g_assert_nonnull (writer);
    g_assert_no_error (error);

    grl_gif_writer_set_quantizer (writer, GRL_GIF_QUANTIZER_MEDIAN_CUT);
    /* palette_scope defaults to GLOBAL */

    g_assert_true (grl_gif_writer_add_frame (writer, frame1, 10, &error));
    g_assert_no_error (error);
    g_assert_true (grl_gif_writer_add_frame (writer, frame2, 10, &error));
    g_assert_no_error (error);
    g_assert_true (grl_gif_writer_close (writer, &error));
    g_assert_no_error (error);
    g_object_unref (writer);

    g_assert_true (g_file_get_contents (path, &data, &len, &error));
    g_assert_no_error (error);
    g_assert_cmpint (memcmp (data, "GIF89a", 6), ==, 0);
    g_assert_cmpuint (count_gce_markers (data, len), ==, 2);

    g_free (data);
    g_unlink (path);
}

/* -------------------------------------------------------------------------
 * Test: enum GType registrations exist
 * ------------------------------------------------------------------------- */

static void
test_enum_types_registered (void)
{
    g_assert_cmpuint (GRL_TYPE_GIF_QUANTIZER,   !=, G_TYPE_INVALID);
    g_assert_cmpuint (GRL_TYPE_GIF_DITHER,      !=, G_TYPE_INVALID);
    g_assert_cmpuint (GRL_TYPE_GIF_PALETTE_SCOPE, !=, G_TYPE_INVALID);
}

/* -------------------------------------------------------------------------
 * Main
 * ------------------------------------------------------------------------- */

int
main (int   argc,
      char *argv[])
{
    g_test_init (&argc, &argv, NULL);

    g_test_add_func ("/gif-quantize/websafe-default-header",
                     test_websafe_default_header);
    g_test_add_func ("/gif-quantize/median-cut-single-frame",
                     test_median_cut_single_frame);
    g_test_add_func ("/gif-quantize/median-cut-many-colors",
                     test_median_cut_many_colors);
    g_test_add_func ("/gif-quantize/transparency-fully-transparent-frame",
                     test_transparency_fully_transparent_frame);
    g_test_add_func ("/gif-quantize/max-colors-clamped",
                     test_max_colors_clamped);
    g_test_add_func ("/gif-quantize/floyd-steinberg-deterministic",
                     test_floyd_steinberg_deterministic);
    g_test_add_func ("/gif-quantize/per-frame-palette-scope",
                     test_per_frame_palette_scope);
    g_test_add_func ("/gif-quantize/global-palette-multi-frame",
                     test_global_palette_multi_frame);
    g_test_add_func ("/gif-quantize/enum-types-registered",
                     test_enum_types_registered);

    return g_test_run ();
}
