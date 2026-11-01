/* grl-audio-device.c
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Audio device singleton implementation.
 */

#include "config.h"
#include "grl-audio-device.h"
#include "grl-audio-queue-private.h"
#include "../grl-enums.h"
#include <raylib.h>
#include <string.h>

/**
 * SECTION:grl-audio-device
 * @title: GrlAudioDevice
 * @short_description: Audio device management singleton
 *
 * #GrlAudioDevice is a singleton that manages the audio subsystem.
 * It must be initialized before any sounds or music can be played.
 *
 * The audio device is typically accessed via grl_audio_device_get_default(),
 * which automatically initializes the device on first access.
 *
 * # Example
 *
 * |[<!-- language="C" -->
 * GrlAudioDevice *audio = grl_audio_device_get_default ();
 *
 * if (grl_audio_device_is_ready (audio))
 * {
 *     grl_audio_device_set_master_volume (audio, 0.8f);
 * }
 * ]|
 *
 * # Threads
 *
 * While the device is open, graylib runs one audio thread (`grl-audio`)
 * that applies the calls made on sounds, background music and callback
 * streams, so the game thread never waits on raylib's audio lock (see
 * #GrlSound).  It starts when the device opens and is joined, after
 * applying everything queued, before the device closes.
 *
 * # Backends
 *
 * On Linux and the BSDs the device opens through ALSA first (PipeWire is
 * reached through pipewire-alsa) and through PulseAudio only if that
 * fails; see #GrlAudioBackend, grl_audio_device_set_preferred_backend()
 * and the `GRL_AUDIO_BACKEND` environment variable.  If no backend opens
 * a device, grl_audio_device_init_audio() returns %FALSE, a message says
 * so, and every sound, music and stream call is silently inert: the game
 * keeps running without sound.
 */

typedef struct
{
    gboolean        is_initialized;
    gfloat          master_volume;
} GrlAudioDevicePrivate;

G_DEFINE_TYPE_WITH_PRIVATE (GrlAudioDevice, grl_audio_device, G_TYPE_OBJECT)

enum
{
    PROP_0,
    PROP_IS_READY,
    PROP_MASTER_VOLUME,
    N_PROPS
};

static GParamSpec *properties[N_PROPS];

/* Singleton instance */
static GrlAudioDevice *default_audio_device = NULL;

static void
grl_audio_device_finalize (GObject *object)
{
    GrlAudioDevice *self = GRL_AUDIO_DEVICE (object);
    GrlAudioDevicePrivate *priv = grl_audio_device_get_instance_private (self);

    if (priv->is_initialized)
    {
        /* the audio thread calls raylib: it finishes and stops first */
        grl_audio_queue_stop ();
        CloseAudioDevice ();
        priv->is_initialized = FALSE;
    }

    if (default_audio_device == self)
        default_audio_device = NULL;

    G_OBJECT_CLASS (grl_audio_device_parent_class)->finalize (object);
}

static void
grl_audio_device_get_property (GObject    *object,
                               guint       prop_id,
                               GValue     *value,
                               GParamSpec *pspec)
{
    GrlAudioDevice *self = GRL_AUDIO_DEVICE (object);
    GrlAudioDevicePrivate *priv = grl_audio_device_get_instance_private (self);

    switch (prop_id)
    {
    case PROP_IS_READY:
        g_value_set_boolean (value, priv->is_initialized);
        break;

    case PROP_MASTER_VOLUME:
        g_value_set_float (value, priv->master_volume);
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
grl_audio_device_set_property (GObject      *object,
                               guint         prop_id,
                               const GValue *value,
                               GParamSpec   *pspec)
{
    GrlAudioDevice *self = GRL_AUDIO_DEVICE (object);

    switch (prop_id)
    {
    case PROP_MASTER_VOLUME:
        grl_audio_device_set_master_volume (self, g_value_get_float (value));
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
grl_audio_device_class_init (GrlAudioDeviceClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->finalize = grl_audio_device_finalize;
    object_class->get_property = grl_audio_device_get_property;
    object_class->set_property = grl_audio_device_set_property;

    /**
     * GrlAudioDevice:is-ready:
     *
     * Whether the audio device is initialized and ready for use.
     */
    properties[PROP_IS_READY] =
        g_param_spec_boolean ("is-ready",
                              "Is Ready",
                              "Whether the audio device is ready",
                              FALSE,
                              G_PARAM_READABLE |
                              G_PARAM_STATIC_STRINGS);

    /**
     * GrlAudioDevice:master-volume:
     *
     * The master volume level (0.0 to 1.0).
     */
    properties[PROP_MASTER_VOLUME] =
        g_param_spec_float ("master-volume",
                            "Master Volume",
                            "The master volume level",
                            0.0f, 1.0f, 1.0f,
                            G_PARAM_READWRITE |
                            G_PARAM_STATIC_STRINGS |
                            G_PARAM_EXPLICIT_NOTIFY);

    g_object_class_install_properties (object_class, N_PROPS, properties);
}

static void
grl_audio_device_init (GrlAudioDevice *self)
{
    GrlAudioDevicePrivate *priv = grl_audio_device_get_instance_private (self);

    priv->is_initialized = FALSE;
    priv->master_volume = 1.0f;
}

/* ==========================================================================
 * Backend order
 *
 * raylib's InitAudioDevice() lets miniaudio try its backends in its fixed
 * order, PulseAudio before ALSA on Linux.  miniaudio 0.11.24's PulseAudio
 * backend dereferences a NULL sink info when libpulse reports an error
 * for the default-sink lookup in ma_device_init() (its callback ignores
 * end-of-list < 0, where libpulse passes no info), which PipeWire's pulse
 * server does now and then when many clients open streams: an
 * intermittent SIGSEGV inside InitAudioDevice().  raylib takes no backend
 * list and its sources are not changed here, so graylib steers miniaudio
 * instead: for ALSA first it points PULSE_SERVER at a socket that does
 * not exist for the duration of the call (libpulse then fails to connect
 * at once and miniaudio moves on to ALSA), and restores the variable
 * right after.  PulseAudio is tried, in miniaudio's own order, only when
 * ALSA did not open a device.
 * ========================================================================== */

#if defined(G_OS_UNIX) && !defined(__APPLE__) && !defined(__ANDROID__) && !defined(__EMSCRIPTEN__)
#define GRL_AUDIO_HAS_PULSE_CHOICE (1)
#endif

#define NO_PULSE_SERVER "unix:/nonexistent/graylib-no-pulseaudio"

static GrlAudioBackend preferred_backend = GRL_AUDIO_BACKEND_AUTO;

/*
 * grl_audio_backend_parse:
 * @name: (nullable): a backend name
 * @backend: (out): where to store it
 *
 * Reads a backend name as `GRL_AUDIO_BACKEND` takes it: `auto`, `alsa`,
 * `pipewire` (ALSA, through pipewire-alsa), `pulse` or `pulseaudio`,
 * `none` (also `off`, `null`, `0`), ignoring case.
 *
 * Returns: %TRUE if @name names a backend
 */
gboolean
grl_audio_backend_parse (const gchar     *name,
                         GrlAudioBackend *backend)
{
    g_return_val_if_fail (backend != NULL, FALSE);

    if (name == NULL || *name == '\0')
        return FALSE;
    if (g_ascii_strcasecmp (name, "auto") == 0 || g_ascii_strcasecmp (name, "default") == 0)
        *backend = GRL_AUDIO_BACKEND_AUTO;
    else if (g_ascii_strcasecmp (name, "alsa") == 0 || g_ascii_strcasecmp (name, "pipewire") == 0)
        *backend = GRL_AUDIO_BACKEND_ALSA;
    else if (g_ascii_strcasecmp (name, "pulse") == 0 || g_ascii_strcasecmp (name, "pulseaudio") == 0)
        *backend = GRL_AUDIO_BACKEND_PULSEAUDIO;
    else if (g_ascii_strcasecmp (name, "none") == 0 || g_ascii_strcasecmp (name, "off") == 0 ||
             g_ascii_strcasecmp (name, "null") == 0 || strcmp (name, "0") == 0)
        *backend = GRL_AUDIO_BACKEND_NONE;
    else
        return FALSE;
    return TRUE;
}

/*
 * grl_audio_device_set_preferred_backend:
 * @backend: the backend to open the device through
 *
 * Sets the backend the next grl_audio_device_init_audio() (or the first
 * grl_audio_device_get_default()) opens the device through; an open
 * device is not reopened.  `GRL_AUDIO_BACKEND` in the environment wins
 * over this.
 */
void
grl_audio_device_set_preferred_backend (GrlAudioBackend backend)
{
    g_return_if_fail (backend >= GRL_AUDIO_BACKEND_AUTO && backend <= GRL_AUDIO_BACKEND_NONE);

    preferred_backend = backend;
}

/*
 * grl_audio_device_get_preferred_backend:
 *
 * Gets the backend the device will open through: `GRL_AUDIO_BACKEND`
 * when it names one, else what grl_audio_device_set_preferred_backend()
 * set (%GRL_AUDIO_BACKEND_AUTO by default).
 *
 * Returns: the effective preference
 */
GrlAudioBackend
grl_audio_device_get_preferred_backend (void)
{
    GrlAudioBackend from_env;
    const gchar    *value;

    value = g_getenv ("GRL_AUDIO_BACKEND");
    if (value != NULL && *value != '\0')
    {
        if (grl_audio_backend_parse (value, &from_env))
            return from_env;
        g_message ("graylib: unknown GRL_AUDIO_BACKEND=%s (auto, alsa, pipewire, pulseaudio, none); using auto",
                   value);
        return GRL_AUDIO_BACKEND_AUTO;
    }
    return preferred_backend;
}

/* raylib's bool read through a byte (graylib's bool/gboolean rule) */
static gboolean
device_ready (void)
{
    unsigned char raw;

    raw = IsAudioDeviceReady ();
    return raw != 0;
}

#ifdef GRL_AUDIO_HAS_PULSE_CHOICE
/* InitAudioDevice() with miniaudio's PulseAudio backend failing at once */
static gboolean
device_open_without_pulse (void)
{
    g_autofree gchar *saved = NULL;
    gboolean          ready;

    saved = g_strdup (g_getenv ("PULSE_SERVER"));
    g_setenv ("PULSE_SERVER", NO_PULSE_SERVER, TRUE);
    InitAudioDevice ();
    if (saved != NULL)
        g_setenv ("PULSE_SERVER", saved, TRUE);
    else
        g_unsetenv ("PULSE_SERVER");

    ready = device_ready ();
    return ready;
}
#endif

/* opens raylib's device through @backend; whether it is ready */
static gboolean
device_open (GrlAudioBackend backend)
{
    if (backend == GRL_AUDIO_BACKEND_NONE)
        return FALSE;

#ifdef GRL_AUDIO_HAS_PULSE_CHOICE
    if (backend == GRL_AUDIO_BACKEND_AUTO || backend == GRL_AUDIO_BACKEND_ALSA)
    {
        if (device_open_without_pulse ())
            return TRUE;
        if (backend == GRL_AUDIO_BACKEND_ALSA)
            return FALSE;
        g_message ("graylib: ALSA opened no audio device; trying PulseAudio");
    }
#endif

    /* miniaudio's own order (PulseAudio first where it exists) */
    InitAudioDevice ();
    return device_ready ();
}

/*
 * grl_audio_device_get_default:
 *
 * Gets the default audio device singleton. The audio device is created
 * and initialized on first call.
 *
 * Returns: (transfer none): The default #GrlAudioDevice
 */
GrlAudioDevice *
grl_audio_device_get_default (void)
{
    if (default_audio_device == NULL)
    {
        default_audio_device = g_object_new (GRL_TYPE_AUDIO_DEVICE, NULL);
        grl_audio_device_init_audio (default_audio_device);
    }

    return default_audio_device;
}

/*
 * grl_audio_device_init_audio:
 * @self: A #GrlAudioDevice
 *
 * Initializes the audio device. This is called automatically by
 * grl_audio_device_get_default(), but can be called explicitly
 * for manual control.
 *
 * Returns: %TRUE if initialization succeeded
 */
gboolean
grl_audio_device_init_audio (GrlAudioDevice *self)
{
    GrlAudioDevicePrivate *priv;
    GrlAudioBackend        backend;

    g_return_val_if_fail (GRL_IS_AUDIO_DEVICE (self), FALSE);

    priv = grl_audio_device_get_instance_private (self);

    if (priv->is_initialized)
        return TRUE;

    backend = grl_audio_device_get_preferred_backend ();
    if (backend == GRL_AUDIO_BACKEND_NONE)
    {
        g_message ("graylib: audio is off (backend 'none'); running without sound");
        return FALSE;
    }

    if (device_open (backend))
    {
        priv->is_initialized = TRUE;
        SetMasterVolume (priv->master_volume);
        grl_audio_queue_start ();
        g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_IS_READY]);
        return TRUE;
    }

    /* not fatal: every audio call is inert without a device */
    g_message ("graylib: could not open an audio device (backend '%s'); running without sound. "
               "Set GRL_AUDIO_BACKEND (auto, alsa, pulseaudio) to try another backend.",
               backend == GRL_AUDIO_BACKEND_ALSA ? "alsa" :
               backend == GRL_AUDIO_BACKEND_PULSEAUDIO ? "pulseaudio" : "auto");
    return FALSE;
}

/*
 * grl_audio_device_close:
 * @self: A #GrlAudioDevice
 *
 * Closes the audio device and releases all audio resources.
 * After calling this, the audio device can be reinitialized
 * with grl_audio_device_init_audio().
 */
void
grl_audio_device_close (GrlAudioDevice *self)
{
    GrlAudioDevicePrivate *priv;

    g_return_if_fail (GRL_IS_AUDIO_DEVICE (self));

    priv = grl_audio_device_get_instance_private (self);

    if (priv->is_initialized)
    {
        /* the audio thread calls raylib: it finishes and stops first */
        grl_audio_queue_stop ();
        CloseAudioDevice ();
        priv->is_initialized = FALSE;
        g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_IS_READY]);
    }
}

/*
 * grl_audio_device_is_ready:
 * @self: A #GrlAudioDevice
 *
 * Checks if the audio device is initialized and ready.
 *
 * Returns: %TRUE if the audio device is ready
 */
gboolean
grl_audio_device_is_ready (GrlAudioDevice *self)
{
    GrlAudioDevicePrivate *priv;

    g_return_val_if_fail (GRL_IS_AUDIO_DEVICE (self), FALSE);

    priv = grl_audio_device_get_instance_private (self);

    return priv->is_initialized && IsAudioDeviceReady ();
}

/*
 * grl_audio_device_set_master_volume:
 * @self: A #GrlAudioDevice
 * @volume: Master volume (0.0 to 1.0)
 *
 * Sets the master volume for all audio output.
 */
void
grl_audio_device_set_master_volume (GrlAudioDevice *self,
                                    gfloat          volume)
{
    GrlAudioDevicePrivate *priv;

    g_return_if_fail (GRL_IS_AUDIO_DEVICE (self));

    priv = grl_audio_device_get_instance_private (self);

    volume = CLAMP (volume, 0.0f, 1.0f);

    if (priv->master_volume != volume)
    {
        priv->master_volume = volume;

        if (priv->is_initialized)
            SetMasterVolume (volume);

        g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_MASTER_VOLUME]);
    }
}

/*
 * grl_audio_device_get_master_volume:
 * @self: A #GrlAudioDevice
 *
 * Gets the current master volume setting.
 *
 * Returns: The master volume (0.0 to 1.0)
 */
gfloat
grl_audio_device_get_master_volume (GrlAudioDevice *self)
{
    GrlAudioDevicePrivate *priv;

    g_return_val_if_fail (GRL_IS_AUDIO_DEVICE (self), 1.0f);

    priv = grl_audio_device_get_instance_private (self);

    return priv->master_volume;
}

/*
 * grl_audio_device_get_master_volume_actual:
 * @self: A #GrlAudioDevice
 *
 * Gets the actual master volume from the audio system.
 *
 * Returns: The actual master volume (0.0 to 1.0)
 */
gfloat
grl_audio_device_get_master_volume_actual (GrlAudioDevice *self)
{
    GrlAudioDevicePrivate *priv;

    g_return_val_if_fail (GRL_IS_AUDIO_DEVICE (self), 1.0f);

    priv = grl_audio_device_get_instance_private (self);

    if (!priv->is_initialized)
        return priv->master_volume;

    return GetMasterVolume ();
}

/*
 * grl_audio_device_set_default_buffer_size:
 * @self: A #GrlAudioDevice
 * @size: The default buffer size in samples
 *
 * Sets the default buffer size for new audio streams.
 */
void
grl_audio_device_set_default_buffer_size (GrlAudioDevice *self,
                                          gint            size)
{
    g_return_if_fail (GRL_IS_AUDIO_DEVICE (self));
    g_return_if_fail (size > 0);

    SetAudioStreamBufferSizeDefault (size);
}
