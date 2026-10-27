/* grl-interpolation.c
 *
 * Copyright 2026 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Render interpolation clock for fixed-timestep simulations.
 */

#include "config.h"
#include "grl-interpolation-private.h"

/**
 * SECTION:grl-interpolation
 * @title: Render interpolation
 * @short_description: Smooth drawing of a fixed-timestep simulation
 *
 * A game that simulates in fixed steps (say 60 per second) on a display
 * with another refresh rate (say 144 Hz) runs 0, 1 or 2 steps per frame.
 * Drawn as is, objects move on a 2,3,2,3 frame cadence (judder). With
 * render interpolation each interpolated object keeps the pose it had at
 * the start of the latest step and is drawn at a blend of that pose and
 * its true pose, by the fraction of a step left in the accumulator.
 *
 * The game loop drives the clock: grl_interpolation_set_enabled() once
 * it runs fixed steps, grl_interpolation_begin_step() and
 * grl_interpolation_end_step() around every step, and
 * grl_interpolation_set_alpha() before drawing. Objects capture their
 * previous pose lazily on their first write in a step (see
 * #GrlInterpolationTrack), so nothing is done for objects that do not
 * move. Getters keep returning the true state; draw code uses the
 * separate render getters (grl_entity_get_render_pose() and friends).
 */

/* The clock. One per process: there is one game loop, and draw code that
 * has no pointer to it still needs the alpha. Step 1 is the state before
 * any game loop ran; tracks created then are fresh for it. Shared with
 * graylib's inline hot paths (grl-interpolation-private.h). */
GrlInterpolationClock grl_interpolation_clock = { FALSE, FALSE, 1, 1.0f };

/* ==========================================================================
 * Global clock
 * ========================================================================== */

void
grl_interpolation_set_enabled (gboolean enabled)
{
    enabled = (enabled != FALSE);
    if (enabled == grl_interpolation_clock.enabled)
        return;

    /* A new step invalidates every captured previous pose, so nothing
     * recorded before the switch is ever blended */
    grl_interpolation_clock.enabled = enabled;
    grl_interpolation_clock.step++;
    if (!enabled)
        grl_interpolation_clock.alpha = 1.0f;
}

gboolean
grl_interpolation_get_enabled (void)
{
    return grl_interpolation_clock.enabled;
}

void
grl_interpolation_begin_step (void)
{
    grl_interpolation_clock.step++;
    grl_interpolation_clock.in_step = TRUE;
}

void
grl_interpolation_end_step (void)
{
    grl_interpolation_clock.in_step = FALSE;
}

gboolean
grl_interpolation_in_step (void)
{
    return grl_interpolation_clock.in_step;
}

guint64
grl_interpolation_get_step (void)
{
    return grl_interpolation_clock.step;
}

void
grl_interpolation_set_alpha (gfloat alpha)
{
    grl_interpolation_clock.alpha = CLAMP (alpha, 0.0f, 1.0f);
}

gfloat
grl_interpolation_get_alpha (void)
{
    return grl_interpolation_clock.enabled ? grl_interpolation_clock.alpha : 1.0f;
}

/* ==========================================================================
 * Blend helpers
 * ========================================================================== */

gfloat
grl_interpolation_lerp (gfloat from,
                        gfloat to,
                        gfloat t)
{
    return grl_interpolation_lerp_inline (from, to, t);
}

gfloat
grl_interpolation_lerp_angle (gfloat from,
                              gfloat to,
                              gfloat t,
                              gfloat full_turn)
{
    return grl_interpolation_lerp_angle_inline (from, to, t, full_turn);
}

/* ==========================================================================
 * Per-object tracks
 * ========================================================================== */

void
grl_interpolation_track_init (GrlInterpolationTrack *track)
{
    g_return_if_fail (track != NULL);

    track->step = grl_interpolation_clock.step;
    track->fresh = TRUE;
    track->enabled = TRUE;
}

void
grl_interpolation_track_reset (GrlInterpolationTrack *track)
{
    g_return_if_fail (track != NULL);

    track->step = grl_interpolation_clock.step;
    track->fresh = TRUE;
}

gboolean
grl_interpolation_track_begin_write (GrlInterpolationTrack *track)
{
    /* Nothing to capture when off, opted out, outside a step, or when
     * the pose was already captured in this step */
    return grl_interpolation_track_begin_write_inline (track);
}

void
grl_interpolation_track_write_float (GrlInterpolationTrack *track,
                                     gfloat                *current,
                                     gfloat                *previous,
                                     gfloat                 value)
{
    grl_interpolation_track_write_float_inline (track, current, previous, value);
}

gboolean
grl_interpolation_track_get_blend (const GrlInterpolationTrack *track,
                                   gfloat                      *alpha)
{
    gfloat a;

    if (!grl_interpolation_track_get_blend_inline (track, &a))
        return FALSE;

    if (alpha != NULL)
        *alpha = a;
    return TRUE;
}

gboolean
grl_interpolation_track_is_cut (const GrlInterpolationTrack *track)
{
    return grl_interpolation_clock.enabled && track->enabled && track->fresh &&
           track->step == grl_interpolation_clock.step;
}
