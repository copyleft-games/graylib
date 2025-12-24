/* grl-sprite.h
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * 2D sprite entity.
 */

#pragma once

#if !defined(GRAYLIB_INSIDE) && !defined(GRAYLIB_COMPILATION)
#error "Only <graylib.h> can be included directly."
#endif

#include <glib-object.h>
#include "../grl-version.h"
#include "../graphics/grl-texture.h"
#include "../math/grl-color.h"
#include "../math/grl-rectangle.h"
#include "grl-entity.h"

G_BEGIN_DECLS

#define GRL_TYPE_SPRITE (grl_sprite_get_type ())

GRL_AVAILABLE_IN_ALL
G_DECLARE_DERIVABLE_TYPE (GrlSprite, grl_sprite, GRL, SPRITE, GrlEntity)

/**
 * GrlSpriteClass:
 * @parent_class: The parent class
 *
 * The class structure for #GrlSprite.
 */
struct _GrlSpriteClass
{
    GrlEntityClass parent_class;

    /*< private >*/
    gpointer _reserved[4];
};

/*
 * Constructors
 */

/**
 * grl_sprite_new:
 *
 * Creates a new empty sprite.
 *
 * Returns: (transfer full): A new #GrlSprite
 */
GRL_AVAILABLE_IN_ALL
GrlSprite *     grl_sprite_new              (void);

/**
 * grl_sprite_new_from_texture:
 * @texture: (transfer none): A #GrlTexture
 *
 * Creates a new sprite with the given texture.
 * The sprite's width and height are set to the texture dimensions.
 *
 * Returns: (transfer full): A new #GrlSprite
 */
GRL_AVAILABLE_IN_ALL
GrlSprite *     grl_sprite_new_from_texture (GrlTexture     *texture);

/**
 * grl_sprite_new_from_file:
 * @filename: Path to the texture file
 * @error: (nullable): Return location for error, or %NULL
 *
 * Creates a new sprite by loading a texture from file.
 *
 * Returns: (transfer full) (nullable): A new #GrlSprite, or %NULL on error
 */
GRL_AVAILABLE_IN_ALL
GrlSprite *     grl_sprite_new_from_file    (const gchar    *filename,
                                             GError        **error);

/*
 * Texture
 */

/**
 * grl_sprite_get_texture:
 * @self: A #GrlSprite
 *
 * Gets the sprite's texture.
 *
 * Returns: (transfer none) (nullable): The texture, or %NULL
 */
GRL_AVAILABLE_IN_ALL
GrlTexture *    grl_sprite_get_texture      (GrlSprite      *self);

/**
 * grl_sprite_set_texture:
 * @self: A #GrlSprite
 * @texture: (transfer none) (nullable): A #GrlTexture, or %NULL
 *
 * Sets the sprite's texture. If @texture is not %NULL, the sprite's
 * width and height are updated to match the texture dimensions
 * (unless a source rectangle is set).
 */
GRL_AVAILABLE_IN_ALL
void            grl_sprite_set_texture      (GrlSprite      *self,
                                             GrlTexture     *texture);

/*
 * Source rectangle (for sprite sheets)
 */

/**
 * grl_sprite_get_source_rect:
 * @self: A #GrlSprite
 *
 * Gets the source rectangle within the texture to draw.
 *
 * Returns: (transfer full) (nullable): The source rectangle, or %NULL
 */
GRL_AVAILABLE_IN_ALL
GrlRectangle *  grl_sprite_get_source_rect  (GrlSprite      *self);

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
GRL_AVAILABLE_IN_ALL
void            grl_sprite_set_source_rect  (GrlSprite      *self,
                                             GrlRectangle   *rect);

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
GRL_AVAILABLE_IN_ALL
void            grl_sprite_set_source_rect_values (GrlSprite *self,
                                                   gfloat     x,
                                                   gfloat     y,
                                                   gfloat     width,
                                                   gfloat     height);

/*
 * Tint color
 */

/**
 * grl_sprite_get_tint:
 * @self: A #GrlSprite
 *
 * Gets the sprite's tint color.
 *
 * Returns: (transfer full): The tint color
 */
GRL_AVAILABLE_IN_ALL
GrlColor *      grl_sprite_get_tint         (GrlSprite      *self);

/**
 * grl_sprite_set_tint:
 * @self: A #GrlSprite
 * @tint: The tint color
 *
 * Sets the sprite's tint color. The texture is multiplied
 * by this color when drawn. Use white (255, 255, 255, 255)
 * for no tinting.
 */
GRL_AVAILABLE_IN_ALL
void            grl_sprite_set_tint         (GrlSprite      *self,
                                             GrlColor       *tint);

/*
 * Flip
 */

/**
 * grl_sprite_get_flip_h:
 * @self: A #GrlSprite
 *
 * Gets whether the sprite is flipped horizontally.
 *
 * Returns: %TRUE if flipped horizontally
 */
GRL_AVAILABLE_IN_ALL
gboolean        grl_sprite_get_flip_h       (GrlSprite      *self);

/**
 * grl_sprite_set_flip_h:
 * @self: A #GrlSprite
 * @flip: Whether to flip horizontally
 *
 * Sets whether the sprite is flipped horizontally.
 */
GRL_AVAILABLE_IN_ALL
void            grl_sprite_set_flip_h       (GrlSprite      *self,
                                             gboolean        flip);

/**
 * grl_sprite_get_flip_v:
 * @self: A #GrlSprite
 *
 * Gets whether the sprite is flipped vertically.
 *
 * Returns: %TRUE if flipped vertically
 */
GRL_AVAILABLE_IN_ALL
gboolean        grl_sprite_get_flip_v       (GrlSprite      *self);

/**
 * grl_sprite_set_flip_v:
 * @self: A #GrlSprite
 * @flip: Whether to flip vertically
 *
 * Sets whether the sprite is flipped vertically.
 */
GRL_AVAILABLE_IN_ALL
void            grl_sprite_set_flip_v       (GrlSprite      *self,
                                             gboolean        flip);

G_END_DECLS
