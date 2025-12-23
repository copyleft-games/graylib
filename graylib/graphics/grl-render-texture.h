/* grl-render-texture.h
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Render texture (framebuffer) for off-screen rendering.
 */

#pragma once

#if !defined(GRAYLIB_INSIDE) && !defined(GRAYLIB_COMPILATION)
#error "Only <graylib.h> can be included directly."
#endif

#include <glib-object.h>
#include "../grl-version.h"
#include "../grl-types.h"

G_BEGIN_DECLS

#define GRL_TYPE_RENDER_TEXTURE (grl_render_texture_get_type())

GRL_AVAILABLE_IN_ALL
G_DECLARE_DERIVABLE_TYPE (GrlRenderTexture, grl_render_texture, GRL, RENDER_TEXTURE, GObject)

/**
 * GrlRenderTextureClass:
 * @parent_class: The parent class
 *
 * The class structure for #GrlRenderTexture.
 */
struct _GrlRenderTextureClass
{
    GObjectClass parent_class;

    /*< private >*/
    gpointer _reserved[8];
};

/**
 * grl_render_texture_new:
 * @width: Width of the render texture
 * @height: Height of the render texture
 *
 * Creates a new render texture (framebuffer) for off-screen rendering.
 *
 * Returns: (transfer full): A new #GrlRenderTexture
 */
GRL_AVAILABLE_IN_ALL
GrlRenderTexture *  grl_render_texture_new              (gint                width,
                                                         gint                height);

/**
 * grl_render_texture_is_valid:
 * @self: A #GrlRenderTexture
 *
 * Checks if the render texture is valid (loaded in GPU).
 *
 * Returns: %TRUE if valid
 */
GRL_AVAILABLE_IN_ALL
gboolean            grl_render_texture_is_valid         (GrlRenderTexture   *self);

/**
 * grl_render_texture_get_width:
 * @self: A #GrlRenderTexture
 *
 * Gets the width of the render texture.
 *
 * Returns: The width in pixels
 */
GRL_AVAILABLE_IN_ALL
gint                grl_render_texture_get_width        (GrlRenderTexture   *self);

/**
 * grl_render_texture_get_height:
 * @self: A #GrlRenderTexture
 *
 * Gets the height of the render texture.
 *
 * Returns: The height in pixels
 */
GRL_AVAILABLE_IN_ALL
gint                grl_render_texture_get_height       (GrlRenderTexture   *self);

/**
 * grl_render_texture_begin:
 * @self: A #GrlRenderTexture
 *
 * Begins drawing to this render texture. All drawing operations
 * until grl_render_texture_end() is called will be rendered to
 * this texture instead of the screen.
 */
GRL_AVAILABLE_IN_ALL
void                grl_render_texture_begin            (GrlRenderTexture   *self);

/**
 * grl_render_texture_end:
 * @self: A #GrlRenderTexture
 *
 * Ends drawing to the render texture and returns to normal screen rendering.
 */
GRL_AVAILABLE_IN_ALL
void                grl_render_texture_end              (GrlRenderTexture   *self);

/**
 * grl_render_texture_get_texture:
 * @self: A #GrlRenderTexture
 *
 * Gets the color buffer texture of the render texture.
 * This can be used to draw the render texture to the screen.
 *
 * Returns: (transfer full): A #GrlTexture representing the color buffer
 */
GRL_AVAILABLE_IN_ALL
GrlTexture *        grl_render_texture_get_texture      (GrlRenderTexture   *self);

G_END_DECLS
