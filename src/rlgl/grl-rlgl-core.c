/* grl-rlgl-core.c
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Core rlgl functions: initialization, cleanup, and utility functions.
 */

#include "config.h"
#include "grl-rlgl.h"
#include <rlgl.h>

/**
 * grl_rlgl_init:
 * @width: Framebuffer width
 * @height: Framebuffer height
 *
 * Initializes the rlgl context (OpenGL buffers, shaders, textures).
 * This is typically called automatically by the window initialization.
 */
void
grl_rlgl_init (gint width,
               gint height)
{
    rlglInit (width, height);
}

/**
 * grl_rlgl_close:
 *
 * Deinitializes the rlgl context and frees all resources.
 */
void
grl_rlgl_close (void)
{
    rlglClose ();
}

/**
 * grl_rlgl_get_version:
 *
 * Gets the current OpenGL version.
 *
 * Returns: The OpenGL version identifier
 */
GrlRlglGlVersion
grl_rlgl_get_version (void)
{
    return (GrlRlglGlVersion)rlGetVersion ();
}

/**
 * grl_rlgl_set_framebuffer_width:
 * @width: New framebuffer width
 *
 * Sets the framebuffer width.
 */
void
grl_rlgl_set_framebuffer_width (gint width)
{
    rlSetFramebufferWidth (width);
}

/**
 * grl_rlgl_get_framebuffer_width:
 *
 * Gets the current framebuffer width.
 *
 * Returns: Framebuffer width in pixels
 */
gint
grl_rlgl_get_framebuffer_width (void)
{
    return rlGetFramebufferWidth ();
}

/**
 * grl_rlgl_set_framebuffer_height:
 * @height: New framebuffer height
 *
 * Sets the framebuffer height.
 */
void
grl_rlgl_set_framebuffer_height (gint height)
{
    rlSetFramebufferHeight (height);
}

/**
 * grl_rlgl_get_framebuffer_height:
 *
 * Gets the current framebuffer height.
 *
 * Returns: Framebuffer height in pixels
 */
gint
grl_rlgl_get_framebuffer_height (void)
{
    return rlGetFramebufferHeight ();
}

/**
 * grl_rlgl_get_texture_id_default:
 *
 * Gets the default texture ID (white 1x1 pixel texture).
 *
 * Returns: Default texture GPU ID
 */
guint
grl_rlgl_get_texture_id_default (void)
{
    return rlGetTextureIdDefault ();
}

/**
 * grl_rlgl_get_shader_id_default:
 *
 * Gets the default shader program ID.
 *
 * Returns: Default shader program GPU ID
 */
guint
grl_rlgl_get_shader_id_default (void)
{
    return rlGetShaderIdDefault ();
}

/**
 * grl_rlgl_load_draw_cube:
 *
 * Loads and draws a unit cube primitive.
 */
void
grl_rlgl_load_draw_cube (void)
{
    rlLoadDrawCube ();
}

/**
 * grl_rlgl_load_draw_quad:
 *
 * Loads and draws a unit quad primitive.
 */
void
grl_rlgl_load_draw_quad (void)
{
    rlLoadDrawQuad ();
}
