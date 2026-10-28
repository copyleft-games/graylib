/* test-frame-sync.c
 *
 * Copyright 2026 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Tests for bounded frames in flight (grl_frame_sync_*, GrlWindow's
 * max-frames-in-flight).  The window tests need a display; under Xvfb
 * (llvmpipe) the context has sync objects, so the fence path runs.
 */

#include <graylib.h>
#include <raylib.h>

#define SKIP_IF_NO_DISPLAY() \
    do { \
        if (g_getenv ("DISPLAY") == NULL && g_getenv ("WAYLAND_DISPLAY") == NULL) \
        { \
            g_test_skip ("No display available (headless environment)"); \
            return; \
        } \
    } while (0)

/* Draws and presents @frames frames through grl_window_end_drawing(),
   checking the in-flight bound after each */
static void
present_frames (GrlWindow *window,
                guint      frames,
                guint      bound)
{
    g_autoptr(GrlColor) bg = NULL;
    guint               i;

    bg = grl_color_new (10, 20, 30, 255);
    for (i = 0; i < frames; i++)
    {
        grl_window_begin_drawing (window);
        grl_window_clear_background (window, bg);
        grl_window_end_drawing (window);
        g_assert_cmpuint (grl_frame_sync_get_pending (), <=, bound);
    }
}

static GrlWindow *
open_window (void)
{
    GrlWindow *window;

    SetTraceLogLevel (LOG_WARNING);
    window = grl_window_new (64, 64, "test-frame-sync");
    if (window != NULL && !grl_window_is_ready (window))
        g_clear_object (&window);
    return window;
}

/* Without a window: the bound is recorded and clamped, and every call on
   the present path is a harmless no-op. */
static void
test_frame_sync_headless (void)
{
    grl_frame_sync_set_max_frames_in_flight (0);
    g_assert_cmpuint (grl_frame_sync_get_max_frames_in_flight (), ==, 0);
    g_assert_false (grl_frame_sync_is_active ());

    grl_frame_sync_set_max_frames_in_flight (2);
    g_assert_cmpuint (grl_frame_sync_get_max_frames_in_flight (), ==, 2);

    grl_frame_sync_set_max_frames_in_flight (99);
    g_assert_cmpuint (grl_frame_sync_get_max_frames_in_flight (), ==,
                      GRL_FRAME_SYNC_MAX_FRAMES);

    /* no GL context: unsupported, inactive, nothing pending */
    g_assert_false (grl_frame_sync_is_supported ());
    g_assert_false (grl_frame_sync_is_active ());
    grl_frame_sync_before_present ();
    g_assert_cmpuint (grl_frame_sync_get_pending (), ==, 0);
    g_assert_cmpfloat (grl_frame_sync_get_last_wait (), ==, 0.0);

    grl_frame_sync_reset ();
    grl_frame_sync_set_max_frames_in_flight (0);
}

/* The bound holds on the end-drawing path (a bound of N leaves at most
   N - 1 frames unfinished after a present), a lowered bound drains at the
   next present, and 0 drops every fence. */
static void
test_frame_sync_window (void)
{
    g_autoptr(GrlWindow) window = NULL;

    SKIP_IF_NO_DISPLAY ();
    window = open_window ();
    if (window == NULL)
    {
        g_test_skip ("Could not open a window");
        return;
    }

    /* off by default: end drawing is plain EndDrawing(), no fences */
    g_assert_cmpuint (grl_window_get_max_frames_in_flight (window), ==, 0);
    present_frames (window, 3, 0);
    g_assert_cmpuint (grl_frame_sync_get_pending (), ==, 0);

    /* raylib's desktop build is GL 3.3 core: sync objects are core */
    g_assert_true (grl_frame_sync_is_supported ());

    grl_window_set_max_frames_in_flight (window, 3);
    g_assert_true (grl_frame_sync_is_active ());
    present_frames (window, 10, 2);
    g_assert_cmpuint (grl_frame_sync_get_pending (), ==, 2);

    /* lowering takes effect at the next present */
    g_object_set (window, "max-frames-in-flight", 1, NULL);
    g_assert_cmpuint (grl_window_get_max_frames_in_flight (window), ==, 1);
    present_frames (window, 1, 0);
    g_assert_cmpuint (grl_frame_sync_get_pending (), ==, 0);
    present_frames (window, 10, 0);
    g_assert_cmpfloat (grl_frame_sync_get_last_wait (), >=, 0.0);
    g_assert_cmpuint (grl_frame_sync_get_timeouts (), ==, 0);

    /* the swap-only path keeps the same bound */
    grl_window_begin_drawing (window);
    grl_window_swap_buffers (window);
    grl_window_poll_events (window);
    g_assert_cmpuint (grl_frame_sync_get_pending (), ==, 0);

    /* with 2, one frame stays in flight across the present */
    grl_window_set_max_frames_in_flight (window, 2);
    present_frames (window, 3, 1);
    g_assert_cmpuint (grl_frame_sync_get_pending (), ==, 1);

    /* 0 turns it off and drops the outstanding fence */
    grl_window_set_max_frames_in_flight (window, 0);
    g_assert_cmpuint (grl_frame_sync_get_pending (), ==, 0);
    g_assert_false (grl_frame_sync_is_active ());
    present_frames (window, 2, 0);

    /* closing the window deletes fences and forgets the probe */
    grl_window_set_max_frames_in_flight (window, 2);
    present_frames (window, 4, 1);
    g_clear_object (&window);
    g_assert_cmpuint (grl_frame_sync_get_pending (), ==, 0);
    g_assert_false (grl_frame_sync_is_supported ());
    g_assert_cmpuint (grl_frame_sync_get_max_frames_in_flight (), ==, 2);
    grl_frame_sync_set_max_frames_in_flight (0);
}

/* grl_frame_sync_end_drawing() is the raw-raylib form */
static void
test_frame_sync_end_drawing (void)
{
    g_autoptr(GrlWindow) window = NULL;
    guint                i;

    SKIP_IF_NO_DISPLAY ();
    window = open_window ();
    if (window == NULL)
    {
        g_test_skip ("Could not open a window");
        return;
    }

    grl_frame_sync_set_max_frames_in_flight (1);
    for (i = 0; i < 5; i++)
    {
        BeginDrawing ();
        ClearBackground (BLACK);
        grl_frame_sync_end_drawing ();
        g_assert_cmpuint (grl_frame_sync_get_pending (), ==, 0);
    }
    grl_frame_sync_set_max_frames_in_flight (0);
}

int
main (int   argc,
      char *argv[])
{
    g_test_init (&argc, &argv, NULL);

    g_test_add_func ("/frame-sync/headless", test_frame_sync_headless);
    g_test_add_func ("/frame-sync/window", test_frame_sync_window);
    g_test_add_func ("/frame-sync/end-drawing", test_frame_sync_end_drawing);

    return g_test_run ();
}
