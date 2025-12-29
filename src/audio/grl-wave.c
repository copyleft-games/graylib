/* grl-wave.c
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * GrlWave implementation - audio wave data manipulation.
 */

#include "config.h"
#include "grl-wave.h"
#include "grl-wave-private.h"
#include <gio/gio.h>
#include <raylib.h>
#include <string.h>

/**
 * SECTION:grl-wave
 * @title: GrlWave
 * @short_description: Raw audio wave data
 *
 * #GrlWave is a GBoxed type that wraps raylib's Wave structure,
 * providing functions for loading, manipulating, and exporting
 * audio wave data.
 *
 * Unlike #GrlSound which is designed for immediate playback,
 * #GrlWave is meant for preprocessing audio data - loading from
 * files, cropping, resampling, format conversion, and exporting.
 *
 * # Example: Loading and cropping a wave
 *
 * |[<!-- language="C" -->
 * g_autoptr(GError) error = NULL;
 * g_autoptr(GrlWave) wave = grl_wave_new_from_file ("sound.wav", &error);
 *
 * if (wave == NULL)
 * {
 *     g_printerr ("Error: %s\n", error->message);
 *     return;
 * }
 *
 * // Crop to first 2 seconds (assuming 44100 Hz)
 * g_autoptr(GrlWave) cropped = grl_wave_crop (wave, 0, 44100 * 2);
 *
 * // Convert to sound for playback
 * g_autoptr(GrlSound) sfx = grl_sound_new_from_grl_wave (cropped);
 * grl_sound_play (sfx);
 * ]|
 */

/*
 * GrlWave structure - wraps raylib Wave
 */
struct _GrlWave
{
    Wave wave;
};

G_DEFINE_BOXED_TYPE (GrlWave, grl_wave, grl_wave_copy, grl_wave_free)

/*
 * Helper to check if wave is valid
 */
static gboolean
wave_is_valid_internal (const Wave *wave)
{
    unsigned char raw;

    raw = IsWaveValid (*wave);
    return raw != 0;
}

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
GrlWave *
grl_wave_new_from_file (const gchar  *filename,
                        GError      **error)
{
    GrlWave *self;
    Wave wave;

    g_return_val_if_fail (filename != NULL, NULL);
    g_return_val_if_fail (error == NULL || *error == NULL, NULL);

    /* Check if file exists */
    if (!FileExists (filename))
    {
        g_set_error (error,
                     G_IO_ERROR,
                     G_IO_ERROR_NOT_FOUND,
                     "Wave file not found: %s",
                     filename);
        return NULL;
    }

    wave = LoadWave (filename);

    if (!wave_is_valid_internal (&wave))
    {
        g_set_error (error,
                     G_IO_ERROR,
                     G_IO_ERROR_FAILED,
                     "Failed to load wave: %s",
                     filename);
        return NULL;
    }

    self = g_new0 (GrlWave, 1);
    self->wave = wave;

    return self;
}

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
GrlWave *
grl_wave_new_from_memory (const gchar   *file_type,
                          const guint8  *data,
                          gsize          data_size,
                          GError       **error)
{
    GrlWave *self;
    Wave wave;

    g_return_val_if_fail (file_type != NULL, NULL);
    g_return_val_if_fail (data != NULL, NULL);
    g_return_val_if_fail (data_size > 0, NULL);
    g_return_val_if_fail (error == NULL || *error == NULL, NULL);

    wave = LoadWaveFromMemory (file_type, data, (int)data_size);

    if (!wave_is_valid_internal (&wave))
    {
        g_set_error (error,
                     G_IO_ERROR,
                     G_IO_ERROR_FAILED,
                     "Failed to load wave from memory (type: %s)",
                     file_type);
        return NULL;
    }

    self = g_new0 (GrlWave, 1);
    self->wave = wave;

    return self;
}

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
GrlWave *
grl_wave_new_from_samples (guint         sample_rate,
                           guint         sample_size,
                           guint         channels,
                           const guint8 *data,
                           gsize         data_size)
{
    GrlWave *self;
    gsize bytes_per_sample;
    guint frame_count;

    g_return_val_if_fail (sample_rate > 0, NULL);
    g_return_val_if_fail (sample_size == 8 || sample_size == 16 || sample_size == 32, NULL);
    g_return_val_if_fail (channels > 0, NULL);
    g_return_val_if_fail (data != NULL, NULL);
    g_return_val_if_fail (data_size > 0, NULL);

    bytes_per_sample = (sample_size / 8) * channels;
    frame_count = (guint)(data_size / bytes_per_sample);

    if (frame_count == 0)
        return NULL;

    self = g_new0 (GrlWave, 1);
    self->wave.frameCount = frame_count;
    self->wave.sampleRate = sample_rate;
    self->wave.sampleSize = sample_size;
    self->wave.channels = channels;
    self->wave.data = g_memdup2 (data, data_size);

    return self;
}

/**
 * grl_wave_copy:
 * @self: A #GrlWave
 *
 * Creates a deep copy of the wave.
 *
 * Returns: (transfer full): A new #GrlWave with copied data
 */
GrlWave *
grl_wave_copy (const GrlWave *self)
{
    GrlWave *copy;
    Wave wave_copy;

    g_return_val_if_fail (self != NULL, NULL);

    wave_copy = WaveCopy (self->wave);

    copy = g_new0 (GrlWave, 1);
    copy->wave = wave_copy;

    return copy;
}

/**
 * grl_wave_free:
 * @self: (nullable): A #GrlWave to free
 *
 * Frees the wave and its associated data.
 */
void
grl_wave_free (GrlWave *self)
{
    if (self == NULL)
        return;

    UnloadWave (self->wave);
    g_free (self);
}

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
guint
grl_wave_get_frame_count (const GrlWave *self)
{
    g_return_val_if_fail (self != NULL, 0);

    return self->wave.frameCount;
}

/**
 * grl_wave_get_sample_rate:
 * @self: A #GrlWave
 *
 * Gets the sample rate (frequency) of the wave.
 *
 * Returns: The sample rate in Hz
 */
guint
grl_wave_get_sample_rate (const GrlWave *self)
{
    g_return_val_if_fail (self != NULL, 0);

    return self->wave.sampleRate;
}

/**
 * grl_wave_get_sample_size:
 * @self: A #GrlWave
 *
 * Gets the bit depth of the wave samples.
 *
 * Returns: The bits per sample (8, 16, or 32)
 */
guint
grl_wave_get_sample_size (const GrlWave *self)
{
    g_return_val_if_fail (self != NULL, 0);

    return self->wave.sampleSize;
}

/**
 * grl_wave_get_channels:
 * @self: A #GrlWave
 *
 * Gets the number of audio channels.
 *
 * Returns: The channel count (1 = mono, 2 = stereo)
 */
guint
grl_wave_get_channels (const GrlWave *self)
{
    g_return_val_if_fail (self != NULL, 0);

    return self->wave.channels;
}

/**
 * grl_wave_get_duration:
 * @self: A #GrlWave
 *
 * Gets the duration of the wave in seconds.
 *
 * Returns: The duration in seconds
 */
gfloat
grl_wave_get_duration (const GrlWave *self)
{
    g_return_val_if_fail (self != NULL, 0.0f);

    if (self->wave.sampleRate == 0)
        return 0.0f;

    return (gfloat)self->wave.frameCount / (gfloat)self->wave.sampleRate;
}

/**
 * grl_wave_is_valid:
 * @self: A #GrlWave
 *
 * Checks if the wave data is valid.
 *
 * Returns: %TRUE if the wave is valid and ready to use
 */
gboolean
grl_wave_is_valid (const GrlWave *self)
{
    g_return_val_if_fail (self != NULL, FALSE);

    return wave_is_valid_internal (&self->wave);
}

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
GrlWave *
grl_wave_crop (const GrlWave *self,
               gint           init_frame,
               gint           final_frame)
{
    GrlWave *result;
    Wave wave_copy;

    g_return_val_if_fail (self != NULL, NULL);
    g_return_val_if_fail (init_frame >= 0, NULL);
    g_return_val_if_fail (final_frame >= init_frame, NULL);

    /* Make a copy to avoid mutating the original */
    wave_copy = WaveCopy (self->wave);

    /* WaveCrop mutates in place, so we operate on the copy */
    WaveCrop (&wave_copy, init_frame, final_frame);

    result = g_new0 (GrlWave, 1);
    result->wave = wave_copy;

    return result;
}

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
GrlWave *
grl_wave_format (const GrlWave *self,
                 guint          sample_rate,
                 guint          sample_size,
                 guint          channels)
{
    GrlWave *result;
    Wave wave_copy;

    g_return_val_if_fail (self != NULL, NULL);
    g_return_val_if_fail (sample_rate > 0, NULL);
    g_return_val_if_fail (sample_size == 8 || sample_size == 16 || sample_size == 32, NULL);
    g_return_val_if_fail (channels > 0, NULL);

    /* Make a copy to avoid mutating the original */
    wave_copy = WaveCopy (self->wave);

    /* WaveFormat mutates in place, so we operate on the copy */
    WaveFormat (&wave_copy, (int)sample_rate, (int)sample_size, (int)channels);

    result = g_new0 (GrlWave, 1);
    result->wave = wave_copy;

    return result;
}

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
gfloat *
grl_wave_load_samples (const GrlWave *self,
                       gsize         *out_count)
{
    gfloat *samples;
    gfloat *result;
    gsize count;

    g_return_val_if_fail (self != NULL, NULL);
    g_return_val_if_fail (out_count != NULL, NULL);

    samples = LoadWaveSamples (self->wave);
    if (samples == NULL)
    {
        *out_count = 0;
        return NULL;
    }

    count = (gsize)self->wave.frameCount * (gsize)self->wave.channels;
    *out_count = count;

    /* Copy to GLib-allocated memory so caller can use g_free() */
    result = g_memdup2 (samples, count * sizeof (gfloat));

    /* Free raylib's copy */
    UnloadWaveSamples (samples);

    return result;
}

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
gboolean
grl_wave_export (const GrlWave *self,
                 const gchar   *filename,
                 GError       **error)
{
    unsigned char raw;

    g_return_val_if_fail (self != NULL, FALSE);
    g_return_val_if_fail (filename != NULL, FALSE);
    g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

    raw = ExportWave (self->wave, filename);
    if (raw == 0)
    {
        g_set_error (error,
                     G_IO_ERROR,
                     G_IO_ERROR_FAILED,
                     "Failed to export wave to: %s",
                     filename);
        return FALSE;
    }

    return TRUE;
}

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
gboolean
grl_wave_export_as_code (const GrlWave *self,
                         const gchar   *filename,
                         GError       **error)
{
    unsigned char raw;

    g_return_val_if_fail (self != NULL, FALSE);
    g_return_val_if_fail (filename != NULL, FALSE);
    g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

    raw = ExportWaveAsCode (self->wave, filename);
    if (raw == 0)
    {
        g_set_error (error,
                     G_IO_ERROR,
                     G_IO_ERROR_FAILED,
                     "Failed to export wave as code to: %s",
                     filename);
        return FALSE;
    }

    return TRUE;
}

/*
 * =============================================================================
 * Private API (for use within audio module only)
 * =============================================================================
 */

/**
 * _grl_wave_get_raylib_wave:
 * @self: A #GrlWave
 *
 * Internal function to get the raylib Wave structure.
 * Used by GrlSound to convert GrlWave to Sound.
 *
 * Returns: Copy of the internal Wave struct (caller does NOT own data pointer)
 */
Wave
_grl_wave_get_raylib_wave (const GrlWave *self)
{
    Wave empty = { 0 };

    g_return_val_if_fail (self != NULL, empty);

    return self->wave;
}
