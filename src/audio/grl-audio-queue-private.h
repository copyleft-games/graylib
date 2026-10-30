/* grl-audio-queue-private.h
 *
 * Copyright 2026 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Internal: the audio command thread (grl-audio-queue.c).  Game-thread
 * calls on a GrlSound, a background GrlMusic or a callback GrlAudioStream
 * become commands on a lock-free list; one process-wide thread applies
 * them through raylib (and so waits on raylib's audio lock instead of the
 * game thread), refills background music and keeps a mirror of each
 * voice's playing state that the getters read.  Not installed and not
 * introspected.
 */

#pragma once

#include <glib.h>

G_BEGIN_DECLS

typedef struct _GrlAudioVoice      GrlAudioVoice;
typedef struct _GrlAudioVoiceClass GrlAudioVoiceClass;

/*
 * GrlAudioVoiceFunc:
 * What a command does to its voice, on the audio thread (or inline when
 * the thread is not running).  @value and @data are the command's.
 */
typedef void (*GrlAudioVoiceFunc) (GrlAudioVoice *voice,
                                   gfloat         value,
                                   gpointer       data);

/*
 * GrlAudioVoiceClass:
 * @refresh: on the audio thread: reads raylib's state of the voice (and
 *     refills it, for music), publishes it with
 *     grl_audio_voice_publish_playing(); returns whether it plays (a voice
 *     that plays stays on the thread's watch list)
 * @release: on the audio thread: stops and unloads the raylib resources,
 *     once, after every command pushed before it
 * @finalize: on any thread: frees what the voice owns after its last
 *     reference (never calls raylib)
 */
struct _GrlAudioVoiceClass
{
    gboolean (*refresh)  (GrlAudioVoice *voice);
    void     (*release)  (GrlAudioVoice *voice);
    void     (*finalize) (GrlAudioVoice *voice);
};

/*
 * GrlAudioVoice:
 * The part of a GrlSound / GrlMusic / GrlAudioStream that the audio
 * thread owns, embedded first in a per-type struct.  Commands and the
 * watch list hold references, so the object can go while commands for it
 * are still queued.  The fields are private to grl-audio-queue.c, except
 * resumable, which the owner's game-thread pause/resume set.
 */
struct _GrlAudioVoice
{
    const GrlAudioVoiceClass *klass;
    gint      ref_count;     /* atomic */

    /* game thread: what the last state command will leave */
    guint     requested;     /* serial of the last state command pushed */
    guint     generation;    /* queue generation it was pushed in, 0 inline */
    gboolean  predicted;     /* playing, as that command will leave it */
    gboolean  resumable;     /* playing when last paused (owner sets it) */

    /* published by the audio thread */
    guint     applied;       /* atomic: serial of the last state command applied */
    gint      playing;       /* atomic: raylib's playing state at the last refresh */

    /* audio thread only */
    gboolean  watched;
    gboolean  released;
};

/* sets up an embedded voice holding one reference (the owner's) */
void
grl_audio_voice_init (GrlAudioVoice            *voice,
                      const GrlAudioVoiceClass *klass);

GrlAudioVoice *
grl_audio_voice_ref (GrlAudioVoice *voice);

/* drops a reference; the last one runs klass->finalize and frees nothing
 * else (the per-type finalize frees the struct) */
void
grl_audio_voice_unref (GrlAudioVoice *voice);

/*
 * grl_audio_voice_push:
 * Queues @func (a setter: volume, pitch, pan...) for the audio thread.
 * @data is owned by the command and freed with @data_free after it ran.
 * Runs @func at once when the thread is not running.
 */
void
grl_audio_voice_push (GrlAudioVoice     *voice,
                      GrlAudioVoiceFunc  func,
                      gfloat             value,
                      gpointer           data,
                      GDestroyNotify     data_free);

/*
 * grl_audio_voice_push_state:
 * As grl_audio_voice_push() for a call that changes whether the voice
 * plays (play, stop, pause, resume, a refill switch): @predicted is what
 * grl_audio_voice_peek_playing() reports until the thread has applied it
 * and read raylib's state back.  @func may be NULL (refresh only).
 */
void
grl_audio_voice_push_state (GrlAudioVoice     *voice,
                            GrlAudioVoiceFunc  func,
                            gfloat             value,
                            gpointer           data,
                            GDestroyNotify     data_free,
                            gboolean           predicted);

/*
 * grl_audio_voice_unwatch:
 * Queues the voice's removal from the watch list (no more refreshes, for
 * a background music handed back to its owner).
 */
void
grl_audio_voice_unwatch (GrlAudioVoice *voice);

/*
 * grl_audio_voice_release:
 * Queues klass->release after everything queued for the voice so far and
 * takes over the caller's reference.  Inline when the thread is not
 * running.
 */
void
grl_audio_voice_release (GrlAudioVoice *voice);

/*
 * grl_audio_voice_peek_playing:
 * The mirror: TRUE with *@playing set while the thread runs and the
 * voice's state commands went through it; FALSE when the caller must ask
 * raylib (no thread, or nothing pushed for the voice since it started).
 */
gboolean
grl_audio_voice_peek_playing (GrlAudioVoice *voice,
                              gboolean      *playing);

/*
 * grl_audio_voice_is_settled:
 * Whether the last state command pushed for @voice has been applied
 * (always TRUE when the thread is not running).
 */
gboolean
grl_audio_voice_is_settled (GrlAudioVoice *voice);

/* for refresh: publishes raylib's playing state of the voice */
void
grl_audio_voice_publish_playing (GrlAudioVoice *voice,
                                 gboolean       playing);

/*
 * grl_audio_queue_start:
 * Starts the audio thread (GrlAudioDevice, once the device is open).
 * Voices pushed from now on go through it.  A no-op when running.
 */
void
grl_audio_queue_start (void);

/*
 * grl_audio_queue_stop:
 * Applies everything queued, empties the watch list and joins the
 * thread (GrlAudioDevice, before the device closes).  Later pushes run
 * inline.  A no-op when not running.
 */
void
grl_audio_queue_stop (void);

/* whether the audio thread runs (pushes are queued) */
gboolean
grl_audio_queue_is_running (void);

/*
 * grl_audio_queue_flush:
 * Waits until every command pushed before the call has been applied.  It
 * blocks (on the audio thread, which may wait on raylib's lock): for the
 * rare switches between queued and direct use of a voice, not per frame.
 */
void
grl_audio_queue_flush (void);

G_END_DECLS
