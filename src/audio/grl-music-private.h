/* grl-music-private.h
 *
 * Copyright 2026 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Internal: the background music streaming thread (grl-music.c), which
 * must stop before raylib's audio device closes.  Not installed.
 */

#pragma once

#include <glib.h>

G_BEGIN_DECLS

/*
 * grl_music_streaming_shutdown:
 * Stops and joins the streaming thread and forgets the tracks it was
 * refilling (they stay loaded; playing one again restarts the thread).
 * Called by GrlAudioDevice before it closes the device.  A no-op when
 * the thread never started.
 */
G_GNUC_INTERNAL void
grl_music_streaming_shutdown (void);

G_END_DECLS
