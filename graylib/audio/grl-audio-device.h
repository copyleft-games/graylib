/* grl-audio-device.h
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Audio device singleton for managing the audio subsystem.
 */

#pragma once

#if !defined(GRAYLIB_INSIDE) && !defined(GRAYLIB_COMPILATION)
#error "Only <graylib.h> can be included directly."
#endif

#include <glib-object.h>
#include "../grl-version.h"

G_BEGIN_DECLS

#define GRL_TYPE_AUDIO_DEVICE (grl_audio_device_get_type ())

GRL_AVAILABLE_IN_ALL
G_DECLARE_DERIVABLE_TYPE (GrlAudioDevice, grl_audio_device, GRL, AUDIO_DEVICE, GObject)

/**
 * GrlAudioDeviceClass:
 * @parent_class: The parent class
 *
 * The class structure for #GrlAudioDevice.
 */
struct _GrlAudioDeviceClass
{
    GObjectClass parent_class;

    /*< private >*/
    gpointer _reserved[8];
};

/*
 * Singleton access
 */

/**
 * grl_audio_device_get_default:
 *
 * Gets the default audio device singleton. The audio device is created
 * and initialized on first call.
 *
 * Returns: (transfer none): The default #GrlAudioDevice
 */
GRL_AVAILABLE_IN_ALL
GrlAudioDevice *    grl_audio_device_get_default        (void);

/*
 * Lifecycle
 */

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
GRL_AVAILABLE_IN_ALL
gboolean            grl_audio_device_init_audio         (GrlAudioDevice     *self);

/**
 * grl_audio_device_close:
 * @self: A #GrlAudioDevice
 *
 * Closes the audio device and releases all audio resources.
 * After calling this, the audio device can be reinitialized
 * with grl_audio_device_init_audio().
 */
GRL_AVAILABLE_IN_ALL
void                grl_audio_device_close              (GrlAudioDevice     *self);

/**
 * grl_audio_device_is_ready:
 * @self: A #GrlAudioDevice
 *
 * Checks if the audio device is initialized and ready.
 *
 * Returns: %TRUE if the audio device is ready
 */
GRL_AVAILABLE_IN_ALL
gboolean            grl_audio_device_is_ready           (GrlAudioDevice     *self);

/*
 * Master volume control
 */

/**
 * grl_audio_device_set_master_volume:
 * @self: A #GrlAudioDevice
 * @volume: Master volume (0.0 to 1.0)
 *
 * Sets the master volume for all audio output.
 */
GRL_AVAILABLE_IN_ALL
void                grl_audio_device_set_master_volume  (GrlAudioDevice     *self,
                                                         gfloat              volume);

/**
 * grl_audio_device_get_master_volume:
 * @self: A #GrlAudioDevice
 *
 * Gets the current master volume.
 *
 * Returns: The master volume (0.0 to 1.0)
 */
GRL_AVAILABLE_IN_ALL
gfloat              grl_audio_device_get_master_volume  (GrlAudioDevice     *self);

G_END_DECLS
