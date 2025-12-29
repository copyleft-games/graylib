/* test-png.c
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Unit tests for GrlPngChunk and GrlPngPalette.
 *
 * Note: Tests that require file I/O are skipped (no test assets).
 * These tests focus on the GBoxed type mechanics and property accessors.
 */

#include <glib.h>
#include <string.h>
#include "src/graphics/grl-png.h"
#include "src/math/grl-color.h"

/*
 * =============================================================================
 * GrlPngChunk Tests
 * =============================================================================
 */

static void
test_png_chunk_type (void)
{
    GType type = grl_png_chunk_get_type ();

    g_assert_true (type != G_TYPE_INVALID);
    g_assert_cmpstr (g_type_name (type), ==, "GrlPngChunk");
}

static void
test_png_chunk_new (void)
{
    const guint8 data[] = { 0x48, 0x65, 0x6C, 0x6C, 0x6F }; /* "Hello" */
    g_autoptr(GrlPngChunk) chunk = NULL;

    chunk = grl_png_chunk_new ("tEXt", data, 5);

    g_assert_nonnull (chunk);
    g_assert_cmpint (chunk->length, ==, 5);
}

static void
test_png_chunk_new_empty (void)
{
    g_autoptr(GrlPngChunk) chunk = NULL;

    chunk = grl_png_chunk_new ("tIME", NULL, 0);

    g_assert_nonnull (chunk);
    g_assert_cmpint (chunk->length, ==, 0);
}

static void
test_png_chunk_copy (void)
{
    const guint8 data[] = { 0x01, 0x02, 0x03, 0x04 };
    g_autoptr(GrlPngChunk) chunk1 = NULL;
    g_autoptr(GrlPngChunk) chunk2 = NULL;

    chunk1 = grl_png_chunk_new ("gAMA", data, 4);
    g_assert_nonnull (chunk1);

    chunk2 = grl_png_chunk_copy (chunk1);
    g_assert_nonnull (chunk2);
    g_assert_true (chunk1 != chunk2);
    g_assert_cmpint (chunk1->length, ==, chunk2->length);
    g_assert_cmpuint (chunk1->crc, ==, chunk2->crc);
}

static void
test_png_chunk_free_null (void)
{
    /* Should not crash when passed NULL */
    grl_png_chunk_free (NULL);
}

static void
test_png_chunk_get_type_string (void)
{
    const guint8 data[] = { 0x00 };
    g_autoptr(GrlPngChunk) chunk = NULL;
    g_autofree gchar *type_string = NULL;

    chunk = grl_png_chunk_new ("tEXt", data, 1);
    type_string = grl_png_chunk_get_type_string (chunk);

    g_assert_nonnull (type_string);
    g_assert_cmpstr (type_string, ==, "tEXt");
}

static void
test_png_chunk_type_ihdr (void)
{
    const guint8 data[] = { 0x00 };
    g_autoptr(GrlPngChunk) chunk = NULL;
    g_autofree gchar *type_string = NULL;

    chunk = grl_png_chunk_new ("IHDR", data, 1);
    type_string = grl_png_chunk_get_type_string (chunk);

    g_assert_cmpstr (type_string, ==, "IHDR");
}

static void
test_png_chunk_type_iend (void)
{
    g_autoptr(GrlPngChunk) chunk = NULL;
    g_autofree gchar *type_string = NULL;

    chunk = grl_png_chunk_new ("IEND", NULL, 0);
    type_string = grl_png_chunk_get_type_string (chunk);

    g_assert_cmpstr (type_string, ==, "IEND");
}

/*
 * =============================================================================
 * GrlPngPalette Tests
 * =============================================================================
 */

static void
test_png_palette_type (void)
{
    GType type = grl_png_palette_get_type ();

    g_assert_true (type != G_TYPE_INVALID);
    g_assert_cmpstr (g_type_name (type), ==, "GrlPngPalette");
}

static void
test_png_palette_new (void)
{
    GrlColor colors[3];
    g_autoptr(GrlPngPalette) palette = NULL;

    colors[0].r = 255; colors[0].g = 0;   colors[0].b = 0;   colors[0].a = 255;
    colors[1].r = 0;   colors[1].g = 255; colors[1].b = 0;   colors[1].a = 255;
    colors[2].r = 0;   colors[2].g = 0;   colors[2].b = 255; colors[2].a = 255;

    palette = grl_png_palette_new (colors, 3);

    g_assert_nonnull (palette);
    g_assert_cmpint (palette->color_count, ==, 3);
}

static void
test_png_palette_new_empty (void)
{
    g_autoptr(GrlPngPalette) palette = NULL;

    palette = grl_png_palette_new_empty (256);

    g_assert_nonnull (palette);
    g_assert_cmpint (palette->color_count, ==, 256);
}

static void
test_png_palette_copy (void)
{
    GrlColor colors[2];
    g_autoptr(GrlPngPalette) palette1 = NULL;
    g_autoptr(GrlPngPalette) palette2 = NULL;

    colors[0].r = 128; colors[0].g = 128; colors[0].b = 128; colors[0].a = 255;
    colors[1].r = 64;  colors[1].g = 64;  colors[1].b = 64;  colors[1].a = 255;

    palette1 = grl_png_palette_new (colors, 2);
    g_assert_nonnull (palette1);

    palette2 = grl_png_palette_copy (palette1);
    g_assert_nonnull (palette2);
    g_assert_true (palette1 != palette2);
    g_assert_cmpint (palette1->color_count, ==, palette2->color_count);
}

static void
test_png_palette_free_null (void)
{
    /* Should not crash when passed NULL */
    grl_png_palette_free (NULL);
}

static void
test_png_palette_get_color (void)
{
    GrlColor colors[3];
    g_autoptr(GrlPngPalette) palette = NULL;
    g_autoptr(GrlColor) result = NULL;

    colors[0].r = 255; colors[0].g = 0;   colors[0].b = 0;   colors[0].a = 255;
    colors[1].r = 0;   colors[1].g = 255; colors[1].b = 0;   colors[1].a = 255;
    colors[2].r = 0;   colors[2].g = 0;   colors[2].b = 255; colors[2].a = 255;

    palette = grl_png_palette_new (colors, 3);
    result = grl_png_palette_get_color (palette, 1);

    g_assert_nonnull (result);
    g_assert_cmpuint (result->r, ==, 0);
    g_assert_cmpuint (result->g, ==, 255);
    g_assert_cmpuint (result->b, ==, 0);
    g_assert_cmpuint (result->a, ==, 255);
}

/* Note: grl_png_palette_get_color with out-of-range index aborts via
 * g_return_if_fail, so we cannot test that case without crashing.
 * The function is designed to fail-fast on invalid input.
 */

static void
test_png_palette_set_color (void)
{
    g_autoptr(GrlPngPalette) palette = NULL;
    g_autoptr(GrlColor) color = NULL;
    g_autoptr(GrlColor) result = NULL;

    palette = grl_png_palette_new_empty (16);
    color = grl_color_new (255, 128, 64, 255);

    grl_png_palette_set_color (palette, 5, color);
    result = grl_png_palette_get_color (palette, 5);

    g_assert_nonnull (result);
    g_assert_cmpuint (result->r, ==, 255);
    g_assert_cmpuint (result->g, ==, 128);
    g_assert_cmpuint (result->b, ==, 64);
    g_assert_cmpuint (result->a, ==, 255);
}

static void
test_png_palette_set_color_first (void)
{
    g_autoptr(GrlPngPalette) palette = NULL;
    g_autoptr(GrlColor) color = NULL;
    g_autoptr(GrlColor) result = NULL;

    palette = grl_png_palette_new_empty (8);
    color = grl_color_new (100, 100, 100, 255);

    grl_png_palette_set_color (palette, 0, color);
    result = grl_png_palette_get_color (palette, 0);

    g_assert_nonnull (result);
    g_assert_cmpuint (result->r, ==, 100);
    g_assert_cmpuint (result->g, ==, 100);
    g_assert_cmpuint (result->b, ==, 100);
}

static void
test_png_palette_set_color_last (void)
{
    g_autoptr(GrlPngPalette) palette = NULL;
    g_autoptr(GrlColor) color = NULL;
    g_autoptr(GrlColor) result = NULL;

    palette = grl_png_palette_new_empty (8);
    color = grl_color_new (200, 200, 200, 255);

    grl_png_palette_set_color (palette, 7, color);
    result = grl_png_palette_get_color (palette, 7);

    g_assert_nonnull (result);
    g_assert_cmpuint (result->r, ==, 200);
    g_assert_cmpuint (result->g, ==, 200);
    g_assert_cmpuint (result->b, ==, 200);
}

/*
 * Test registration
 */

int
main (int   argc,
      char *argv[])
{
    g_test_init (&argc, &argv, NULL);

    /* GrlPngChunk */
    g_test_add_func ("/png-chunk/type", test_png_chunk_type);
    g_test_add_func ("/png-chunk/new", test_png_chunk_new);
    g_test_add_func ("/png-chunk/new-empty", test_png_chunk_new_empty);
    g_test_add_func ("/png-chunk/copy", test_png_chunk_copy);
    g_test_add_func ("/png-chunk/free-null", test_png_chunk_free_null);
    g_test_add_func ("/png-chunk/get-type-string", test_png_chunk_get_type_string);
    g_test_add_func ("/png-chunk/type-ihdr", test_png_chunk_type_ihdr);
    g_test_add_func ("/png-chunk/type-iend", test_png_chunk_type_iend);

    /* GrlPngPalette */
    g_test_add_func ("/png-palette/type", test_png_palette_type);
    g_test_add_func ("/png-palette/new", test_png_palette_new);
    g_test_add_func ("/png-palette/new-empty", test_png_palette_new_empty);
    g_test_add_func ("/png-palette/copy", test_png_palette_copy);
    g_test_add_func ("/png-palette/free-null", test_png_palette_free_null);
    g_test_add_func ("/png-palette/get-color", test_png_palette_get_color);
    /* Out-of-range test skipped: function aborts on invalid input */
    g_test_add_func ("/png-palette/set-color", test_png_palette_set_color);
    g_test_add_func ("/png-palette/set-color-first", test_png_palette_set_color_first);
    g_test_add_func ("/png-palette/set-color-last", test_png_palette_set_color_last);

    return g_test_run ();
}
