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
#include <stdbool.h>
#include "grl-audio-processor-private.h"
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
	/* Immutable canonical playback PCM; independent voices share this snapshot,
	 * never an opaque native buffer. Legacy aliases retain their root owner. */
	GBytes         *pcm;
	GrlSound       *alias_source;

    /* per-voice processors, made on first use */
    GrlAudioProcessorSet *processors;
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

/* Bound all metadata arithmetic before copying any PCM. Retention has a
 * documented budget; oversized legacy sounds remain playable, but cannot be
 * used as independent-voice sources. */
static gboolean
pcm_size (guint frames, guint sample_size, guint channels, gsize *size)
{
	gsize frame_bytes;

	if ((sample_size != 8 && sample_size != 16 && sample_size != 32) ||
	    channels == 0 || channels > G_MAXINT || frames == 0)
		return FALSE;
	if (channels > G_MAXSIZE / (sample_size / 8))
		return FALSE;
	frame_bytes = (gsize)channels * (sample_size / 8);
	if (frames > G_MAXSIZE / frame_bytes)
		return FALSE;
	*size = (gsize)frames * frame_bytes;
	return TRUE;
}

/* Use only public Wave conversion and Sound stream-format fields. The same
 * SDK conversion feeds the playback buffer and this immutable snapshot. */
static GBytes *
capture_pcm (Wave source, Sound sound)
{
	Wave copy;
	gsize input_size, output_size;

	if (source.data == NULL || source.sampleRate == 0 || source.sampleRate > G_MAXINT ||
	    sound.stream.sampleRate == 0 || sound.stream.sampleRate > G_MAXINT ||
	    !pcm_size (source.frameCount, source.sampleSize, source.channels, &input_size) ||
	    !pcm_size (sound.frameCount, sound.stream.sampleSize, sound.stream.channels, &output_size) ||
	    input_size > GRL_SOUND_VOICE_MAX_PCM_BYTES || output_size > GRL_SOUND_VOICE_MAX_PCM_BYTES)
		return NULL;
	copy = WaveCopy (source);
	if (copy.data == NULL)
		return NULL;
	WaveFormat (&copy, (gint)sound.stream.sampleRate,
	            (gint)sound.stream.sampleSize, (gint)sound.stream.channels);
	if (copy.data == NULL || copy.frameCount != sound.frameCount || copy.sampleRate != sound.stream.sampleRate ||
	    copy.sampleSize != sound.stream.sampleSize || copy.channels != sound.stream.channels)
	{
		UnloadWave (copy);
		return NULL;
	}
	return g_bytes_new_with_free_func (copy.data, output_size, MemFree, copy.data);
}

/* A single creation path keeps file/raw/decoded/resource ownership identical. */
static GrlSound *
sound_from_wave (Wave wave, GBytes *canonical)
{
	GrlSound *self;
	Sound sound;

	sound = LoadSoundFromWave (wave);
	if (sound.frameCount == 0)
	{
		if (sound.stream.buffer != NULL)
			UnloadSound (sound);
		return NULL;
	}
	self = g_object_new (GRL_TYPE_SOUND, NULL);
	self->sound = sound;
	self->is_loaded = TRUE;
	if (canonical != NULL && wave.frameCount == sound.frameCount &&
	    wave.sampleRate == sound.stream.sampleRate && wave.sampleSize == sound.stream.sampleSize &&
	    wave.channels == sound.stream.channels)
		self->pcm = g_bytes_ref (canonical);
	else
		self->pcm = capture_pcm (wave, sound);
	return self;
}

/*
 * The processors leave raylib's stream before the voice can be unloaded
 * (raylib does not free a stream's processors when it unloads it).
 */
static void
grl_sound_dispose (GObject *object)
{
    GrlSound *self = (GrlSound *)object;

    if (self->processors != NULL)
    {
		/* Stop this voice before detaching callbacks. The host keeps the device
		 * alive until its voices have been released, as for native unload. */
		if (self->is_loaded)
			StopSound (self->sound);
        grl_audio_processor_set_clear (self->processors, self->is_loaded ? &self->sound.stream : NULL, TRUE);
        g_clear_pointer (&self->processors, grl_audio_processor_set_free);
    }

    G_OBJECT_CLASS (grl_sound_parent_class)->dispose (object);
}

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
	g_clear_pointer (&self->pcm, g_bytes_unref);
	/* Native alias unload must precede releasing its retained sample owner. */
	g_clear_object (&self->alias_source);

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

    object_class->dispose = grl_sound_dispose;
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

/*
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
    g_autoptr(GrlWave) wave = NULL;

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

    wave = grl_wave_new_from_file (filename, error);
    if (wave == NULL)
        return NULL;
    self = grl_sound_new_from_grl_wave (wave);

    /* Check if sound loaded successfully */
    if (self == NULL)
    {
        g_set_error (error,
                     G_IO_ERROR,
                     G_IO_ERROR_FAILED,
                     "Failed to load sound: %s",
                     filename);
        return NULL;
    }

    self->filename = g_strdup (filename);

    return self;
}

/*
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
    Wave wave;
    gsize frame_bytes;

    g_return_val_if_fail (data != NULL, NULL);
    g_return_val_if_fail (data_size > 0, NULL);
	/* Reject invalid division/counts before the public SDK can consume them. */
	if (sample_rate == 0 || sample_rate > G_MAXINT || channels == 0 || channels > G_MAXINT ||
	    (sample_size != 8 && sample_size != 16 && sample_size != 32))
		return NULL;
	if (channels > G_MAXSIZE / (sample_size / 8))
		return NULL;
	frame_bytes = (gsize)channels * (sample_size / 8);
	if (data_size % frame_bytes != 0 || data_size / frame_bytes == 0 ||
	    data_size / frame_bytes > G_MAXUINT)
		return NULL;

    /* Create Wave structure */
    wave.frameCount = (guint)(data_size / frame_bytes);
    wave.sampleRate = sample_rate;
    wave.sampleSize = sample_size;
    wave.channels = channels;
    wave.data = (gpointer)data;
    return sound_from_wave (wave, NULL);
}

/*
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
	if (data_size > G_MAXINT)
	{
		g_set_error_literal (error, G_IO_ERROR, G_IO_ERROR_INVALID_DATA,
		                     "Encoded sound exceeds the SDK's signed byte-count range");
		return NULL;
	}

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

/*
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

/*
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
    Wave raylib_wave;

    g_return_val_if_fail (wave != NULL, NULL);

    /* Get the internal raylib Wave struct */
    raylib_wave = _grl_wave_get_raylib_wave (wave);

    return sound_from_wave (raylib_wave, NULL);
}

/*
 * grl_sound_new_alias:
 * @source: A #GrlSound to create an alias from
 *
 * Creates a sound alias that shares the same sample data as the source.
 * This is a lightweight way to have multiple sounds that use the same
 * audio data but can have independent volume, pitch, and pan settings.
 *
 * Aliases retain their shared root owner; callers may release the source
 * reference without invalidating the samples. Independent voices use the
 * separate grl_sound_new_voice() contract.
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
	/* Flatten ownership: aliases never form a source->alias reference cycle. */
	self->alias_source = g_object_ref (source->alias_source != NULL ? source->alias_source : source);

    return self;
}

/* Explicit independent policy: no LoadSoundAlias/opaque AudioBuffer access.
 * Source updates and lifetime are independent after this snapshot is taken. */
GrlSound *
grl_sound_new_voice (GrlSound *source, GError **error)
{
	GrlSound *owner;
	GrlSound *self;
	Wave wave;
	gsize size, expected;
	unsigned char ready;

	g_return_val_if_fail (GRL_IS_SOUND (source), NULL);
	g_return_val_if_fail (error == NULL || *error == NULL, NULL);
	owner = source->alias_source != NULL ? source->alias_source : source;
	if (!owner->is_loaded || owner->pcm == NULL)
	{
		g_set_error_literal (error, G_IO_ERROR, G_IO_ERROR_NOT_SUPPORTED,
		                     "Independent voice requires a retained PCM snapshot (maximum 256 MiB)");
		return NULL;
	}
	ready = IsAudioDeviceReady ();
	if (!ready)
	{
		g_set_error_literal (error, G_IO_ERROR, G_IO_ERROR_NOT_INITIALIZED, "Independent voice requires an audio device");
		return NULL;
	}
	wave.data = (gpointer)g_bytes_get_data (owner->pcm, &size);
	wave.frameCount = owner->sound.frameCount;
	wave.sampleRate = owner->sound.stream.sampleRate;
	wave.sampleSize = owner->sound.stream.sampleSize;
	wave.channels = owner->sound.stream.channels;
	if (!pcm_size (wave.frameCount, wave.sampleSize, wave.channels, &expected) || expected != size)
	{
		g_set_error_literal (error, G_IO_ERROR, G_IO_ERROR_INVALID_DATA, "Retained PCM metadata no longer matches its bytes");
		return NULL;
	}
	self = sound_from_wave (wave, owner->pcm);
	if (self == NULL)
		g_set_error_literal (error, G_IO_ERROR, G_IO_ERROR_FAILED, "Cannot allocate an independent sound voice");
	else if (self->pcm == NULL)
	{
		g_clear_object (&self);
		g_set_error_literal (error, G_IO_ERROR, G_IO_ERROR_NOT_SUPPORTED,
		                     "Converted independent voice exceeds the retained PCM contract");
	}
	return self;
}

/*
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

/*
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

/*
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

/*
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

/*
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

/*
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

/*
 * grl_sound_update:
 * @self: A #GrlSound
 * @data: Packed playback bytes, borrowed for this call
 * @sample_count: Number of complete playback frames to update
 *
 * Updates the sound buffer with new sample data. This can be used
 * for real-time audio generation or streaming.
 *
 * The format must match the converted playback buffer; see grl-sound.h.
 */
void
grl_sound_update (GrlSound      *self,
                  gconstpointer  data,
                  gint           sample_count)
{
	gsize size;

	g_return_if_fail (GRL_IS_SOUND (self));
	g_return_if_fail (data != NULL);
	g_return_if_fail (sample_count > 0);
	if (!self->is_loaded)
		return;
	g_return_if_fail ((guint)sample_count <= self->sound.frameCount);
	g_return_if_fail (pcm_size ((guint)sample_count, self->sound.stream.sampleSize, self->sound.stream.channels, &size));
	g_return_if_fail (grl_sound_update_checked (self, data, size, (guint)sample_count, NULL));
}

/* Validate every external count before native writes or snapshot copying.
 * Native aliases still share storage: updates through any alias replace the
 * root's retained PCM too; independent voices retain their own snapshots. */
gboolean
grl_sound_update_checked (GrlSound *self, const guint8 *data,
                           gsize data_size, guint frame_count, GError **error)
{
	GrlSound *owner;
	g_autoptr(GBytes) replacement = NULL;
	gsize size, total, retained_size;
	const guint8 *retained;
	guint8 *copy;
	unsigned char ready;

	g_return_val_if_fail (GRL_IS_SOUND (self), FALSE);
	g_return_val_if_fail (error == NULL || *error == NULL, FALSE);
	if (!self->is_loaded || data == NULL || frame_count > G_MAXINT ||
	    frame_count > self->sound.frameCount ||
	    !pcm_size (frame_count, self->sound.stream.sampleSize, self->sound.stream.channels, &size) || size != data_size)
	{
		g_set_error_literal (error, G_IO_ERROR, G_IO_ERROR_INVALID_ARGUMENT,
		                     "Sound update requires exact playback bytes within the allocated frame count");
		return FALSE;
	}
	ready = IsAudioDeviceReady ();
	if (!ready)
	{
		g_set_error_literal (error, G_IO_ERROR, G_IO_ERROR_NOT_INITIALIZED, "Sound update requires an audio device");
		return FALSE;
	}
	owner = self->alias_source != NULL ? self->alias_source : self;
	if (owner->pcm != NULL && pcm_size (owner->sound.frameCount, owner->sound.stream.sampleSize,
	                                  owner->sound.stream.channels, &total) && total <= GRL_SOUND_VOICE_MAX_PCM_BYTES)
	{
		retained = g_bytes_get_data (owner->pcm, &retained_size);
		if (retained_size == total && size <= total)
		{
			copy = g_memdup2 (retained, total);
			memcpy (copy, data, size);
			replacement = g_bytes_new_take (copy, total);
		}
	}
	UpdateSound (self->sound, data, (gint)frame_count);
	g_clear_pointer (&owner->pcm, g_bytes_unref);
	owner->pcm = g_steal_pointer (&replacement);
	return TRUE;
}

/*
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

/*
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

/*
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

/*
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

/*
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

/*
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

/*
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

/*
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

/*
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

/*
 * Per-voice processors (see grl-audio-processor.c): the set is made on
 * first use; an unloaded object keeps its processors without a raylib
 * stream, where only grl_sound_process() runs them.
 */
guint
grl_sound_add_processor (GrlSound            *self,
                         GrlAudioProcessFunc  func,
                         gpointer             user_data,
                         GDestroyNotify       destroy)
{
    g_return_val_if_fail (GRL_IS_SOUND (self), 0);
    g_return_val_if_fail (func != NULL, 0);

    if (self->processors == NULL)
        self->processors = grl_audio_processor_set_new ();
    return grl_audio_processor_set_add (self->processors, self->is_loaded ? &self->sound.stream : NULL,
                                        func, user_data, destroy);
}

gboolean
grl_sound_remove_processor (GrlSound *self,
                            guint     id)
{
    g_return_val_if_fail (GRL_IS_SOUND (self), FALSE);

    if (self->processors == NULL || id == 0)
        return FALSE;
    return grl_audio_processor_set_remove (self->processors, self->is_loaded ? &self->sound.stream : NULL, id);
}

void
grl_sound_clear_processors (GrlSound *self)
{
    g_return_if_fail (GRL_IS_SOUND (self));

    if (self->processors != NULL)
        grl_audio_processor_set_clear (self->processors, self->is_loaded ? &self->sound.stream : NULL, FALSE);
}

guint
grl_sound_get_n_processors (GrlSound *self)
{
    g_return_val_if_fail (GRL_IS_SOUND (self), 0);

    return grl_audio_processor_set_get_count (self->processors);
}

void
grl_sound_process (GrlSound *self,
                   gfloat   *frames,
                   guint     n_frames)
{
    g_return_if_fail (GRL_IS_SOUND (self));
    g_return_if_fail (frames != NULL || n_frames == 0);

    grl_audio_processor_set_run (self->processors, frames, n_frames);
}
