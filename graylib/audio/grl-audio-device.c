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
#include <raylib.h>

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

/**
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

/**
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

    g_return_val_if_fail (GRL_IS_AUDIO_DEVICE (self), FALSE);

    priv = grl_audio_device_get_instance_private (self);

    if (priv->is_initialized)
        return TRUE;

    InitAudioDevice ();

    if (IsAudioDeviceReady ())
    {
        priv->is_initialized = TRUE;
        SetMasterVolume (priv->master_volume);
        g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_IS_READY]);
        return TRUE;
    }

    return FALSE;
}

/**
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
        CloseAudioDevice ();
        priv->is_initialized = FALSE;
        g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_IS_READY]);
    }
}

/**
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

/**
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

/**
 * grl_audio_device_get_master_volume:
 * @self: A #GrlAudioDevice
 *
 * Gets the current master volume.
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
