/* grl-utils.h
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Utility functions for graylib.
 */

#pragma once

#if !defined(GRAYLIB_INSIDE) && !defined(GRAYLIB_COMPILATION)
#error "Only <graylib.h> can be included directly."
#endif

#include <glib.h>
#include "grl-version.h"

G_BEGIN_DECLS

/*
 * Random number generation
 */

GRL_AVAILABLE_IN_ALL
void grl_set_random_seed  (guint seed);

GRL_AVAILABLE_IN_ALL
gint grl_get_random_value (gint  min,
                           gint  max);

G_END_DECLS
