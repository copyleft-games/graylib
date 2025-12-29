/* grl-math-utils.c
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Simple math utility functions. These are implemented directly
 * rather than wrapping raymath to avoid symbol conflicts with
 * raylib's internal raymath usage.
 */

#include "config.h"
#include "grl-math-utils.h"

#include <math.h>

/*
 * Constants
 */
#define GRL_DEG2RAD (0.017453292519943295f)
#define GRL_RAD2DEG (57.29577951308232f)
#define GRL_EPSILON (0.000001f)

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
gfloat
grl_math_clamp (gfloat value,
                gfloat min,
                gfloat max)
{
    gfloat result;

    result = value;
    if (result < min)
        result = min;
    else if (result > max)
        result = max;

    return result;
}

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
gint
grl_math_clamp_int (gint value,
                    gint min,
                    gint max)
{
    gint result;

    result = value;
    if (result < min)
        result = min;
    else if (result > max)
        result = max;

    return result;
}

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
gfloat
grl_math_lerp (gfloat start,
               gfloat end,
               gfloat amount)
{
    return start + amount * (end - start);
}

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
gfloat
grl_math_normalize (gfloat value,
                    gfloat start,
                    gfloat end)
{
    return (value - start) / (end - start);
}

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
gfloat
grl_math_remap (gfloat value,
                gfloat input_start,
                gfloat input_end,
                gfloat output_start,
                gfloat output_end)
{
    gfloat normalized;

    normalized = (value - input_start) / (input_end - input_start);
    return output_start + normalized * (output_end - output_start);
}

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
gfloat
grl_math_wrap (gfloat value,
               gfloat min,
               gfloat max)
{
    return value - (max - min) * floorf ((value - min) / (max - min));
}

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
gboolean
grl_math_float_equals (gfloat x,
                       gfloat y)
{
    gfloat diff;

    diff = fabsf (x - y);
    return diff <= GRL_EPSILON * fmaxf (1.0f, fmaxf (fabsf (x), fabsf (y)));
}

/**
 * grl_math_deg_to_rad:
 * @degrees: Angle in degrees
 *
 * Converts degrees to radians.
 *
 * Returns: Angle in radians
 */
gfloat
grl_math_deg_to_rad (gfloat degrees)
{
    return degrees * GRL_DEG2RAD;
}

/**
 * grl_math_rad_to_deg:
 * @radians: Angle in radians
 *
 * Converts radians to degrees.
 *
 * Returns: Angle in degrees
 */
gfloat
grl_math_rad_to_deg (gfloat radians)
{
    return radians * GRL_RAD2DEG;
}
