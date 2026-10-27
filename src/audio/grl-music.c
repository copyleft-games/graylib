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
#include "grl-audio-processor-private.h"
#include "grl-music-private.h"
#include <gio/gio.h>
#include <raylib.h>

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

struct _GrlMusic
{
    GObject         parent_instance;

    Music           music;
    gboolean        is_loaded;
    gfloat          volume;
    gfloat          pitch;
    gfloat          pan;
    gboolean        looping;
    gchar          *filename;

    /* For memory-loaded music: raylib streams from this buffer */
    guint8         *memory_data;
    gsize           memory_size;

    /* per-voice processors, made on first use */
    GrlAudioProcessorSet *processors;

    /*
     * Background streaming (see below).  background is the caller's
     * switch; active (played, not stopped or ended), paused and
     * registered are written under pump_lock while background is on (the
     * streaming thread reads them), else by the caller alone.  The
     * cached_* values are what the streaming thread last saw, read
     * without the lock.
     */
    gboolean        background;
    gboolean        active;
    gboolean        paused;
    gboolean        registered;
    gint            cached_playing;      /* atomic, boolean */
    gint            cached_time_played;  /* atomic, the gfloat's bits */
};

G_DEFINE_FINAL_TYPE (GrlMusic, grl_music, G_TYPE_OBJECT)

/* ==========================================================================
 * Background streaming
 *
 * One process-wide thread refills every background-streamed music that
 * is active, PUMP_INTERVAL_US apart, by calling raylib's
 * UpdateMusicStream(), which decodes a sub-buffer for each one the device
 * has consumed.  pump_lock guards the list and is held around every
 * raylib call that touches such a music's decoder or Music struct, on
 * the thread and in the control calls, so two threads never decode or
 * seek one music at once (raylib's own lock does not cover the decoder,
 * and its decode scratch buffer is shared by every music).  Lock order:
 * pump_lock, then raylib's audio lock (inside raylib's functions); the
 * device thread takes only raylib's, and processors may not call back
 * into graylib.  The thread holds no reference: dispose unregisters
 * under the lock, so it never sees a freed music.  A generation counter
 * lets grl_music_streaming_shutdown() join a thread while a new one may
 * already have been started for later playback.
 * ========================================================================== */

#define PUMP_INTERVAL_US (5000)

static GMutex     pump_lock;
static GCond      pump_cond;
static GThread   *pump_thread;      /* NULL until the first background play */
static GPtrArray *pump_tracks;      /* GrlMusic *, borrowed, registered */
static guint      pump_generation;  /* the running thread quits when it moves on */

typedef union
{
    gfloat f;
    gint   i;
} FloatBits;

/* raylib's bool read through a byte (graylib's bool/gboolean rule) */
static gboolean
pump_device_ready (void)
{
    unsigned char raw;

    raw = IsAudioDeviceReady ();
    return raw != 0;
}

/* publishes the music's playing state and position (pump_lock held) */
static void
pump_refresh (GrlMusic *self)
{
    unsigned char raw;
    FloatBits     bits;

    if (!self->is_loaded)
        return;
    raw = IsMusicStreamPlaying (self->music);
    bits.f = GetMusicTimePlayed (self->music);
    g_atomic_int_set (&self->cached_playing, raw != 0);
    g_atomic_int_set (&self->cached_time_played, bits.i);
}

/* the streaming thread: refill, publish, drop what ended, sleep */
static gpointer
pump_main (gpointer data)
{
    guint generation;

    generation = GPOINTER_TO_UINT (data);
    g_mutex_lock (&pump_lock);
    while (generation == pump_generation)
    {
        guint  i;
        gint64 deadline;

        /* nothing to stream: sleep until a play (or shutdown) signals */
        if (pump_tracks == NULL || pump_tracks->len == 0)
        {
            g_cond_wait (&pump_cond, &pump_lock);
            continue;
        }

        if (pump_device_ready ())
        {
            i = 0;
            while (i < pump_tracks->len)
            {
                GrlMusic *music = g_ptr_array_index (pump_tracks, i);

                UpdateMusicStream (music->music);
                pump_refresh (music);

                /* ended (raylib stopped it at the end of a non-looping track) */
                if (!g_atomic_int_get (&music->cached_playing) && !music->paused)
                {
                    music->active = FALSE;
                    music->registered = FALSE;
                    g_ptr_array_remove_index_fast (pump_tracks, i);
                    continue;
                }
                i++;
            }
        }

        /* a play or shutdown wakes it early; a spurious wake costs a pass */
        deadline = g_get_monotonic_time () + PUMP_INTERVAL_US;
        (void)g_cond_wait_until (&pump_cond, &pump_lock, deadline);
    }
    g_mutex_unlock (&pump_lock);
    return NULL;
}

/* adds an active music to the thread's list, starting it (pump_lock held) */
static void
pump_register (GrlMusic *self)
{
    if (self->registered || !self->is_loaded)
        return;
    if (pump_tracks == NULL)
        pump_tracks = g_ptr_array_new ();
    g_ptr_array_add (pump_tracks, self);
    self->registered = TRUE;
    if (pump_thread == NULL)
        pump_thread = g_thread_new ("grl-music", pump_main, GUINT_TO_POINTER (pump_generation));
    g_cond_broadcast (&pump_cond);
}

/* takes a music off the list (pump_lock held) */
static void
pump_unregister (GrlMusic *self)
{
    if (!self->registered)
        return;
    g_ptr_array_remove_fast (pump_tracks, self);
    self->registered = FALSE;
}

/* pump_lock when the music streams in the background; returns whether taken */
static gboolean
music_lock (GrlMusic *self)
{
    if (!self->background)
        return FALSE;
    g_mutex_lock (&pump_lock);
    return TRUE;
}

void
grl_music_streaming_shutdown (void)
{
    GThread *thread;
    guint    i;

    g_mutex_lock (&pump_lock);
    thread = pump_thread;
    pump_thread = NULL;
    pump_generation++;
    if (pump_tracks != NULL)
    {
        for (i = 0; i < pump_tracks->len; i++)
            ((GrlMusic *)g_ptr_array_index (pump_tracks, i))->registered = FALSE;
        g_ptr_array_set_size (pump_tracks, 0);
    }
    g_cond_broadcast (&pump_cond);
    g_mutex_unlock (&pump_lock);

    if (thread != NULL)
        g_thread_join (thread);
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

    /* off the streaming thread's list before anything goes away */
    if (self->background)
    {
        g_mutex_lock (&pump_lock);
        pump_unregister (self);
        g_mutex_unlock (&pump_lock);
    }

    if (self->processors != NULL)
    {
        grl_audio_processor_set_clear (self->processors, self->is_loaded ? &self->music.stream : NULL, TRUE);
        g_clear_pointer (&self->processors, grl_audio_processor_set_free);
    }

    G_OBJECT_CLASS (grl_music_parent_class)->dispose (object);
}

static void
grl_music_finalize (GObject *object)
{
    GrlMusic *self = GRL_MUSIC (object);

    if (self->is_loaded)
    {
        StopMusicStream (self->music);
        UnloadMusicStream (self->music);
        self->is_loaded = FALSE;
    }

    g_clear_pointer (&self->filename, g_free);
    g_clear_pointer (&self->memory_data, g_free);

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
    self->memory_data = NULL;
    self->memory_size = 0;
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
    GrlMusic *self;
    Music music;

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

    music = LoadMusicStream (filename);

    /* Check if music loaded successfully */
    if (music.frameCount == 0)
    {
        g_set_error (error,
                     G_IO_ERROR,
                     G_IO_ERROR_FAILED,
                     "Failed to load music: %s",
                     filename);
        return NULL;
    }

    self = g_object_new (GRL_TYPE_MUSIC, NULL);
    self->music = music;
    /* raylib loads music looping; the looping property starts FALSE */
    self->music.looping = self->looping;
    self->is_loaded = TRUE;
    self->filename = g_strdup (filename);

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
    GrlMusic *self;
    Music music;
    guint8 *data_copy;

    g_return_val_if_fail (file_type != NULL, NULL);
    g_return_val_if_fail (data != NULL, NULL);
    g_return_val_if_fail (data_size > 0, NULL);
    g_return_val_if_fail (error == NULL || *error == NULL, NULL);

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

    /* Check if music loaded successfully */
    if (music.frameCount == 0)
    {
        g_free (data_copy);
        g_set_error (error,
                     G_IO_ERROR,
                     G_IO_ERROR_FAILED,
                     "Failed to load music from memory (type: %s, size: %" G_GSIZE_FORMAT ")",
                     file_type, data_size);
        return NULL;
    }

    self = g_object_new (GRL_TYPE_MUSIC, NULL);
    self->music = music;
    /* raylib loads music looping; the looping property starts FALSE */
    self->music.looping = self->looping;
    self->is_loaded = TRUE;
    self->filename = g_strdup_printf ("(memory:%s)", file_type);
    self->memory_data = data_copy;
    self->memory_size = data_size;

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
    gboolean locked;

    g_return_if_fail (GRL_IS_MUSIC (self));

    if (!self->is_loaded)
        return;

    locked = music_lock (self);
    PlayMusicStream (self->music);
    self->active = TRUE;
    self->paused = FALSE;
    if (locked)
    {
        /* the thread's first refill follows at once (it is woken) */
        pump_refresh (self);
        pump_register (self);
        g_mutex_unlock (&pump_lock);
    }
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
    gboolean locked;

    g_return_if_fail (GRL_IS_MUSIC (self));

    if (!self->is_loaded)
        return;

    locked = music_lock (self);
    StopMusicStream (self->music);
    self->active = FALSE;
    self->paused = FALSE;
    if (locked)
    {
        pump_unregister (self);
        pump_refresh (self);
        g_mutex_unlock (&pump_lock);
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
    gboolean locked;

    g_return_if_fail (GRL_IS_MUSIC (self));

    if (!self->is_loaded)
        return;

    locked = music_lock (self);
    PauseMusicStream (self->music);
    self->paused = TRUE;
    if (locked)
    {
        pump_refresh (self);
        g_mutex_unlock (&pump_lock);
    }
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
    gboolean locked;

    g_return_if_fail (GRL_IS_MUSIC (self));

    if (!self->is_loaded)
        return;

    locked = music_lock (self);
    ResumeMusicStream (self->music);
    self->paused = FALSE;
    if (locked)
    {
        pump_refresh (self);
        g_mutex_unlock (&pump_lock);
    }
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

    UpdateMusicStream (self->music);
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
    unsigned char raw;

    g_return_val_if_fail (GRL_IS_MUSIC (self), FALSE);

    if (!self->is_loaded)
        return FALSE;

    /* what the streaming thread last saw: no wait on a decode */
    if (self->background)
        return g_atomic_int_get (&self->cached_playing) != 0;

    /* Fix bool/gboolean ABI mismatch - use unsigned char intermediate */
    raw = IsMusicStreamPlaying (self->music);
    return raw != 0;
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

        if (self->is_loaded)
        {
            gboolean locked;

            /* the streaming thread reads the Music struct */
            locked = music_lock (self);
            self->music.looping = looping;
            if (locked)
                g_mutex_unlock (&pump_lock);
        }

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
    gboolean locked;

    g_return_if_fail (GRL_IS_MUSIC (self));

    if (!self->is_loaded)
        return;

    locked = music_lock (self);
    SeekMusicStream (self->music, position);
    if (locked)
    {
        pump_refresh (self);
        g_mutex_unlock (&pump_lock);
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

    return GetMusicTimeLength (self->music);
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

    g_return_val_if_fail (GRL_IS_MUSIC (self), 0.0f);

    if (!self->is_loaded)
        return 0.0f;

    /* what the streaming thread last saw: no wait on a decode */
    if (self->background)
    {
        bits.i = g_atomic_int_get (&self->cached_time_played);
        return bits.f;
    }

    return GetMusicTimePlayed (self->music);
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

    g_mutex_lock (&pump_lock);
    self->background = enabled;
    if (enabled)
    {
        pump_refresh (self);
        if (self->active)
            pump_register (self);
    }
    else
    {
        pump_unregister (self);
    }
    g_mutex_unlock (&pump_lock);

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
            SetMusicVolume (self->music, volume);

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
            SetMusicPitch (self->music, pitch);

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
            SetMusicPan (self->music, pan); /* raylib 6.0: pan is -1.0..1.0 */

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
    return grl_audio_processor_set_add (self->processors, self->is_loaded ? &self->music.stream : NULL,
                                        func, user_data, destroy);
}

gboolean
grl_music_remove_processor (GrlMusic *self,
                            guint     id)
{
    g_return_val_if_fail (GRL_IS_MUSIC (self), FALSE);

    if (self->processors == NULL || id == 0)
        return FALSE;
    return grl_audio_processor_set_remove (self->processors, self->is_loaded ? &self->music.stream : NULL, id);
}

void
grl_music_clear_processors (GrlMusic *self)
{
    g_return_if_fail (GRL_IS_MUSIC (self));

    if (self->processors != NULL)
        grl_audio_processor_set_clear (self->processors, self->is_loaded ? &self->music.stream : NULL, FALSE);
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
