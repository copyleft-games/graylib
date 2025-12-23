/* grl-texture.h
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * GPU texture type for rendering images.
 */

#pragma once

#if !defined(GRAYLIB_INSIDE) && !defined(GRAYLIB_COMPILATION)
#error "Only <graylib.h> can be included directly."
#endif

#include <glib-object.h>
#include "../grl-version.h"
#include "../grl-enums.h"
#include "grl-image.h"

G_BEGIN_DECLS

#define GRL_TYPE_TEXTURE (grl_texture_get_type ())

GRL_AVAILABLE_IN_ALL
G_DECLARE_FINAL_TYPE (GrlTexture, grl_texture, GRL, TEXTURE, GObject)

/*
 * Constructors
 */

/**
 * grl_texture_new_from_file:
 * @filename: Path to image file to load
 *
 * Creates a new texture by loading an image file directly to GPU memory.
 * Supports most common image formats (PNG, JPG, BMP, TGA, etc.).
 *
 * Returns: (transfer full): A new #GrlTexture, or %NULL on error
 */
GRL_AVAILABLE_IN_ALL
GrlTexture *        grl_texture_new_from_file   (const gchar        *filename);

/**
 * grl_texture_new_from_image:
 * @image: A #GrlImage to upload to GPU
 *
 * Creates a new texture by uploading a CPU-side image to GPU memory.
 * This is the recommended way to create textures when you need to
 * manipulate the image data before rendering.
 *
 * Returns: (transfer full): A new #GrlTexture
 */
GRL_AVAILABLE_IN_ALL
GrlTexture *        grl_texture_new_from_image  (GrlImage           *image);

/**
 * grl_texture_new_from_memory:
 * @file_type: Image format hint (e.g. ".png", ".jpg")
 * @data: Raw image file data
 * @data_size: Size of @data in bytes
 *
 * Creates a new texture from raw image file data in memory.
 *
 * Returns: (transfer full): A new #GrlTexture, or %NULL on error
 */
GRL_AVAILABLE_IN_ALL
GrlTexture *        grl_texture_new_from_memory (const gchar        *file_type,
                                                 const guint8       *data,
                                                 gsize               data_size);

/*
 * Properties
 */

/**
 * grl_texture_get_width:
 * @self: A #GrlTexture
 *
 * Gets the width of the texture in pixels.
 *
 * Returns: Texture width in pixels
 */
GRL_AVAILABLE_IN_ALL
gint                grl_texture_get_width       (GrlTexture         *self);

/**
 * grl_texture_get_height:
 * @self: A #GrlTexture
 *
 * Gets the height of the texture in pixels.
 *
 * Returns: Texture height in pixels
 */
GRL_AVAILABLE_IN_ALL
gint                grl_texture_get_height      (GrlTexture         *self);

/**
 * grl_texture_get_mipmaps:
 * @self: A #GrlTexture
 *
 * Gets the number of mipmap levels in the texture.
 *
 * Returns: Number of mipmap levels (1 = base only)
 */
GRL_AVAILABLE_IN_ALL
gint                grl_texture_get_mipmaps     (GrlTexture         *self);

/**
 * grl_texture_get_format:
 * @self: A #GrlTexture
 *
 * Gets the pixel format of the texture.
 *
 * Returns: The #GrlPixelFormat of the texture
 */
GRL_AVAILABLE_IN_ALL
GrlPixelFormat      grl_texture_get_format      (GrlTexture         *self);

/**
 * grl_texture_is_valid:
 * @self: A #GrlTexture
 *
 * Checks if the texture was loaded successfully and is ready for use.
 *
 * Returns: %TRUE if valid, %FALSE otherwise
 */
GRL_AVAILABLE_IN_ALL
gboolean            grl_texture_is_valid        (GrlTexture         *self);

/*
 * Texture configuration
 */

/**
 * grl_texture_gen_mipmaps:
 * @self: A #GrlTexture
 *
 * Generates mipmaps for the texture on the GPU.
 * Mipmaps improve rendering quality when textures are scaled down.
 */
GRL_AVAILABLE_IN_ALL
void                grl_texture_gen_mipmaps     (GrlTexture         *self);

/**
 * grl_texture_set_filter:
 * @self: A #GrlTexture
 * @filter: The #GrlTextureFilter mode to use
 *
 * Sets the texture filtering mode. This affects how the texture
 * looks when scaled.
 *
 * - %GRL_TEXTURE_FILTER_POINT: Pixelated look (nearest neighbor)
 * - %GRL_TEXTURE_FILTER_BILINEAR: Smooth scaling
 * - %GRL_TEXTURE_FILTER_TRILINEAR: Smooth with mipmaps
 */
GRL_AVAILABLE_IN_ALL
void                grl_texture_set_filter      (GrlTexture         *self,
                                                 GrlTextureFilter    filter);

/**
 * grl_texture_set_wrap:
 * @self: A #GrlTexture
 * @wrap: The #GrlTextureWrap mode to use
 *
 * Sets the texture wrapping mode. This affects how texture coordinates
 * outside [0,1] are handled.
 *
 * - %GRL_TEXTURE_WRAP_REPEAT: Tile the texture
 * - %GRL_TEXTURE_WRAP_CLAMP: Extend edge pixels
 * - %GRL_TEXTURE_WRAP_MIRROR_REPEAT: Mirror and tile
 */
GRL_AVAILABLE_IN_ALL
void                grl_texture_set_wrap        (GrlTexture         *self,
                                                 GrlTextureWrap      wrap);

/*
 * Texture updates
 */

/**
 * grl_texture_update:
 * @self: A #GrlTexture
 * @image: A #GrlImage with new pixel data
 *
 * Updates the texture data on the GPU with new image data.
 * The image must have the same dimensions as the texture.
 */
GRL_AVAILABLE_IN_ALL
void                grl_texture_update          (GrlTexture         *self,
                                                 GrlImage           *image);

/**
 * grl_texture_update_rec:
 * @self: A #GrlTexture
 * @rect: The region to update
 * @pixels: Raw pixel data matching the texture format
 *
 * Updates a rectangular region of the texture with new pixel data.
 */
GRL_AVAILABLE_IN_ALL
void                grl_texture_update_rec      (GrlTexture         *self,
                                                 const GrlRectangle *rect,
                                                 const guint8       *pixels);

/*
 * Conversion
 */

/**
 * grl_texture_to_image:
 * @self: A #GrlTexture
 *
 * Downloads the texture data from GPU to create a CPU-side image.
 * This is useful for saving textures or further manipulation.
 *
 * Returns: (transfer full): A new #GrlImage with texture data
 */
GRL_AVAILABLE_IN_ALL
GrlImage *          grl_texture_to_image        (GrlTexture         *self);

/*
 * Internal - for library use only
 */

gpointer            grl_texture_get_handle      (GrlTexture         *self);

/**
 * grl_texture_new_from_handle:
 * @handle: A pointer to a raylib Texture2D structure
 *
 * Creates a GrlTexture wrapper around an existing raylib texture.
 * This is used internally by GrlRenderTexture. The texture data
 * is NOT owned by this wrapper - do not unload it.
 *
 * Returns: (transfer full): A new #GrlTexture wrapping the handle
 */
GrlTexture *        grl_texture_new_from_handle (gpointer            handle);

G_END_DECLS
