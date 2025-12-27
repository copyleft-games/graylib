/* grl-rlgl-state.c
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Render state management for rlgl.
 */

#include "config.h"
#include "grl-rlgl.h"
#include <rlgl.h>

/**
 * grl_rlgl_enable_color_blend:
 *
 * Enables color blending.
 */
void
grl_rlgl_enable_color_blend (void)
{
    rlEnableColorBlend ();
}

/**
 * grl_rlgl_disable_color_blend:
 *
 * Disables color blending.
 */
void
grl_rlgl_disable_color_blend (void)
{
    rlDisableColorBlend ();
}

/**
 * grl_rlgl_enable_depth_test:
 *
 * Enables depth testing.
 */
void
grl_rlgl_enable_depth_test (void)
{
    rlEnableDepthTest ();
}

/**
 * grl_rlgl_disable_depth_test:
 *
 * Disables depth testing.
 */
void
grl_rlgl_disable_depth_test (void)
{
    rlDisableDepthTest ();
}

/**
 * grl_rlgl_enable_depth_mask:
 *
 * Enables writing to the depth buffer.
 */
void
grl_rlgl_enable_depth_mask (void)
{
    rlEnableDepthMask ();
}

/**
 * grl_rlgl_disable_depth_mask:
 *
 * Disables writing to the depth buffer.
 */
void
grl_rlgl_disable_depth_mask (void)
{
    rlDisableDepthMask ();
}

/**
 * grl_rlgl_enable_backface_culling:
 *
 * Enables backface culling.
 */
void
grl_rlgl_enable_backface_culling (void)
{
    rlEnableBackfaceCulling ();
}

/**
 * grl_rlgl_disable_backface_culling:
 *
 * Disables backface culling.
 */
void
grl_rlgl_disable_backface_culling (void)
{
    rlDisableBackfaceCulling ();
}

/**
 * grl_rlgl_color_mask:
 * @r: Enable red channel writing
 * @g: Enable green channel writing
 * @b: Enable blue channel writing
 * @a: Enable alpha channel writing
 *
 * Sets which color channels can be written to.
 */
void
grl_rlgl_color_mask (gboolean r,
                     gboolean g,
                     gboolean b,
                     gboolean a)
{
    rlColorMask (r, g, b, a);
}

/**
 * grl_rlgl_set_cull_face:
 * @mode: Cull mode (front or back)
 *
 * Sets which faces to cull.
 */
void
grl_rlgl_set_cull_face (GrlRlglCullMode mode)
{
    rlSetCullFace ((int)mode);
}

/**
 * grl_rlgl_enable_scissor_test:
 *
 * Enables scissor testing.
 */
void
grl_rlgl_enable_scissor_test (void)
{
    rlEnableScissorTest ();
}

/**
 * grl_rlgl_disable_scissor_test:
 *
 * Disables scissor testing.
 */
void
grl_rlgl_disable_scissor_test (void)
{
    rlDisableScissorTest ();
}

/**
 * grl_rlgl_scissor:
 * @x: Scissor rectangle X position
 * @y: Scissor rectangle Y position
 * @width: Scissor rectangle width
 * @height: Scissor rectangle height
 *
 * Defines the scissor rectangle.
 */
void
grl_rlgl_scissor (gint x,
                  gint y,
                  gint width,
                  gint height)
{
    rlScissor (x, y, width, height);
}

/**
 * grl_rlgl_enable_wire_mode:
 *
 * Enables wireframe rendering mode.
 */
void
grl_rlgl_enable_wire_mode (void)
{
    rlEnableWireMode ();
}

/**
 * grl_rlgl_enable_point_mode:
 *
 * Enables point cloud rendering mode.
 */
void
grl_rlgl_enable_point_mode (void)
{
    rlEnablePointMode ();
}

/**
 * grl_rlgl_disable_wire_mode:
 *
 * Disables wireframe/point rendering (back to solid fill).
 */
void
grl_rlgl_disable_wire_mode (void)
{
    rlDisableWireMode ();
}

/**
 * grl_rlgl_set_line_width:
 * @width: Line width in pixels
 *
 * Sets the line drawing width.
 */
void
grl_rlgl_set_line_width (gfloat width)
{
    rlSetLineWidth (width);
}

/**
 * grl_rlgl_get_line_width:
 *
 * Gets the current line drawing width.
 *
 * Returns: Current line width in pixels
 */
gfloat
grl_rlgl_get_line_width (void)
{
    return rlGetLineWidth ();
}

/**
 * grl_rlgl_enable_smooth_lines:
 *
 * Enables line antialiasing.
 */
void
grl_rlgl_enable_smooth_lines (void)
{
    rlEnableSmoothLines ();
}

/**
 * grl_rlgl_disable_smooth_lines:
 *
 * Disables line antialiasing.
 */
void
grl_rlgl_disable_smooth_lines (void)
{
    rlDisableSmoothLines ();
}

/**
 * grl_rlgl_enable_stereo_render:
 *
 * Enables stereo rendering.
 */
void
grl_rlgl_enable_stereo_render (void)
{
    rlEnableStereoRender ();
}

/**
 * grl_rlgl_disable_stereo_render:
 *
 * Disables stereo rendering.
 */
void
grl_rlgl_disable_stereo_render (void)
{
    rlDisableStereoRender ();
}

/**
 * grl_rlgl_is_stereo_render_enabled:
 *
 * Checks if stereo rendering is enabled.
 *
 * Returns: %TRUE if stereo render is enabled
 */
gboolean
grl_rlgl_is_stereo_render_enabled (void)
{
    /* Fix bool/gboolean ABI mismatch */
    unsigned char raw = rlIsStereoRenderEnabled ();
    return raw != 0;
}

/**
 * grl_rlgl_clear_color:
 * @r: Red component (0-255)
 * @g: Green component (0-255)
 * @b: Blue component (0-255)
 * @a: Alpha component (0-255)
 *
 * Sets the clear color for the framebuffer.
 */
void
grl_rlgl_clear_color (guint8 r,
                      guint8 g,
                      guint8 b,
                      guint8 a)
{
    rlClearColor (r, g, b, a);
}

/**
 * grl_rlgl_clear_screen_buffers:
 *
 * Clears the color and depth buffers.
 */
void
grl_rlgl_clear_screen_buffers (void)
{
    rlClearScreenBuffers ();
}

/**
 * grl_rlgl_check_errors:
 *
 * Checks and logs OpenGL errors.
 */
void
grl_rlgl_check_errors (void)
{
    rlCheckErrors ();
}

/**
 * grl_rlgl_set_blend_mode:
 * @mode: Blend mode to set
 *
 * Sets the blending mode using predefined modes.
 */
void
grl_rlgl_set_blend_mode (GrlBlendMode mode)
{
    rlSetBlendMode ((int)mode);
}

/**
 * grl_rlgl_set_blend_factors:
 * @src_factor: Source blend factor (GL constant)
 * @dst_factor: Destination blend factor (GL constant)
 * @equation: Blend equation (GL constant)
 *
 * Sets custom blend factors.
 */
void
grl_rlgl_set_blend_factors (gint src_factor,
                            gint dst_factor,
                            gint equation)
{
    rlSetBlendFactors (src_factor, dst_factor, equation);
}

/**
 * grl_rlgl_set_blend_factors_separate:
 * @src_rgb: Source RGB blend factor
 * @dst_rgb: Destination RGB blend factor
 * @src_alpha: Source alpha blend factor
 * @dst_alpha: Destination alpha blend factor
 * @eq_rgb: RGB blend equation
 * @eq_alpha: Alpha blend equation
 *
 * Sets separate blend factors for RGB and alpha channels.
 */
void
grl_rlgl_set_blend_factors_separate (gint src_rgb,
                                     gint dst_rgb,
                                     gint src_alpha,
                                     gint dst_alpha,
                                     gint eq_rgb,
                                     gint eq_alpha)
{
    rlSetBlendFactorsSeparate (src_rgb, dst_rgb, src_alpha, dst_alpha, eq_rgb, eq_alpha);
}
