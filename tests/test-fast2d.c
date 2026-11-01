/* test-fast2d.c
 *
 * Copyright 2026 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * The fast 2D vertex path (src/drawing/grl-fast2d.c) renders what
 * raylib's immediate-mode functions render: every scene is drawn into a
 * render texture once with the fast path off (graylib calls raylib's
 * DrawTexturePro(), DrawRectanglePro(), DrawRectangleRounded(), ...)
 * and once with it on, and the pixels must be identical. The scenes mix
 * fast draws with raylib draws, cross the batch's vertex limit and its
 * draw-call limit, use a 2D camera, a pushed rlgl transform and the
 * depth test, and check that the path survives a window being reopened
 * and another batch being activated behind its back. Each scene also
 * checks that the fast path was actually taken.
 *
 * Linked against the static library: grl_fast2d_get_quads_written() is
 * internal. Needs a display and a GL context; skips cleanly without one.
 */

#include <glib.h>
#include <string.h>
#include "deps/raylib/src/raylib.h"
#include "deps/raylib/src/rlgl.h"
#include "src/core/grl-window.h"
#include "src/graphics/grl-font.h"
#include "src/graphics/grl-texture.h"
#include "src/drawing/grl-draw.h"
#include "src/drawing/grl-fast2d-private.h"
#include "src/math/grl-color.h"
#include "src/math/grl-vector2.h"
#include "src/math/grl-rectangle.h"

#define TARGET_W (320)
#define TARGET_H (240)

typedef void (*SceneFunc) (gpointer data);

typedef struct
{
    GrlTexture *checker;    /* 8x8 two-colour texture */
    GrlTexture *stripes;    /* 4x2 texture */
    GrlFont    *font;
} Assets;

static GrlWindow *
open_window (void)
{
    GrlWindow   *window;
    const gchar *display = g_getenv ("DISPLAY");
    const gchar *wayland = g_getenv ("WAYLAND_DISPLAY");

    if ((display == NULL || display[0] == '\0') &&
        (wayland == NULL || wayland[0] == '\0'))
        return NULL;

    SetTraceLogLevel (LOG_WARNING);
    SetConfigFlags (FLAG_WINDOW_HIDDEN);
    window = grl_window_new (64, 64, "test-fast2d");
    if (window != NULL && !grl_window_is_ready (window))
        g_clear_object (&window);
    return window;
}

static GrlTexture *
make_texture (gint w,
              gint h,
              gint seed)
{
    Image      image;
    Texture2D  tex;
    gint       x;
    gint       y;
    GrlTexture *texture;

    image = GenImageColor (w, h, BLACK);
    for (y = 0; y < h; y++)
    {
        for (x = 0; x < w; x++)
        {
            Color c;

            c.r = (guint8) (40 + 23 * x + seed * 50);
            c.g = (guint8) (200 - 31 * y + seed * 20);
            c.b = (guint8) (((x + y + seed) & 1) ? 230 : 60);
            c.a = (guint8) (((x * 7 + y) % 5 == 0) ? 128 : 255);
            ImageDrawPixel (&image, x, y, c);
        }
    }
    tex = LoadTextureFromImage (image);
    UnloadImage (image);
    texture = grl_texture_new_from_handle (&tex);
    return texture;
}

/*
 * render:
 *
 * Draws @scene into a fresh render texture with the fast path @fast and
 * returns the pixels. The fast path's batch is installed before the
 * comparison renders (one flush) so both renders start from an empty,
 * identical batch.
 */
static Image
render (SceneFunc scene,
        gpointer  data,
        gboolean  fast)
{
    RenderTexture2D target = LoadRenderTexture (TARGET_W, TARGET_H);
    Image           image;

    grl_draw_set_fast_path_enabled (fast);
    BeginTextureMode (target);
    ClearBackground ((Color){ 12, 20, 34, 255 });
    scene (data);
    EndTextureMode ();
    grl_draw_set_fast_path_enabled (TRUE);

    image = LoadImageFromTexture (target.texture);
    UnloadRenderTexture (target);
    return image;
}

/* Renders @scene both ways and asserts equal, non-blank pixels and fast
 * quads written (at least @min_fast) in the fast render. */
static void
assert_same (SceneFunc scene,
             gpointer  data,
             guint64   min_fast)
{
    Image         slow;
    Image         quick;
    guint64       before;
    guint64       written;
    gsize         n;
    gsize         k;
    gsize         differ;
    const guchar *a;
    const guchar *b;

    /* make sure the batch is installed: the first fast draw flushes once */
    grl_draw_set_fast_path_enabled (TRUE);
    grl_fast2d_begin ();
    rlDrawRenderBatchActive ();

    slow = render (scene, data, FALSE);
    before = grl_fast2d_get_quads_written ();
    quick = render (scene, data, TRUE);
    written = grl_fast2d_get_quads_written () - before;

    g_assert_cmpint (slow.width, ==, quick.width);
    g_assert_cmpint (slow.height, ==, quick.height);
    g_assert_cmpint (slow.format, ==, quick.format);
    n = (gsize) GetPixelDataSize (slow.width, slow.height, slow.format);
    a = slow.data;
    b = quick.data;
    differ = 0;
    for (k = 0; k < n; k++)
        if (a[k] != b[k])
            differ++;
    if (differ != 0)
        g_test_message ("%" G_GSIZE_FORMAT " of %" G_GSIZE_FORMAT " bytes differ",
                        differ, n);
    g_assert_cmpuint (differ, ==, 0);

    /* something was drawn */
    {
        gboolean ink = FALSE;

        for (k = 0; k < n && !ink; k += 4)
            ink = a[k] != 12 || a[k + 1] != 20 || a[k + 2] != 34;
        g_assert_true (ink);
    }

    g_assert_cmpuint (written, >=, min_fast);
    UnloadImage (slow);
    UnloadImage (quick);
}

#define OPEN_OR_SKIP(window) \
    do { \
        window = open_window (); \
        if (window == NULL) \
        { \
            g_test_skip ("No display / GL context available"); \
            return; \
        } \
    } while (0)

/* ==========================================================================
 * Scenes
 * ========================================================================== */

/* scene_mixed:
 * Every fast-path draw (texture forms, rectangles, rounded rectangles with
 * even, odd and automatic segment counts, circles, outlines, text) with
 * raylib's own lines, triangles, circle outlines and gradients in between,
 * and shapes interleaved with two textures (draw-call switches). */
static void
scene_mixed (gpointer data)
{
    Assets *assets = data;
    guint   i;

    for (i = 0; i < 60; i++)
    {
        GrlColor     color = { (guint8) (i * 37), (guint8) (255 - i * 3), (guint8) (i * 11), (guint8) (128 + i) };
        GrlRectangle rect = { (gfloat) (i % 10) * 31.0f + 0.25f, (gfloat) (i / 10) * 37.0f + 0.5f, 27.0f, 21.0f + (gfloat) (i % 4) };
        GrlRectangle src = { 1.0f, 2.0f, 6.0f, 5.0f };
        GrlRectangle dst = { rect.x + 3.0f, rect.y + 2.0f, 12.0f, 9.0f };
        GrlVector2   origin = { 0.0f, 0.0f };
        GrlVector2   pos = { rect.x, rect.y };
        GrlVector2   center = { rect.x + 14.0f, rect.y + 10.0f };

        switch (i % 12)
        {
        case 0:
            grl_draw_rectangle ((gint) rect.x, (gint) rect.y, 20, 13, &color);
            break;
        case 1:
            grl_draw_rectangle_rounded (&rect, 0.4f, 6, &color);
            break;
        case 2:
            grl_draw_rectangle_rounded (&rect, 0.9f, (gint) (i % 5), &color);   /* auto segments */
            break;
        case 3:
            grl_draw_rectangle_rounded (&rect, 0.5f, 7, &color);                /* odd */
            break;
        case 4:
            grl_draw_texture_pro (assets->checker, &src, &dst, &origin, 0.0f, &color);
            break;
        case 5:
            {
                GrlRectangle flip = { 7.0f, 6.0f, -6.0f, -5.0f };               /* flipped source */
                GrlRectangle neg = { dst.x + 12.0f, dst.y + 9.0f, -12.0f, -9.0f };
                GrlVector2   o = { 2.0f, 3.0f };

                grl_draw_texture_pro (assets->checker, &flip, &neg, &o, 0.0f, &color);
            }
            break;
        case 6:
            {
                GrlVector2 o = { 6.0f, 4.5f };

                grl_draw_texture_pro (assets->stripes, &src, &dst, &o, 33.0f + (gfloat) i, &color);
                grl_draw_rectangle_pro (&rect, &o, -21.5f, &color);
            }
            break;
        case 7:
            grl_draw_circle_v (&center, 8.5f, &color);
            DrawCircleLines ((gint) center.x, (gint) center.y, 10.0f, RED);
            break;
        case 8:
            grl_draw_rectangle_lines_ex (&rect, 3.0f, &color);
            DrawLine ((gint) rect.x, (gint) rect.y, (gint) (rect.x + 20), (gint) (rect.y + 15), GREEN);
            break;
        case 9:
            grl_draw_texture (assets->stripes, (gint) rect.x, (gint) rect.y, &color);
            grl_draw_texture_v (assets->checker, &pos, &color);
            grl_draw_texture_ex (assets->checker, &center, 15.0f, 1.5f, &color);
            break;
        case 10:
            grl_draw_texture_rec (assets->checker, &src, &pos, &color);
            DrawTriangle ((Vector2){ rect.x, rect.y + 20 }, (Vector2){ rect.x + 20, rect.y + 20 },
                          (Vector2){ rect.x + 10, rect.y }, YELLOW);
            break;
        default:
            {
                GrlVector2 tpos = { rect.x + 0.5f, rect.y + 3.25f };

                grl_draw_text ("Hi there", (gint) rect.x, (gint) rect.y, 10, &color);
                grl_draw_text_ex (assets->font, "q~?\xc3\xa9", &tpos, 13.0f, 1.5f, &color);
                DrawRectangleGradientH ((gint) rect.x, (gint) rect.y + 15, 20, 5, RED, BLUE);
            }
            break;
        }
    }

    /* immediate-mode vertices that inherit rlgl's current colour and
     * texture coordinate from the draw before them */
    {
        GrlColor     color = { 90, 200, 120, 255 };
        GrlRectangle src = { 1.0f, 1.0f, 3.0f, 3.0f };
        GrlRectangle dst = { 250.0f, 200.0f, 10.0f, 10.0f };
        GrlVector2   origin = { 0.0f, 0.0f };

        grl_draw_texture_pro (assets->checker, &src, &dst, &origin, 0.0f, &color);
        rlBegin (RL_QUADS);
        rlVertex2f (270.0f, 200.0f);
        rlVertex2f (270.0f, 220.0f);
        rlVertex2f (290.0f, 220.0f);
        rlVertex2f (290.0f, 200.0f);
        rlEnd ();
    }
}

/* scene_overflow:
 * More quads than the batch holds (8192), so it flushes mid-scene, and
 * more draw-call switches than its 256 draw slots. */
static void
scene_overflow (gpointer data)
{
    Assets      *assets = data;
    GrlRectangle src = { 0.0f, 0.0f, 8.0f, 8.0f };
    GrlVector2   origin = { 0.0f, 0.0f };
    guint        i;

    for (i = 0; i < 9000; i++)
    {
        GrlColor color = { (guint8) i, (guint8) (i >> 3), 200, 255 };

        grl_draw_rectangle ((gint) (i % 317), (gint) ((i / 317) % 239), 3, 3, &color);
    }
    for (i = 0; i < 700; i++)
    {
        GrlColor     color = { 255, (guint8) i, (guint8) (i * 3), 255 };
        GrlRectangle dst = { (gfloat) (i % 300), (gfloat) ((i / 300) * 7 % 230), 5.0f, 5.0f };

        /* every 4th draw switches texture: hundreds of draw calls */
        grl_draw_texture_pro ((i / 4) % 2 ? assets->checker : assets->stripes,
                              &src, &dst, &origin, 0.0f, &color);
        grl_draw_texture_pro ((i / 4) % 2 ? assets->checker : assets->stripes,
                              &src, &dst, &origin, 0.0f, &color);
    }
}

/* scene_camera:
 * A 2D camera (fast), a pushed rlgl translation and scale (raylib's path)
 * and a push with no transform (fast). */
static void
scene_camera (gpointer data)
{
    Assets      *assets = data;
    Camera2D     camera;
    GrlColor     color = { 230, 120, 40, 255 };
    GrlRectangle rect = { 10.0f, 10.0f, 40.0f, 25.0f };
    GrlRectangle src = { 0.0f, 0.0f, 8.0f, 8.0f };
    GrlRectangle dst = { 60.0f, 15.0f, 16.0f, 16.0f };
    GrlVector2   origin = { 0.0f, 0.0f };
    guint        i;

    memset (&camera, 0, sizeof camera);
    camera.offset = (Vector2){ 160.0f, 120.0f };
    camera.target = (Vector2){ 80.0f, 40.0f };
    camera.rotation = 12.0f;
    camera.zoom = 1.7f;

    BeginMode2D (camera);
    for (i = 0; i < 10; i++)
    {
        rect.x += 3.0f;
        dst.y += 5.0f;
        grl_draw_rectangle_rounded (&rect, 0.3f, 6, &color);
        grl_draw_texture_pro (assets->checker, &src, &dst, &origin, 0.0f, &color);
        grl_draw_text ("cam", (gint) rect.x, (gint) rect.y + 30, 10, &color);
    }
    EndMode2D ();

    rlPushMatrix ();
    rlTranslatef (30.0f, 150.0f, 0.0f);
    rlScalef (1.5f, 0.75f, 1.0f);
    grl_draw_rectangle_rec (&rect, &color);
    grl_draw_texture_pro (assets->checker, &src, &dst, &origin, 0.0f, &color);
    grl_draw_text ("pushed", 5, 5, 10, &color);
    rlPopMatrix ();

    rlPushMatrix ();
    rect.y = 200.0f;
    grl_draw_rectangle_rounded (&rect, 0.5f, 6, &color);
    rlPopMatrix ();
}

/* scene_depth:
 * Overlapping quads under the depth test: the result depends on every
 * vertex's z, i.e. on the batch depth stepping exactly as before. */
static void
scene_depth (gpointer data)
{
    Assets      *assets = data;
    GrlRectangle src = { 0.0f, 0.0f, 8.0f, 8.0f };
    GrlVector2   origin = { 0.0f, 0.0f };
    guint        i;

    rlEnableDepthTest ();
    for (i = 0; i < 40; i++)
    {
        GrlColor     color = { (guint8) (i * 50), (guint8) (i * 20), 255 - (guint8) (i * 6), 255 };
        GrlRectangle rect = { (gfloat) (i * 6), (gfloat) (i * 4), 60.0f, 50.0f };
        GrlRectangle dst = { (gfloat) (i * 6) + 10.0f, (gfloat) (i * 4) + 30.0f, 40.0f, 40.0f };

        grl_draw_rectangle_rec (&rect, &color);
        grl_draw_texture_pro (assets->stripes, &src, &dst, &origin, 0.0f, &color);
        grl_draw_rectangle_rounded (&rect, 0.6f, 5, &color);
        grl_draw_text ("depth", (gint) rect.x, (gint) rect.y, 20, &color);
    }
    rlDrawRenderBatchActive ();
    rlDisableDepthTest ();
}

/* ==========================================================================
 * Tests
 * ========================================================================== */

static Assets *
assets_new (void)
{
    Assets *assets = g_new0 (Assets, 1);

    assets->checker = make_texture (8, 8, 0);
    assets->stripes = make_texture (4, 2, 1);
    assets->font = grl_font_new_default ();
    return assets;
}

static void
assets_free (Assets *assets)
{
    g_clear_object (&assets->checker);
    g_clear_object (&assets->stripes);
    g_clear_object (&assets->font);
    g_free (assets);
}

static void
test_fast2d_mixed (void)
{
    g_autoptr(GrlWindow) window = NULL;
    Assets              *assets;

    OPEN_OR_SKIP (window);
    assets = assets_new ();
    assert_same (scene_mixed, assets, 40);
    assets_free (assets);
}

static void
test_fast2d_overflow (void)
{
    g_autoptr(GrlWindow) window = NULL;
    Assets              *assets;

    OPEN_OR_SKIP (window);
    assets = assets_new ();
    assert_same (scene_overflow, assets, 9000);
    assets_free (assets);
}

static void
test_fast2d_camera (void)
{
    g_autoptr(GrlWindow) window = NULL;
    Assets              *assets;

    OPEN_OR_SKIP (window);
    assets = assets_new ();
    assert_same (scene_camera, assets, 20);
    assets_free (assets);
}

static void
test_fast2d_depth (void)
{
    g_autoptr(GrlWindow) window = NULL;
    Assets              *assets;

    OPEN_OR_SKIP (window);
    assets = assets_new ();
    assert_same (scene_depth, assets, 40);
    assets_free (assets);
}

/* Another batch activated behind graylib's back (and raylib's default
 * batch restored): the fast path notices it is not the active batch,
 * draws through raylib, and installs itself again on the next draw. */
static void
test_fast2d_foreign_batch (void)
{
    g_autoptr(GrlWindow) window = NULL;
    Assets              *assets;
    rlRenderBatch        other;

    OPEN_OR_SKIP (window);
    assets = assets_new ();
    assert_same (scene_mixed, assets, 40);

    other = rlLoadRenderBatch (1, 1024);
    rlSetRenderBatchActive (&other);
    assert_same (scene_mixed, assets, 40);
    rlSetRenderBatchActive (NULL);
    rlUnloadRenderBatch (other);

    rlSetRenderBatchActive (NULL);
    assert_same (scene_mixed, assets, 40);
    assets_free (assets);
}

/* A second window in the same process (GrlWindow shuts the batch down
 * before CloseWindow(); a new one is installed in the new context). */
static void
test_fast2d_reopen (void)
{
    GrlWindow *window;
    Assets    *assets;
    guint      round;

    for (round = 0; round < 2; round++)
    {
        window = open_window ();
        if (window == NULL)
        {
            g_test_skip ("No display / GL context available");
            return;
        }
        assets = assets_new ();
        assert_same (scene_mixed, assets, 40);
        assets_free (assets);
        g_object_unref (window);
    }
}

/* The switch and its default */
static void
test_fast2d_switch (void)
{
    grl_draw_set_fast_path_enabled (FALSE);
    g_assert_false (grl_draw_get_fast_path_enabled ());
    g_assert_false (grl_fast2d_begin ());
    grl_draw_set_fast_path_enabled (TRUE);
    g_assert_true (grl_draw_get_fast_path_enabled ());
}

int
main (int   argc,
      char *argv[])
{
    g_test_init (&argc, &argv, NULL);

    g_test_add_func ("/fast2d/switch", test_fast2d_switch);
    g_test_add_func ("/fast2d/mixed", test_fast2d_mixed);
    g_test_add_func ("/fast2d/overflow", test_fast2d_overflow);
    g_test_add_func ("/fast2d/camera", test_fast2d_camera);
    g_test_add_func ("/fast2d/depth", test_fast2d_depth);
    g_test_add_func ("/fast2d/foreign-batch", test_fast2d_foreign_batch);
    g_test_add_func ("/fast2d/reopen", test_fast2d_reopen);

    return g_test_run ();
}
