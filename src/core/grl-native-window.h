/* grl-native-window.h
 *
 * Copyright 2026 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Native window size changes raylib does not follow by itself (Android:
 * a foldable opened or closed, split screen, a freeform window resized).
 */

#pragma once

#if !defined(GRAYLIB_INSIDE) && !defined(GRAYLIB_COMPILATION)
#error "Only <graylib.h> can be included directly."
#endif

#include <glib.h>
#include "grl-version.h"

G_BEGIN_DECLS

GRL_AVAILABLE_IN_ALL
gboolean     grl_native_window_can_resize (void);

GRL_AVAILABLE_IN_ALL
gboolean     grl_native_window_resize     (gint width,
                                           gint height);

G_END_DECLS
