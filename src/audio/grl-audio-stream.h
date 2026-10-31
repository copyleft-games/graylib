/* grl-audio-stream.h
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Audio stream for real-time audio generation and processing.
 */

#pragma once

#if !defined(GRAYLIB_INSIDE) && !defined(GRAYLIB_COMPILATION)
#error "Only <graylib.h> can be included directly."
#endif

#include <glib-object.h>
#include "../grl-version.h"
#include "grl-audio-processor.h"

G_BEGIN_DECLS

#define GRL_TYPE_AUDIO_STREAM (grl_audio_stream_get_type ())

/**
 * GRL_AUDIO_STREAM_MAX_CALLBACKS:
 *
 * How many audio streams can have a callback at the same time.
 */
#define GRL_AUDIO_STREAM_MAX_CALLBACKS (64)

GRL_AVAILABLE_IN_ALL
G_DECLARE_FINAL_TYPE (GrlAudioStream, grl_audio_stream, GRL, AUDIO_STREAM, GObject)

/*
 * Constructors
 */

/**
 * grl_audio_stream_new:
 * @sample_rate: Sample rate in Hz (e.g., 44100, 48000)
 * @sample_size: Bits per sample (8, 16, or 32)
 * @channels: Number of channels (1 for mono, 2 for stereo)
 *
 * Creates a new audio stream for real-time audio generation.
 * This is useful for procedural audio, synthesizers, or custom
 * audio processing.
 *
 * You must periodically call grl_audio_stream_update() to fill
 * the audio buffers with sample data.
 *
 * Returns: (transfer full) (nullable): A new #GrlAudioStream, or %NULL on error
 */
GRL_AVAILABLE_IN_ALL
GrlAudioStream *    grl_audio_stream_new                (guint               sample_rate,
                                                         guint               sample_size,
                                                         guint               channels);

/*
 * Playback control
 */

/**
 * grl_audio_stream_play:
 * @self: A #GrlAudioStream
 *
 * Starts playing the audio stream.
 */
GRL_AVAILABLE_IN_ALL
void                grl_audio_stream_play               (GrlAudioStream     *self);

/**
 * grl_audio_stream_stop:
 * @self: A #GrlAudioStream
 *
 * Stops the audio stream.
 */
GRL_AVAILABLE_IN_ALL
void                grl_audio_stream_stop               (GrlAudioStream     *self);

/**
 * grl_audio_stream_pause:
 * @self: A #GrlAudioStream
 *
 * Pauses the audio stream.
 */
GRL_AVAILABLE_IN_ALL
void                grl_audio_stream_pause              (GrlAudioStream     *self);

/**
 * grl_audio_stream_resume:
 * @self: A #GrlAudioStream
 *
 * Resumes a paused audio stream.
 */
GRL_AVAILABLE_IN_ALL
void                grl_audio_stream_resume             (GrlAudioStream     *self);

/**
 * grl_audio_stream_is_playing:
 * @self: A #GrlAudioStream
 *
 * Checks if the audio stream is currently playing.
 *
 * Returns: %TRUE if the stream is playing
 */
GRL_AVAILABLE_IN_ALL
gboolean            grl_audio_stream_is_playing         (GrlAudioStream     *self);

/**
 * grl_audio_stream_is_valid:
 * @self: A #GrlAudioStream
 *
 * Checks if the audio stream is valid (buffers initialized).
 *
 * Returns: %TRUE if the stream is valid
 */
GRL_AVAILABLE_IN_ALL
gboolean            grl_audio_stream_is_valid           (GrlAudioStream     *self);

/*
 * Buffer management
 */

/**
 * grl_audio_stream_update:
 * @self: A #GrlAudioStream
 * @data: (array zero-terminated=0) (element-type guint8): Packed audio bytes
 * @frame_count: Number of frames to write
 *
 * Updates the audio stream buffers with new sample data.
 * Call this periodically when grl_audio_stream_is_processed()
 * returns %TRUE to keep the audio playing smoothly.
 *
 * The caller retains @data, which is read only during this call. Its size
 * must be @frame_count * channels * (sample_size / 8) bytes in the stream's
 * configured format. @frame_count is not a byte-array length; bindings must
 * check the computed byte count before calling. No terminator is used.
 *
 * Does nothing while the stream has a callback
 * (grl_audio_stream_set_callback()): raylib then reads the callback
 * instead of the buffers.
 */
GRL_AVAILABLE_IN_ALL
void                grl_audio_stream_update             (GrlAudioStream     *self,
                                                         gconstpointer       data,
                                                         gint                frame_count);

/**
 * grl_audio_stream_is_processed:
 * @self: A #GrlAudioStream
 *
 * Checks if the audio stream buffers require refill.
 * When this returns %TRUE, call grl_audio_stream_update()
 * to provide more audio data.  Always %FALSE while the stream has a
 * callback, so a pump loop written for buffers does no work then.
 *
 * Returns: %TRUE if the buffer needs more data
 */
GRL_AVAILABLE_IN_ALL
gboolean            grl_audio_stream_is_processed       (GrlAudioStream     *self);

/*
 * Callback streaming
 */

/**
 * GrlAudioStreamCallback:
 * @buffer: (not nullable): the block to fill: @n_frames interleaved
 *   frames in the stream's own format (sample size and channels)
 * @n_frames: the frames to write
 * @user_data: (closure): the data given to grl_audio_stream_set_callback()
 *
 * Fills one block of a playing stream on the audio device's thread
 * (miniaudio's mixer thread), with raylib's audio lock held.  It must
 * write all @n_frames frames, must not block, must not call graylib's or
 * raylib's audio functions, and must not wait for a lock another thread
 * holds while calling them.  Blocks are small (at most 4096 bytes, so
 * 512 frames of 32-bit stereo) and come in bursts of one device period.
 */
typedef void (*GrlAudioStreamCallback) (gpointer buffer,
                                        guint    n_frames,
                                        gpointer user_data);

/**
 * grl_audio_stream_set_callback:
 * @self: A #GrlAudioStream
 * @callback: (nullable) (scope notified) (closure user_data) (destroy destroy):
 *   the function that fills the stream, or %NULL to remove the current one
 * @user_data: data for @callback
 * @destroy: (nullable): frees @user_data once @callback can no longer run
 *
 * Lets the audio device pull the stream's samples from @callback on its
 * own thread, instead of the game pushing them with
 * grl_audio_stream_update() (which, like grl_audio_stream_is_processed(),
 * then does nothing).  The callback runs only while the stream plays.
 *
 * Replacing or removing a callback detaches it first: when this returns,
 * the old callback is not running and never runs again, and its @destroy
 * has been called.  Disposing of the stream removes the callback the
 * same way.  Wraps raylib's SetAudioStreamCallback(), whose callback has
 * no user data, through a table of %GRL_AUDIO_STREAM_MAX_CALLBACKS
 * trampolines; when every one is taken this logs a warning, calls
 * @destroy at once and leaves the stream without a callback.
 */
GRL_AVAILABLE_IN_ALL
void                grl_audio_stream_set_callback       (GrlAudioStream         *self,
                                                         GrlAudioStreamCallback  callback,
                                                         gpointer                user_data,
                                                         GDestroyNotify          destroy);

/**
 * grl_audio_stream_has_callback:
 * @self: A #GrlAudioStream
 *
 * Returns: %TRUE while the stream is filled by a callback
 */
GRL_AVAILABLE_IN_ALL
gboolean            grl_audio_stream_has_callback       (GrlAudioStream     *self);

/*
 * Properties
 */

/**
 * grl_audio_stream_get_sample_rate:
 * @self: A #GrlAudioStream
 *
 * Gets the sample rate of the audio stream.
 *
 * Returns: The sample rate in Hz
 */
GRL_AVAILABLE_IN_ALL
guint               grl_audio_stream_get_sample_rate    (GrlAudioStream     *self);

/**
 * grl_audio_stream_get_sample_size:
 * @self: A #GrlAudioStream
 *
 * Gets the sample size (bit depth) of the audio stream.
 *
 * Returns: Bits per sample (8, 16, or 32)
 */
GRL_AVAILABLE_IN_ALL
guint               grl_audio_stream_get_sample_size    (GrlAudioStream     *self);

/**
 * grl_audio_stream_get_channels:
 * @self: A #GrlAudioStream
 *
 * Gets the number of channels in the audio stream.
 *
 * Returns: Number of channels (1 for mono, 2 for stereo)
 */
GRL_AVAILABLE_IN_ALL
guint               grl_audio_stream_get_channels       (GrlAudioStream     *self);

/*
 * Volume and pitch
 */

/**
 * grl_audio_stream_set_volume:
 * @self: A #GrlAudioStream
 * @volume: Volume level (0.0 to 1.0)
 *
 * Sets the volume for this audio stream.
 */
GRL_AVAILABLE_IN_ALL
void                grl_audio_stream_set_volume         (GrlAudioStream     *self,
                                                         gfloat              volume);

/**
 * grl_audio_stream_get_volume:
 * @self: A #GrlAudioStream
 *
 * Gets the current volume level.
 *
 * Returns: The volume level (0.0 to 1.0)
 */
GRL_AVAILABLE_IN_ALL
gfloat              grl_audio_stream_get_volume         (GrlAudioStream     *self);

/**
 * grl_audio_stream_set_pitch:
 * @self: A #GrlAudioStream
 * @pitch: Pitch multiplier (1.0 = normal pitch)
 *
 * Sets the pitch for this audio stream.
 */
GRL_AVAILABLE_IN_ALL
void                grl_audio_stream_set_pitch          (GrlAudioStream     *self,
                                                         gfloat              pitch);

/**
 * grl_audio_stream_get_pitch:
 * @self: A #GrlAudioStream
 *
 * Gets the current pitch multiplier.
 *
 * Returns: The pitch multiplier
 */
GRL_AVAILABLE_IN_ALL
gfloat              grl_audio_stream_get_pitch          (GrlAudioStream     *self);

/**
 * grl_audio_stream_set_pan:
 * @self: A #GrlAudioStream
 * @pan: Pan position (-1.0 = left, 0.0 = center, 1.0 = right)
 *
 * Sets the stereo pan position for this audio stream.
 */
GRL_AVAILABLE_IN_ALL
void                grl_audio_stream_set_pan            (GrlAudioStream     *self,
                                                         gfloat              pan);

/**
 * grl_audio_stream_get_pan:
 * @self: A #GrlAudioStream
 *
 * Gets the current pan position.
 *
 * Returns: The pan position (-1.0 to 1.0)
 */
GRL_AVAILABLE_IN_ALL
gfloat              grl_audio_stream_get_pan            (GrlAudioStream     *self);

/*
 * Global settings
 */

/**
 * grl_audio_stream_set_default_buffer_size:
 * @size: Default buffer size for new audio streams
 *
 * Sets the default buffer size for new audio streams.
 * This affects all subsequently created streams.
 */
GRL_AVAILABLE_IN_ALL
void                grl_audio_stream_set_default_buffer_size (gint          size);

/*
 * Per-voice processors
 */

/**
 * grl_audio_stream_add_processor:
 * @self: A #GrlAudioStream
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
 * through grl_audio_stream_process().  While the audio device is closed it is not
 * attached to raylib either.
 *
 * Returns: the processor's id (never 0), or 0 when all
 *   %GRL_AUDIO_PROCESSOR_MAX_SLOTS slots are taken (@destroy has then
 *   already been called)
 */
GRL_AVAILABLE_IN_ALL
guint               grl_audio_stream_add_processor (GrlAudioStream     *self,
                                         GrlAudioProcessFunc  func,
                                         gpointer             user_data,
                                         GDestroyNotify       destroy);

/**
 * grl_audio_stream_remove_processor:
 * @self: A #GrlAudioStream
 * @id: an id grl_audio_stream_add_processor() returned
 *
 * Removes one processor; once this returns it no longer runs.
 *
 * Returns: %TRUE when @id was one of this voice's processors
 */
GRL_AVAILABLE_IN_ALL
gboolean            grl_audio_stream_remove_processor (GrlAudioStream     *self,
                                         guint                id);

/**
 * grl_audio_stream_clear_processors:
 * @self: A #GrlAudioStream
 *
 * Removes every processor of this voice.
 */
GRL_AVAILABLE_IN_ALL
void                grl_audio_stream_clear_processors (GrlAudioStream     *self);

/**
 * grl_audio_stream_get_n_processors:
 * @self: A #GrlAudioStream
 *
 * Gets how many processors this voice has.
 *
 * Returns: the number of processors
 */
GRL_AVAILABLE_IN_ALL
guint               grl_audio_stream_get_n_processors (GrlAudioStream     *self);

/**
 * grl_audio_stream_process: (skip)
 * @self: A #GrlAudioStream
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
void                grl_audio_stream_process          (GrlAudioStream     *self,
                                         gfloat              *frames,
                                         guint                n_frames);

G_END_DECLS
