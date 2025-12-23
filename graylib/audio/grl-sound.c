/* grl-sound.c
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Sound effect implementation.
 */

#include "config.h"
#include "grl-sound.h"
#include <gio/gio.h>
#include <raylib.h>
#include <string.h>

/**
 * SECTION:grl-sound
 * @title: GrlSound
 * @short_description: Sound effect for short audio clips
 *
 * #GrlSound represents a sound effect that is loaded entirely into memory.
 * This makes it suitable for short sounds like explosions, button clicks,
 * or other effects that need to be played instantly without latency.
 *
 * For longer audio like background music, use #GrlMusic instead, which
 * streams the audio from disk.
 *
 * # Example
 *
 * |[<!-- language="C" -->
 * g_autoptr(GError) error = NULL;
 * g_autoptr(GrlSound) sfx = grl_sound_new_from_file ("shoot.wav", &error);
 *
 * if (sfx == NULL)
 * {
 *     g_printerr ("Error loading sound: %s\n", error->message);
 *     return;
 * }
 *
 * grl_sound_set_volume (sfx, 0.8f);
 * grl_sound_play (sfx);
 * ]|
 */

struct _GrlSound
{
    GObject         parent_instance;

    Sound           sound;
    gboolean        is_loaded;
    gfloat          volume;
    gfloat          pitch;
    gfloat          pan;
    gchar          *filename;
};

G_DEFINE_FINAL_TYPE (GrlSound, grl_sound, G_TYPE_OBJECT)

enum
{
    PROP_0,
    PROP_FILENAME,
    PROP_VOLUME,
    PROP_PITCH,
    PROP_PAN,
    PROP_IS_PLAYING,
    N_PROPS
};

static GParamSpec *properties[N_PROPS];

static void
grl_sound_finalize (GObject *object)
{
    GrlSound *self = GRL_SOUND (object);

    if (self->is_loaded)
    {
        UnloadSound (self->sound);
        self->is_loaded = FALSE;
    }

    g_clear_pointer (&self->filename, g_free);

    G_OBJECT_CLASS (grl_sound_parent_class)->finalize (object);
}

static void
grl_sound_get_property (GObject    *object,
                        guint       prop_id,
                        GValue     *value,
                        GParamSpec *pspec)
{
    GrlSound *self = GRL_SOUND (object);

    switch (prop_id)
    {
    case PROP_FILENAME:
        g_value_set_string (value, self->filename);
        break;

    case PROP_VOLUME:
        g_value_set_float (value, self->volume);
        break;

    case PROP_PITCH:
        g_value_set_float (value, self->pitch);
        break;

    case PROP_PAN:
        g_value_set_float (value, self->pan);
        break;

    case PROP_IS_PLAYING:
        g_value_set_boolean (value, grl_sound_is_playing (self));
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
grl_sound_set_property (GObject      *object,
                        guint         prop_id,
                        const GValue *value,
                        GParamSpec   *pspec)
{
    GrlSound *self = GRL_SOUND (object);

    switch (prop_id)
    {
    case PROP_VOLUME:
        grl_sound_set_volume (self, g_value_get_float (value));
        break;

    case PROP_PITCH:
        grl_sound_set_pitch (self, g_value_get_float (value));
        break;

    case PROP_PAN:
        grl_sound_set_pan (self, g_value_get_float (value));
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
grl_sound_class_init (GrlSoundClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->finalize = grl_sound_finalize;
    object_class->get_property = grl_sound_get_property;
    object_class->set_property = grl_sound_set_property;

    /**
     * GrlSound:filename:
     *
     * The filename this sound was loaded from, or %NULL if created
     * from raw data.
     */
    properties[PROP_FILENAME] =
        g_param_spec_string ("filename",
                             "Filename",
                             "The source filename",
                             NULL,
                             G_PARAM_READABLE |
                             G_PARAM_STATIC_STRINGS);

    /**
     * GrlSound:volume:
     *
     * The volume level (0.0 to 1.0).
     */
    properties[PROP_VOLUME] =
        g_param_spec_float ("volume",
                            "Volume",
                            "Volume level",
                            0.0f, 1.0f, 1.0f,
                            G_PARAM_READWRITE |
                            G_PARAM_STATIC_STRINGS |
                            G_PARAM_EXPLICIT_NOTIFY);

    /**
     * GrlSound:pitch:
     *
     * The pitch multiplier (1.0 = normal pitch).
     */
    properties[PROP_PITCH] =
        g_param_spec_float ("pitch",
                            "Pitch",
                            "Pitch multiplier",
                            0.1f, 10.0f, 1.0f,
                            G_PARAM_READWRITE |
                            G_PARAM_STATIC_STRINGS |
                            G_PARAM_EXPLICIT_NOTIFY);

    /**
     * GrlSound:pan:
     *
     * The stereo pan position (-1.0 = left, 0.0 = center, 1.0 = right).
     */
    properties[PROP_PAN] =
        g_param_spec_float ("pan",
                            "Pan",
                            "Stereo pan position",
                            -1.0f, 1.0f, 0.0f,
                            G_PARAM_READWRITE |
                            G_PARAM_STATIC_STRINGS |
                            G_PARAM_EXPLICIT_NOTIFY);

    /**
     * GrlSound:is-playing:
     *
     * Whether the sound is currently playing.
     */
    properties[PROP_IS_PLAYING] =
        g_param_spec_boolean ("is-playing",
                              "Is Playing",
                              "Whether the sound is playing",
                              FALSE,
                              G_PARAM_READABLE |
                              G_PARAM_STATIC_STRINGS);

    g_object_class_install_properties (object_class, N_PROPS, properties);
}

static void
grl_sound_init (GrlSound *self)
{
    self->is_loaded = FALSE;
    self->volume = 1.0f;
    self->pitch = 1.0f;
    self->pan = 0.0f;
    self->filename = NULL;
}

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
GrlSound *
grl_sound_new_from_file (const gchar  *filename,
                         GError      **error)
{
    GrlSound *self;
    Sound sound;

    g_return_val_if_fail (filename != NULL, NULL);
    g_return_val_if_fail (error == NULL || *error == NULL, NULL);

    /* Check if file exists */
    if (!FileExists (filename))
    {
        g_set_error (error,
                     G_IO_ERROR,
                     G_IO_ERROR_NOT_FOUND,
                     "Sound file not found: %s",
                     filename);
        return NULL;
    }

    sound = LoadSound (filename);

    /* Check if sound loaded successfully */
    if (sound.frameCount == 0)
    {
        g_set_error (error,
                     G_IO_ERROR,
                     G_IO_ERROR_FAILED,
                     "Failed to load sound: %s",
                     filename);
        return NULL;
    }

    self = g_object_new (GRL_TYPE_SOUND, NULL);
    self->sound = sound;
    self->is_loaded = TRUE;
    self->filename = g_strdup (filename);

    return self;
}

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
GrlSound *
grl_sound_new_from_wave (const guint8 *data,
                         gsize         data_size,
                         guint         sample_rate,
                         guint         sample_size,
                         guint         channels)
{
    GrlSound *self;
    Wave wave;
    Sound sound;

    g_return_val_if_fail (data != NULL, NULL);
    g_return_val_if_fail (data_size > 0, NULL);

    /* Create Wave structure */
    wave.frameCount = (unsigned int)(data_size / (sample_size / 8) / channels);
    wave.sampleRate = sample_rate;
    wave.sampleSize = sample_size;
    wave.channels = channels;
    wave.data = g_memdup2 (data, data_size);

    /* Load sound from wave */
    sound = LoadSoundFromWave (wave);

    /* Free the wave data copy (sound has its own copy) */
    g_free (wave.data);

    if (sound.frameCount == 0)
        return NULL;

    self = g_object_new (GRL_TYPE_SOUND, NULL);
    self->sound = sound;
    self->is_loaded = TRUE;

    return self;
}

/**
 * grl_sound_play:
 * @self: A #GrlSound
 *
 * Plays the sound effect. If the sound is already playing,
 * it will be restarted from the beginning.
 */
void
grl_sound_play (GrlSound *self)
{
    g_return_if_fail (GRL_IS_SOUND (self));

    if (!self->is_loaded)
        return;

    PlaySound (self->sound);
}

/**
 * grl_sound_stop:
 * @self: A #GrlSound
 *
 * Stops the sound if it is currently playing.
 */
void
grl_sound_stop (GrlSound *self)
{
    g_return_if_fail (GRL_IS_SOUND (self));

    if (!self->is_loaded)
        return;

    StopSound (self->sound);
}

/**
 * grl_sound_pause:
 * @self: A #GrlSound
 *
 * Pauses the sound. Use grl_sound_resume() to continue playback.
 */
void
grl_sound_pause (GrlSound *self)
{
    g_return_if_fail (GRL_IS_SOUND (self));

    if (!self->is_loaded)
        return;

    PauseSound (self->sound);
}

/**
 * grl_sound_resume:
 * @self: A #GrlSound
 *
 * Resumes a paused sound.
 */
void
grl_sound_resume (GrlSound *self)
{
    g_return_if_fail (GRL_IS_SOUND (self));

    if (!self->is_loaded)
        return;

    ResumeSound (self->sound);
}

/**
 * grl_sound_is_playing:
 * @self: A #GrlSound
 *
 * Checks if the sound is currently playing.
 *
 * Returns: %TRUE if the sound is playing
 */
gboolean
grl_sound_is_playing (GrlSound *self)
{
    g_return_val_if_fail (GRL_IS_SOUND (self), FALSE);

    if (!self->is_loaded)
        return FALSE;

    return IsSoundPlaying (self->sound);
}

/**
 * grl_sound_set_volume:
 * @self: A #GrlSound
 * @volume: Volume level (0.0 to 1.0)
 *
 * Sets the volume for this sound effect.
 */
void
grl_sound_set_volume (GrlSound *self,
                      gfloat    volume)
{
    g_return_if_fail (GRL_IS_SOUND (self));

    volume = CLAMP (volume, 0.0f, 1.0f);

    if (self->volume != volume)
    {
        self->volume = volume;

        if (self->is_loaded)
            SetSoundVolume (self->sound, volume);

        g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_VOLUME]);
    }
}

/**
 * grl_sound_get_volume:
 * @self: A #GrlSound
 *
 * Gets the current volume level.
 *
 * Returns: The volume level (0.0 to 1.0)
 */
gfloat
grl_sound_get_volume (GrlSound *self)
{
    g_return_val_if_fail (GRL_IS_SOUND (self), 1.0f);

    return self->volume;
}

/**
 * grl_sound_set_pitch:
 * @self: A #GrlSound
 * @pitch: Pitch multiplier (1.0 = normal pitch)
 *
 * Sets the pitch for this sound effect.
 * Values less than 1.0 lower the pitch, greater than 1.0 raise it.
 */
void
grl_sound_set_pitch (GrlSound *self,
                     gfloat    pitch)
{
    g_return_if_fail (GRL_IS_SOUND (self));

    pitch = CLAMP (pitch, 0.1f, 10.0f);

    if (self->pitch != pitch)
    {
        self->pitch = pitch;

        if (self->is_loaded)
            SetSoundPitch (self->sound, pitch);

        g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_PITCH]);
    }
}

/**
 * grl_sound_get_pitch:
 * @self: A #GrlSound
 *
 * Gets the current pitch multiplier.
 *
 * Returns: The pitch multiplier
 */
gfloat
grl_sound_get_pitch (GrlSound *self)
{
    g_return_val_if_fail (GRL_IS_SOUND (self), 1.0f);

    return self->pitch;
}

/**
 * grl_sound_set_pan:
 * @self: A #GrlSound
 * @pan: Pan position (-1.0 = left, 0.0 = center, 1.0 = right)
 *
 * Sets the stereo pan position for this sound effect.
 */
void
grl_sound_set_pan (GrlSound *self,
                   gfloat    pan)
{
    g_return_if_fail (GRL_IS_SOUND (self));

    pan = CLAMP (pan, -1.0f, 1.0f);

    if (self->pan != pan)
    {
        self->pan = pan;

        if (self->is_loaded)
            SetSoundPan (self->sound, (pan + 1.0f) / 2.0f); /* Convert to 0.0-1.0 */

        g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_PAN]);
    }
}

/**
 * grl_sound_get_pan:
 * @self: A #GrlSound
 *
 * Gets the current pan position.
 *
 * Returns: The pan position (-1.0 to 1.0)
 */
gfloat
grl_sound_get_pan (GrlSound *self)
{
    g_return_val_if_fail (GRL_IS_SOUND (self), 0.0f);

    return self->pan;
}

/**
 * grl_sound_play_multi:
 * @self: A #GrlSound
 *
 * Plays the sound without stopping any currently playing instances.
 * This allows the same sound to overlap with itself.
 */
void
grl_sound_play_multi (GrlSound *self)
{
    g_return_if_fail (GRL_IS_SOUND (self));

    if (!self->is_loaded)
        return;

    PlaySound (self->sound);
}

/**
 * grl_sound_stop_multi:
 * @self: A #GrlSound
 *
 * Stops all playing instances of this sound (for multi-sounds).
 */
void
grl_sound_stop_multi (GrlSound *self)
{
    g_return_if_fail (GRL_IS_SOUND (self));

    if (!self->is_loaded)
        return;

    StopSound (self->sound);
}

/**
 * grl_sound_get_sounds_playing:
 * @self: A #GrlSound
 *
 * Gets the number of currently playing instances of this sound.
 *
 * Returns: The number of playing instances
 */
gint
grl_sound_get_sounds_playing (GrlSound *self)
{
    g_return_val_if_fail (GRL_IS_SOUND (self), 0);

    if (!self->is_loaded)
        return 0;

    /* raylib doesn't track multi-sound instances, return 1 if playing */
    return IsSoundPlaying (self->sound) ? 1 : 0;
}
