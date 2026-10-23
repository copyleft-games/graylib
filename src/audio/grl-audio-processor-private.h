/* grl-audio-processor-private.h
 *
 * Copyright 2026 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * The processor list a GrlSound, GrlMusic or GrlAudioStream keeps, and
 * the trampoline slots that carry each processor into raylib (see
 * grl-audio-processor.c).  Not installed and not introspected.
 */

#pragma once

#include <glib.h>
#include <raylib.h>

#include "grl-audio-processor.h"

G_BEGIN_DECLS

typedef struct _GrlAudioProcessorSet GrlAudioProcessorSet;

/* an empty set; the owner frees it with grl_audio_processor_set_free()
 * after grl_audio_processor_set_clear() */
GrlAudioProcessorSet *
grl_audio_processor_set_new (void);

/*
 * Adds a processor and attaches it to @stream (NULL: the voice holds no
 * raylib stream, e.g. an unloaded object; the processor then only runs
 * through grl_audio_processor_set_run()).  Returns its id, or 0 when no
 * trampoline slot is free (@destroy is then called on @user_data at
 * once, so ownership always passes).
 */
guint
grl_audio_processor_set_add (GrlAudioProcessorSet *set,
                             const AudioStream    *stream,
                             GrlAudioProcessFunc   func,
                             gpointer              user_data,
                             GDestroyNotify        destroy);

/* detaches and drops one processor (its destroy notify runs); FALSE for
 * an unknown id */
gboolean
grl_audio_processor_set_remove (GrlAudioProcessorSet *set,
                                const AudioStream    *stream,
                                guint                 id);

/*
 * Detaches and drops every processor.  @unloading: the owner unloads
 * @stream's buffer right after, so a slot whose detach raylib could not
 * take (the device already closed) can be reused at once.
 */
void
grl_audio_processor_set_clear (GrlAudioProcessorSet *set,
                               const AudioStream    *stream,
                               gboolean              unloading);

void
grl_audio_processor_set_free (GrlAudioProcessorSet *set);

guint
grl_audio_processor_set_get_count (GrlAudioProcessorSet *set);

/* runs every processor in order on @frames (n_frames x 2 floats) */
void
grl_audio_processor_set_run (GrlAudioProcessorSet *set,
                             gfloat               *frames,
                             guint                 n_frames);

G_END_DECLS
