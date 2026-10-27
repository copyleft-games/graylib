/* grl-interpolation-private.h
 *
 * Copyright 2026 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Inline forms of the render interpolation track operations, for graylib's
 * own hot paths (entity setters, sprite draws): the exported functions
 * cross the PLT on every call, these compile to a few compares.
 */

#pragma once

#include <math.h>
#include "grl-interpolation.h"

G_BEGIN_DECLS

/* The process-wide clock (grl-interpolation.c) */
typedef struct
{
    gboolean enabled;
    gboolean in_step;
    guint64  step;
    gfloat   alpha;
} GrlInterpolationClock;

G_GNUC_INTERNAL extern GrlInterpolationClock grl_interpolation_clock;

static inline gboolean
grl_interpolation_track_begin_write_inline (GrlInterpolationTrack *track)
{
    if (G_LIKELY (!grl_interpolation_clock.enabled) || !track->enabled ||
        !grl_interpolation_clock.in_step ||
        track->step == grl_interpolation_clock.step)
        return FALSE;

    track->step = grl_interpolation_clock.step;
    track->fresh = FALSE;
    return TRUE;
}

static inline void
grl_interpolation_track_write_float_inline (GrlInterpolationTrack *track,
                                            gfloat                *current,
                                            gfloat                *previous,
                                            gfloat                 value)
{
    if (grl_interpolation_clock.enabled && track->enabled)
    {
        if (grl_interpolation_clock.in_step)
        {
            /* A fresh (new or teleported) owner does not streak */
            if (track->fresh)
                *previous = value;
        }
        else if (track->step == grl_interpolation_clock.step)
        {
            /* Per-frame write over an in-flight blend: shift the previous
             * pose too, so the write shows in full at once */
            *previous += value - *current;
        }
    }

    *current = value;
}

static inline gboolean
grl_interpolation_track_get_blend_inline (const GrlInterpolationTrack *track,
                                          gfloat                      *alpha)
{
    if (G_LIKELY (!grl_interpolation_clock.enabled) || !track->enabled ||
        track->fresh || track->step != grl_interpolation_clock.step)
        return FALSE;

    *alpha = grl_interpolation_clock.alpha;
    return TRUE;
}

static inline gfloat
grl_interpolation_lerp_inline (gfloat from,
                               gfloat to,
                               gfloat t)
{
    return from + (to - from) * t;
}

static inline gfloat
grl_interpolation_lerp_angle_inline (gfloat from,
                                     gfloat to,
                                     gfloat t,
                                     gfloat full_turn)
{
    gfloat diff;
    gfloat half;

    if (t >= 1.0f)
        return to;

    /* Fold the difference into [-half, half) so the blend takes the
     * shorter way round; fmodf only for differences past half a turn */
    diff = to - from;
    half = full_turn * 0.5f;
    if (full_turn > 0.0f && (diff >= half || diff < -half))
    {
        diff = fmodf (diff, full_turn);
        if (diff >= half)
            diff -= full_turn;
        else if (diff < -half)
            diff += full_turn;
    }

    return from + diff * t;
}

G_END_DECLS
