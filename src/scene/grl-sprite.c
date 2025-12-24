/* grl-sprite.c
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * 2D sprite entity.
 */

#include "config.h"
#include "grl-sprite.h"
#include "../drawing/grl-draw.h"
#include <gio/gio.h>

/**
 * SECTION:grl-sprite
 * @title: GrlSprite
 * @short_description: 2D sprite entity
 *
 * #GrlSprite is a 2D sprite entity that can display a texture or
 * a portion of a texture (for sprite sheets). It extends #GrlEntity
 * and provides texture rendering with support for tinting, flipping,
 * and source rectangle selection.
 *
 * # Creating a Sprite
 *
 * |[<!-- language="C" -->
 * g_autoptr(GrlTexture) texture = grl_texture_new_from_file ("player.png", NULL);
 * g_autoptr(GrlSprite) sprite = grl_sprite_new_from_texture (texture);
 *
 * // Position the sprite
 * grl_entity_set_position_xy (GRL_ENTITY (sprite), 100.0f, 100.0f);
 *
 * // Center the origin for rotation
 * grl_entity_center_origin (GRL_ENTITY (sprite));
 * ]|
 *
 * # Using Sprite Sheets
 *
 * |[<!-- language="C" -->
 * g_autoptr(GrlTexture) sheet = grl_texture_new_from_file ("spritesheet.png", NULL);
 * g_autoptr(GrlSprite) sprite = grl_sprite_new_from_texture (sheet);
 *
 * // Select a 32x32 region starting at (64, 0)
 * grl_sprite_set_source_rect_values (sprite, 64.0f, 0.0f, 32.0f, 32.0f);
 * ]|
 */

typedef struct
{
    GrlTexture   *texture;
    GrlRectangle *source_rect;
    GrlColor     *tint;
    gboolean      flip_h;
    gboolean      flip_v;
} GrlSpritePrivate;

G_DEFINE_TYPE_WITH_PRIVATE (GrlSprite, grl_sprite, GRL_TYPE_ENTITY)

enum
{
    PROP_0,
    PROP_TEXTURE,
    PROP_SOURCE_RECT,
    PROP_TINT,
    PROP_FLIP_H,
    PROP_FLIP_V,
    N_PROPS
};

static GParamSpec *properties[N_PROPS];

/*
 * Virtual method overrides
 */

static void
grl_sprite_draw (GrlEntity *entity)
{
    GrlSprite *self = GRL_SPRITE (entity);
    GrlSpritePrivate *priv = grl_sprite_get_instance_private (self);
    gfloat x, y, width, height;
    gfloat rotation, scale;
    g_autoptr(GrlVector2) origin = NULL;
    g_autoptr(GrlRectangle) source = NULL;
    g_autoptr(GrlRectangle) dest = NULL;
    gfloat src_x, src_y, src_w, src_h;

    /* No texture, nothing to draw */
    if (priv->texture == NULL)
        return;

    /* Get entity properties */
    x = grl_entity_get_x (entity);
    y = grl_entity_get_y (entity);
    width = grl_entity_get_width (entity);
    height = grl_entity_get_height (entity);
    rotation = grl_entity_get_rotation (entity);
    scale = grl_entity_get_scale (entity);
    origin = grl_entity_get_origin (entity);

    /* Determine source rectangle */
    if (priv->source_rect != NULL)
    {
        src_x = grl_rectangle_get_x (priv->source_rect);
        src_y = grl_rectangle_get_y (priv->source_rect);
        src_w = grl_rectangle_get_width (priv->source_rect);
        src_h = grl_rectangle_get_height (priv->source_rect);
    }
    else
    {
        /* Use full texture */
        src_x = 0.0f;
        src_y = 0.0f;
        src_w = (gfloat)grl_texture_get_width (priv->texture);
        src_h = (gfloat)grl_texture_get_height (priv->texture);
    }

    /* Apply flip by negating source dimensions */
    if (priv->flip_h)
        src_w = -src_w;

    if (priv->flip_v)
        src_h = -src_h;

    /* Create source and destination rectangles */
    source = grl_rectangle_new (src_x, src_y,
                                priv->flip_h ? -src_w : src_w,
                                priv->flip_v ? -src_h : src_h);

    /* Apply scale to destination */
    dest = grl_rectangle_new (x, y, width * scale, height * scale);

    /* Draw the texture */
    grl_draw_texture_pro (priv->texture, source, dest, origin,
                          rotation, priv->tint);
}

static GrlRectangle *
grl_sprite_get_bounds (GrlEntity *entity)
{
    gfloat x, y, width, height, scale;
    g_autoptr(GrlVector2) origin = NULL;

    x = grl_entity_get_x (entity);
    y = grl_entity_get_y (entity);
    width = grl_entity_get_width (entity);
    height = grl_entity_get_height (entity);
    scale = grl_entity_get_scale (entity);
    origin = grl_entity_get_origin (entity);

    /* Adjust for origin and scale */
    return grl_rectangle_new (x - grl_vector2_get_x (origin) * scale,
                              y - grl_vector2_get_y (origin) * scale,
                              width * scale,
                              height * scale);
}

/*
 * GObject implementation
 */

static void
grl_sprite_finalize (GObject *object)
{
    GrlSprite *self = GRL_SPRITE (object);
    GrlSpritePrivate *priv = grl_sprite_get_instance_private (self);

    g_clear_object (&priv->texture);
    g_clear_pointer (&priv->source_rect, grl_rectangle_free);
    g_clear_pointer (&priv->tint, grl_color_free);

    G_OBJECT_CLASS (grl_sprite_parent_class)->finalize (object);
}

static void
grl_sprite_get_property (GObject    *object,
                         guint       prop_id,
                         GValue     *value,
                         GParamSpec *pspec)
{
    GrlSprite *self = GRL_SPRITE (object);
    GrlSpritePrivate *priv = grl_sprite_get_instance_private (self);

    switch (prop_id)
    {
    case PROP_TEXTURE:
        g_value_set_object (value, priv->texture);
        break;

    case PROP_SOURCE_RECT:
        g_value_set_object (value, priv->source_rect);
        break;

    case PROP_TINT:
        g_value_set_object (value, priv->tint);
        break;

    case PROP_FLIP_H:
        g_value_set_boolean (value, priv->flip_h);
        break;

    case PROP_FLIP_V:
        g_value_set_boolean (value, priv->flip_v);
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
grl_sprite_set_property (GObject      *object,
                         guint         prop_id,
                         const GValue *value,
                         GParamSpec   *pspec)
{
    GrlSprite *self = GRL_SPRITE (object);

    switch (prop_id)
    {
    case PROP_TEXTURE:
        grl_sprite_set_texture (self, g_value_get_object (value));
        break;

    case PROP_SOURCE_RECT:
        grl_sprite_set_source_rect (self, g_value_get_object (value));
        break;

    case PROP_TINT:
        grl_sprite_set_tint (self, g_value_get_object (value));
        break;

    case PROP_FLIP_H:
        grl_sprite_set_flip_h (self, g_value_get_boolean (value));
        break;

    case PROP_FLIP_V:
        grl_sprite_set_flip_v (self, g_value_get_boolean (value));
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
grl_sprite_class_init (GrlSpriteClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);
    GrlEntityClass *entity_class = GRL_ENTITY_CLASS (klass);

    object_class->finalize = grl_sprite_finalize;
    object_class->get_property = grl_sprite_get_property;
    object_class->set_property = grl_sprite_set_property;

    /* Override entity virtual methods */
    entity_class->draw = grl_sprite_draw;
    entity_class->get_bounds = grl_sprite_get_bounds;

    /**
     * GrlSprite:texture:
     *
     * The texture to draw.
     */
    properties[PROP_TEXTURE] =
        g_param_spec_object ("texture",
                             "Texture",
                             "The texture to draw",
                             GRL_TYPE_TEXTURE,
                             G_PARAM_READWRITE |
                             G_PARAM_EXPLICIT_NOTIFY |
                             G_PARAM_STATIC_STRINGS);

    /**
     * GrlSprite:source-rect:
     *
     * The source rectangle within the texture to draw.
     * If %NULL, the entire texture is used.
     */
    properties[PROP_SOURCE_RECT] =
        g_param_spec_boxed ("source-rect",
                            "Source Rectangle",
                            "The source rectangle within the texture",
                            GRL_TYPE_RECTANGLE,
                            G_PARAM_READWRITE |
                            G_PARAM_EXPLICIT_NOTIFY |
                            G_PARAM_STATIC_STRINGS);

    /**
     * GrlSprite:tint:
     *
     * The tint color applied to the texture.
     */
    properties[PROP_TINT] =
        g_param_spec_boxed ("tint",
                            "Tint",
                            "The tint color applied to the texture",
                            GRL_TYPE_COLOR,
                            G_PARAM_READWRITE |
                            G_PARAM_EXPLICIT_NOTIFY |
                            G_PARAM_STATIC_STRINGS);

    /**
     * GrlSprite:flip-h:
     *
     * Whether to flip the sprite horizontally.
     */
    properties[PROP_FLIP_H] =
        g_param_spec_boolean ("flip-h",
                              "Flip Horizontal",
                              "Whether to flip horizontally",
                              FALSE,
                              G_PARAM_READWRITE |
                              G_PARAM_EXPLICIT_NOTIFY |
                              G_PARAM_STATIC_STRINGS);

    /**
     * GrlSprite:flip-v:
     *
     * Whether to flip the sprite vertically.
     */
    properties[PROP_FLIP_V] =
        g_param_spec_boolean ("flip-v",
                              "Flip Vertical",
                              "Whether to flip vertically",
                              FALSE,
                              G_PARAM_READWRITE |
                              G_PARAM_EXPLICIT_NOTIFY |
                              G_PARAM_STATIC_STRINGS);

    g_object_class_install_properties (object_class, N_PROPS, properties);
}

static void
grl_sprite_init (GrlSprite *self)
{
    GrlSpritePrivate *priv = grl_sprite_get_instance_private (self);

    priv->texture = NULL;
    priv->source_rect = NULL;
    priv->tint = grl_color_new (255, 255, 255, 255); /* White = no tint */
    priv->flip_h = FALSE;
    priv->flip_v = FALSE;
}

/*
 * Public API
 */

/**
 * grl_sprite_new:
 *
 * Creates a new empty sprite.
 *
 * Returns: (transfer full): A new #GrlSprite
 */
GrlSprite *
grl_sprite_new (void)
{
    return g_object_new (GRL_TYPE_SPRITE, NULL);
}

/**
 * grl_sprite_new_from_texture:
 * @texture: (transfer none): A #GrlTexture
 *
 * Creates a new sprite with the given texture.
 * The sprite's width and height are set to the texture dimensions.
 *
 * Returns: (transfer full): A new #GrlSprite
 */
GrlSprite *
grl_sprite_new_from_texture (GrlTexture *texture)
{
    GrlSprite *sprite;

    g_return_val_if_fail (GRL_IS_TEXTURE (texture), NULL);

    sprite = g_object_new (GRL_TYPE_SPRITE,
                           "texture", texture,
                           NULL);

    /* Set size to texture dimensions */
    grl_entity_set_width (GRL_ENTITY (sprite),
                          (gfloat)grl_texture_get_width (texture));
    grl_entity_set_height (GRL_ENTITY (sprite),
                           (gfloat)grl_texture_get_height (texture));

    return sprite;
}

/**
 * grl_sprite_new_from_file:
 * @filename: Path to the texture file
 * @error: (nullable): Return location for error, or %NULL
 *
 * Creates a new sprite by loading a texture from file.
 *
 * Returns: (transfer full) (nullable): A new #GrlSprite, or %NULL on error
 */
GrlSprite *
grl_sprite_new_from_file (const gchar  *filename,
                          GError      **error)
{
    g_autoptr(GrlTexture) texture = NULL;

    g_return_val_if_fail (filename != NULL, NULL);

    texture = grl_texture_new_from_file (filename);

    if (texture == NULL)
    {
        g_set_error (error,
                     G_IO_ERROR,
                     G_IO_ERROR_FAILED,
                     "Failed to load texture from '%s'",
                     filename);
        return NULL;
    }

    return grl_sprite_new_from_texture (texture);
}

/**
 * grl_sprite_get_texture:
 * @self: A #GrlSprite
 *
 * Gets the sprite's texture.
 *
 * Returns: (transfer none) (nullable): The texture, or %NULL
 */
GrlTexture *
grl_sprite_get_texture (GrlSprite *self)
{
    GrlSpritePrivate *priv;

    g_return_val_if_fail (GRL_IS_SPRITE (self), NULL);

    priv = grl_sprite_get_instance_private (self);

    return priv->texture;
}

/**
 * grl_sprite_set_texture:
 * @self: A #GrlSprite
 * @texture: (transfer none) (nullable): A #GrlTexture, or %NULL
 *
 * Sets the sprite's texture. If @texture is not %NULL and no source
 * rectangle is set, the sprite's width and height are updated to
 * match the texture dimensions.
 */
void
grl_sprite_set_texture (GrlSprite  *self,
                        GrlTexture *texture)
{
    GrlSpritePrivate *priv;

    g_return_if_fail (GRL_IS_SPRITE (self));

    priv = grl_sprite_get_instance_private (self);

    if (priv->texture == texture)
        return;

    g_clear_object (&priv->texture);

    if (texture != NULL)
    {
        priv->texture = g_object_ref (texture);

        /* Update size if no source rect is set */
        if (priv->source_rect == NULL)
        {
            grl_entity_set_width (GRL_ENTITY (self),
                                  (gfloat)grl_texture_get_width (texture));
            grl_entity_set_height (GRL_ENTITY (self),
                                   (gfloat)grl_texture_get_height (texture));
        }
    }

    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_TEXTURE]);
}

/**
 * grl_sprite_get_source_rect:
 * @self: A #GrlSprite
 *
 * Gets the source rectangle within the texture to draw.
 *
 * Returns: (transfer full) (nullable): The source rectangle, or %NULL
 */
GrlRectangle *
grl_sprite_get_source_rect (GrlSprite *self)
{
    GrlSpritePrivate *priv;

    g_return_val_if_fail (GRL_IS_SPRITE (self), NULL);

    priv = grl_sprite_get_instance_private (self);

    if (priv->source_rect == NULL)
        return NULL;

    return grl_rectangle_copy (priv->source_rect);
}

/**
 * grl_sprite_set_source_rect:
 * @self: A #GrlSprite
 * @rect: (nullable): The source rectangle, or %NULL for full texture
 *
 * Sets the source rectangle within the texture to draw.
 * This is useful for sprite sheets where multiple sprites
 * share a single texture.
 *
 * The sprite's width and height are updated to match the
 * source rectangle dimensions.
 */
void
grl_sprite_set_source_rect (GrlSprite    *self,
                            GrlRectangle *rect)
{
    GrlSpritePrivate *priv;

    g_return_if_fail (GRL_IS_SPRITE (self));

    priv = grl_sprite_get_instance_private (self);

    g_clear_pointer (&priv->source_rect, grl_rectangle_free);

    if (rect != NULL)
    {
        priv->source_rect = grl_rectangle_copy (rect);

        /* Update entity size to match source rect */
        grl_entity_set_width (GRL_ENTITY (self),
                              grl_rectangle_get_width (rect));
        grl_entity_set_height (GRL_ENTITY (self),
                               grl_rectangle_get_height (rect));
    }
    else if (priv->texture != NULL)
    {
        /* Revert to full texture size */
        grl_entity_set_width (GRL_ENTITY (self),
                              (gfloat)grl_texture_get_width (priv->texture));
        grl_entity_set_height (GRL_ENTITY (self),
                               (gfloat)grl_texture_get_height (priv->texture));
    }

    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_SOURCE_RECT]);
}

/**
 * grl_sprite_set_source_rect_values:
 * @self: A #GrlSprite
 * @x: X offset in texture
 * @y: Y offset in texture
 * @width: Width of the region
 * @height: Height of the region
 *
 * Sets the source rectangle using individual values.
 */
void
grl_sprite_set_source_rect_values (GrlSprite *self,
                                   gfloat     x,
                                   gfloat     y,
                                   gfloat     width,
                                   gfloat     height)
{
    g_autoptr(GrlRectangle) rect = NULL;

    g_return_if_fail (GRL_IS_SPRITE (self));

    rect = grl_rectangle_new (x, y, width, height);
    grl_sprite_set_source_rect (self, rect);
}

/**
 * grl_sprite_get_tint:
 * @self: A #GrlSprite
 *
 * Gets the sprite's tint color.
 *
 * Returns: (transfer full): The tint color
 */
GrlColor *
grl_sprite_get_tint (GrlSprite *self)
{
    GrlSpritePrivate *priv;

    g_return_val_if_fail (GRL_IS_SPRITE (self), NULL);

    priv = grl_sprite_get_instance_private (self);

    return grl_color_copy (priv->tint);
}

/**
 * grl_sprite_set_tint:
 * @self: A #GrlSprite
 * @tint: The tint color
 *
 * Sets the sprite's tint color. The texture is multiplied
 * by this color when drawn. Use white (255, 255, 255, 255)
 * for no tinting.
 */
void
grl_sprite_set_tint (GrlSprite *self,
                     GrlColor  *tint)
{
    GrlSpritePrivate *priv;

    g_return_if_fail (GRL_IS_SPRITE (self));
    g_return_if_fail (tint != NULL);

    priv = grl_sprite_get_instance_private (self);

    g_clear_pointer (&priv->tint, grl_color_free);
    priv->tint = grl_color_copy (tint);

    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_TINT]);
}

/**
 * grl_sprite_get_flip_h:
 * @self: A #GrlSprite
 *
 * Gets whether the sprite is flipped horizontally.
 *
 * Returns: %TRUE if flipped horizontally
 */
gboolean
grl_sprite_get_flip_h (GrlSprite *self)
{
    GrlSpritePrivate *priv;

    g_return_val_if_fail (GRL_IS_SPRITE (self), FALSE);

    priv = grl_sprite_get_instance_private (self);

    return priv->flip_h;
}

/**
 * grl_sprite_set_flip_h:
 * @self: A #GrlSprite
 * @flip: Whether to flip horizontally
 *
 * Sets whether the sprite is flipped horizontally.
 */
void
grl_sprite_set_flip_h (GrlSprite *self,
                       gboolean   flip)
{
    GrlSpritePrivate *priv;

    g_return_if_fail (GRL_IS_SPRITE (self));

    priv = grl_sprite_get_instance_private (self);

    flip = !!flip;

    if (priv->flip_h == flip)
        return;

    priv->flip_h = flip;

    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_FLIP_H]);
}

/**
 * grl_sprite_get_flip_v:
 * @self: A #GrlSprite
 *
 * Gets whether the sprite is flipped vertically.
 *
 * Returns: %TRUE if flipped vertically
 */
gboolean
grl_sprite_get_flip_v (GrlSprite *self)
{
    GrlSpritePrivate *priv;

    g_return_val_if_fail (GRL_IS_SPRITE (self), FALSE);

    priv = grl_sprite_get_instance_private (self);

    return priv->flip_v;
}

/**
 * grl_sprite_set_flip_v:
 * @self: A #GrlSprite
 * @flip: Whether to flip vertically
 *
 * Sets whether the sprite is flipped vertically.
 */
void
grl_sprite_set_flip_v (GrlSprite *self,
                       gboolean   flip)
{
    GrlSpritePrivate *priv;

    g_return_if_fail (GRL_IS_SPRITE (self));

    priv = grl_sprite_get_instance_private (self);

    flip = !!flip;

    if (priv->flip_v == flip)
        return;

    priv->flip_v = flip;

    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_FLIP_V]);
}
