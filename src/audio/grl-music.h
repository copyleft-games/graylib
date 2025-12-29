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
 * loop while music is playing to ensure continuous playback.
 */
GRL_AVAILABLE_IN_ALL
void                grl_music_update                (GrlMusic           *self);

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

G_END_DECLS
