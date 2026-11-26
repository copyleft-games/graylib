/* test-window-adopt.c
 *
 * Copyright 2026 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * One window per process (docs/api/core.org): a GrlWindow made while the
 * raylib window is already open adopts it (no second InitWindow()), takes
 * its own title, and never closes the window when it goes away; the
 * window's opener does.  Needs a display; skips without one.
 */

#include <graylib.h>
#include <raylib.h>

/* the first window, hidden, or NULL without a display */
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
    window = grl_window_new (64, 64, "test-window-adopt");
    if (window != NULL && !grl_window_is_ready (window))
        g_clear_object (&window);
    return window;
}

/* a second window adopts the first's; releasing it leaves the window
 * open; releasing the opener closes it */
static void
test_adopt (void)
{
    g_autoptr(GrlWindow) second = NULL;
    GrlWindow *first;

    first = open_window ();
    if (first == NULL)
    {
        g_test_skip ("No display / GL context available");
        return;
    }
    g_assert_true (IsWindowReady ());

    second = grl_window_new (32, 32, "adopted");
    g_assert_true (grl_window_is_ready (second));
    g_assert_cmpstr (grl_window_get_title (second), ==, "adopted");

    /* both draw into the same window */
    grl_window_begin_drawing (second);
    grl_window_end_drawing (second);

    g_clear_object (&second);
    g_assert_true (IsWindowReady ());
    g_assert_true (grl_window_is_ready (first));

    g_object_unref (first);
    g_assert_false (IsWindowReady ());
}

/* after the opener closed the window, a new GrlWindow opens one again */
static void
test_reopen (void)
{
    GrlWindow *window;

    window = open_window ();
    if (window == NULL)
    {
        g_test_skip ("No display / GL context available");
        return;
    }
    g_assert_true (IsWindowReady ());
    g_object_unref (window);
    g_assert_false (IsWindowReady ());
}

int
main (int   argc,
      char *argv[])
{
    g_test_init (&argc, &argv, NULL);

    g_test_add_func ("/window/adopt", test_adopt);
    g_test_add_func ("/window/reopen", test_reopen);

    return g_test_run ();
}
