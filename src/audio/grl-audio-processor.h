/* grl-audio-processor.h
 *
 * Copyright 2026 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Per-voice audio processors: a callback with user data run on one
 * sound's, music stream's or audio stream's samples before raylib mixes
 * them into the device output.
 */

#pragma once

#if !defined(GRAYLIB_INSIDE) && !defined(GRAYLIB_COMPILATION)
#error "Only <graylib.h> can be included directly."
#endif

#include <glib.h>
#include "../grl-version.h"

G_BEGIN_DECLS

/**
 * GRL_AUDIO_PROCESSOR_CHANNELS:
 *
 * The channel count of every block a #GrlAudioProcessFunc receives:
 * raylib converts each voice to the device's mixing format (interleaved
 * 32-bit float stereo) before its processors run, whatever the voice's
 * own format.
 */
#define GRL_AUDIO_PROCESSOR_CHANNELS (2)

/**
 * GRL_AUDIO_PROCESSOR_MAX_SLOTS:
 *
 * How many processors can be attached at once across all voices.
 * raylib's per-stream processor is a plain C callback without user
 * data, so graylib hands each attached processor one of a fixed table
 * of trampolines; grl_sound_add_processor() and its siblings return 0
 * when every slot is taken.
 */
#define GRL_AUDIO_PROCESSOR_MAX_SLOTS (256)

/**
 * GrlAudioProcessFunc:
 * @frames: the voice's samples, @n_frames x @channels interleaved floats,
 *   rewritten in place
 * @n_frames: frames in the block
 * @channels: channels per frame (always %GRL_AUDIO_PROCESSOR_CHANNELS)
 * @user_data: the data given when the processor was added
 *
 * A processor of one voice's samples.  On a playing voice it runs on
 * the audio device's thread with raylib's audio lock held: it must not
 * block, must not call graylib's or raylib's audio functions, and must
 * not take a lock that a thread holds while calling them.
 *
 * The samples are the voice's before raylib applies its volume and pan
 * (raylib mixes them in after every processor ran), so a processor that
 * needs the voice's level reads grl_sound_get_volume() and
 * grl_sound_get_pan() (or the music's or stream's) itself.
 */
typedef void (*GrlAudioProcessFunc) (gfloat   *frames,
                                     guint     n_frames,
                                     guint     channels,
                                     gpointer  user_data);

/**
 * grl_audio_processor_get_n_free_slots:
 *
 * Gets how many processors can still be attached (of
 * %GRL_AUDIO_PROCESSOR_MAX_SLOTS).
 *
 * Returns: the free trampoline slots
 */
GRL_AVAILABLE_IN_ALL
guint               grl_audio_processor_get_n_free_slots (void);

G_END_DECLS
