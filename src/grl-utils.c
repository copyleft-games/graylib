/* grl-utils.c
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Utility functions implementation.
 */

#include "config.h"
#include "grl-utils.h"
#include <raylib.h>

/**
 * grl_set_random_seed:
 * @seed: The seed value for the random number generator
 *
 * Sets the seed for raylib's internal random number generator.
 * Use this for reproducible random sequences.
 *
 * Example:
 * |[<!-- language="C" -->
 * // Set seed for reproducible results
 * grl_set_random_seed (12345);
 *
 * // Now random values will be the same each run
 * gint value = grl_get_random_value (0, 100);
 * ]|
 */
void
grl_set_random_seed (guint seed)
{
    SetRandomSeed (seed);
}

/**
 * grl_get_random_value:
 * @min: Minimum value (inclusive)
 * @max: Maximum value (inclusive)
 *
 * Gets a random value between @min and @max (both inclusive).
 *
 * Returns: A random integer in the range [min, max]
 */
gint
grl_get_random_value (gint min,
                      gint max)
{
    return GetRandomValue (min, max);
}
