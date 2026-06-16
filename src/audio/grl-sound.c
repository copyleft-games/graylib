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
#include "grl-wave.h"
#include "grl-wave-private.h"
#include "../resources/grl-resource-pack.h"
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
    gboolean        is_alias;
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
        /* Use appropriate unload function based on whether this is an alias */
        if (self->is_alias)
            UnloadSoundAlias (self->sound);
        else
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
    self->is_alias = FALSE;
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
GrlSound *
grl_sound_new_from_memory (const gchar  *file_type,
                           const guint8 *data,
                           gsize         data_size,
                           GError      **error)
{
    GrlSound *self;
    g_autoptr(GrlWave) wave = NULL;

    g_return_val_if_fail (file_type != NULL, NULL);
    g_return_val_if_fail (data != NULL, NULL);
    g_return_val_if_fail (data_size > 0, NULL);
    g_return_val_if_fail (error == NULL || *error == NULL, NULL);

    /* Load wave from memory buffer */
    wave = grl_wave_new_from_memory (file_type, data, data_size, error);
    if (wave == NULL)
        return NULL;

    /* Create sound from wave */
    self = grl_sound_new_from_grl_wave (wave);
    if (self == NULL)
    {
        g_set_error (error,
                     G_IO_ERROR,
                     G_IO_ERROR_FAILED,
                     "Failed to create sound from wave data");
        return NULL;
    }

    return self;
}

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
GrlSound *
grl_sound_new_from_resource (GrlResourcePack *pack,
                             guint32          resource_id,
                             const gchar     *file_type,
                             GError         **error)
{
    GrlSound *sound;
    guint8 *data;
    gsize size;
    const gchar *type_hint;

    g_return_val_if_fail (GRL_IS_RESOURCE_PACK (pack), NULL);
    g_return_val_if_fail (error == NULL || *error == NULL, NULL);

    /* Load raw data from resource pack */
    data = grl_resource_pack_load_raw (pack, resource_id, &size, error);
    if (data == NULL)
        return NULL;

    /* Use provided file type or try to detect from magic bytes */
    if (file_type != NULL)
    {
        type_hint = file_type;
    }
    else
    {
        /* Try to detect format from magic bytes */
        if (size >= 4 && memcmp (data, "RIFF", 4) == 0)
            type_hint = ".wav";
        else if (size >= 4 && memcmp (data, "OggS", 4) == 0)
            type_hint = ".ogg";
        else if (size >= 4 && memcmp (data, "fLaC", 4) == 0)
            type_hint = ".flac";
        else if (size >= 3 && data[0] == 0xFF && (data[1] & 0xE0) == 0xE0)
            type_hint = ".mp3"; /* MP3 frame sync */
        else if (size >= 4 && memcmp (data, "ID3", 3) == 0)
            type_hint = ".mp3"; /* MP3 with ID3 tag */
        else
            type_hint = ".wav"; /* Default fallback */
    }

    /* Load sound from memory */
    sound = grl_sound_new_from_memory (type_hint, data, size, error);
    g_free (data);

    return sound;
}

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
GrlSound *
grl_sound_new_from_grl_wave (GrlWave *wave)
{
    GrlSound *self;
    Wave raylib_wave;
    Sound sound;

    g_return_val_if_fail (wave != NULL, NULL);

    /* Get the internal raylib Wave struct */
    raylib_wave = _grl_wave_get_raylib_wave (wave);

    /* Load sound from wave (raylib makes its own copy of the data) */
    sound = LoadSoundFromWave (raylib_wave);

    if (sound.frameCount == 0)
        return NULL;

    self = g_object_new (GRL_TYPE_SOUND, NULL);
    self->sound = sound;
    self->is_loaded = TRUE;

    return self;
}

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
GrlSound *
grl_sound_new_alias (GrlSound *source)
{
    GrlSound *self;
    Sound alias;

    g_return_val_if_fail (GRL_IS_SOUND (source), NULL);
    g_return_val_if_fail (source->is_loaded, NULL);

    alias = LoadSoundAlias (source->sound);

    if (alias.frameCount == 0)
        return NULL;

    self = g_object_new (GRL_TYPE_SOUND, NULL);
    self->sound = alias;
    self->is_loaded = TRUE;
    self->is_alias = TRUE;

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
    unsigned char raw;

    g_return_val_if_fail (GRL_IS_SOUND (self), FALSE);

    if (!self->is_loaded)
        return FALSE;

    /* Fix bool/gboolean ABI mismatch - use unsigned char intermediate */
    raw = IsSoundPlaying (self->sound);
    return raw != 0;
}

/**
 * grl_sound_is_alias:
 * @self: A #GrlSound
 *
 * Checks if this sound is an alias (created with grl_sound_new_alias()).
 *
 * Returns: %TRUE if this is an alias sound
 */
gboolean
grl_sound_is_alias (GrlSound *self)
{
    g_return_val_if_fail (GRL_IS_SOUND (self), FALSE);

    return self->is_alias;
}

/**
 * grl_sound_update:
 * @self: A #GrlSound
 * @data: (array length=sample_count): New sample data
 * @sample_count: Number of samples to update
 *
 * Updates the sound buffer with new sample data. This can be used
 * for real-time audio generation or streaming.
 *
 * The sample data format must match the format the sound was
 * originally created with.
 */
void
grl_sound_update (GrlSound      *self,
                  gconstpointer  data,
                  gint           sample_count)
{
    g_return_if_fail (GRL_IS_SOUND (self));
    g_return_if_fail (data != NULL);
    g_return_if_fail (sample_count > 0);

    if (!self->is_loaded)
        return;

    UpdateSound (self->sound, data, sample_count);
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
            SetSoundPan (self->sound, pan); /* raylib 6.0: pan is -1.0..1.0 */

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
    unsigned char raw;

    g_return_val_if_fail (GRL_IS_SOUND (self), 0);

    if (!self->is_loaded)
        return 0;

    /* raylib doesn't track multi-sound instances, return 1 if playing */
    /* Fix bool/gboolean ABI mismatch - use unsigned char intermediate */
    raw = IsSoundPlaying (self->sound);
    return raw != 0 ? 1 : 0;
}
