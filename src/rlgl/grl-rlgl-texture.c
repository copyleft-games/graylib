/* grl-rlgl-texture.c
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Texture binding operations for rlgl.
 */

#include "config.h"
#include "grl-rlgl.h"
#include <rlgl.h>

/**
 * grl_rlgl_active_texture_slot:
 * @slot: Texture slot to activate (0-7)
 *
 * Activates the specified texture slot for binding.
 */
void
grl_rlgl_active_texture_slot (gint slot)
{
    rlActiveTextureSlot (slot);
}

/**
 * grl_rlgl_enable_texture:
 * @id: Texture GPU ID
 *
 * Binds a 2D texture.
 */
void
grl_rlgl_enable_texture (guint id)
{
    rlEnableTexture (id);
}

/**
 * grl_rlgl_disable_texture:
 *
 * Unbinds the current 2D texture.
 */
void
grl_rlgl_disable_texture (void)
{
    rlDisableTexture ();
}

/**
 * grl_rlgl_enable_texture_cubemap:
 * @id: Cubemap texture GPU ID
 *
 * Binds a cubemap texture.
 */
void
grl_rlgl_enable_texture_cubemap (guint id)
{
    rlEnableTextureCubemap (id);
}

/**
 * grl_rlgl_disable_texture_cubemap:
 *
 * Unbinds the current cubemap texture.
 */
void
grl_rlgl_disable_texture_cubemap (void)
{
    rlDisableTextureCubemap ();
}

/**
 * grl_rlgl_texture_parameters:
 * @id: Texture GPU ID
 * @param: Parameter to set (GL constant)
 * @value: Value to set
 *
 * Sets texture parameters (filtering, wrapping, etc.).
 */
void
grl_rlgl_texture_parameters (guint id,
                             gint param,
                             gint value)
{
    rlTextureParameters (id, param, value);
}

/**
 * grl_rlgl_cubemap_parameters:
 * @id: Cubemap texture GPU ID
 * @param: Parameter to set (GL constant)
 * @value: Value to set
 *
 * Sets cubemap texture parameters.
 */
void
grl_rlgl_cubemap_parameters (guint id,
                             gint param,
                             gint value)
{
    rlCubemapParameters (id, param, value);
}
