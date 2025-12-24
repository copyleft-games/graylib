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

G_BEGIN_DECLS

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

G_END_DECLS
