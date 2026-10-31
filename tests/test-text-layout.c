/* test-text-layout.c
 *
 * Copyright 2026 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * graylib draws and measures text with its own O(1) glyph lookup
 * (src/text/grl-text-layout-private.h) instead of raylib's DrawTextEx()
 * and MeasureTextEx(). These tests hold it to raylib's results: the same
 * measurements bit for bit, and the same pixels when drawn.
 *
 * Needs a display and a GL context; skips cleanly without one.
 */

#include <glib.h>
#include <string.h>
#include "deps/raylib/src/raylib.h"
#include "deps/raylib/src/rlgl.h"
#include "src/core/grl-window.h"
#include "src/graphics/grl-font.h"
#include "src/drawing/grl-draw.h"
#include "src/math/grl-color.h"
#include "src/math/grl-vector2.h"

#define SKIP_IF_NO_DISPLAY() \
    do { \
        if (g_getenv ("DISPLAY") == NULL && g_getenv ("WAYLAND_DISPLAY") == NULL) \
        { \
            g_test_skip ("No display available (headless environment)"); \
            return; \
        } \
    } while (0)

/* Single-line strings: ASCII, spaces/tabs, codepoints the default font
 * lacks (fall back to '?'), and multi-byte UTF-8. Multi-line text is
 * delegated to raylib, so one is included to check that path too. */
static const gchar *samples[] = {
    "Hello, World!",
    "The quick brown fox jumps over the lazy dog 0123456789",
    "  leading and trailing  ",
    "tab\tseparated\tvalues",
    "~!@#$%^&*()_+{}|:\"<>?`-=[]\\;',./",
    "caf\xc3\xa9 na\xc3\xafve \xe2\x82\xac 5",
    "emoji \xf0\x9f\x98\x80 here",
    "two\nlines",
    "x",
};

static GrlWindow *
open_window (void)
{
    GrlWindow *window;

    window = grl_window_new (256, 128, "test-text-layout");
    if (window != NULL && !grl_window_is_ready (window))
        g_clear_object (&window);
    return window;
}

static void
test_text_layout_measure_matches_raylib (void)
{
    g_autoptr(GrlWindow) window = NULL;
    g_autoptr(GrlFont)   font = NULL;
    Font                 raw;
    gfloat               sizes[] = { 8.0f, 10.0f, 13.5f, 20.0f, 31.0f };
    gfloat               spacings[] = { 0.0f, 1.0f, 2.5f };
    guint                i;
    guint                s;
    guint                p;

    SKIP_IF_NO_DISPLAY ();
    window = open_window ();
    if (window == NULL)
    {
        g_test_skip ("Window/GL context not ready in this environment");
        return;
    }

    font = grl_font_new_default ();
    g_assert_nonnull (font);
    raw = GetFontDefault ();

    for (i = 0; i < G_N_ELEMENTS (samples); i++)
    {
        /* MeasureText() vs grl_measure_text() (default font, int sizes) */
        for (s = 6; s <= 40; s += 7)
            g_assert_cmpint (grl_measure_text (samples[i], (gint) s), ==,
                             MeasureText (samples[i], (gint) s));

        /* MeasureTextEx() vs grl_font_measure_text(_xy) */
        for (s = 0; s < G_N_ELEMENTS (sizes); s++)
        {
            for (p = 0; p < G_N_ELEMENTS (spacings); p++)
            {
                g_autoptr(GrlVector2) boxed = NULL;
                Vector2               expect;
                gfloat                w;
                gfloat                h;

                expect = MeasureTextEx (raw, samples[i], sizes[s], spacings[p]);
                grl_font_measure_text_xy (font, samples[i], sizes[s],
                                          spacings[p], &w, &h);
                boxed = grl_font_measure_text (font, samples[i], sizes[s],
                                               spacings[p]);

                g_assert_true (w == expect.x);
                g_assert_true (h == expect.y);
                g_assert_true (boxed->x == expect.x);
                g_assert_true (boxed->y == expect.y);
            }
        }
    }
}

/* Captures the back buffer after flushing raylib's batch. */
static Image
capture (void)
{
    rlDrawRenderBatchActive ();
    return LoadImageFromScreen ();
}

/* Renders @text twice, once through raylib and once through graylib, and
 * returns whether the pixels are identical. */
static gboolean
render_matches (GrlFont     *font,
                const gchar *text,
                gfloat       size,
                gfloat       spacing,
                gboolean     use_default)
{
    Font       raw;
    Image      ia;
    Image      ib;
    gboolean   same;
    GrlColor   tint = { 230, 200, 40, 255 };
    Color      rtint = { 230, 200, 40, 255 };
    GrlVector2 pos = { 3.25f, 5.5f };
    Vector2    rpos = { 3.25f, 5.5f };

    raw = GetFontDefault ();

    BeginDrawing ();
    ClearBackground (BLACK);
    if (use_default)
        DrawText (text, 3, 5, (gint) size, rtint);
    else
        DrawTextEx (raw, text, rpos, size, spacing, rtint);
    ia = capture ();

    ClearBackground (BLACK);
    if (use_default)
        grl_draw_text (text, 3, 5, (gint) size, &tint);
    else
        grl_draw_text_ex (font, text, &pos, size, spacing, &tint);
    ib = capture ();
    EndDrawing ();

    /* Every sample has visible glyphs: a blank capture would make the
     * comparison meaningless */
    {
        const guchar *px = ia.data;
        gsize         n = (gsize) GetPixelDataSize (ia.width, ia.height, ia.format);
        gsize         k;
        gboolean      ink = FALSE;

        for (k = 0; k < n && !ink; k++)
            ink = px[k] != 0 && (k % 4) != 3;
        g_assert_true (ink);
    }

    same = ia.width == ib.width && ia.height == ib.height &&
           ia.format == ib.format &&
           memcmp (ia.data, ib.data,
                   (gsize) GetPixelDataSize (ia.width, ia.height, ia.format)) == 0;
    UnloadImage (ia);
    UnloadImage (ib);

    return same;
}

static void
test_text_layout_pixels_match_raylib (void)
{
    g_autoptr(GrlWindow) window = NULL;
    g_autoptr(GrlFont)   font = NULL;
    guint                i;

    SKIP_IF_NO_DISPLAY ();
    window = open_window ();
    if (window == NULL)
    {
        g_test_skip ("Window/GL context not ready in this environment");
        return;
    }

    font = grl_font_new_default ();
    g_assert_nonnull (font);

    for (i = 0; i < G_N_ELEMENTS (samples); i++)
    {
        g_assert_true (render_matches (font, samples[i], 10.0f, 1.0f, TRUE));
        g_assert_true (render_matches (font, samples[i], 20.0f, 2.0f, TRUE));
        g_assert_true (render_matches (font, samples[i], 17.5f, 1.5f, FALSE));
        g_assert_true (render_matches (font, samples[i], 30.0f, 0.0f, FALSE));
    }
}

int
main (int   argc,
      char *argv[])
{
    g_test_init (&argc, &argv, NULL);

    g_test_add_func ("/text-layout/measure-matches-raylib",
                     test_text_layout_measure_matches_raylib);
    g_test_add_func ("/text-layout/pixels-match-raylib",
                     test_text_layout_pixels_match_raylib);

    return g_test_run ();
}
