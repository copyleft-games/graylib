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

G_BEGIN_DECLS

#define GRL_TYPE_AUDIO_STREAM (grl_audio_stream_get_type ())

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
 * @data: (array length=frame_count): Audio sample data
 * @frame_count: Number of frames to write
 *
 * Updates the audio stream buffers with new sample data.
 * Call this periodically when grl_audio_stream_is_processed()
 * returns %TRUE to keep the audio playing smoothly.
 *
 * The data format must match the stream's sample_size and channels.
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
 * to provide more audio data.
 *
 * Returns: %TRUE if the buffer needs more data
 */
GRL_AVAILABLE_IN_ALL
gboolean            grl_audio_stream_is_processed       (GrlAudioStream     *self);

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

G_END_DECLS
