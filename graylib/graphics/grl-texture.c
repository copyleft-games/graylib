/* grl-texture.c
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * GPU texture implementation.
 */

#include "config.h"
#include "grl-texture.h"
#include "grl-graphics-private.h"
#include <raylib.h>

/**
 * SECTION:grl-texture
 * @title: GrlTexture
 * @short_description: GPU texture for rendering
 *
 * #GrlTexture represents an image stored in GPU memory (VRAM). Textures
 * can be loaded from files, created from #GrlImage objects, or loaded
 * from raw memory.
 *
 * Unlike #GrlImage, textures cannot be directly manipulated. To modify
 * a texture, download it to an image with grl_texture_to_image(),
 * modify the image, then upload again with grl_texture_new_from_image().
 *
 * Example - Loading and drawing a texture:
 * |[<!-- language="C" -->
 * g_autoptr(GrlTexture) tex = grl_texture_new_from_file ("sprite.png");
 * grl_texture_set_filter (tex, GRL_TEXTURE_FILTER_POINT);
 *
 * // In draw loop:
 * grl_draw_texture (tex, 100, 100, white);
 * ]|
 *
 * Example - Creating texture from manipulated image:
 * |[<!-- language="C" -->
 * g_autoptr(GrlImage) img = grl_image_new_from_file ("tileset.png");
 * grl_image_resize (img, 256, 256);
 * grl_image_gen_mipmaps (img);
 *
 * g_autoptr(GrlTexture) tex = grl_texture_new_from_image (img);
 * grl_texture_set_filter (tex, GRL_TEXTURE_FILTER_TRILINEAR);
 * ]|
 */

struct _GrlTexture
{
    GObject parent_instance;

    Texture2D handle;
    gboolean valid;
    gboolean owns_handle;  /* TRUE if we should unload the texture on finalize */
};

G_DEFINE_TYPE (GrlTexture, grl_texture, G_TYPE_OBJECT)

enum
{
    PROP_0,
    PROP_WIDTH,
    PROP_HEIGHT,
    PROP_FORMAT,
    PROP_MIPMAPS,
    PROP_VALID,
    N_PROPS
};

static GParamSpec *properties[N_PROPS];

/*
 * Helper macros
 */

#define GRL_TO_RAYLIB_RECTANGLE(r) \
    ((Rectangle){ .x = (r)->x, .y = (r)->y, .width = (r)->width, .height = (r)->height })

/*
 * Private helper to create GrlTexture from raylib Texture2D
 */

static GrlTexture *
grl_texture_new_internal (Texture2D handle,
                          gboolean  owns_handle)
{
    GrlTexture *self;

    self = g_object_new (GRL_TYPE_TEXTURE, NULL);
    self->handle = handle;
    self->valid = IsTextureValid (handle);
    self->owns_handle = owns_handle;

    return self;
}

/*
 * GObject virtual methods
 */

static void
grl_texture_get_property (GObject    *object,
                          guint       prop_id,
                          GValue     *value,
                          GParamSpec *pspec)
{
    GrlTexture *self = GRL_TEXTURE (object);

    switch (prop_id)
    {
        case PROP_WIDTH:
            g_value_set_int (value, self->handle.width);
            break;
        case PROP_HEIGHT:
            g_value_set_int (value, self->handle.height);
            break;
        case PROP_FORMAT:
            g_value_set_enum (value, (GrlPixelFormat)self->handle.format);
            break;
        case PROP_MIPMAPS:
            g_value_set_int (value, self->handle.mipmaps);
            break;
        case PROP_VALID:
            g_value_set_boolean (value, self->valid);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
grl_texture_finalize (GObject *object)
{
    GrlTexture *self = GRL_TEXTURE (object);

    /*
     * Unload texture from GPU memory if we own it.
     * We check id > 0 because raylib uses 0 for invalid textures.
     * Textures created from render texture handles do NOT own their data.
     */
    if (self->owns_handle && self->valid && self->handle.id > 0)
    {
        UnloadTexture (self->handle);
        self->handle.id = 0;
        self->valid = FALSE;
    }

    G_OBJECT_CLASS (grl_texture_parent_class)->finalize (object);
}

static void
grl_texture_class_init (GrlTextureClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->get_property = grl_texture_get_property;
    object_class->finalize = grl_texture_finalize;

    /**
     * GrlTexture:width:
     *
     * The width of the texture in pixels.
     */
    properties[PROP_WIDTH] =
        g_param_spec_int ("width", NULL, NULL,
                          0, G_MAXINT, 0,
                          G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

    /**
     * GrlTexture:height:
     *
     * The height of the texture in pixels.
     */
    properties[PROP_HEIGHT] =
        g_param_spec_int ("height", NULL, NULL,
                          0, G_MAXINT, 0,
                          G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

    /**
     * GrlTexture:format:
     *
     * The pixel format of the texture.
     */
    properties[PROP_FORMAT] =
        g_param_spec_enum ("format", NULL, NULL,
                           GRL_TYPE_PIXEL_FORMAT,
                           GRL_PIXELFORMAT_UNCOMPRESSED_R8G8B8A8,
                           G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

    /**
     * GrlTexture:mipmaps:
     *
     * The number of mipmap levels in the texture.
     */
    properties[PROP_MIPMAPS] =
        g_param_spec_int ("mipmaps", NULL, NULL,
                          0, G_MAXINT, 1,
                          G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

    /**
     * GrlTexture:valid:
     *
     * Whether the texture is valid and ready for use.
     */
    properties[PROP_VALID] =
        g_param_spec_boolean ("valid", NULL, NULL,
                              FALSE,
                              G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

    g_object_class_install_properties (object_class, N_PROPS, properties);
}

static void
grl_texture_init (GrlTexture *self)
{
    self->handle = (Texture2D){ 0 };
    self->valid = FALSE;
}

/*
 * =============================================================================
 * Constructors
 * =============================================================================
 */

/**
 * grl_texture_new_from_file:
 * @filename: Path to image file to load
 *
 * Creates a new texture by loading an image file directly to GPU memory.
 *
 * Returns: (transfer full): A new #GrlTexture
 */
GrlTexture *
grl_texture_new_from_file (const gchar *filename)
{
    Texture2D tex;

    g_return_val_if_fail (filename != NULL, NULL);

    tex = LoadTexture (filename);

    return grl_texture_new_internal (tex, TRUE);
}

/**
 * grl_texture_new_from_image:
 * @image: A #GrlImage to upload to GPU
 *
 * Creates a new texture from a CPU-side image.
 *
 * Returns: (transfer full): A new #GrlTexture
 */
GrlTexture *
grl_texture_new_from_image (GrlImage *image)
{
    Image *img_handle;
    Texture2D tex;

    g_return_val_if_fail (GRL_IS_IMAGE (image), NULL);

    img_handle = (Image *)grl_image_get_handle (image);
    tex = LoadTextureFromImage (*img_handle);

    return grl_texture_new_internal (tex, TRUE);
}

/**
 * grl_texture_new_from_memory:
 * @file_type: Image format hint (e.g. ".png", ".jpg")
 * @data: (array length=data_size): Raw image file data
 * @data_size: Size of @data in bytes
 *
 * Creates a new texture from raw image file data in memory.
 *
 * Returns: (transfer full): A new #GrlTexture
 */
GrlTexture *
grl_texture_new_from_memory (const gchar  *file_type,
                             const guint8 *data,
                             gsize         data_size)
{
    Image img;
    Texture2D tex;

    g_return_val_if_fail (file_type != NULL, NULL);
    g_return_val_if_fail (data != NULL, NULL);
    g_return_val_if_fail (data_size > 0, NULL);

    /* Load image from memory, then upload to GPU, then free the image */
    img = LoadImageFromMemory (file_type, (const unsigned char *)data, (int)data_size);
    tex = LoadTextureFromImage (img);
    UnloadImage (img);

    return grl_texture_new_internal (tex, TRUE);
}

/*
 * =============================================================================
 * Properties
 * =============================================================================
 */

/**
 * grl_texture_get_width:
 * @self: A #GrlTexture
 *
 * Gets the width of the texture in pixels.
 *
 * Returns: Texture width
 */
gint
grl_texture_get_width (GrlTexture *self)
{
    g_return_val_if_fail (GRL_IS_TEXTURE (self), 0);

    return self->handle.width;
}

/**
 * grl_texture_get_height:
 * @self: A #GrlTexture
 *
 * Gets the height of the texture in pixels.
 *
 * Returns: Texture height
 */
gint
grl_texture_get_height (GrlTexture *self)
{
    g_return_val_if_fail (GRL_IS_TEXTURE (self), 0);

    return self->handle.height;
}

/**
 * grl_texture_get_mipmaps:
 * @self: A #GrlTexture
 *
 * Gets the number of mipmap levels in the texture.
 *
 * Returns: Number of mipmap levels
 */
gint
grl_texture_get_mipmaps (GrlTexture *self)
{
    g_return_val_if_fail (GRL_IS_TEXTURE (self), 0);

    return self->handle.mipmaps;
}

/**
 * grl_texture_get_format:
 * @self: A #GrlTexture
 *
 * Gets the pixel format of the texture.
 *
 * Returns: The #GrlPixelFormat
 */
GrlPixelFormat
grl_texture_get_format (GrlTexture *self)
{
    g_return_val_if_fail (GRL_IS_TEXTURE (self), GRL_PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);

    return (GrlPixelFormat)self->handle.format;
}

/**
 * grl_texture_is_valid:
 * @self: A #GrlTexture
 *
 * Checks if the texture is valid and ready for use.
 *
 * Returns: %TRUE if valid
 */
gboolean
grl_texture_is_valid (GrlTexture *self)
{
    g_return_val_if_fail (GRL_IS_TEXTURE (self), FALSE);

    return self->valid;
}

/*
 * =============================================================================
 * Texture Configuration
 * =============================================================================
 */

/**
 * grl_texture_gen_mipmaps:
 * @self: A #GrlTexture
 *
 * Generates mipmaps for the texture on the GPU.
 */
void
grl_texture_gen_mipmaps (GrlTexture *self)
{
    g_return_if_fail (GRL_IS_TEXTURE (self));
    g_return_if_fail (self->valid);

    GenTextureMipmaps (&self->handle);
}

/**
 * grl_texture_set_filter:
 * @self: A #GrlTexture
 * @filter: The #GrlTextureFilter mode
 *
 * Sets the texture filtering mode.
 */
void
grl_texture_set_filter (GrlTexture      *self,
                        GrlTextureFilter filter)
{
    g_return_if_fail (GRL_IS_TEXTURE (self));
    g_return_if_fail (self->valid);

    SetTextureFilter (self->handle, (int)filter);
}

/**
 * grl_texture_set_wrap:
 * @self: A #GrlTexture
 * @wrap: The #GrlTextureWrap mode
 *
 * Sets the texture wrapping mode.
 */
void
grl_texture_set_wrap (GrlTexture    *self,
                      GrlTextureWrap wrap)
{
    g_return_if_fail (GRL_IS_TEXTURE (self));
    g_return_if_fail (self->valid);

    SetTextureWrap (self->handle, (int)wrap);
}

/*
 * =============================================================================
 * Texture Updates
 * =============================================================================
 */

/**
 * grl_texture_update:
 * @self: A #GrlTexture
 * @image: A #GrlImage with new pixel data
 *
 * Updates the texture with new image data.
 * The image must match the texture dimensions.
 */
void
grl_texture_update (GrlTexture *self,
                    GrlImage   *image)
{
    Image *img_handle;

    g_return_if_fail (GRL_IS_TEXTURE (self));
    g_return_if_fail (GRL_IS_IMAGE (image));
    g_return_if_fail (self->valid);

    img_handle = (Image *)grl_image_get_handle (image);
    UpdateTexture (self->handle, img_handle->data);
}

/**
 * grl_texture_update_rec:
 * @self: A #GrlTexture
 * @rect: The region to update
 * @pixels: (array): Raw pixel data
 *
 * Updates a rectangular region of the texture.
 */
void
grl_texture_update_rec (GrlTexture         *self,
                        const GrlRectangle *rect,
                        const guint8       *pixels)
{
    g_return_if_fail (GRL_IS_TEXTURE (self));
    g_return_if_fail (rect != NULL);
    g_return_if_fail (pixels != NULL);
    g_return_if_fail (self->valid);

    UpdateTextureRec (self->handle, GRL_TO_RAYLIB_RECTANGLE (rect), pixels);
}

/*
 * =============================================================================
 * Conversion
 * =============================================================================
 */

/**
 * grl_texture_to_image:
 * @self: A #GrlTexture
 *
 * Downloads texture data from GPU to create a CPU-side image.
 *
 * Returns: (transfer full): A new #GrlImage
 */
GrlImage *
grl_texture_to_image (GrlTexture *self)
{
    Image img;

    g_return_val_if_fail (GRL_IS_TEXTURE (self), NULL);
    g_return_val_if_fail (self->valid, NULL);

    img = LoadImageFromTexture (self->handle);

    return grl_image_new_from_raw_handle (&img);
}

/*
 * =============================================================================
 * Internal
 * =============================================================================
 */

/**
 * grl_texture_get_handle:
 * @self: A #GrlTexture
 *
 * Gets the internal raylib Texture2D handle.
 * This is for internal use by drawing functions.
 *
 * Returns: (transfer none): Pointer to the Texture2D handle
 */
gpointer
grl_texture_get_handle (GrlTexture *self)
{
    g_return_val_if_fail (GRL_IS_TEXTURE (self), NULL);

    return &self->handle;
}

/**
 * grl_texture_new_from_handle:
 * @handle: A pointer to a raylib Texture2D structure
 *
 * Creates a GrlTexture wrapper around an existing raylib texture.
 * This is used internally by GrlRenderTexture. The texture data
 * is NOT owned by this wrapper and will NOT be unloaded on finalize.
 *
 * Returns: (transfer full): A new #GrlTexture wrapping the handle
 */
GrlTexture *
grl_texture_new_from_handle (gpointer handle)
{
    Texture2D *tex;

    g_return_val_if_fail (handle != NULL, NULL);

    tex = (Texture2D *)handle;

    /* Pass FALSE for owns_handle - we don't own this texture data */
    return grl_texture_new_internal (*tex, FALSE);
}
