/* grl-math-utils.h
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Math utility functions for common operations like clamping,
 * interpolation, and remapping.
 */

#pragma once

#if !defined(GRAYLIB_INSIDE) && !defined(GRAYLIB_COMPILATION)
#error "Only <graylib.h> can be included directly."
#endif

#include <glib.h>
#include "../grl-version.h"

G_BEGIN_DECLS

/*
 * =============================================================================
 * Scalar Utilities
 * =============================================================================
 */

/**
 * grl_math_clamp:
 * @value: The value to clamp
 * @min: Minimum value
 * @max: Maximum value
 *
 * Clamps a value between minimum and maximum bounds.
 *
 * Returns: The clamped value
 */
GRL_AVAILABLE_IN_ALL
gfloat      grl_math_clamp          (gfloat value,
                                     gfloat min,
                                     gfloat max);

/**
 * grl_math_clamp_int:
 * @value: The value to clamp
 * @min: Minimum value
 * @max: Maximum value
 *
 * Clamps an integer value between minimum and maximum bounds.
 *
 * Returns: The clamped value
 */
GRL_AVAILABLE_IN_ALL
gint        grl_math_clamp_int      (gint   value,
                                     gint   min,
                                     gint   max);

/**
 * grl_math_lerp:
 * @start: Start value
 * @end: End value
 * @amount: Interpolation amount (0.0 to 1.0)
 *
 * Linearly interpolates between two values.
 *
 * Returns: The interpolated value
 */
GRL_AVAILABLE_IN_ALL
gfloat      grl_math_lerp           (gfloat start,
                                     gfloat end,
                                     gfloat amount);

/**
 * grl_math_normalize:
 * @value: The value to normalize
 * @start: Start of range
 * @end: End of range
 *
 * Normalizes a value from a range to 0.0-1.0.
 * This is the inverse of lerp.
 *
 * Returns: The normalized value (0.0 to 1.0)
 */
GRL_AVAILABLE_IN_ALL
gfloat      grl_math_normalize      (gfloat value,
                                     gfloat start,
                                     gfloat end);

/**
 * grl_math_remap:
 * @value: The value to remap
 * @input_start: Start of input range
 * @input_end: End of input range
 * @output_start: Start of output range
 * @output_end: End of output range
 *
 * Remaps a value from one range to another.
 *
 * Returns: The remapped value
 */
GRL_AVAILABLE_IN_ALL
gfloat      grl_math_remap          (gfloat value,
                                     gfloat input_start,
                                     gfloat input_end,
                                     gfloat output_start,
                                     gfloat output_end);

/**
 * grl_math_wrap:
 * @value: The value to wrap
 * @min: Minimum of range
 * @max: Maximum of range
 *
 * Wraps a value within a range, like modulo but works with floats
 * and handles negative values correctly.
 *
 * Returns: The wrapped value
 */
GRL_AVAILABLE_IN_ALL
gfloat      grl_math_wrap           (gfloat value,
                                     gfloat min,
                                     gfloat max);

/**
 * grl_math_float_equals:
 * @x: First value
 * @y: Second value
 *
 * Checks if two floats are approximately equal using epsilon comparison.
 * Uses a relative epsilon based on the magnitude of the values.
 *
 * Returns: %TRUE if the values are approximately equal
 */
GRL_AVAILABLE_IN_ALL
gboolean    grl_math_float_equals   (gfloat x,
                                     gfloat y);

/*
 * =============================================================================
 * Angle Utilities
 * =============================================================================
 */

/**
 * grl_math_deg_to_rad:
 * @degrees: Angle in degrees
 *
 * Converts degrees to radians.
 *
 * Returns: Angle in radians
 */
GRL_AVAILABLE_IN_ALL
gfloat      grl_math_deg_to_rad     (gfloat degrees);

/**
 * grl_math_rad_to_deg:
 * @radians: Angle in radians
 *
 * Converts radians to degrees.
 *
 * Returns: Angle in degrees
 */
GRL_AVAILABLE_IN_ALL
gfloat      grl_math_rad_to_deg     (gfloat radians);

G_END_DECLS
