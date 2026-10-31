/* grl-interpolation.h
 *
 * Copyright 2026 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Render interpolation clock for fixed-timestep simulations.
 */

#pragma once

#if !defined(GRAYLIB_INSIDE) && !defined(GRAYLIB_COMPILATION)
#error "Only <graylib.h> can be included directly."
#endif

#include <glib-object.h>
#include "../grl-version.h"

G_BEGIN_DECLS

/**
 * GrlInterpolationTrack:
 * @step: the simulation step in which the owner's previous pose was last
 *   captured (or reset)
 * @fresh: %TRUE while the owner was created or teleported in @step; writes
 *   then move the previous pose along with the current one
 * @enabled: whether the owner takes part in interpolation at all
 *
 * Per-object bookkeeping for render interpolation, embedded in the private
 * data of an interpolated type (#GrlEntity and the libregnum transforms,
 * shapes and cameras). An owner keeps a copy of its pose as it was at the
 * start of the latest simulation step (the "previous" pose) next to its
 * true pose, and draws a blend of the two.
 *
 * The previous pose is captured lazily: the first write in a step copies
 * the current pose into it (grl_interpolation_track_begin_write()), so
 * objects that do not move cost nothing. An owner whose @step is not the
 * latest step did not move in it, and its render pose is its true pose.
 *
 * The fields are private to the owner; use the grl_interpolation_track_*()
 * functions.
 */
typedef struct _GrlInterpolationTrack GrlInterpolationTrack;

struct _GrlInterpolationTrack
{
    /*< private >*/
    guint64  step;
    gboolean fresh;
    gboolean enabled;
};

/* ==========================================================================
 * Global clock
 * ========================================================================== */

/**
 * grl_interpolation_set_enabled:
 * @enabled: whether render interpolation is active
 *
 * Turns render interpolation on or off for the whole process. A
 * fixed-timestep game loop (libregnum's LrgGameTemplate) enables it;
 * while it is off every render pose equals the true pose and setters do
 * no extra work beyond one branch. Changing the state starts a new step,
 * so stale previous poses are never blended.
 */
GRL_AVAILABLE_IN_ALL
void            grl_interpolation_set_enabled   (gboolean        enabled);

/**
 * grl_interpolation_get_enabled:
 *
 * Gets whether render interpolation is active.
 *
 * Returns: %TRUE when render poses are blended
 */
GRL_AVAILABLE_IN_ALL
gboolean        grl_interpolation_get_enabled   (void);

/**
 * grl_interpolation_begin_step:
 *
 * Marks the start of one fixed simulation step. Every interpolated object
 * written during the step captures its previous pose on its first write.
 * Steps must not nest; call grl_interpolation_end_step() when the step's
 * simulation is done.
 */
GRL_AVAILABLE_IN_ALL
void            grl_interpolation_begin_step    (void);

/**
 * grl_interpolation_end_step:
 *
 * Marks the end of the current fixed simulation step. Writes made outside
 * a step (per-frame code such as camera smoothing or mouse look) are
 * applied to the previous pose as well, so they show up in full at once
 * and do not disturb an in-flight blend.
 */
GRL_AVAILABLE_IN_ALL
void            grl_interpolation_end_step      (void);

/**
 * grl_interpolation_in_step:
 *
 * Gets whether a fixed simulation step is running.
 *
 * Returns: %TRUE between grl_interpolation_begin_step() and
 *   grl_interpolation_end_step()
 */
GRL_AVAILABLE_IN_ALL
gboolean        grl_interpolation_in_step       (void);

/**
 * grl_interpolation_get_step:
 *
 * Gets the serial number of the latest simulation step. It grows by one
 * with every grl_interpolation_begin_step() and with every change of
 * grl_interpolation_set_enabled().
 *
 * Returns: the latest step serial
 */
GRL_AVAILABLE_IN_ALL
guint64         grl_interpolation_get_step      (void);

/**
 * grl_interpolation_set_alpha:
 * @alpha: blend factor between the previous and the current pose, clamped
 *   to 0..1
 *
 * Publishes the render blend factor for the coming draw: the simulation
 * time left in the accumulator divided by the fixed step. 0 draws the
 * pose at the start of the latest step, 1 the true pose.
 */
GRL_AVAILABLE_IN_ALL
void            grl_interpolation_set_alpha     (gfloat          alpha);

/**
 * grl_interpolation_get_alpha:
 *
 * Gets the render blend factor published by the game loop, for draw code
 * that blends its own state. It is 1 while interpolation is disabled.
 *
 * Returns: the blend factor (0..1)
 */
GRL_AVAILABLE_IN_ALL
gfloat          grl_interpolation_get_alpha     (void);

/* ==========================================================================
 * Blend helpers
 * ========================================================================== */

/**
 * grl_interpolation_lerp:
 * @from: value at @t = 0
 * @to: value at @t = 1
 * @t: blend factor
 *
 * Linear blend of two values.
 *
 * Returns: @from + (@to - @from) * @t
 */
GRL_AVAILABLE_IN_ALL
gfloat          grl_interpolation_lerp          (gfloat          from,
                                                 gfloat          to,
                                                 gfloat          t);

/**
 * grl_interpolation_lerp_angle:
 * @from: angle at @t = 0
 * @to: angle at @t = 1
 * @t: blend factor
 * @full_turn: the size of a full turn in the angle's unit (360 for
 *   degrees, 2 pi for radians)
 *
 * Blends two angles along the shorter arc, so 350 to 10 degrees passes
 * through 0 instead of sweeping back through 180. The result is not
 * wrapped: at @t = 0 it is exactly @from, at @t = 1 exactly @to.
 *
 * Returns: the blended angle
 */
GRL_AVAILABLE_IN_ALL
gfloat          grl_interpolation_lerp_angle    (gfloat          from,
                                                 gfloat          to,
                                                 gfloat          t,
                                                 gfloat          full_turn);

/* ==========================================================================
 * Per-object tracks (for implementers of interpolated types)
 * ========================================================================== */

/**
 * grl_interpolation_track_init:
 * @track: (out caller-allocates): a #GrlInterpolationTrack
 *
 * Initialises a track for a new object: enabled, and fresh for the
 * current step, so the object starts with previous == current and writes
 * made in the step it was created in do not streak.
 */
GRL_AVAILABLE_IN_ALL
void            grl_interpolation_track_init        (GrlInterpolationTrack *track);

/**
 * grl_interpolation_track_reset:
 * @track: a #GrlInterpolationTrack
 *
 * Marks a teleport: the owner copies its current pose into its previous
 * pose and calls this, and for the rest of the current step its writes
 * move both poses. Used by the teleport and reset-interpolation calls.
 */
GRL_AVAILABLE_IN_ALL
void            grl_interpolation_track_reset       (GrlInterpolationTrack *track);

/**
 * grl_interpolation_track_begin_write:
 * @track: a #GrlInterpolationTrack
 *
 * Call before changing any interpolated field of the owner. Returns
 * %TRUE when the owner must copy its current pose into its previous pose
 * first: the first write inside a new simulation step.
 *
 * Returns: %TRUE if the previous pose must be captured now
 */
GRL_AVAILABLE_IN_ALL
gboolean        grl_interpolation_track_begin_write (GrlInterpolationTrack *track);

/**
 * grl_interpolation_track_write_float:
 * @track: a #GrlInterpolationTrack
 * @current: (inout): the true value
 * @previous: (inout): the matching previous-pose value
 * @value: the new true value
 *
 * Stores @value in @current and keeps @previous consistent: inside a step
 * the previous value is left alone (unless the owner is fresh, when it
 * follows); outside a step it is shifted by the same amount so per-frame
 * writes take effect at once. Call grl_interpolation_track_begin_write()
 * first.
 */
GRL_AVAILABLE_IN_ALL
void            grl_interpolation_track_write_float (GrlInterpolationTrack *track,
                                                     gfloat                *current,
                                                     gfloat                *previous,
                                                     gfloat                 value);

/**
 * grl_interpolation_track_get_blend:
 * @track: a #GrlInterpolationTrack
 * @alpha: (out) (optional): return location for the blend factor
 *
 * Decides how the owner is drawn. Returns %TRUE when the render pose is
 * a blend of the previous and the current pose with *@alpha; %FALSE when
 * it is simply the current pose (interpolation off, owner opted out, or
 * the owner did not move in the latest step).
 *
 * Returns: %TRUE if the render pose must be blended
 */
GRL_AVAILABLE_IN_ALL
gboolean        grl_interpolation_track_get_blend   (const GrlInterpolationTrack *track,
                                                     gfloat                      *alpha);

/**
 * grl_interpolation_track_is_cut:
 * @track: a #GrlInterpolationTrack
 *
 * Gets whether the owner was teleported (or created) in the current step,
 * so state derived from it should be teleported too.
 *
 * Returns: %TRUE if the owner's motion in this step is a cut
 */
GRL_AVAILABLE_IN_ALL
gboolean        grl_interpolation_track_is_cut      (const GrlInterpolationTrack *track);

G_END_DECLS
