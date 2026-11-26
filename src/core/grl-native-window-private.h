/* grl-native-window-private.h
 *
 * Copyright 2026 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Inside graylib only: the frame boundary grl-native-window.c needs.
 */

#pragma once

#include <glib.h>

G_BEGIN_DECLS

/*
 * grl_native_window_end_frame_private:
 *
 * Called before each EndDrawing() (grl_frame_sync_end_drawing()): ends
 * the one frame grl_native_window_resize() reports as resized, since
 * raylib's Android platform never clears that flag itself.
 */
void         grl_native_window_end_frame_private (void);

G_END_DECLS
