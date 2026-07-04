/* test-input-event-mods.c
 *
 * Copyright 2026 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Unit tests for the event-carried modifier tracker (grl_input_get_event_mods
 * and friends).  The recorder is pure state driven by
 * grl_input_record_event_mods / grl_input_record_focus, so everything here
 * runs headless; grl_input_event_mods_init needs a window and must simply
 * fail cleanly without one.
 */

#include <glib.h>
#include "src/core/grl-input.h"

/*
 * Before any event: serial 0, mods 0 (callers use serial to fall back).
 */
static void
test_event_mods_initial (void)
{
    g_assert_cmpuint (grl_input_get_event_mods_serial (), ==, 0);
    g_assert_cmpuint (grl_input_get_event_mods (), ==, 0);
    g_assert_cmpuint (grl_input_get_focus_generation (), ==, 0);
}

/*
 * Recording replaces the mask verbatim and bumps the serial each time.
 */
static void
test_event_mods_record (void)
{
    grl_input_record_event_mods (GRL_EVENT_MOD_SUPER | GRL_EVENT_MOD_SHIFT);
    g_assert_cmpuint (grl_input_get_event_mods (), ==,
                      GRL_EVENT_MOD_SUPER | GRL_EVENT_MOD_SHIFT);
    g_assert_cmpuint (grl_input_get_event_mods_serial (), ==, 1);

    grl_input_record_event_mods (GRL_EVENT_MOD_CONTROL);
    g_assert_cmpuint (grl_input_get_event_mods (), ==, GRL_EVENT_MOD_CONTROL);
    g_assert_cmpuint (grl_input_get_event_mods_serial (), ==, 2);
}

/*
 * The screenshot-chord scenario: Super goes down (event carries super),
 * the compositor swallows the release, and the NEXT event's mask -- which
 * the window system computes from its own xkb state -- no longer contains
 * super.  The tracker must report the new truth immediately; nothing
 * latches.
 */
static void
test_event_mods_swallowed_release_self_corrects (void)
{
    /* Super pressed: its own key event carries the super bit. */
    grl_input_record_event_mods (GRL_EVENT_MOD_SUPER);
    g_assert_cmpuint (grl_input_get_event_mods () & GRL_EVENT_MOD_SUPER,
                      !=, 0);

    /* Chord + release swallowed by the compositor: no event recorded.
       The user then types 'j'; its event carries the REAL current state. */
    grl_input_record_event_mods (0);
    g_assert_cmpuint (grl_input_get_event_mods (), ==, 0);
}

/*
 * Mods of value 0 still bump the serial (a plain keypress is an event and
 * must overwrite a stale nonzero mask).
 */
static void
test_event_mods_zero_still_counts (void)
{
    guint64 before = grl_input_get_event_mods_serial ();

    grl_input_record_event_mods (GRL_EVENT_MOD_ALT);
    grl_input_record_event_mods (0);
    g_assert_cmpuint (grl_input_get_event_mods (), ==, 0);
    g_assert_cmpuint (grl_input_get_event_mods_serial (), ==, before + 2);
}

/*
 * Focus generation bumps on BOTH edges, so a loss+regain that straddles
 * one poll is still visible as generation += 2.
 */
static void
test_focus_generation_both_edges (void)
{
    guint before = grl_input_get_focus_generation ();

    grl_input_record_focus (FALSE);
    grl_input_record_focus (TRUE);
    g_assert_cmpuint (grl_input_get_focus_generation (), ==, before + 2);
}

/*
 * Without a window, arming the GLFW hooks must fail cleanly (headless).
 */
static void
test_event_mods_init_headless (void)
{
    g_assert_false (grl_input_event_mods_init ());
    g_assert_false (grl_input_event_mods_armed ());
}

int
main (int argc, char *argv[])
{
    g_test_init (&argc, &argv, NULL);

    g_test_add_func ("/input/event-mods/initial", test_event_mods_initial);
    g_test_add_func ("/input/event-mods/record", test_event_mods_record);
    g_test_add_func ("/input/event-mods/swallowed-release",
                     test_event_mods_swallowed_release_self_corrects);
    g_test_add_func ("/input/event-mods/zero-still-counts",
                     test_event_mods_zero_still_counts);
    g_test_add_func ("/input/event-mods/focus-generation",
                     test_focus_generation_both_edges);
    g_test_add_func ("/input/event-mods/init-headless",
                     test_event_mods_init_headless);

    return g_test_run ();
}
