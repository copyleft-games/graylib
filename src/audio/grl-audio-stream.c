/* grl-audio-stream.c
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Audio stream implementation for real-time audio generation.
 */

#include "config.h"
#include "grl-audio-stream.h"
#include <raylib.h>

/**
 * SECTION:grl-audio-stream
 * @title: GrlAudioStream
 * @short_description: Real-time audio generation and streaming
 *
 * #GrlAudioStream provides low-level access to audio streaming for
 * real-time audio generation, procedural audio, synthesizers, or
 * custom audio processing.
 *
 * Unlike #GrlSound (which loads audio into memory) or #GrlMusic
 * (which streams from a file), #GrlAudioStream expects you to
 * generate or provide the audio data in real-time.
 *
 * # Example
 *
 * |[<!-- language="C" -->
 * #define SAMPLE_RATE 44100
 * #define SAMPLE_SIZE 16
 * #define CHANNELS 1
 *
 * g_autoptr(GrlAudioStream) stream = grl_audio_stream_new (SAMPLE_RATE, SAMPLE_SIZE, CHANNELS);
 * grl_audio_stream_play (stream);
 *
 * /<!-- -->* In game loop *<!-- -->/
 * while (!should_quit)
 * {
 *     if (grl_audio_stream_is_processed (stream))
 *     {
 *         /<!-- -->* Generate audio samples *<!-- -->/
 *         gint16 samples[1024];
 *         generate_sine_wave (samples, 1024, 440.0f);
 *         grl_audio_stream_update (stream, samples, 1024);
 *     }
 * }
 * ]|
 */

struct _GrlAudioStream
{
    GObject         parent_instance;

    AudioStream     stream;
    gboolean        is_loaded;
    gfloat          volume;
    gfloat          pitch;
    gfloat          pan;
    guint           sample_rate;
    guint           sample_size;
    guint           channels;
};

G_DEFINE_FINAL_TYPE (GrlAudioStream, grl_audio_stream, G_TYPE_OBJECT)

enum
{
    PROP_0,
    PROP_SAMPLE_RATE,
    PROP_SAMPLE_SIZE,
    PROP_CHANNELS,
    PROP_VOLUME,
    PROP_PITCH,
    PROP_PAN,
    PROP_IS_PLAYING,
    N_PROPS
};

static GParamSpec *properties[N_PROPS];

static void
grl_audio_stream_finalize (GObject *object)
{
    GrlAudioStream *self = GRL_AUDIO_STREAM (object);

    if (self->is_loaded)
    {
        StopAudioStream (self->stream);
        UnloadAudioStream (self->stream);
        self->is_loaded = FALSE;
    }

    G_OBJECT_CLASS (grl_audio_stream_parent_class)->finalize (object);
}

static void
grl_audio_stream_get_property (GObject    *object,
                               guint       prop_id,
                               GValue     *value,
                               GParamSpec *pspec)
{
    GrlAudioStream *self = GRL_AUDIO_STREAM (object);

    switch (prop_id)
    {
    case PROP_SAMPLE_RATE:
        g_value_set_uint (value, self->sample_rate);
        break;

    case PROP_SAMPLE_SIZE:
        g_value_set_uint (value, self->sample_size);
        break;

    case PROP_CHANNELS:
        g_value_set_uint (value, self->channels);
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
        g_value_set_boolean (value, grl_audio_stream_is_playing (self));
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
grl_audio_stream_set_property (GObject      *object,
                               guint         prop_id,
                               const GValue *value,
                               GParamSpec   *pspec)
{
    GrlAudioStream *self = GRL_AUDIO_STREAM (object);

    switch (prop_id)
    {
    case PROP_VOLUME:
        grl_audio_stream_set_volume (self, g_value_get_float (value));
        break;

    case PROP_PITCH:
        grl_audio_stream_set_pitch (self, g_value_get_float (value));
        break;

    case PROP_PAN:
        grl_audio_stream_set_pan (self, g_value_get_float (value));
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
grl_audio_stream_class_init (GrlAudioStreamClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->finalize = grl_audio_stream_finalize;
    object_class->get_property = grl_audio_stream_get_property;
    object_class->set_property = grl_audio_stream_set_property;

    /**
     * GrlAudioStream:sample-rate:
     *
     * The sample rate in Hz.
     */
    properties[PROP_SAMPLE_RATE] =
        g_param_spec_uint ("sample-rate",
                           "Sample Rate",
                           "Sample rate in Hz",
                           1, 192000, 44100,
                           G_PARAM_READABLE |
                           G_PARAM_STATIC_STRINGS);

    /**
     * GrlAudioStream:sample-size:
     *
     * Bits per sample (8, 16, or 32).
     */
    properties[PROP_SAMPLE_SIZE] =
        g_param_spec_uint ("sample-size",
                           "Sample Size",
                           "Bits per sample",
                           8, 32, 16,
                           G_PARAM_READABLE |
                           G_PARAM_STATIC_STRINGS);

    /**
     * GrlAudioStream:channels:
     *
     * Number of channels (1 for mono, 2 for stereo).
     */
    properties[PROP_CHANNELS] =
        g_param_spec_uint ("channels",
                           "Channels",
                           "Number of audio channels",
                           1, 8, 2,
                           G_PARAM_READABLE |
                           G_PARAM_STATIC_STRINGS);

    /**
     * GrlAudioStream:volume:
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
     * GrlAudioStream:pitch:
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
     * GrlAudioStream:pan:
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
     * GrlAudioStream:is-playing:
     *
     * Whether the audio stream is currently playing.
     */
    properties[PROP_IS_PLAYING] =
        g_param_spec_boolean ("is-playing",
                              "Is Playing",
                              "Whether the stream is playing",
                              FALSE,
                              G_PARAM_READABLE |
                              G_PARAM_STATIC_STRINGS);

    g_object_class_install_properties (object_class, N_PROPS, properties);
}

static void
grl_audio_stream_init (GrlAudioStream *self)
{
    self->is_loaded = FALSE;
    self->volume = 1.0f;
    self->pitch = 1.0f;
    self->pan = 0.0f;
    self->sample_rate = 0;
    self->sample_size = 0;
    self->channels = 0;
}

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
GrlAudioStream *
grl_audio_stream_new (guint sample_rate,
                      guint sample_size,
                      guint channels)
{
    GrlAudioStream *self;
    AudioStream stream;
    unsigned char raw;

    g_return_val_if_fail (sample_rate > 0, NULL);
    g_return_val_if_fail (sample_size == 8 || sample_size == 16 || sample_size == 32, NULL);
    g_return_val_if_fail (channels > 0 && channels <= 8, NULL);

    stream = LoadAudioStream (sample_rate, sample_size, channels);

    /* Check if stream is valid */
    raw = IsAudioStreamValid (stream);
    if (raw == 0)
    {
        return NULL;
    }

    self = g_object_new (GRL_TYPE_AUDIO_STREAM, NULL);
    self->stream = stream;
    self->is_loaded = TRUE;
    self->sample_rate = sample_rate;
    self->sample_size = sample_size;
    self->channels = channels;

    return self;
}

/**
 * grl_audio_stream_play:
 * @self: A #GrlAudioStream
 *
 * Starts playing the audio stream.
 */
void
grl_audio_stream_play (GrlAudioStream *self)
{
    g_return_if_fail (GRL_IS_AUDIO_STREAM (self));

    if (!self->is_loaded)
        return;

    PlayAudioStream (self->stream);
}

/**
 * grl_audio_stream_stop:
 * @self: A #GrlAudioStream
 *
 * Stops the audio stream.
 */
void
grl_audio_stream_stop (GrlAudioStream *self)
{
    g_return_if_fail (GRL_IS_AUDIO_STREAM (self));

    if (!self->is_loaded)
        return;

    StopAudioStream (self->stream);
}

/**
 * grl_audio_stream_pause:
 * @self: A #GrlAudioStream
 *
 * Pauses the audio stream.
 */
void
grl_audio_stream_pause (GrlAudioStream *self)
{
    g_return_if_fail (GRL_IS_AUDIO_STREAM (self));

    if (!self->is_loaded)
        return;

    PauseAudioStream (self->stream);
}

/**
 * grl_audio_stream_resume:
 * @self: A #GrlAudioStream
 *
 * Resumes a paused audio stream.
 */
void
grl_audio_stream_resume (GrlAudioStream *self)
{
    g_return_if_fail (GRL_IS_AUDIO_STREAM (self));

    if (!self->is_loaded)
        return;

    ResumeAudioStream (self->stream);
}

/**
 * grl_audio_stream_is_playing:
 * @self: A #GrlAudioStream
 *
 * Checks if the audio stream is currently playing.
 *
 * Returns: %TRUE if the stream is playing
 */
gboolean
grl_audio_stream_is_playing (GrlAudioStream *self)
{
    unsigned char raw;

    g_return_val_if_fail (GRL_IS_AUDIO_STREAM (self), FALSE);

    if (!self->is_loaded)
        return FALSE;

    /* Fix bool/gboolean ABI mismatch - use unsigned char intermediate */
    raw = IsAudioStreamPlaying (self->stream);
    return raw != 0;
}

/**
 * grl_audio_stream_is_valid:
 * @self: A #GrlAudioStream
 *
 * Checks if the audio stream is valid (buffers initialized).
 *
 * Returns: %TRUE if the stream is valid
 */
gboolean
grl_audio_stream_is_valid (GrlAudioStream *self)
{
    unsigned char raw;

    g_return_val_if_fail (GRL_IS_AUDIO_STREAM (self), FALSE);

    if (!self->is_loaded)
        return FALSE;

    /* Fix bool/gboolean ABI mismatch - use unsigned char intermediate */
    raw = IsAudioStreamValid (self->stream);
    return raw != 0;
}

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
void
grl_audio_stream_update (GrlAudioStream *self,
                         gconstpointer   data,
                         gint            frame_count)
{
    g_return_if_fail (GRL_IS_AUDIO_STREAM (self));
    g_return_if_fail (data != NULL);
    g_return_if_fail (frame_count > 0);

    if (!self->is_loaded)
        return;

    UpdateAudioStream (self->stream, data, frame_count);
}

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
gboolean
grl_audio_stream_is_processed (GrlAudioStream *self)
{
    unsigned char raw;

    g_return_val_if_fail (GRL_IS_AUDIO_STREAM (self), FALSE);

    if (!self->is_loaded)
        return FALSE;

    /* Fix bool/gboolean ABI mismatch - use unsigned char intermediate */
    raw = IsAudioStreamProcessed (self->stream);
    return raw != 0;
}

/**
 * grl_audio_stream_get_sample_rate:
 * @self: A #GrlAudioStream
 *
 * Gets the sample rate of the audio stream.
 *
 * Returns: The sample rate in Hz
 */
guint
grl_audio_stream_get_sample_rate (GrlAudioStream *self)
{
    g_return_val_if_fail (GRL_IS_AUDIO_STREAM (self), 0);

    return self->sample_rate;
}

/**
 * grl_audio_stream_get_sample_size:
 * @self: A #GrlAudioStream
 *
 * Gets the sample size (bit depth) of the audio stream.
 *
 * Returns: Bits per sample (8, 16, or 32)
 */
guint
grl_audio_stream_get_sample_size (GrlAudioStream *self)
{
    g_return_val_if_fail (GRL_IS_AUDIO_STREAM (self), 0);

    return self->sample_size;
}

/**
 * grl_audio_stream_get_channels:
 * @self: A #GrlAudioStream
 *
 * Gets the number of channels in the audio stream.
 *
 * Returns: Number of channels (1 for mono, 2 for stereo)
 */
guint
grl_audio_stream_get_channels (GrlAudioStream *self)
{
    g_return_val_if_fail (GRL_IS_AUDIO_STREAM (self), 0);

    return self->channels;
}

/**
 * grl_audio_stream_set_volume:
 * @self: A #GrlAudioStream
 * @volume: Volume level (0.0 to 1.0)
 *
 * Sets the volume for this audio stream.
 */
void
grl_audio_stream_set_volume (GrlAudioStream *self,
                             gfloat          volume)
{
    g_return_if_fail (GRL_IS_AUDIO_STREAM (self));

    volume = CLAMP (volume, 0.0f, 1.0f);

    if (self->volume != volume)
    {
        self->volume = volume;

        if (self->is_loaded)
            SetAudioStreamVolume (self->stream, volume);

        g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_VOLUME]);
    }
}

/**
 * grl_audio_stream_get_volume:
 * @self: A #GrlAudioStream
 *
 * Gets the current volume level.
 *
 * Returns: The volume level (0.0 to 1.0)
 */
gfloat
grl_audio_stream_get_volume (GrlAudioStream *self)
{
    g_return_val_if_fail (GRL_IS_AUDIO_STREAM (self), 1.0f);

    return self->volume;
}

/**
 * grl_audio_stream_set_pitch:
 * @self: A #GrlAudioStream
 * @pitch: Pitch multiplier (1.0 = normal pitch)
 *
 * Sets the pitch for this audio stream.
 */
void
grl_audio_stream_set_pitch (GrlAudioStream *self,
                            gfloat          pitch)
{
    g_return_if_fail (GRL_IS_AUDIO_STREAM (self));

    pitch = CLAMP (pitch, 0.1f, 10.0f);

    if (self->pitch != pitch)
    {
        self->pitch = pitch;

        if (self->is_loaded)
            SetAudioStreamPitch (self->stream, pitch);

        g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_PITCH]);
    }
}

/**
 * grl_audio_stream_get_pitch:
 * @self: A #GrlAudioStream
 *
 * Gets the current pitch multiplier.
 *
 * Returns: The pitch multiplier
 */
gfloat
grl_audio_stream_get_pitch (GrlAudioStream *self)
{
    g_return_val_if_fail (GRL_IS_AUDIO_STREAM (self), 1.0f);

    return self->pitch;
}

/**
 * grl_audio_stream_set_pan:
 * @self: A #GrlAudioStream
 * @pan: Pan position (-1.0 = left, 0.0 = center, 1.0 = right)
 *
 * Sets the stereo pan position for this audio stream.
 */
void
grl_audio_stream_set_pan (GrlAudioStream *self,
                          gfloat          pan)
{
    g_return_if_fail (GRL_IS_AUDIO_STREAM (self));

    pan = CLAMP (pan, -1.0f, 1.0f);

    if (self->pan != pan)
    {
        self->pan = pan;

        if (self->is_loaded)
            SetAudioStreamPan (self->stream, pan); /* raylib 6.0: pan is -1.0..1.0 */

        g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_PAN]);
    }
}

/**
 * grl_audio_stream_get_pan:
 * @self: A #GrlAudioStream
 *
 * Gets the current pan position.
 *
 * Returns: The pan position (-1.0 to 1.0)
 */
gfloat
grl_audio_stream_get_pan (GrlAudioStream *self)
{
    g_return_val_if_fail (GRL_IS_AUDIO_STREAM (self), 0.0f);

    return self->pan;
}

/**
 * grl_audio_stream_set_default_buffer_size:
 * @size: Default buffer size for new audio streams
 *
 * Sets the default buffer size for new audio streams.
 * This affects all subsequently created streams.
 */
void
grl_audio_stream_set_default_buffer_size (gint size)
{
    g_return_if_fail (size > 0);

    SetAudioStreamBufferSizeDefault (size);
}
