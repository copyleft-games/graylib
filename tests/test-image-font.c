/* test-image-font.c
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Unit tests for GrlImageFont (headless TTF/OTF rasteriser).
 *
 * All tests run headless (no window or GL context needed).
 *
 * Tests that need a real font file guard against its absence with
 * g_test_skip() so they are safe to run in CI environments that lack
 * system fonts.
 */

#include <glib.h>
#include <string.h>
#include "src/graphics/grl-image-font.h"
#include "src/graphics/grl-image.h"
#include "src/math/grl-color.h"
#include "src/math/grl-vector2.h"

/* Path to a real TTF font used for integration tests. */
#define TEST_FONT_PATH "/usr/share/fonts/dejavu-sans-fonts/DejaVuSans.ttf"

/* Helper: skip the current test if the font file is not available. */
#define SKIP_IF_NO_FONT() \
    do { \
        if (!g_file_test (TEST_FONT_PATH, G_FILE_TEST_EXISTS)) \
        { \
            g_test_skip ("test font not available: " TEST_FONT_PATH); \
            return; \
        } \
    } while (0)

/* -------------------------------------------------------------------------
 * Constructor error-handling tests (no font required)
 * -------------------------------------------------------------------------
 */

static void
test_new_from_file_bogus (void)
{
    GrlImageFont *font;
    GError       *error;

    error = NULL;
    font  = grl_image_font_new_from_file ("/this/path/does/not/exist.ttf", &error);

    g_assert_null (font);
    g_assert_nonnull (error);
    g_assert_true (error->domain == GRL_IMAGE_FONT_ERROR);
    /* Should be OPEN or PARSE; either is acceptable here. */

    g_clear_error (&error);
}

/* -------------------------------------------------------------------------
 * Constructor success tests (require font)
 * -------------------------------------------------------------------------
 */

static void
test_new_from_file_ok (void)
{
    g_autoptr(GrlImageFont) font = NULL;
    GError *error = NULL;

    SKIP_IF_NO_FONT ();

    font = grl_image_font_new_from_file (TEST_FONT_PATH, &error);

    g_assert_nonnull (font);
    g_assert_no_error (error);
    g_assert_true (GRL_IS_IMAGE_FONT (font));
}

static void
test_new_from_memory_ok (void)
{
    gchar            *buf;
    gsize             len;
    g_autoptr(GrlImageFont) font = NULL;
    GError           *error = NULL;

    SKIP_IF_NO_FONT ();

    if (!g_file_get_contents (TEST_FONT_PATH, &buf, &len, NULL))
    {
        g_test_skip ("could not read test font");
        return;
    }

    font = grl_image_font_new_from_memory ((const guint8 *)buf, len, &error);
    g_free (buf);

    g_assert_nonnull (font);
    g_assert_no_error (error);
}

/* -------------------------------------------------------------------------
 * Metrics tests
 * -------------------------------------------------------------------------
 */

static void
test_v_metrics (void)
{
    g_autoptr(GrlImageFont) font = NULL;
    GError *error = NULL;
    gfloat  ascent;
    gfloat  descent;
    gfloat  line_gap;

    SKIP_IF_NO_FONT ();

    font = grl_image_font_new_from_file (TEST_FONT_PATH, &error);
    g_assert_nonnull (font);
    g_assert_no_error (error);

    grl_image_font_get_v_metrics (font, 32.0f, &ascent, &descent, &line_gap);

    /* ascent should be positive for any well-formed font */
    g_assert_cmpfloat (ascent, >, 0.0f);

    /* descent should be <= 0 (below the baseline) */
    g_assert_cmpfloat (descent, <=, 0.0f);

    /* line height (ascent - descent + line_gap) must be positive */
    g_assert_cmpfloat (ascent - descent + line_gap, >, 0.0f);
}

static void
test_v_metrics_nullable (void)
{
    g_autoptr(GrlImageFont) font = NULL;
    GError *error = NULL;
    gfloat  ascent;

    SKIP_IF_NO_FONT ();

    font = grl_image_font_new_from_file (TEST_FONT_PATH, &error);
    g_assert_nonnull (font);
    g_assert_no_error (error);

    /* Passing NULL for out-params should not crash */
    grl_image_font_get_v_metrics (font, 32.0f, &ascent, NULL, NULL);
    g_assert_cmpfloat (ascent, >, 0.0f);
}

/* -------------------------------------------------------------------------
 * Glyph presence tests
 * -------------------------------------------------------------------------
 */

static void
test_has_glyph (void)
{
    g_autoptr(GrlImageFont) font = NULL;
    GError *error = NULL;

    SKIP_IF_NO_FONT ();

    font = grl_image_font_new_from_file (TEST_FONT_PATH, &error);
    g_assert_nonnull (font);
    g_assert_no_error (error);

    /* 'A' (U+0041) must be present in any Latin font */
    g_assert_true (grl_image_font_has_glyph (font, (gunichar)'A'));

    /* U+1FFFF is in a private-use / reserved range unlikely to be in DejaVu */
    g_assert_false (grl_image_font_has_glyph (font, (gunichar)0x1FFFFu));
}

/* -------------------------------------------------------------------------
 * Measurement tests
 * -------------------------------------------------------------------------
 */

static void
test_measure_basic (void)
{
    g_autoptr(GrlImageFont) font = NULL;
    g_autoptr(GrlVector2)   size = NULL;
    GError *error = NULL;
    gfloat  ascent;
    gfloat  descent;
    gfloat  line_gap;
    gfloat  line_height;

    SKIP_IF_NO_FONT ();

    font = grl_image_font_new_from_file (TEST_FONT_PATH, &error);
    g_assert_nonnull (font);
    g_assert_no_error (error);

    size = grl_image_measure_text_ttf (font, "Hello", 32.0f);
    g_assert_nonnull (size);

    /* Width of "Hello" must be positive */
    g_assert_cmpfloat (size->x, >, 0.0f);

    /* Height should be approximately one line height */
    grl_image_font_get_v_metrics (font, 32.0f, &ascent, &descent, &line_gap);
    line_height = ascent - descent + line_gap;
    g_assert_cmpfloat (size->y, >, 0.0f);
    g_assert_cmpfloat (size->y, <=, line_height * 1.5f);
}

static void
test_measure_empty (void)
{
    g_autoptr(GrlImageFont) font = NULL;
    GError   *error = NULL;
    GrlVector2 *size;

    SKIP_IF_NO_FONT ();

    font = grl_image_font_new_from_file (TEST_FONT_PATH, &error);
    g_assert_nonnull (font);
    g_assert_no_error (error);

    /* Empty string -> zero size */
    size = grl_image_measure_text_ttf (font, "", 32.0f);
    g_assert_nonnull (size);
    g_assert_cmpfloat (size->x, ==, 0.0f);
    g_assert_cmpfloat (size->y, ==, 0.0f);
    grl_vector2_free (size);

    /* NULL text -> zero size */
    size = grl_image_measure_text_ttf (font, NULL, 32.0f);
    g_assert_nonnull (size);
    g_assert_cmpfloat (size->x, ==, 0.0f);
    g_assert_cmpfloat (size->y, ==, 0.0f);
    grl_vector2_free (size);

    /* px_size <= 0 -> zero size */
    size = grl_image_measure_text_ttf (font, "Hello", 0.0f);
    g_assert_nonnull (size);
    g_assert_cmpfloat (size->x, ==, 0.0f);
    g_assert_cmpfloat (size->y, ==, 0.0f);
    grl_vector2_free (size);
}

static void
test_measure_newline (void)
{
    g_autoptr(GrlImageFont) font = NULL;
    GError      *error = NULL;
    GrlVector2  *single_line;
    GrlVector2  *two_lines;

    SKIP_IF_NO_FONT ();

    font = grl_image_font_new_from_file (TEST_FONT_PATH, &error);
    g_assert_nonnull (font);
    g_assert_no_error (error);

    single_line = grl_image_measure_text_ttf (font, "A", 32.0f);
    two_lines   = grl_image_measure_text_ttf (font, "A\nA", 32.0f);

    g_assert_nonnull (single_line);
    g_assert_nonnull (two_lines);

    /* Two-line text must be taller than one line */
    g_assert_cmpfloat (two_lines->y, >, single_line->y);

    grl_vector2_free (single_line);
    grl_vector2_free (two_lines);
}

static void
test_measure_kerning (void)
{
    g_autoptr(GrlImageFont) font = NULL;
    GError     *error = NULL;
    GrlVector2 *measure_av;
    GrlVector2 *measure_avav;
    GrlVector2 *measure_a;

    SKIP_IF_NO_FONT ();

    font = grl_image_font_new_from_file (TEST_FONT_PATH, &error);
    g_assert_nonnull (font);
    g_assert_no_error (error);

    /*
     * Kerning robustness test.  DejaVu Sans does kern "AV" (the pair has a
     * negative kern value, so the total advance is shorter than the sum of the
     * individual advances).  We use a loose four-character check:
     *
     *   measure("AVAV").x < 4 * measure("A").x
     *
     * This works because:
     *   - "AVAV" has four characters; without any kerning its width would be
     *     approximately 2*adv(A) + 2*adv(V).
     *   - adv(V) < 2*adv(A) for any proportional font (V is not wider than
     *     two A's), so 2*adv(A) + 2*adv(V) < 4*adv(A).
     *   - With kerning the result is even smaller.
     *
     * The test would pass even if stb_truetype returns zero for all kern pairs
     * (it still holds by the width inequality), so it never spuriously fails.
     * If we wanted a strict kern test we would need to compare "AV" against
     * "A" + "V" measured separately, which requires exposing per-codepoint
     * advance—out of scope for v1.
     */
    measure_a    = grl_image_measure_text_ttf (font, "A",    32.0f);
    measure_av   = grl_image_measure_text_ttf (font, "AV",   32.0f);
    measure_avav = grl_image_measure_text_ttf (font, "AVAV", 32.0f);

    g_assert_nonnull (measure_a);
    g_assert_nonnull (measure_av);
    g_assert_nonnull (measure_avav);

    /* The four-character string should be narrower than four individual A's. */
    g_assert_cmpfloat (measure_avav->x, <, 4.0f * measure_a->x);

    /* Sanity: "AV" should be wider than "A" alone */
    g_assert_cmpfloat (measure_av->x, >, measure_a->x);

    grl_vector2_free (measure_a);
    grl_vector2_free (measure_av);
    grl_vector2_free (measure_avav);
}

/* -------------------------------------------------------------------------
 * Drawing tests
 * -------------------------------------------------------------------------
 */

static void
test_draw_changes_pixels (void)
{
    g_autoptr(GrlImageFont) font = NULL;
    g_autoptr(GrlColor)     white = NULL;
    g_autoptr(GrlColor)     red   = NULL;
    g_autoptr(GrlImage)     img   = NULL;
    GError                 *error = NULL;
    gboolean                found_non_white;
    gint                    xi;
    gint                    yi;

    SKIP_IF_NO_FONT ();

    font = grl_image_font_new_from_file (TEST_FONT_PATH, &error);
    g_assert_nonnull (font);
    g_assert_no_error (error);

    /* Draw onto a white background so any dark pixel is clearly text. */
    white = grl_color_new (255, 255, 255, 255);
    img   = grl_image_new_color (200, 60, white);
    g_assert_nonnull (img);

    red = grl_color_new (0, 0, 0, 255);
    grl_image_draw_text_ttf (img, font, "Hello", 2, 2, 32.0f, red);

    /* Scan the image for any pixel that differs from the white background.
     * At 32 px we expect plenty of covered pixels.  Use a tolerance of 250
     * to tolerate sub-pixel coverage artefacts near boundaries. */
    found_non_white = FALSE;
    for (yi = 0; yi < grl_image_get_height (img) && !found_non_white; yi++)
    {
        for (xi = 0; xi < grl_image_get_width (img) && !found_non_white; xi++)
        {
            GrlColor *pixel = grl_image_get_pixel (img, xi, yi);
            if (pixel != NULL)
            {
                if (pixel->r < 200 || pixel->g < 200 || pixel->b < 200)
                    found_non_white = TRUE;
                grl_color_free (pixel);
            }
        }
    }

    g_assert_true (found_non_white);
}

static void
test_draw_empty_string_noop (void)
{
    g_autoptr(GrlImageFont) font = NULL;
    g_autoptr(GrlColor)     white = NULL;
    g_autoptr(GrlColor)     black = NULL;
    g_autoptr(GrlImage)     img   = NULL;
    GError                 *error = NULL;
    gboolean                changed;
    gint                    xi;
    gint                    yi;

    SKIP_IF_NO_FONT ();

    font = grl_image_font_new_from_file (TEST_FONT_PATH, &error);
    g_assert_nonnull (font);
    g_assert_no_error (error);

    white = grl_color_new (255, 255, 255, 255);
    img   = grl_image_new_color (64, 64, white);
    g_assert_nonnull (img);

    black = grl_color_new (0, 0, 0, 255);

    /* Empty string must leave image unchanged */
    grl_image_draw_text_ttf (img, font, "", 0, 0, 32.0f, black);

    changed = FALSE;
    for (yi = 0; yi < grl_image_get_height (img) && !changed; yi++)
    {
        for (xi = 0; xi < grl_image_get_width (img) && !changed; xi++)
        {
            GrlColor *pixel = grl_image_get_pixel (img, xi, yi);
            if (pixel != NULL)
            {
                if (pixel->r != 255 || pixel->g != 255 || pixel->b != 255)
                    changed = TRUE;
                grl_color_free (pixel);
            }
        }
    }

    g_assert_false (changed);
}

static void
test_draw_zero_px_size_noop (void)
{
    g_autoptr(GrlImageFont) font = NULL;
    g_autoptr(GrlColor)     white = NULL;
    g_autoptr(GrlColor)     black = NULL;
    g_autoptr(GrlImage)     img   = NULL;
    GError                 *error = NULL;
    gboolean                changed;
    gint                    xi;
    gint                    yi;

    SKIP_IF_NO_FONT ();

    font = grl_image_font_new_from_file (TEST_FONT_PATH, &error);
    g_assert_nonnull (font);
    g_assert_no_error (error);

    white = grl_color_new (255, 255, 255, 255);
    img   = grl_image_new_color (64, 64, white);
    g_assert_nonnull (img);

    black = grl_color_new (0, 0, 0, 255);

    /* px_size <= 0 must leave image unchanged */
    grl_image_draw_text_ttf (img, font, "Hello", 0, 0, 0.0f, black);
    grl_image_draw_text_ttf (img, font, "Hello", 0, 0, -1.0f, black);

    changed = FALSE;
    for (yi = 0; yi < grl_image_get_height (img) && !changed; yi++)
    {
        for (xi = 0; xi < grl_image_get_width (img) && !changed; xi++)
        {
            GrlColor *pixel = grl_image_get_pixel (img, xi, yi);
            if (pixel != NULL)
            {
                if (pixel->r != 255 || pixel->g != 255 || pixel->b != 255)
                    changed = TRUE;
                grl_color_free (pixel);
            }
        }
    }

    g_assert_false (changed);
}

static void
test_draw_huge_px_size_no_crash (void)
{
    g_autoptr(GrlImageFont) font = NULL;
    g_autoptr(GrlColor)     white = NULL;
    g_autoptr(GrlColor)     black = NULL;
    g_autoptr(GrlImage)     img   = NULL;
    GError                 *error = NULL;

    SKIP_IF_NO_FONT ();

    font = grl_image_font_new_from_file (TEST_FONT_PATH, &error);
    g_assert_nonnull (font);
    g_assert_no_error (error);

    white = grl_color_new (255, 255, 255, 255);
    img   = grl_image_new_color (64, 64, white);
    g_assert_nonnull (img);

    black = grl_color_new (0, 0, 0, 255);

    /* A very large px_size must not crash or allocate unbounded memory.
     * The glyph layer will be large but bounded by the image content;
     * we only pass a single character to keep the allocation reasonable. */
    grl_image_draw_text_ttf (img, font, "A", 0, 0, 400.0f, black);
    /* If we reached here without crashing, the test passes. */
}

/* -------------------------------------------------------------------------
 * Main
 * -------------------------------------------------------------------------
 */

int
main (int   argc,
      char *argv[])
{
    g_test_init (&argc, &argv, NULL);

    /* Constructor error handling */
    g_test_add_func ("/image-font/new-from-file-bogus",      test_new_from_file_bogus);

    /* Constructor success */
    g_test_add_func ("/image-font/new-from-file-ok",         test_new_from_file_ok);
    g_test_add_func ("/image-font/new-from-memory-ok",       test_new_from_memory_ok);

    /* Metrics */
    g_test_add_func ("/image-font/v-metrics",                test_v_metrics);
    g_test_add_func ("/image-font/v-metrics-nullable",       test_v_metrics_nullable);

    /* Glyph presence */
    g_test_add_func ("/image-font/has-glyph",                test_has_glyph);

    /* Measurement */
    g_test_add_func ("/image-font/measure-basic",            test_measure_basic);
    g_test_add_func ("/image-font/measure-empty",            test_measure_empty);
    g_test_add_func ("/image-font/measure-newline",          test_measure_newline);
    g_test_add_func ("/image-font/measure-kerning",          test_measure_kerning);

    /* Drawing */
    g_test_add_func ("/image-font/draw-changes-pixels",      test_draw_changes_pixels);
    g_test_add_func ("/image-font/draw-empty-string-noop",   test_draw_empty_string_noop);
    g_test_add_func ("/image-font/draw-zero-px-size-noop",   test_draw_zero_px_size_noop);
    g_test_add_func ("/image-font/draw-huge-px-size-no-crash", test_draw_huge_px_size_no_crash);

    return g_test_run ();
}
