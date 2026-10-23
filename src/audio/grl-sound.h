/* grl-sound.h
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Sound effect type for short audio clips loaded entirely into memory.
 */

#pragma once

#if !defined(GRAYLIB_INSIDE) && !defined(GRAYLIB_COMPILATION)
#error "Only <graylib.h> can be included directly."
#endif

#include <glib-object.h>
#include "../grl-version.h"
#include "grl-audio-processor.h"

G_BEGIN_DECLS

/* Forward declaration for GrlWave */
typedef struct _GrlWave GrlWave;

#define GRL_TYPE_SOUND (grl_sound_get_type ())

GRL_AVAILABLE_IN_ALL
G_DECLARE_FINAL_TYPE (GrlSound, grl_sound, GRL, SOUND, GObject)

/*
 * Constructors
 */

/**
 * grl_sound_new_from_file:
 * @filename: Path to the audio file
 * @error: (nullable): Return location for error, or %NULL
 *
 * Loads a sound effect from a file. The entire audio file is loaded
 * into memory, making this suitable for short sound effects.
 *
 * Supported formats: WAV, OGG, MP3, FLAC (depending on raylib build)
 *
 * Returns: (transfer full) (nullable): A new #GrlSound, or %NULL on error
 */
GRL_AVAILABLE_IN_ALL
GrlSound *          grl_sound_new_from_file         (const gchar        *filename,
                                                     GError            **error);

/**
 * grl_sound_new_from_wave:
 * @data: (array length=data_size): Raw wave data
 * @data_size: Size of @data in bytes
 * @sample_rate: Sample rate in Hz
 * @sample_size: Bits per sample (8, 16, or 32)
 * @channels: Number of channels (1 for mono, 2 for stereo)
 *
 * Creates a sound from raw wave data.
 *
 * Returns: (transfer full) (nullable): A new #GrlSound, or %NULL on error
 */
GRL_AVAILABLE_IN_ALL
GrlSound *          grl_sound_new_from_wave         (const guint8       *data,
                                                     gsize               data_size,
                                                     guint               sample_rate,
                                                     guint               sample_size,
                                                     guint               channels);

/**
 * grl_sound_new_from_memory:
 * @file_type: File type extension (e.g., ".wav", ".ogg")
 * @data: (array length=data_size): Audio file data in memory
 * @data_size: Size of @data in bytes
 * @error: (nullable): Return location for error, or %NULL
 *
 * Loads a sound from a memory buffer containing an audio file.
 *
 * The @file_type parameter specifies the audio format. It should be
 * a file extension including the dot (e.g., ".wav", ".ogg", ".mp3").
 *
 * Returns: (transfer full) (nullable): A new #GrlSound, or %NULL on error
 */
GRL_AVAILABLE_IN_ALL
GrlSound *          grl_sound_new_from_memory       (const gchar        *file_type,
                                                     const guint8       *data,
                                                     gsize               data_size,
                                                     GError            **error);

/**
 * grl_sound_new_from_grl_wave:
 * @wave: A #GrlWave containing the audio data
 *
 * Creates a sound from a #GrlWave object. This allows you to load
 * and manipulate audio data with #GrlWave (cropping, resampling, etc.)
 * before converting it to a playable sound.
 *
 * Returns: (transfer full) (nullable): A new #GrlSound, or %NULL on error
 */
GRL_AVAILABLE_IN_ALL
GrlSound *          grl_sound_new_from_grl_wave     (GrlWave            *wave);

/* Forward declaration */
typedef struct _GrlResourcePack GrlResourcePack;

/**
 * grl_sound_new_from_resource:
 * @pack: A #GrlResourcePack
 * @resource_id: The resource ID to load
 * @file_type: (nullable): File type hint (e.g., ".wav", ".ogg"), or %NULL to auto-detect
 * @error: (nullable): Return location for error, or %NULL
 *
 * Loads a sound from a resource pack.
 *
 * If @file_type is %NULL, the function will attempt to auto-detect the
 * format from the resource data. Providing a hint improves reliability.
 *
 * Returns: (transfer full) (nullable): A new #GrlSound, or %NULL on error
 */
GRL_AVAILABLE_IN_ALL
GrlSound *          grl_sound_new_from_resource     (GrlResourcePack    *pack,
                                                     guint32             resource_id,
                                                     const gchar        *file_type,
                                                     GError            **error);

/**
 * grl_sound_new_alias:
 * @source: A #GrlSound to create an alias from
 *
 * Creates a sound alias that shares the same sample data as the source.
 * This is a lightweight way to have multiple sounds that use the same
 * audio data but can have independent volume, pitch, and pan settings.
 *
 * Aliases do not own the sample data - the source sound must remain
 * valid for as long as any aliases exist.
 *
 * Returns: (transfer full) (nullable): A new #GrlSound alias
 */
GRL_AVAILABLE_IN_ALL
GrlSound *          grl_sound_new_alias             (GrlSound           *source);

/*
 * Playback control
 */

/**
 * grl_sound_play:
 * @self: A #GrlSound
 *
 * Plays the sound effect. If the sound is already playing,
 * it will be restarted from the beginning.
 */
GRL_AVAILABLE_IN_ALL
void                grl_sound_play                  (GrlSound           *self);

/**
 * grl_sound_stop:
 * @self: A #GrlSound
 *
 * Stops the sound if it is currently playing.
 */
GRL_AVAILABLE_IN_ALL
void                grl_sound_stop                  (GrlSound           *self);

/**
 * grl_sound_pause:
 * @self: A #GrlSound
 *
 * Pauses the sound. Use grl_sound_resume() to continue playback.
 */
GRL_AVAILABLE_IN_ALL
void                grl_sound_pause                 (GrlSound           *self);

/**
 * grl_sound_resume:
 * @self: A #GrlSound
 *
 * Resumes a paused sound.
 */
GRL_AVAILABLE_IN_ALL
void                grl_sound_resume                (GrlSound           *self);

/**
 * grl_sound_is_playing:
 * @self: A #GrlSound
 *
 * Checks if the sound is currently playing.
 *
 * Returns: %TRUE if the sound is playing
 */
GRL_AVAILABLE_IN_ALL
gboolean            grl_sound_is_playing            (GrlSound           *self);

/**
 * grl_sound_is_alias:
 * @self: A #GrlSound
 *
 * Checks if this sound is an alias (created with grl_sound_new_alias()).
 *
 * Returns: %TRUE if this is an alias sound
 */
GRL_AVAILABLE_IN_ALL
gboolean            grl_sound_is_alias              (GrlSound           *self);

/**
 * grl_sound_update: (skip)
 * @self: A #GrlSound
 * @data: New sample data, @sample_count samples in the sound's format
 * @sample_count: Number of samples to update
 *
 * Updates the sound buffer with new sample data. This can be used
 * for real-time audio generation or streaming.
 *
 * The sample data format must match the format the sound was
 * originally created with.  Not available to language bindings: the
 * buffer's byte size depends on that format, which GObject Introspection
 * cannot describe.
 */
GRL_AVAILABLE_IN_ALL
void                grl_sound_update                (GrlSound           *self,
                                                     gconstpointer       data,
                                                     gint                sample_count);

/*
 * Volume and pitch
 */

/**
 * grl_sound_set_volume:
 * @self: A #GrlSound
 * @volume: Volume level (0.0 to 1.0)
 *
 * Sets the volume for this sound effect.
 */
GRL_AVAILABLE_IN_ALL
void                grl_sound_set_volume            (GrlSound           *self,
                                                     gfloat              volume);

/**
 * grl_sound_get_volume:
 * @self: A #GrlSound
 *
 * Gets the current volume level.
 *
 * Returns: The volume level (0.0 to 1.0)
 */
GRL_AVAILABLE_IN_ALL
gfloat              grl_sound_get_volume            (GrlSound           *self);

/**
 * grl_sound_set_pitch:
 * @self: A #GrlSound
 * @pitch: Pitch multiplier (1.0 = normal pitch)
 *
 * Sets the pitch for this sound effect.
 * Values less than 1.0 lower the pitch, greater than 1.0 raise it.
 */
GRL_AVAILABLE_IN_ALL
void                grl_sound_set_pitch             (GrlSound           *self,
                                                     gfloat              pitch);

/**
 * grl_sound_get_pitch:
 * @self: A #GrlSound
 *
 * Gets the current pitch multiplier.
 *
 * Returns: The pitch multiplier
 */
GRL_AVAILABLE_IN_ALL
gfloat              grl_sound_get_pitch             (GrlSound           *self);

/**
 * grl_sound_set_pan:
 * @self: A #GrlSound
 * @pan: Pan position (-1.0 = left, 0.0 = center, 1.0 = right)
 *
 * Sets the stereo pan position for this sound effect.
 */
GRL_AVAILABLE_IN_ALL
void                grl_sound_set_pan               (GrlSound           *self,
                                                     gfloat              pan);

/**
 * grl_sound_get_pan:
 * @self: A #GrlSound
 *
 * Gets the current pan position.
 *
 * Returns: The pan position (-1.0 to 1.0)
 */
GRL_AVAILABLE_IN_ALL
gfloat              grl_sound_get_pan               (GrlSound           *self);

/*
 * Multi-sound support
 */

/**
 * grl_sound_play_multi:
 * @self: A #GrlSound
 *
 * Plays the sound without stopping any currently playing instances.
 * This allows the same sound to overlap with itself.
 */
GRL_AVAILABLE_IN_ALL
void                grl_sound_play_multi            (GrlSound           *self);

/**
 * grl_sound_stop_multi:
 * @self: A #GrlSound
 *
 * Stops all playing instances of this sound (for multi-sounds).
 */
GRL_AVAILABLE_IN_ALL
void                grl_sound_stop_multi            (GrlSound           *self);

/**
 * grl_sound_get_sounds_playing:
 * @self: A #GrlSound
 *
 * Gets the number of currently playing instances of this sound.
 *
 * Returns: The number of playing instances
 */
GRL_AVAILABLE_IN_ALL
gint                grl_sound_get_sounds_playing    (GrlSound           *self);

/*
 * Per-voice processors
 */

/**
 * grl_sound_add_processor:
 * @self: A #GrlSound
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
 * through grl_sound_process().  While the audio device is closed it is not
 * attached to raylib either.
 *
 * Returns: the processor's id (never 0), or 0 when all
 *   %GRL_AUDIO_PROCESSOR_MAX_SLOTS slots are taken (@destroy has then
 *   already been called)
 */
GRL_AVAILABLE_IN_ALL
guint               grl_sound_add_processor (GrlSound           *self,
                                         GrlAudioProcessFunc  func,
                                         gpointer             user_data,
                                         GDestroyNotify       destroy);

/**
 * grl_sound_remove_processor:
 * @self: A #GrlSound
 * @id: an id grl_sound_add_processor() returned
 *
 * Removes one processor; once this returns it no longer runs.
 *
 * Returns: %TRUE when @id was one of this voice's processors
 */
GRL_AVAILABLE_IN_ALL
gboolean            grl_sound_remove_processor (GrlSound           *self,
                                         guint                id);

/**
 * grl_sound_clear_processors:
 * @self: A #GrlSound
 *
 * Removes every processor of this voice.
 */
GRL_AVAILABLE_IN_ALL
void                grl_sound_clear_processors (GrlSound           *self);

/**
 * grl_sound_get_n_processors:
 * @self: A #GrlSound
 *
 * Gets how many processors this voice has.
 *
 * Returns: the number of processors
 */
GRL_AVAILABLE_IN_ALL
guint               grl_sound_get_n_processors (GrlSound           *self);

/**
 * grl_sound_process: (skip)
 * @self: A #GrlSound
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
void                grl_sound_process          (GrlSound           *self,
                                         gfloat              *frames,
                                         guint                n_frames);

G_END_DECLS
