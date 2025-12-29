/* grl-wave.h
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Audio wave data type for loading and manipulating audio samples.
 * Unlike GrlSound, GrlWave stores raw sample data that can be
 * manipulated before being loaded as a sound.
 */

#pragma once

#if !defined(GRAYLIB_INSIDE) && !defined(GRAYLIB_COMPILATION)
#error "Only <graylib.h> can be included directly."
#endif

#include <glib-object.h>
#include "../grl-version.h"

G_BEGIN_DECLS

/**
 * GrlWave:
 *
 * A GBoxed type representing raw audio wave data.
 *
 * #GrlWave stores audio sample data in memory and provides functions
 * for loading, manipulating, and exporting wave data. Unlike #GrlSound,
 * which is ready for playback, #GrlWave is meant for preprocessing
 * audio data before converting it to a sound.
 *
 * Common use cases:
 * - Loading audio files and manipulating them (crop, resample, format conversion)
 * - Generating procedural audio
 * - Exporting modified audio to files
 *
 * To play a wave, convert it to a #GrlSound using grl_sound_new_from_grl_wave().
 */
typedef struct _GrlWave GrlWave;

#define GRL_TYPE_WAVE (grl_wave_get_type ())

GRL_AVAILABLE_IN_ALL
GType           grl_wave_get_type           (void) G_GNUC_CONST;

/*
 * Constructors
 */

/**
 * grl_wave_new_from_file:
 * @filename: (type filename): Path to the audio file
 * @error: (nullable): Return location for error, or %NULL
 *
 * Loads wave data from an audio file.
 *
 * Supported formats: WAV, OGG, MP3, FLAC (depending on raylib build)
 *
 * Returns: (transfer full) (nullable): A new #GrlWave, or %NULL on error
 */
GRL_AVAILABLE_IN_ALL
GrlWave *       grl_wave_new_from_file      (const gchar    *filename,
                                             GError        **error);

/**
 * grl_wave_new_from_memory:
 * @file_type: File type extension (e.g., ".wav", ".ogg")
 * @data: (array length=data_size): Audio file data in memory
 * @data_size: Size of @data in bytes
 * @error: (nullable): Return location for error, or %NULL
 *
 * Loads wave data from a memory buffer containing an audio file.
 *
 * The @file_type parameter specifies the audio format. It should be
 * a file extension including the dot (e.g., ".wav", ".ogg", ".mp3").
 *
 * Returns: (transfer full) (nullable): A new #GrlWave, or %NULL on error
 */
GRL_AVAILABLE_IN_ALL
GrlWave *       grl_wave_new_from_memory    (const gchar    *file_type,
                                             const guint8   *data,
                                             gsize           data_size,
                                             GError        **error);

/**
 * grl_wave_new_from_samples:
 * @sample_rate: Sample rate in Hz (e.g., 44100)
 * @sample_size: Bits per sample (8, 16, or 32)
 * @channels: Number of channels (1 = mono, 2 = stereo)
 * @data: (array length=data_size): Raw sample data
 * @data_size: Size of @data in bytes
 *
 * Creates a wave from raw sample data.
 *
 * The data should be in the format specified by @sample_size:
 * - 8-bit: unsigned char (0-255, 128 = silence)
 * - 16-bit: signed short (-32768 to 32767)
 * - 32-bit: float (-1.0 to 1.0)
 *
 * Returns: (transfer full) (nullable): A new #GrlWave, or %NULL on error
 */
GRL_AVAILABLE_IN_ALL
GrlWave *       grl_wave_new_from_samples   (guint           sample_rate,
                                             guint           sample_size,
                                             guint           channels,
                                             const guint8   *data,
                                             gsize           data_size);

/**
 * grl_wave_copy:
 * @self: A #GrlWave
 *
 * Creates a deep copy of the wave.
 *
 * Returns: (transfer full): A new #GrlWave with copied data
 */
GRL_AVAILABLE_IN_ALL
GrlWave *       grl_wave_copy               (const GrlWave  *self);

/**
 * grl_wave_free:
 * @self: (nullable): A #GrlWave to free
 *
 * Frees the wave and its associated data.
 */
GRL_AVAILABLE_IN_ALL
void            grl_wave_free               (GrlWave        *self);

/*
 * Properties
 */

/**
 * grl_wave_get_frame_count:
 * @self: A #GrlWave
 *
 * Gets the total number of frames in the wave.
 *
 * A frame is one sample per channel. For stereo audio, each frame
 * contains a left and right sample.
 *
 * Returns: The frame count
 */
GRL_AVAILABLE_IN_ALL
guint           grl_wave_get_frame_count    (const GrlWave  *self);

/**
 * grl_wave_get_sample_rate:
 * @self: A #GrlWave
 *
 * Gets the sample rate (frequency) of the wave.
 *
 * Returns: The sample rate in Hz
 */
GRL_AVAILABLE_IN_ALL
guint           grl_wave_get_sample_rate    (const GrlWave  *self);

/**
 * grl_wave_get_sample_size:
 * @self: A #GrlWave
 *
 * Gets the bit depth of the wave samples.
 *
 * Returns: The bits per sample (8, 16, or 32)
 */
GRL_AVAILABLE_IN_ALL
guint           grl_wave_get_sample_size    (const GrlWave  *self);

/**
 * grl_wave_get_channels:
 * @self: A #GrlWave
 *
 * Gets the number of audio channels.
 *
 * Returns: The channel count (1 = mono, 2 = stereo)
 */
GRL_AVAILABLE_IN_ALL
guint           grl_wave_get_channels       (const GrlWave  *self);

/**
 * grl_wave_get_duration:
 * @self: A #GrlWave
 *
 * Gets the duration of the wave in seconds.
 *
 * Returns: The duration in seconds
 */
GRL_AVAILABLE_IN_ALL
gfloat          grl_wave_get_duration       (const GrlWave  *self);

/**
 * grl_wave_is_valid:
 * @self: A #GrlWave
 *
 * Checks if the wave data is valid.
 *
 * Returns: %TRUE if the wave is valid and ready to use
 */
GRL_AVAILABLE_IN_ALL
gboolean        grl_wave_is_valid           (const GrlWave  *self);

/*
 * Manipulation
 */

/**
 * grl_wave_crop:
 * @self: A #GrlWave
 * @init_frame: Starting frame (inclusive)
 * @final_frame: Ending frame (inclusive)
 *
 * Creates a new wave containing only the specified frame range.
 *
 * This is non-destructive - the original wave is not modified.
 *
 * Returns: (transfer full): A new cropped #GrlWave
 */
GRL_AVAILABLE_IN_ALL
GrlWave *       grl_wave_crop               (const GrlWave  *self,
                                             gint            init_frame,
                                             gint            final_frame);

/**
 * grl_wave_format:
 * @self: A #GrlWave
 * @sample_rate: New sample rate in Hz
 * @sample_size: New bits per sample (8, 16, or 32)
 * @channels: New channel count (1 or 2)
 *
 * Creates a new wave with converted format settings.
 *
 * This performs resampling and format conversion as needed.
 * This is non-destructive - the original wave is not modified.
 *
 * Returns: (transfer full): A new reformatted #GrlWave
 */
GRL_AVAILABLE_IN_ALL
GrlWave *       grl_wave_format             (const GrlWave  *self,
                                             guint           sample_rate,
                                             guint           sample_size,
                                             guint           channels);

/*
 * Sample Access
 */

/**
 * grl_wave_load_samples:
 * @self: A #GrlWave
 * @out_count: (out): Return location for the number of samples
 *
 * Loads all samples as normalized 32-bit floats.
 *
 * The returned array contains all samples across all channels,
 * interleaved for multi-channel audio. Values range from -1.0 to 1.0.
 *
 * The returned array should be freed with g_free() when no longer needed.
 *
 * Returns: (transfer full) (array length=out_count): Float sample array
 */
GRL_AVAILABLE_IN_ALL
gfloat *        grl_wave_load_samples       (const GrlWave  *self,
                                             gsize          *out_count);

/*
 * Export
 */

/**
 * grl_wave_export:
 * @self: A #GrlWave
 * @filename: (type filename): Output file path
 * @error: (nullable): Return location for error, or %NULL
 *
 * Exports the wave data to a WAV file.
 *
 * Returns: %TRUE on success, %FALSE on error
 */
GRL_AVAILABLE_IN_ALL
gboolean        grl_wave_export             (const GrlWave  *self,
                                             const gchar    *filename,
                                             GError        **error);

/**
 * grl_wave_export_as_code:
 * @self: A #GrlWave
 * @filename: (type filename): Output file path
 * @error: (nullable): Return location for error, or %NULL
 *
 * Exports the wave data as a C header file with the sample data
 * as an array. Useful for embedding audio in programs.
 *
 * Returns: %TRUE on success, %FALSE on error
 */
GRL_AVAILABLE_IN_ALL
gboolean        grl_wave_export_as_code     (const GrlWave  *self,
                                             const gchar    *filename,
                                             GError        **error);

G_DEFINE_AUTOPTR_CLEANUP_FUNC (GrlWave, grl_wave_free)

G_END_DECLS
