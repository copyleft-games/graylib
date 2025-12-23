/* grl-render-texture.c
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#include "grl-render-texture.h"
#include "grl-texture.h"
#include <raylib.h>

/**
 * SECTION:grl-render-texture
 * @Title: GrlRenderTexture
 * @Short_description: Render texture for off-screen rendering
 *
 * #GrlRenderTexture provides a framebuffer object for rendering to a texture
 * instead of directly to the screen. This is useful for post-processing effects,
 * screen transitions, and other advanced rendering techniques.
 *
 * # Example Usage
 *
 * |[<!-- language="C" -->
 * g_autoptr(GrlRenderTexture) target = grl_render_texture_new (800, 600);
 *
 * // Draw to render texture
 * grl_render_texture_begin (target);
 * grl_draw_clear_background (bg_color);
 * // Draw game world here
 * grl_render_texture_end (target);
 *
 * // Draw render texture to screen (possibly with shader effects)
 * g_autoptr(GrlTexture) tex = grl_render_texture_get_texture (target);
 * // Note: Render textures are flipped vertically in OpenGL
 * ]|
 */

typedef struct
{
    RenderTexture2D render_texture;
    gint            width;
    gint            height;
    gboolean        valid;
} GrlRenderTexturePrivate;

G_DEFINE_TYPE_WITH_PRIVATE (GrlRenderTexture, grl_render_texture, G_TYPE_OBJECT)

enum
{
    PROP_0,
    PROP_WIDTH,
    PROP_HEIGHT,
    PROP_VALID,
    N_PROPS
};

static GParamSpec *properties[N_PROPS];

static void
grl_render_texture_finalize (GObject *object)
{
    GrlRenderTexture *self = GRL_RENDER_TEXTURE (object);
    GrlRenderTexturePrivate *priv = grl_render_texture_get_instance_private (self);

    if (priv->valid)
    {
        UnloadRenderTexture (priv->render_texture);
        priv->valid = FALSE;
    }

    G_OBJECT_CLASS (grl_render_texture_parent_class)->finalize (object);
}

static void
grl_render_texture_get_property (GObject    *object,
                                 guint       prop_id,
                                 GValue     *value,
                                 GParamSpec *pspec)
{
    GrlRenderTexture *self = GRL_RENDER_TEXTURE (object);
    GrlRenderTexturePrivate *priv = grl_render_texture_get_instance_private (self);

    switch (prop_id)
    {
    case PROP_WIDTH:
        g_value_set_int (value, priv->width);
        break;

    case PROP_HEIGHT:
        g_value_set_int (value, priv->height);
        break;

    case PROP_VALID:
        g_value_set_boolean (value, priv->valid);
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
grl_render_texture_class_init (GrlRenderTextureClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->finalize = grl_render_texture_finalize;
    object_class->get_property = grl_render_texture_get_property;

    /**
     * GrlRenderTexture:width:
     *
     * The width of the render texture in pixels.
     */
    properties[PROP_WIDTH] =
        g_param_spec_int ("width",
                          "Width",
                          "Render texture width",
                          0, G_MAXINT, 0,
                          G_PARAM_READABLE |
                          G_PARAM_STATIC_STRINGS);

    /**
     * GrlRenderTexture:height:
     *
     * The height of the render texture in pixels.
     */
    properties[PROP_HEIGHT] =
        g_param_spec_int ("height",
                          "Height",
                          "Render texture height",
                          0, G_MAXINT, 0,
                          G_PARAM_READABLE |
                          G_PARAM_STATIC_STRINGS);

    /**
     * GrlRenderTexture:valid:
     *
     * Whether the render texture is valid and ready to use.
     */
    properties[PROP_VALID] =
        g_param_spec_boolean ("valid",
                              "Valid",
                              "Whether render texture is valid",
                              FALSE,
                              G_PARAM_READABLE |
                              G_PARAM_STATIC_STRINGS);

    g_object_class_install_properties (object_class, N_PROPS, properties);
}

static void
grl_render_texture_init (GrlRenderTexture *self)
{
    GrlRenderTexturePrivate *priv = grl_render_texture_get_instance_private (self);

    priv->width = 0;
    priv->height = 0;
    priv->valid = FALSE;
}

/**
 * grl_render_texture_new:
 * @width: Width of the render texture
 * @height: Height of the render texture
 *
 * Creates a new render texture.
 *
 * Returns: (transfer full): A new #GrlRenderTexture
 */
GrlRenderTexture *
grl_render_texture_new (gint width,
                        gint height)
{
    GrlRenderTexture *self;
    GrlRenderTexturePrivate *priv;
    RenderTexture2D rt;

    g_return_val_if_fail (width > 0, NULL);
    g_return_val_if_fail (height > 0, NULL);

    self = g_object_new (GRL_TYPE_RENDER_TEXTURE, NULL);
    priv = grl_render_texture_get_instance_private (self);

    rt = LoadRenderTexture (width, height);

    if (IsRenderTextureValid (rt))
    {
        priv->render_texture = rt;
        priv->width = width;
        priv->height = height;
        priv->valid = TRUE;
    }
    else
    {
        g_warning ("Failed to create render texture of size %dx%d", width, height);
    }

    return self;
}

/**
 * grl_render_texture_is_valid:
 * @self: A #GrlRenderTexture
 *
 * Checks if the render texture is valid.
 *
 * Returns: %TRUE if valid
 */
gboolean
grl_render_texture_is_valid (GrlRenderTexture *self)
{
    GrlRenderTexturePrivate *priv;

    g_return_val_if_fail (GRL_IS_RENDER_TEXTURE (self), FALSE);

    priv = grl_render_texture_get_instance_private (self);
    return priv->valid;
}

/**
 * grl_render_texture_get_width:
 * @self: A #GrlRenderTexture
 *
 * Gets the width.
 *
 * Returns: The width in pixels
 */
gint
grl_render_texture_get_width (GrlRenderTexture *self)
{
    GrlRenderTexturePrivate *priv;

    g_return_val_if_fail (GRL_IS_RENDER_TEXTURE (self), 0);

    priv = grl_render_texture_get_instance_private (self);
    return priv->width;
}

/**
 * grl_render_texture_get_height:
 * @self: A #GrlRenderTexture
 *
 * Gets the height.
 *
 * Returns: The height in pixels
 */
gint
grl_render_texture_get_height (GrlRenderTexture *self)
{
    GrlRenderTexturePrivate *priv;

    g_return_val_if_fail (GRL_IS_RENDER_TEXTURE (self), 0);

    priv = grl_render_texture_get_instance_private (self);
    return priv->height;
}

/**
 * grl_render_texture_begin:
 * @self: A #GrlRenderTexture
 *
 * Begins drawing to this render texture.
 */
void
grl_render_texture_begin (GrlRenderTexture *self)
{
    GrlRenderTexturePrivate *priv;

    g_return_if_fail (GRL_IS_RENDER_TEXTURE (self));

    priv = grl_render_texture_get_instance_private (self);

    if (priv->valid)
        BeginTextureMode (priv->render_texture);
}

/**
 * grl_render_texture_end:
 * @self: A #GrlRenderTexture
 *
 * Ends drawing to the render texture.
 */
void
grl_render_texture_end (GrlRenderTexture *self)
{
    g_return_if_fail (GRL_IS_RENDER_TEXTURE (self));

    EndTextureMode ();
}

/**
 * grl_render_texture_get_texture:
 * @self: A #GrlRenderTexture
 *
 * Gets the color buffer texture.
 *
 * Returns: (transfer full): A #GrlTexture representing the color buffer
 */
GrlTexture *
grl_render_texture_get_texture (GrlRenderTexture *self)
{
    GrlRenderTexturePrivate *priv;
    GrlTexture *texture;

    g_return_val_if_fail (GRL_IS_RENDER_TEXTURE (self), NULL);

    priv = grl_render_texture_get_instance_private (self);

    if (!priv->valid)
        return NULL;

    /*
     * Create a GrlTexture that wraps the render texture's color buffer.
     * We use the internal constructor that takes a handle.
     */
    texture = grl_texture_new_from_handle (&priv->render_texture.texture);

    return texture;
}
