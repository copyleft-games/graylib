/* grl-music.h
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Streaming music type for long audio tracks.
 */

#pragma once

#if !defined(GRAYLIB_INSIDE) && !defined(GRAYLIB_COMPILATION)
#error "Only <graylib.h> can be included directly."
#endif

#include <glib-object.h>
#include "../grl-version.h"
#include "grl-audio-processor.h"

G_BEGIN_DECLS

#define GRL_TYPE_MUSIC (grl_music_get_type ())

GRL_AVAILABLE_IN_ALL
G_DECLARE_FINAL_TYPE (GrlMusic, grl_music, GRL, MUSIC, GObject)

/*
 * Constructors
 */

/**
 * grl_music_new_from_file:
 * @filename: Path to the music file
 * @error: (nullable): Return location for error, or %NULL
 *
 * Loads music from a file. The music is streamed from disk rather
 * than loaded entirely into memory, making this suitable for
 * background music and other long audio tracks.
 *
 * Supported formats: WAV, OGG, MP3, FLAC, XM, MOD (depending on raylib build)
 *
 * Returns: (transfer full) (nullable): A new #GrlMusic, or %NULL on error
 */
GRL_AVAILABLE_IN_ALL
GrlMusic *          grl_music_new_from_file         (const gchar        *filename,
                                                     GError            **error);

/**
 * grl_music_new_from_memory:
 * @file_type: Music file type extension (e.g., ".ogg", ".mp3", ".wav")
 * @data: (array length=data_size): Music file data in memory
 * @data_size: Size of @data in bytes
 * @error: (nullable): Return location for error, or %NULL
 *
 * Loads music from memory data. The data is copied internally
 * and will be streamed during playback.
 *
 * This is useful for loading music from embedded resources,
 * archives, or network downloads without writing to disk.
 *
 * Supported formats: WAV, OGG, MP3, FLAC (depending on raylib build)
 *
 * Returns: (transfer full) (nullable): A new #GrlMusic, or %NULL on error
 */
GRL_AVAILABLE_IN_ALL
GrlMusic *          grl_music_new_from_memory       (const gchar        *file_type,
                                                     const guint8       *data,
                                                     gsize               data_size,
                                                     GError            **error);

/*
 * Playback control
 */

/**
 * grl_music_play:
 * @self: A #GrlMusic
 *
 * Starts playing the music. If the music was paused, it resumes
 * from the current position.
 */
GRL_AVAILABLE_IN_ALL
void                grl_music_play                  (GrlMusic           *self);

/**
 * grl_music_stop:
 * @self: A #GrlMusic
 *
 * Stops the music and resets the playback position to the beginning.
 */
GRL_AVAILABLE_IN_ALL
void                grl_music_stop                  (GrlMusic           *self);

/**
 * grl_music_pause:
 * @self: A #GrlMusic
 *
 * Pauses the music at the current position.
 */
GRL_AVAILABLE_IN_ALL
void                grl_music_pause                 (GrlMusic           *self);

/**
 * grl_music_resume:
 * @self: A #GrlMusic
 *
 * Resumes paused music from the current position.
 */
GRL_AVAILABLE_IN_ALL
void                grl_music_resume                (GrlMusic           *self);

/**
 * grl_music_update:
 * @self: A #GrlMusic
 *
 * Updates the music stream buffers. This MUST be called in the main
 * loop while music is playing to ensure continuous playback, unless the
 * music streams in the background (grl_music_set_background_streaming()),
 * where it does nothing.
 */
GRL_AVAILABLE_IN_ALL
void                grl_music_update                (GrlMusic           *self);

/**
 * grl_music_set_background_streaming:
 * @self: A #GrlMusic
 * @enabled: %TRUE to refill the buffers on graylib's streaming thread
 *
 * Moves the decoding off the caller's thread.  A background-streamed
 * music is refilled by one process-wide streaming thread, started on
 * first use, which calls raylib's UpdateMusicStream() for every such
 * music between grl_music_play() and its end or grl_music_stop(), about
 * 200 times a second; grl_music_update() then does nothing.  Stopped or
 * finished music costs the thread nothing, and with none playing it
 * sleeps.  The thread stops when the audio device is closed.
 *
 * The control calls (play, stop, pause, resume, seek, looping) take the
 * thread's lock, so they never run while this music is being decoded
 * (they may wait for one decode).  grl_music_is_playing() and
 * grl_music_get_time_played() return what the thread last saw instead of
 * asking raylib, so they never wait: the position is at most one refill
 * period (about 5 ms) old, and a music that ended is reported stopped
 * within that period.
 *
 * Off by default.  Can be switched at any time; switching it off while
 * the music plays hands the refills back to grl_music_update().
 */
GRL_AVAILABLE_IN_ALL
void                grl_music_set_background_streaming (GrlMusic    *self,
                                                        gboolean     enabled);

/**
 * grl_music_get_background_streaming:
 * @self: A #GrlMusic
 *
 * Returns: %TRUE when the music is refilled on graylib's streaming thread
 */
GRL_AVAILABLE_IN_ALL
gboolean            grl_music_get_background_streaming (GrlMusic    *self);

/**
 * grl_music_is_playing:
 * @self: A #GrlMusic
 *
 * Checks if the music is currently playing.
 *
 * Returns: %TRUE if the music is playing
 */
GRL_AVAILABLE_IN_ALL
gboolean            grl_music_is_playing            (GrlMusic           *self);

/*
 * Looping
 */

/**
 * grl_music_set_looping:
 * @self: A #GrlMusic
 * @looping: Whether to loop the music
 *
 * Sets whether the music should loop when it reaches the end.
 */
GRL_AVAILABLE_IN_ALL
void                grl_music_set_looping           (GrlMusic           *self,
                                                     gboolean            looping);

/**
 * grl_music_get_looping:
 * @self: A #GrlMusic
 *
 * Gets whether the music is set to loop.
 *
 * Returns: %TRUE if the music loops
 */
GRL_AVAILABLE_IN_ALL
gboolean            grl_music_get_looping           (GrlMusic           *self);

/*
 * Seeking
 */

/**
 * grl_music_seek:
 * @self: A #GrlMusic
 * @position: Position in seconds
 *
 * Seeks to a specific position in the music.
 */
GRL_AVAILABLE_IN_ALL
void                grl_music_seek                  (GrlMusic           *self,
                                                     gfloat              position);

/**
 * grl_music_get_time_length:
 * @self: A #GrlMusic
 *
 * Gets the total length of the music in seconds.
 *
 * Returns: The music length in seconds
 */
GRL_AVAILABLE_IN_ALL
gfloat              grl_music_get_time_length       (GrlMusic           *self);

/**
 * grl_music_get_time_played:
 * @self: A #GrlMusic
 *
 * Gets the current playback position in seconds.
 *
 * Returns: The current position in seconds
 */
GRL_AVAILABLE_IN_ALL
gfloat              grl_music_get_time_played       (GrlMusic           *self);

/*
 * Volume and pitch
 */

/**
 * grl_music_set_volume:
 * @self: A #GrlMusic
 * @volume: Volume level (0.0 to 1.0)
 *
 * Sets the volume for this music track.
 */
GRL_AVAILABLE_IN_ALL
void                grl_music_set_volume            (GrlMusic           *self,
                                                     gfloat              volume);

/**
 * grl_music_get_volume:
 * @self: A #GrlMusic
 *
 * Gets the current volume level.
 *
 * Returns: The volume level (0.0 to 1.0)
 */
GRL_AVAILABLE_IN_ALL
gfloat              grl_music_get_volume            (GrlMusic           *self);

/**
 * grl_music_set_pitch:
 * @self: A #GrlMusic
 * @pitch: Pitch multiplier (1.0 = normal pitch)
 *
 * Sets the pitch for this music track.
 */
GRL_AVAILABLE_IN_ALL
void                grl_music_set_pitch             (GrlMusic           *self,
                                                     gfloat              pitch);

/**
 * grl_music_get_pitch:
 * @self: A #GrlMusic
 *
 * Gets the current pitch multiplier.
 *
 * Returns: The pitch multiplier
 */
GRL_AVAILABLE_IN_ALL
gfloat              grl_music_get_pitch             (GrlMusic           *self);

/**
 * grl_music_set_pan:
 * @self: A #GrlMusic
 * @pan: Pan position (-1.0 = left, 0.0 = center, 1.0 = right)
 *
 * Sets the stereo pan position for this music track.
 */
GRL_AVAILABLE_IN_ALL
void                grl_music_set_pan               (GrlMusic           *self,
                                                     gfloat              pan);

/**
 * grl_music_get_pan:
 * @self: A #GrlMusic
 *
 * Gets the current pan position.
 *
 * Returns: The pan position (-1.0 to 1.0)
 */
GRL_AVAILABLE_IN_ALL
gfloat              grl_music_get_pan               (GrlMusic           *self);

/*
 * Per-voice processors
 */

/**
 * grl_music_add_processor:
 * @self: A #GrlMusic
 * @func: (scope notified) (closure user_data) (destroy destroy): the processor
 * @user_data: data for @func
 * @destroy: (nullable): frees @user_data when the processor is removed
 *
 * Adds a processor of this voice's samples, run after the processors
 * added before it each time raylib mixes the voice (see
 * #GrlAudioProcessFunc for the thread and the format it runs in).  The
 * processor stays until it is removed, the processors are cleared or the
 * voice is disposed; @destroy then runs.
 *
 * On an object without loaded audio the processor is kept and runs only
 * through grl_music_process().  While the audio device is closed it is not
 * attached to raylib either.
 *
 * Returns: the processor's id (never 0), or 0 when all
 *   %GRL_AUDIO_PROCESSOR_MAX_SLOTS slots are taken (@destroy has then
 *   already been called)
 */
GRL_AVAILABLE_IN_ALL
guint               grl_music_add_processor (GrlMusic           *self,
                                         GrlAudioProcessFunc  func,
                                         gpointer             user_data,
                                         GDestroyNotify       destroy);

/**
 * grl_music_remove_processor:
 * @self: A #GrlMusic
 * @id: an id grl_music_add_processor() returned
 *
 * Removes one processor; once this returns it no longer runs.
 *
 * Returns: %TRUE when @id was one of this voice's processors
 */
GRL_AVAILABLE_IN_ALL
gboolean            grl_music_remove_processor (GrlMusic           *self,
                                         guint                id);

/**
 * grl_music_clear_processors:
 * @self: A #GrlMusic
 *
 * Removes every processor of this voice.
 */
GRL_AVAILABLE_IN_ALL
void                grl_music_clear_processors (GrlMusic           *self);

/**
 * grl_music_get_n_processors:
 * @self: A #GrlMusic
 *
 * Gets how many processors this voice has.
 *
 * Returns: the number of processors
 */
GRL_AVAILABLE_IN_ALL
guint               grl_music_get_n_processors (GrlMusic           *self);

/**
 * grl_music_process: (skip)
 * @self: A #GrlMusic
 * @frames: @n_frames x %GRL_AUDIO_PROCESSOR_CHANNELS interleaved floats
 * @n_frames: frames in @frames
 *
 * Runs this voice's processors over @frames in place, in order, as the
 * device does before mixing the voice.  For offline rendering and tests;
 * never call it on a playing voice's processors from another thread.
 * Not available to language bindings (the block's size is in frames,
 * not floats).
 */
GRL_AVAILABLE_IN_ALL
void                grl_music_process          (GrlMusic           *self,
                                         gfloat              *frames,
                                         guint                n_frames);

G_END_DECLS
