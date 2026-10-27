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

/*
 * grl_rlgl_load_texture:
 * @data: (nullable): pixel data, every level packed largest first
 * @width: width of level 0
 * @height: height of level 0
 * @format: the pixel format
 * @mipmap_count: the number of levels
 *
 * Uploads raw or block-compressed texture data.
 *
 * Returns: the GL texture id, or 0 on failure
 */
guint
grl_rlgl_load_texture (gconstpointer  data,
                       gint           width,
                       gint           height,
                       GrlPixelFormat format,
                       gint           mipmap_count)
{
    g_return_val_if_fail (width > 0 && height > 0, 0);
    g_return_val_if_fail (mipmap_count >= 1, 0);

    return rlLoadTexture (data, width, height, (int) format, mipmap_count);
}

/*
 * grl_rlgl_update_texture:
 * @id: a texture id
 * @offset_x: left of the region
 * @offset_y: top of the region
 * @width: region width
 * @height: region height
 * @format: the pixel format
 * @data: the region's pixels
 *
 * Replaces a region of level 0.
 */
void
grl_rlgl_update_texture (guint          id,
                         gint           offset_x,
                         gint           offset_y,
                         gint           width,
                         gint           height,
                         GrlPixelFormat format,
                         gconstpointer  data)
{
    g_return_if_fail (data != NULL);

    rlUpdateTexture (id, offset_x, offset_y, width, height, (int) format, data);
}

/*
 * grl_rlgl_unload_texture:
 * @id: a texture id
 *
 * Frees a texture made with grl_rlgl_load_texture().
 */
void
grl_rlgl_unload_texture (guint id)
{
    rlUnloadTexture (id);
}
