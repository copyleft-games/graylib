/* grl-frame-sync.h
 *
 * Copyright 2026 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Bounded frames in flight: GL fences that keep the CPU from running more
 * than a set number of frames ahead of the GPU (a "low latency" mode).
 */

#pragma once

#if !defined(GRAYLIB_INSIDE) && !defined(GRAYLIB_COMPILATION)
#error "Only <graylib.h> can be included directly."
#endif

#include <glib.h>
#include "grl-version.h"

G_BEGIN_DECLS

/**
 * GRL_FRAME_SYNC_MAX_FRAMES:
 *
 * The largest value grl_frame_sync_set_max_frames_in_flight() accepts.
 * Larger requests are clamped to it.
 */
#define GRL_FRAME_SYNC_MAX_FRAMES (4)

GRL_AVAILABLE_IN_ALL
void                grl_frame_sync_set_max_frames_in_flight (guint frames);

GRL_AVAILABLE_IN_ALL
guint               grl_frame_sync_get_max_frames_in_flight (void);

GRL_AVAILABLE_IN_ALL
gboolean            grl_frame_sync_is_supported             (void);

GRL_AVAILABLE_IN_ALL
gboolean            grl_frame_sync_is_active                (void);

GRL_AVAILABLE_IN_ALL
void                grl_frame_sync_before_present           (void);

GRL_AVAILABLE_IN_ALL
void                grl_frame_sync_end_drawing              (void);

GRL_AVAILABLE_IN_ALL
gdouble             grl_frame_sync_get_last_wait            (void);

GRL_AVAILABLE_IN_ALL
guint               grl_frame_sync_get_pending              (void);

GRL_AVAILABLE_IN_ALL
guint64             grl_frame_sync_get_timeouts             (void);

GRL_AVAILABLE_IN_ALL
void                grl_frame_sync_reset                    (void);

G_END_DECLS
