/* grl-music.c
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Streaming music implementation.
 */

#include "config.h"
#include "grl-music.h"
#include "grl-audio-cleanup-private.h"
#include "grl-audio-processor-private.h"
#include "grl-audio-queue-private.h"
#include <gio/gio.h>
#include <raylib.h>
#include <string.h>

/**
 * SECTION:grl-music
 * @title: GrlMusic
 * @short_description: Streaming music for long audio tracks
 *
 * #GrlMusic represents streaming audio that is loaded in chunks from disk.
 * This makes it suitable for background music and other long audio tracks
 * that would consume too much memory if loaded entirely.
 *
 * Unlike #GrlSound, music requires periodic updates via grl_music_update()
 * to stream new audio data from disk, unless it streams in the background
 * (grl_music_set_background_streaming()), where graylib's streaming
 * thread refills it.
 *
 * # Example
 *
 * |[<!-- language="C" -->
 * g_autoptr(GError) error = NULL;
 * g_autoptr(GrlMusic) music = grl_music_new_from_file ("background.ogg", &error);
 *
 * if (music == NULL)
 * {
 *     g_printerr ("Error loading music: %s\n", error->message);
 *     return;
 * }
 *
 * grl_music_set_volume (music, 0.5f);
 * grl_music_set_looping (music, TRUE);
 * grl_music_play (music);
 *
 * /<!-- -->* In game loop *<!-- -->/
 * while (!should_quit)
 * {
 *     grl_music_update (music);  /<!-- -->* Required for streaming! *<!-- -->/
 *     /<!-- -->* ... game logic ... *<!-- -->/
 * }
 * ]|
 */

/*
 * The part the audio thread owns (grl-audio-queue-private.h).  The raylib
 * Music lives here for every music: a music that streams in the
 * background belongs to the audio thread, which takes every call that
 * touches it as a command; any other music is the caller's, called
 * directly.  memory_data is what raylib streams a memory-loaded music
 * from, freed only after the unload.  wav_extra_context is the
 * constructor-owned WAV decoder object the pinned SDK's unload does not
 * free (grl-audio-cleanup-private.h, docs/audio-ownership-contract.org);
 * the voice's release frees it right after the native unload.
 */
typedef struct
{
    GrlAudioVoice   parent;
    Music           music;
    guint8         *memory_data;
    gpointer        wav_extra_context;
    gint            time_bits;      /* atomic: the gfloat's bits, published */
} MusicVoice;

struct _GrlMusic
{
    GObject         parent_instance;

    MusicVoice     *voice;
    gboolean        is_loaded;
    gfloat          volume;
    gfloat          pitch;
    gfloat          pan;
    gboolean        looping;
    gchar          *filename;

    /* For memory-loaded music: its size (the data is the voice's) */
    gsize           memory_size;

    /* per-voice processors, made on first use */
    GrlAudioProcessorSet *processors;

    /*
     * Background streaming (see below): the caller's switch, and the
     * position a queued seek or stop will leave, reported by
     * get_time_played() until the audio thread has applied it.
     */
    gboolean        background;
    gboolean        time_pending;
    gfloat          pending_time;
};

G_DEFINE_FINAL_TYPE (GrlMusic, grl_music, G_TYPE_OBJECT)

/* ==========================================================================
 * Background streaming
 *
 * A background music is refilled by graylib's audio thread
 * (grl-audio-queue.c): its voice's refresh calls raylib's
 * UpdateMusicStream(), which decodes a sub-buffer for each one the device
 * has consumed, about every 5 ms while it plays, and publishes the playing
 * state and position.  Every call that touches such a music's Music struct
 * or decoder (play, stop, pause, resume, seek, looping, volume, pitch,
 * pan, unload) is a command on the same thread, so two threads never
 * decode or seek one music at once (raylib's own lock does not cover the
 * decoder, and its decode scratch buffer is shared by every music) and
 * the caller never waits on a decode or on raylib's audio lock.
 * is_playing() and get_time_played() read what the thread last
 * published, or what the caller's last call will leave while it is still
 * queued.
 * ========================================================================== */

typedef union
{
    gfloat f;
    gint   i;
} FloatBits;

#define MUSIC_OF(voice) (((MusicVoice *)(voice))->music)

/* the audio thread: refill, then publish the state and position */
static gboolean
music_voice_refresh (GrlAudioVoice *voice)
{
    MusicVoice    *mv = (MusicVoice *)voice;
    unsigned char  raw;
    FloatBits      bits;

    UpdateMusicStream (mv->music);
    raw = IsMusicStreamPlaying (mv->music);
    bits.f = GetMusicTimePlayed (mv->music);
    g_atomic_int_set (&mv->time_bits, bits.i);
    grl_audio_voice_publish_playing (voice, raw != 0);
    return raw != 0;
}

static void
music_voice_release (GrlAudioVoice *voice)
{
    MusicVoice *mv = (MusicVoice *)voice;

    StopMusicStream (mv->music);
    grl_audio_unload_music_owned (mv->music, g_steal_pointer (&mv->wav_extra_context),
                                  UnloadMusicStream, MemFree);
}

static void
music_voice_finalize (GrlAudioVoice *voice)
{
    MusicVoice *mv = (MusicVoice *)voice;

    g_free (mv->memory_data);
    g_free (mv);
}

static const GrlAudioVoiceClass music_voice_class = {
    music_voice_refresh,
    music_voice_release,
    music_voice_finalize
};

static void
music_voice_play (GrlAudioVoice *voice,
                  gfloat         value,
                  gpointer       data)
{
    PlayMusicStream (MUSIC_OF (voice));
}

static void
music_voice_stop (GrlAudioVoice *voice,
                  gfloat         value,
                  gpointer       data)
{
    StopMusicStream (MUSIC_OF (voice));
}

static void
music_voice_pause (GrlAudioVoice *voice,
                   gfloat         value,
                   gpointer       data)
{
    PauseMusicStream (MUSIC_OF (voice));
}

static void
music_voice_resume (GrlAudioVoice *voice,
                    gfloat         value,
                    gpointer       data)
{
    ResumeMusicStream (MUSIC_OF (voice));
}

static void
music_voice_seek (GrlAudioVoice *voice,
                  gfloat         value,
                  gpointer       data)
{
    SeekMusicStream (MUSIC_OF (voice), value);
}

/* @value: non-zero to loop (UpdateMusicStream reads the struct's flag) */
static void
music_voice_set_looping (GrlAudioVoice *voice,
                         gfloat         value,
                         gpointer       data)
{
    MUSIC_OF (voice).looping = value != 0.0f;
}

static void
music_voice_set_volume (GrlAudioVoice *voice,
                        gfloat         value,
                        gpointer       data)
{
    SetMusicVolume (MUSIC_OF (voice), value);
}

static void
music_voice_set_pitch (GrlAudioVoice *voice,
                       gfloat         value,
                       gpointer       data)
{
    SetMusicPitch (MUSIC_OF (voice), value);
}

static void
music_voice_set_pan (GrlAudioVoice *voice,
                     gfloat         value,
                     gpointer       data)
{
    SetMusicPan (MUSIC_OF (voice), value); /* raylib 6.0: pan is -1.0..1.0 */
}

/* raylib's playing state, asked directly (bool read through a byte) */
static gboolean
music_playing_direct (GrlMusic *self)
{
    unsigned char raw;

    raw = IsMusicStreamPlaying (self->voice->music);
    return raw != 0;
}

/* a plain call: queued in the background, else made at once */
static void
music_call (GrlMusic          *self,
            GrlAudioVoiceFunc  func,
            gfloat             value)
{
    if (self->background)
        grl_audio_voice_push (&self->voice->parent, func, value, NULL, NULL);
    else
        func (&self->voice->parent, value, NULL);
}

/* a call that changes whether it plays; @predicted as for push_state */
static void
music_call_state (GrlMusic          *self,
                  GrlAudioVoiceFunc  func,
                  gfloat             value,
                  gboolean           predicted)
{
    if (self->background)
        grl_audio_voice_push_state (&self->voice->parent, func, value, NULL, NULL, predicted);
    else
        func (&self->voice->parent, value, NULL);
}

enum
{
    PROP_0,
    PROP_FILENAME,
    PROP_VOLUME,
    PROP_PITCH,
    PROP_PAN,
    PROP_LOOPING,
    PROP_IS_PLAYING,
    PROP_TIME_LENGTH,
    PROP_TIME_PLAYED,
    PROP_BACKGROUND_STREAMING,
    N_PROPS
};

static GParamSpec *properties[N_PROPS];

/*
 * The processors leave raylib's stream before the voice can be unloaded
 * (raylib does not free a stream's processors when it unloads it).
 */
static void
grl_music_dispose (GObject *object)
{
    GrlMusic *self = (GrlMusic *)object;

    if (self->processors != NULL)
    {
        grl_audio_processor_set_clear (self->processors, self->is_loaded ? &self->voice->music.stream : NULL, TRUE);
        g_clear_pointer (&self->processors, grl_audio_processor_set_free);
    }

    G_OBJECT_CLASS (grl_music_parent_class)->dispose (object);
}

static void
grl_music_finalize (GObject *object)
{
    GrlMusic *self = GRL_MUSIC (object);

    /* stopped and unloaded after every call queued before it (the voice
     * frees the memory data after that); an unloaded voice just goes */
    if (self->is_loaded)
        grl_audio_voice_release (&g_steal_pointer (&self->voice)->parent);
    else if (self->voice != NULL)
        grl_audio_voice_unref (&g_steal_pointer (&self->voice)->parent);
    self->is_loaded = FALSE;

    g_clear_pointer (&self->filename, g_free);

    G_OBJECT_CLASS (grl_music_parent_class)->finalize (object);
}

static void
grl_music_get_property (GObject    *object,
                        guint       prop_id,
                        GValue     *value,
                        GParamSpec *pspec)
{
    GrlMusic *self = GRL_MUSIC (object);

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

    case PROP_LOOPING:
        g_value_set_boolean (value, self->looping);
        break;

    case PROP_IS_PLAYING:
        g_value_set_boolean (value, grl_music_is_playing (self));
        break;

    case PROP_TIME_LENGTH:
        g_value_set_float (value, grl_music_get_time_length (self));
        break;

    case PROP_TIME_PLAYED:
        g_value_set_float (value, grl_music_get_time_played (self));
        break;

    case PROP_BACKGROUND_STREAMING:
        g_value_set_boolean (value, self->background);
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
grl_music_set_property (GObject      *object,
                        guint         prop_id,
                        const GValue *value,
                        GParamSpec   *pspec)
{
    GrlMusic *self = GRL_MUSIC (object);

    switch (prop_id)
    {
    case PROP_VOLUME:
        grl_music_set_volume (self, g_value_get_float (value));
        break;

    case PROP_PITCH:
        grl_music_set_pitch (self, g_value_get_float (value));
        break;

    case PROP_PAN:
        grl_music_set_pan (self, g_value_get_float (value));
        break;

    case PROP_LOOPING:
        grl_music_set_looping (self, g_value_get_boolean (value));
        break;

    case PROP_BACKGROUND_STREAMING:
        grl_music_set_background_streaming (self, g_value_get_boolean (value));
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
grl_music_class_init (GrlMusicClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->dispose = grl_music_dispose;
    object_class->finalize = grl_music_finalize;
    object_class->get_property = grl_music_get_property;
    object_class->set_property = grl_music_set_property;

    /**
     * GrlMusic:filename:
     *
     * The filename this music was loaded from.
     */
    properties[PROP_FILENAME] =
        g_param_spec_string ("filename",
                             "Filename",
                             "The source filename",
                             NULL,
                             G_PARAM_READABLE |
                             G_PARAM_STATIC_STRINGS);

    /**
     * GrlMusic:volume:
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
     * GrlMusic:pitch:
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
     * GrlMusic:pan:
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
     * GrlMusic:looping:
     *
     * Whether the music should loop when it reaches the end.
     */
    properties[PROP_LOOPING] =
        g_param_spec_boolean ("looping",
                              "Looping",
                              "Whether to loop the music",
                              FALSE,
                              G_PARAM_READWRITE |
                              G_PARAM_STATIC_STRINGS |
                              G_PARAM_EXPLICIT_NOTIFY);

    /**
     * GrlMusic:is-playing:
     *
     * Whether the music is currently playing.
     */
    properties[PROP_IS_PLAYING] =
        g_param_spec_boolean ("is-playing",
                              "Is Playing",
                              "Whether the music is playing",
                              FALSE,
                              G_PARAM_READABLE |
                              G_PARAM_STATIC_STRINGS);

    /**
     * GrlMusic:time-length:
     *
     * The total length of the music in seconds.
     */
    properties[PROP_TIME_LENGTH] =
        g_param_spec_float ("time-length",
                            "Time Length",
                            "Total length in seconds",
                            0.0f, G_MAXFLOAT, 0.0f,
                            G_PARAM_READABLE |
                            G_PARAM_STATIC_STRINGS);

    /**
     * GrlMusic:time-played:
     *
     * The current playback position in seconds.
     */
    properties[PROP_TIME_PLAYED] =
        g_param_spec_float ("time-played",
                            "Time Played",
                            "Current position in seconds",
                            0.0f, G_MAXFLOAT, 0.0f,
                            G_PARAM_READABLE |
                            G_PARAM_STATIC_STRINGS);

    /**
     * GrlMusic:background-streaming:
     *
     * Whether graylib's streaming thread refills the music (see
     * grl_music_set_background_streaming()).
     */
    properties[PROP_BACKGROUND_STREAMING] =
        g_param_spec_boolean ("background-streaming",
                              "Background Streaming",
                              "Whether the music is refilled on graylib's streaming thread",
                              FALSE,
                              G_PARAM_READWRITE |
                              G_PARAM_STATIC_STRINGS |
                              G_PARAM_EXPLICIT_NOTIFY);

    g_object_class_install_properties (object_class, N_PROPS, properties);
}

static void
grl_music_init (GrlMusic *self)
{
    self->is_loaded = FALSE;
    self->volume = 1.0f;
    self->pitch = 1.0f;
    self->pan = 0.0f;
    self->looping = FALSE;
    self->filename = NULL;
    self->memory_size = 0;
    self->voice = g_new0 (MusicVoice, 1);
    grl_audio_voice_init (&self->voice->parent, &music_voice_class);
}

/*
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
GrlMusic *
grl_music_new_from_file (const gchar  *filename,
                         GError      **error)
{
    GrlMusic      *self;
    Music          music;
    gpointer       extra_context;
    const gchar   *dot;
    unsigned char  ready;

    g_return_val_if_fail (filename != NULL, NULL);
    g_return_val_if_fail (error == NULL || *error == NULL, NULL);

    /* Check if file exists */
    if (!FileExists (filename))
    {
        g_set_error (error,
                     G_IO_ERROR,
                     G_IO_ERROR_NOT_FOUND,
                     "Music file not found: %s",
                     filename);
        return NULL;
    }

    /* never opens a device implicitly */
    ready = IsAudioDeviceReady ();
    if (!ready)
    {
        g_set_error_literal (error, G_IO_ERROR, G_IO_ERROR_NOT_INITIALIZED,
                             "Music loading requires an audio device");
        return NULL;
    }
    music = LoadMusicStream (filename);
    dot = strrchr (filename, '.');
    extra_context = grl_audio_wav_extra_context (music,
        g_ascii_strcasecmp (dot != NULL ? dot : "", ".wav") == 0,
        GRL_SDK_WAV_EXTRA_FREE && RAYLIB_VERSION_MAJOR == 6 && RAYLIB_VERSION_MINOR == 0);

    /* Check if music loaded successfully */
    if (music.frameCount == 0 || music.stream.buffer == NULL)
    {
        /* A zero-frame decoded WAV can still own its decoder and stream. */
        if (music.ctxData != NULL)
            grl_audio_unload_music_owned (music, extra_context, UnloadMusicStream, MemFree);
        g_set_error (error,
                     G_IO_ERROR,
                     G_IO_ERROR_FAILED,
                     "Failed to load music: %s",
                     filename);
        return NULL;
    }

    self = g_object_new (GRL_TYPE_MUSIC, NULL);
    self->voice->music = music;
    /* raylib loads music looping; the looping property starts FALSE */
    self->voice->music.looping = self->looping;
    self->is_loaded = TRUE;
    self->filename = g_strdup (filename);
    /* No decoder-private fields: ctxData is part of raylib's public Music. */
    self->voice->wav_extra_context = extra_context;

    return self;
}

/*
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
GrlMusic *
grl_music_new_from_memory (const gchar  *file_type,
                           const guint8 *data,
                           gsize         data_size,
                           GError      **error)
{
    GrlMusic      *self;
    Music          music;
    guint8        *data_copy;
    gpointer       extra_context;
    unsigned char  ready;

    g_return_val_if_fail (file_type != NULL, NULL);
    g_return_val_if_fail (data != NULL, NULL);
    g_return_val_if_fail (data_size > 0, NULL);
    g_return_val_if_fail (error == NULL || *error == NULL, NULL);
    if (data_size > G_MAXINT)
    {
        g_set_error_literal (error, G_IO_ERROR, G_IO_ERROR_INVALID_DATA,
                             "Encoded music exceeds the SDK byte-count range");
        return NULL;
    }
    ready = IsAudioDeviceReady ();
    if (!ready)
    {
        g_set_error_literal (error, G_IO_ERROR, G_IO_ERROR_NOT_INITIALIZED,
                             "Music loading requires an audio device");
        return NULL;
    }

    /* Copy the data - raylib streams from this buffer, so it must persist */
    data_copy = g_memdup2 (data, data_size);
    if (data_copy == NULL)
    {
        g_set_error (error,
                     G_IO_ERROR,
                     G_IO_ERROR_NO_SPACE,
                     "Failed to allocate memory for music data");
        return NULL;
    }

    /* Load music stream from memory */
    music = LoadMusicStreamFromMemory (file_type,
                                       (const unsigned char *)data_copy,
                                       (int)data_size);
    extra_context = grl_audio_wav_extra_context (music,
        g_ascii_strcasecmp (file_type, ".wav") == 0,
        GRL_SDK_WAV_EXTRA_FREE && RAYLIB_VERSION_MAJOR == 6 && RAYLIB_VERSION_MINOR == 0);

    /* Check if music loaded successfully */
    if (music.frameCount == 0 || music.stream.buffer == NULL)
    {
        if (music.ctxData != NULL)
            grl_audio_unload_music_owned (music, extra_context, UnloadMusicStream, MemFree);
        g_free (data_copy);
        g_set_error (error,
                     G_IO_ERROR,
                     G_IO_ERROR_FAILED,
                     "Failed to load music from memory (type: %s, size: %" G_GSIZE_FORMAT ")",
                     file_type, data_size);
        return NULL;
    }

    self = g_object_new (GRL_TYPE_MUSIC, NULL);
    self->voice->music = music;
    /* raylib loads music looping; the looping property starts FALSE */
    self->voice->music.looping = self->looping;
    self->is_loaded = TRUE;
    self->filename = g_strdup_printf ("(memory:%s)", file_type);
    self->voice->memory_data = data_copy;
    self->memory_size = data_size;
    self->voice->wav_extra_context = extra_context;

    return self;
}

/*
 * grl_music_play:
 * @self: A #GrlMusic
 *
 * Starts playing the music. If the music was paused, it resumes
 * from the current position.
 */
void
grl_music_play (GrlMusic *self)
{
    g_return_if_fail (GRL_IS_MUSIC (self));

    if (!self->is_loaded)
        return;

    /* in the background the thread's first refill follows at once */
    music_call_state (self, music_voice_play, 0.0f, TRUE);
}

/*
 * grl_music_stop:
 * @self: A #GrlMusic
 *
 * Stops the music and resets the playback position to the beginning.
 */
void
grl_music_stop (GrlMusic *self)
{
    g_return_if_fail (GRL_IS_MUSIC (self));

    if (!self->is_loaded)
        return;

    music_call_state (self, music_voice_stop, 0.0f, FALSE);
    if (self->background)
    {
        /* raylib's stop rewinds */
        self->time_pending = TRUE;
        self->pending_time = 0.0f;
    }
}

/*
 * grl_music_pause:
 * @self: A #GrlMusic
 *
 * Pauses the music at the current position.
 */
void
grl_music_pause (GrlMusic *self)
{
    g_return_if_fail (GRL_IS_MUSIC (self));

    if (!self->is_loaded)
        return;

    /* resume() predicts playing only if it played when paused */
    self->voice->parent.resumable = grl_music_is_playing (self);
    music_call_state (self, music_voice_pause, 0.0f, FALSE);
}

/*
 * grl_music_resume:
 * @self: A #GrlMusic
 *
 * Resumes paused music from the current position.
 */
void
grl_music_resume (GrlMusic *self)
{
    g_return_if_fail (GRL_IS_MUSIC (self));

    if (!self->is_loaded)
        return;

    music_call_state (self, music_voice_resume, 0.0f, self->voice->parent.resumable);
}

/*
 * grl_music_update:
 * @self: A #GrlMusic
 *
 * Updates the music stream buffers. This MUST be called in the main
 * loop while music is playing to ensure continuous playback.
 */
void
grl_music_update (GrlMusic *self)
{
    g_return_if_fail (GRL_IS_MUSIC (self));

    /* a background music is refilled by the streaming thread */
    if (!self->is_loaded || self->background)
        return;

    UpdateMusicStream (self->voice->music);
}

/*
 * grl_music_is_playing:
 * @self: A #GrlMusic
 *
 * Checks if the music is currently playing.
 *
 * Returns: %TRUE if the music is playing
 */
gboolean
grl_music_is_playing (GrlMusic *self)
{
    gboolean playing;

    g_return_val_if_fail (GRL_IS_MUSIC (self), FALSE);

    if (!self->is_loaded)
        return FALSE;

    /* the audio thread's mirror: no wait on a decode or raylib's lock */
    if (self->background && grl_audio_voice_peek_playing (&self->voice->parent, &playing))
        return playing;

    return music_playing_direct (self);
}

/*
 * grl_music_set_looping:
 * @self: A #GrlMusic
 * @looping: Whether to loop the music
 *
 * Sets whether the music should loop when it reaches the end.
 */
void
grl_music_set_looping (GrlMusic *self,
                       gboolean  looping)
{
    g_return_if_fail (GRL_IS_MUSIC (self));

    if (self->looping != looping)
    {
        self->looping = looping;

        /* in the background the thread owns the Music struct */
        if (self->is_loaded)
            music_call (self, music_voice_set_looping, looping ? 1.0f : 0.0f);

        g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_LOOPING]);
    }
}

/*
 * grl_music_get_looping:
 * @self: A #GrlMusic
 *
 * Gets whether the music is set to loop.
 *
 * Returns: %TRUE if the music loops
 */
gboolean
grl_music_get_looping (GrlMusic *self)
{
    g_return_val_if_fail (GRL_IS_MUSIC (self), FALSE);

    return self->looping;
}

/*
 * grl_music_seek:
 * @self: A #GrlMusic
 * @position: Position in seconds
 *
 * Seeks to a specific position in the music.
 */
void
grl_music_seek (GrlMusic *self,
                gfloat    position)
{
    g_return_if_fail (GRL_IS_MUSIC (self));

    if (!self->is_loaded)
        return;

    if (self->background)
    {
        /* a seek does not change whether it plays; the refresh after it
         * publishes the new position, reported at once meanwhile */
        music_call_state (self, music_voice_seek, position, grl_music_is_playing (self));
        self->time_pending = TRUE;
        self->pending_time = position;
    }
    else
    {
        SeekMusicStream (self->voice->music, position);
    }
}

/*
 * grl_music_get_time_length:
 * @self: A #GrlMusic
 *
 * Gets the total length of the music in seconds.
 *
 * Returns: The music length in seconds
 */
gfloat
grl_music_get_time_length (GrlMusic *self)
{
    g_return_val_if_fail (GRL_IS_MUSIC (self), 0.0f);

    if (!self->is_loaded)
        return 0.0f;

    return GetMusicTimeLength (self->voice->music);
}

/*
 * grl_music_get_time_played:
 * @self: A #GrlMusic
 *
 * Gets the current playback position in seconds.
 *
 * Returns: The current position in seconds
 */
gfloat
grl_music_get_time_played (GrlMusic *self)
{
    FloatBits bits;
    gboolean  playing;

    g_return_val_if_fail (GRL_IS_MUSIC (self), 0.0f);

    if (!self->is_loaded)
        return 0.0f;

    /* what the audio thread last published: no wait on a decode */
    if (self->background && grl_audio_voice_peek_playing (&self->voice->parent, &playing))
    {
        /* a seek or stop still queued: the position it will leave */
        if (self->time_pending && !grl_audio_voice_is_settled (&self->voice->parent))
            return self->pending_time;
        self->time_pending = FALSE;
        bits.i = g_atomic_int_get (&self->voice->time_bits);
        return bits.f;
    }

    return GetMusicTimePlayed (self->voice->music);
}

/*
 * grl_music_set_background_streaming:
 * Switching on publishes the current state and puts an active music on
 * the thread's list; switching off takes it off (the caller's
 * grl_music_update() refills it again).
 */
void
grl_music_set_background_streaming (GrlMusic *self,
                                    gboolean  enabled)
{
    g_return_if_fail (GRL_IS_MUSIC (self));

    enabled = enabled != FALSE;
    if (self->background == enabled)
        return;

    if (enabled)
    {
        /* the thread takes over: a refresh seeds the mirror and, if it
         * plays, puts it on the refill list (one direct read here) */
        self->background = TRUE;
        if (self->is_loaded)
            grl_audio_voice_push_state (&self->voice->parent, NULL, 0.0f, NULL, NULL,
                                        music_playing_direct (self));
    }
    else
    {
        /* handed back once the thread let go of it (the one blocking
         * call here: it waits for what is queued) */
        if (self->is_loaded)
        {
            grl_audio_voice_unwatch (&self->voice->parent);
            grl_audio_queue_flush ();
        }
        self->background = FALSE;
        self->time_pending = FALSE;
    }

    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_BACKGROUND_STREAMING]);
}

gboolean
grl_music_get_background_streaming (GrlMusic *self)
{
    g_return_val_if_fail (GRL_IS_MUSIC (self), FALSE);

    return self->background;
}

/*
 * grl_music_set_volume:
 * @self: A #GrlMusic
 * @volume: Volume level (0.0 to 1.0)
 *
 * Sets the volume for this music track.
 */
void
grl_music_set_volume (GrlMusic *self,
                      gfloat    volume)
{
    g_return_if_fail (GRL_IS_MUSIC (self));

    volume = CLAMP (volume, 0.0f, 1.0f);

    if (self->volume != volume)
    {
        self->volume = volume;

        if (self->is_loaded)
            music_call (self, music_voice_set_volume, volume);

        g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_VOLUME]);
    }
}

/*
 * grl_music_get_volume:
 * @self: A #GrlMusic
 *
 * Gets the current volume level.
 *
 * Returns: The volume level (0.0 to 1.0)
 */
gfloat
grl_music_get_volume (GrlMusic *self)
{
    g_return_val_if_fail (GRL_IS_MUSIC (self), 1.0f);

    return self->volume;
}

/*
 * grl_music_set_pitch:
 * @self: A #GrlMusic
 * @pitch: Pitch multiplier (1.0 = normal pitch)
 *
 * Sets the pitch for this music track.
 */
void
grl_music_set_pitch (GrlMusic *self,
                     gfloat    pitch)
{
    g_return_if_fail (GRL_IS_MUSIC (self));

    pitch = CLAMP (pitch, 0.1f, 10.0f);

    if (self->pitch != pitch)
    {
        self->pitch = pitch;

        if (self->is_loaded)
            music_call (self, music_voice_set_pitch, pitch);

        g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_PITCH]);
    }
}

/*
 * grl_music_get_pitch:
 * @self: A #GrlMusic
 *
 * Gets the current pitch multiplier.
 *
 * Returns: The pitch multiplier
 */
gfloat
grl_music_get_pitch (GrlMusic *self)
{
    g_return_val_if_fail (GRL_IS_MUSIC (self), 1.0f);

    return self->pitch;
}

/*
 * grl_music_set_pan:
 * @self: A #GrlMusic
 * @pan: Pan position (-1.0 = left, 0.0 = center, 1.0 = right)
 *
 * Sets the stereo pan position for this music track.
 */
void
grl_music_set_pan (GrlMusic *self,
                   gfloat    pan)
{
    g_return_if_fail (GRL_IS_MUSIC (self));

    pan = CLAMP (pan, -1.0f, 1.0f);

    if (self->pan != pan)
    {
        self->pan = pan;

        if (self->is_loaded)
            music_call (self, music_voice_set_pan, pan);

        g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_PAN]);
    }
}

/*
 * grl_music_get_pan:
 * @self: A #GrlMusic
 *
 * Gets the current pan position.
 *
 * Returns: The pan position (-1.0 to 1.0)
 */
gfloat
grl_music_get_pan (GrlMusic *self)
{
    g_return_val_if_fail (GRL_IS_MUSIC (self), 0.0f);

    return self->pan;
}

/*
 * Per-voice processors (see grl-audio-processor.c): the set is made on
 * first use; an unloaded object keeps its processors without a raylib
 * stream, where only grl_music_process() runs them.
 */
guint
grl_music_add_processor (GrlMusic            *self,
                         GrlAudioProcessFunc  func,
                         gpointer             user_data,
                         GDestroyNotify       destroy)
{
    g_return_val_if_fail (GRL_IS_MUSIC (self), 0);
    g_return_val_if_fail (func != NULL, 0);

    if (self->processors == NULL)
        self->processors = grl_audio_processor_set_new ();
    return grl_audio_processor_set_add (self->processors, self->is_loaded ? &self->voice->music.stream : NULL,
                                        func, user_data, destroy);
}

gboolean
grl_music_remove_processor (GrlMusic *self,
                            guint     id)
{
    g_return_val_if_fail (GRL_IS_MUSIC (self), FALSE);

    if (self->processors == NULL || id == 0)
        return FALSE;
    return grl_audio_processor_set_remove (self->processors, self->is_loaded ? &self->voice->music.stream : NULL, id);
}

void
grl_music_clear_processors (GrlMusic *self)
{
    g_return_if_fail (GRL_IS_MUSIC (self));

    if (self->processors != NULL)
        grl_audio_processor_set_clear (self->processors, self->is_loaded ? &self->voice->music.stream : NULL, FALSE);
}

guint
grl_music_get_n_processors (GrlMusic *self)
{
    g_return_val_if_fail (GRL_IS_MUSIC (self), 0);

    return grl_audio_processor_set_get_count (self->processors);
}

void
grl_music_process (GrlMusic *self,
                   gfloat   *frames,
                   guint     n_frames)
{
    g_return_if_fail (GRL_IS_MUSIC (self));
    g_return_if_fail (frames != NULL || n_frames == 0);

    grl_audio_processor_set_run (self->processors, frames, n_frames);
}
