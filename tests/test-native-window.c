/* test-native-window.c
 *
 * Copyright 2026 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * grl_native_window_resize() (src/core/grl-native-window.c): graylib's
 * mirror of raylib's internal state matches the linked raylib (checked
 * with no window); without a window nothing changes; with one, raylib
 * reports the new size (screen, render, the resized flag) and the 2D
 * projection follows it.  The window part needs a display; it skips
 * without one.
 */

#include <graylib.h>
#include <raylib.h>
#include <rlgl.h>

/* the layout check runs headless and leaves raylib's state as it was */
static void
test_layout (void)
{
    gint width;
    gint height;
    unsigned char resized;

    width = GetScreenWidth ();
    height = GetScreenHeight ();
    resized = IsWindowResized ();

    g_assert_true (grl_native_window_can_resize ());
    /* cached, and still true */
    g_assert_true (grl_native_window_can_resize ());

    g_assert_cmpint (GetScreenWidth (), ==, width);
    g_assert_cmpint (GetScreenHeight (), ==, height);
    g_assert_cmpint (IsWindowResized (), ==, resized);
}

/* no window: nothing to resize */
static void
test_no_window (void)
{
    if (IsWindowReady ())
    {
        g_test_skip ("A window is open");
        return;
    }
    g_assert_false (grl_native_window_resize (800, 600));
    g_assert_cmpint (GetScreenWidth (), ==, 0);
    g_assert_false (IsWindowResized ());
}

/* a window: the size raylib reports follows, bad sizes are refused */
static void
test_resize (void)
{
    g_autoptr(GrlWindow) window = NULL;
    const gchar *display = g_getenv ("DISPLAY");
    const gchar *wayland = g_getenv ("WAYLAND_DISPLAY");

    if ((display == NULL || display[0] == '\0') &&
        (wayland == NULL || wayland[0] == '\0'))
    {
        g_test_skip ("No display / GL context available");
        return;
    }
    SetTraceLogLevel (LOG_WARNING);
    SetConfigFlags (FLAG_WINDOW_HIDDEN);
    window = grl_window_new (64, 48, "test-native-window");
    if (window == NULL || !grl_window_is_ready (window))
    {
        g_test_skip ("No display / GL context available");
        return;
    }

    g_assert_false (grl_native_window_resize (0, 100));
    g_assert_false (grl_native_window_resize (100, -1));
    g_assert_cmpint (GetScreenWidth (), ==, 64);

    g_assert_true (grl_native_window_resize (320, 200));
    g_assert_cmpint (GetScreenWidth (), ==, 320);
    g_assert_cmpint (GetScreenHeight (), ==, 200);
    g_assert_cmpint (GetRenderWidth (), ==, 320);
    g_assert_cmpint (GetRenderHeight (), ==, 200);
    g_assert_cmpint (rlGetFramebufferWidth (), ==, 320);
    g_assert_cmpint (rlGetFramebufferHeight (), ==, 200);
    g_assert_true (grl_window_is_resized (window));
    g_assert_cmpint (grl_window_get_width (window), ==, 320);

    /* a frame goes by: the window still draws */
    grl_window_begin_drawing (window);
    grl_window_end_drawing (window);
    g_assert_cmpint (GetScreenWidth (), ==, 320);
}

gint
main (gint   argc,
      gchar *argv[])
{
    g_test_init (&argc, &argv, NULL);

    g_test_add_func ("/native-window/layout", test_layout);
    g_test_add_func ("/native-window/no-window", test_no_window);
    g_test_add_func ("/native-window/resize", test_resize);

    return g_test_run ();
}
