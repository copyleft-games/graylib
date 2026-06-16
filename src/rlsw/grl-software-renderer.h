/* grl-software-renderer.h
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Headless, no-GPU software OpenGL-1.1 rasterizer.
 *
 * #GrlSoftwareRenderer wraps raylib's standalone, header-only software
 * rasterizer ("rlsw"). It rasterizes immediate-mode OpenGL-1.1-style geometry
 * entirely on the CPU into an internal framebuffer, with no GPU, no GL context
 * and no window required. This makes it usable in genuinely headless
 * environments (CI, asset bakers, servers) where #GrlTexture / #GrlRenderTexture
 * (which need a live GL context) cannot run.
 */

#pragma once

#if !defined(GRAYLIB_INSIDE) && !defined(GRAYLIB_COMPILATION)
#error "Only <graylib.h> can be included directly."
#endif

#include <glib-object.h>
#include "../grl-version.h"
#include "../grl-types.h"
#include "../graphics/grl-image.h"

G_BEGIN_DECLS

/*
 * =============================================================================
 * Enumerations
 *
 * These mirror the rlsw SW* enumerations. The numeric values match the
 * underlying rlsw enumerators (themselves OpenGL token values) so they can be
 * cast straight across the ABI boundary.
 * =============================================================================
 */

/**
 * GrlSwState:
 * @GRL_SW_STATE_SCISSOR_TEST: Enable/disable the scissor test (SW_SCISSOR_TEST)
 * @GRL_SW_STATE_TEXTURE_2D: Enable/disable 2D texturing (SW_TEXTURE_2D)
 * @GRL_SW_STATE_DEPTH_TEST: Enable/disable the depth test (SW_DEPTH_TEST)
 * @GRL_SW_STATE_CULL_FACE: Enable/disable face culling (SW_CULL_FACE)
 * @GRL_SW_STATE_BLEND: Enable/disable blending (SW_BLEND)
 *
 * Toggleable rasterizer state capabilities, used with
 * grl_software_renderer_enable() and grl_software_renderer_disable().
 */
typedef enum
{
    GRL_SW_STATE_SCISSOR_TEST = 0x0C11,
    GRL_SW_STATE_TEXTURE_2D   = 0x0DE1,
    GRL_SW_STATE_DEPTH_TEST   = 0x0B71,
    GRL_SW_STATE_CULL_FACE    = 0x0B44,
    GRL_SW_STATE_BLEND        = 0x0BE2
} GrlSwState;

GRL_AVAILABLE_IN_ALL
GType grl_sw_state_get_type (void) G_GNUC_CONST;
#define GRL_TYPE_SW_STATE (grl_sw_state_get_type ())

/**
 * GrlSwBuffer:
 * @GRL_SW_BUFFER_COLOR: Clear the color buffer (SW_COLOR_BUFFER_BIT)
 * @GRL_SW_BUFFER_DEPTH: Clear the depth buffer (SW_DEPTH_BUFFER_BIT)
 *
 * Framebuffer clear bitmask flags, combined with bitwise OR and passed to
 * grl_software_renderer_clear().
 */
typedef enum /*< flags >*/
{
    GRL_SW_BUFFER_COLOR = 0x00004000,
    GRL_SW_BUFFER_DEPTH = 0x00000100
} GrlSwBuffer;

GRL_AVAILABLE_IN_ALL
GType grl_sw_buffer_get_type (void) G_GNUC_CONST;
#define GRL_TYPE_SW_BUFFER (grl_sw_buffer_get_type ())

/**
 * GrlSwMatrixMode:
 * @GRL_SW_MATRIX_MODE_PROJECTION: The projection matrix stack (SW_PROJECTION)
 * @GRL_SW_MATRIX_MODE_MODELVIEW: The modelview matrix stack (SW_MODELVIEW)
 * @GRL_SW_MATRIX_MODE_TEXTURE: The texture matrix stack (SW_TEXTURE)
 *
 * Selects which matrix stack subsequent matrix operations affect, used with
 * grl_software_renderer_matrix_mode().
 */
typedef enum
{
    GRL_SW_MATRIX_MODE_MODELVIEW  = 0x1700,
    GRL_SW_MATRIX_MODE_PROJECTION = 0x1701,
    GRL_SW_MATRIX_MODE_TEXTURE    = 0x1702
} GrlSwMatrixMode;

GRL_AVAILABLE_IN_ALL
GType grl_sw_matrix_mode_get_type (void) G_GNUC_CONST;
#define GRL_TYPE_SW_MATRIX_MODE (grl_sw_matrix_mode_get_type ())

/**
 * GrlSwDrawMode:
 * @GRL_SW_DRAW_MODE_POINTS: Draw a sequence of points (SW_POINTS)
 * @GRL_SW_DRAW_MODE_LINES: Draw a sequence of line segments (SW_LINES)
 * @GRL_SW_DRAW_MODE_TRIANGLES: Draw a sequence of triangles (SW_TRIANGLES)
 * @GRL_SW_DRAW_MODE_QUADS: Draw a sequence of quads (SW_QUADS)
 *
 * Primitive assembly mode, used with grl_software_renderer_begin() and the
 * vertex-array draw functions.
 */
typedef enum
{
    GRL_SW_DRAW_MODE_POINTS    = 0x0000,
    GRL_SW_DRAW_MODE_LINES     = 0x0001,
    GRL_SW_DRAW_MODE_TRIANGLES = 0x0004,
    GRL_SW_DRAW_MODE_QUADS     = 0x0007
} GrlSwDrawMode;

GRL_AVAILABLE_IN_ALL
GType grl_sw_draw_mode_get_type (void) G_GNUC_CONST;
#define GRL_TYPE_SW_DRAW_MODE (grl_sw_draw_mode_get_type ())

/**
 * GrlSwPolygonMode:
 * @GRL_SW_POLYGON_MODE_POINT: Rasterize polygons as points (SW_POINT)
 * @GRL_SW_POLYGON_MODE_LINE: Rasterize polygons as wireframe lines (SW_LINE)
 * @GRL_SW_POLYGON_MODE_FILL: Rasterize polygons as filled faces (SW_FILL)
 *
 * Polygon rasterization mode, used with grl_software_renderer_polygon_mode().
 */
typedef enum
{
    GRL_SW_POLYGON_MODE_POINT = 0x1B00,
    GRL_SW_POLYGON_MODE_LINE  = 0x1B01,
    GRL_SW_POLYGON_MODE_FILL  = 0x1B02
} GrlSwPolygonMode;

GRL_AVAILABLE_IN_ALL
GType grl_sw_polygon_mode_get_type (void) G_GNUC_CONST;
#define GRL_TYPE_SW_POLYGON_MODE (grl_sw_polygon_mode_get_type ())

/**
 * GrlSwFace:
 * @GRL_SW_FACE_FRONT: Cull front-facing polygons (SW_FRONT)
 * @GRL_SW_FACE_BACK: Cull back-facing polygons (SW_BACK)
 *
 * Selects which polygon faces are culled, used with
 * grl_software_renderer_cull_face().
 */
typedef enum
{
    GRL_SW_FACE_FRONT = 0x0404,
    GRL_SW_FACE_BACK  = 0x0405
} GrlSwFace;

GRL_AVAILABLE_IN_ALL
GType grl_sw_face_get_type (void) G_GNUC_CONST;
#define GRL_TYPE_SW_FACE (grl_sw_face_get_type ())

/**
 * GrlSwFactor:
 * @GRL_SW_FACTOR_ZERO: Factor of (0, 0, 0, 0) (SW_ZERO)
 * @GRL_SW_FACTOR_ONE: Factor of (1, 1, 1, 1) (SW_ONE)
 * @GRL_SW_FACTOR_SRC_COLOR: Source color factor (SW_SRC_COLOR)
 * @GRL_SW_FACTOR_ONE_MINUS_SRC_COLOR: 1 - source color (SW_ONE_MINUS_SRC_COLOR)
 * @GRL_SW_FACTOR_SRC_ALPHA: Source alpha factor (SW_SRC_ALPHA)
 * @GRL_SW_FACTOR_ONE_MINUS_SRC_ALPHA: 1 - source alpha (SW_ONE_MINUS_SRC_ALPHA)
 * @GRL_SW_FACTOR_DST_ALPHA: Destination alpha factor (SW_DST_ALPHA)
 * @GRL_SW_FACTOR_ONE_MINUS_DST_ALPHA: 1 - destination alpha (SW_ONE_MINUS_DST_ALPHA)
 * @GRL_SW_FACTOR_DST_COLOR: Destination color factor (SW_DST_COLOR)
 * @GRL_SW_FACTOR_ONE_MINUS_DST_COLOR: 1 - destination color (SW_ONE_MINUS_DST_COLOR)
 * @GRL_SW_FACTOR_SRC_ALPHA_SATURATE: Saturated source alpha (SW_SRC_ALPHA_SATURATE)
 *
 * Blend factors for grl_software_renderer_blend_func().
 */
typedef enum
{
    GRL_SW_FACTOR_ZERO                = 0,
    GRL_SW_FACTOR_ONE                 = 1,
    GRL_SW_FACTOR_SRC_COLOR           = 0x0300,
    GRL_SW_FACTOR_ONE_MINUS_SRC_COLOR = 0x0301,
    GRL_SW_FACTOR_SRC_ALPHA           = 0x0302,
    GRL_SW_FACTOR_ONE_MINUS_SRC_ALPHA = 0x0303,
    GRL_SW_FACTOR_DST_ALPHA           = 0x0304,
    GRL_SW_FACTOR_ONE_MINUS_DST_ALPHA = 0x0305,
    GRL_SW_FACTOR_DST_COLOR           = 0x0306,
    GRL_SW_FACTOR_ONE_MINUS_DST_COLOR = 0x0307,
    GRL_SW_FACTOR_SRC_ALPHA_SATURATE  = 0x0308
} GrlSwFactor;

GRL_AVAILABLE_IN_ALL
GType grl_sw_factor_get_type (void) G_GNUC_CONST;
#define GRL_TYPE_SW_FACTOR (grl_sw_factor_get_type ())

/**
 * GrlSwFormat:
 * @GRL_SW_FORMAT_LUMINANCE: Single-channel luminance (SW_LUMINANCE)
 * @GRL_SW_FORMAT_LUMINANCE_ALPHA: Luminance + alpha (SW_LUMINANCE_ALPHA)
 * @GRL_SW_FORMAT_RGB: Three-channel RGB (SW_RGB)
 * @GRL_SW_FORMAT_RGBA: Four-channel RGBA (SW_RGBA)
 * @GRL_SW_FORMAT_DEPTH_COMPONENT: Depth component (SW_DEPTH_COMPONENT)
 *
 * Pixel component layout, used by grl_software_renderer_tex_image_2d() and
 * the pixel-readback functions.
 */
typedef enum
{
    GRL_SW_FORMAT_LUMINANCE       = 0x1909,
    GRL_SW_FORMAT_LUMINANCE_ALPHA = 0x190A,
    GRL_SW_FORMAT_RGB             = 0x1907,
    GRL_SW_FORMAT_RGBA            = 0x1908,
    GRL_SW_FORMAT_DEPTH_COMPONENT = 0x1902
} GrlSwFormat;

GRL_AVAILABLE_IN_ALL
GType grl_sw_format_get_type (void) G_GNUC_CONST;
#define GRL_TYPE_SW_FORMAT (grl_sw_format_get_type ())

/**
 * GrlSwType:
 * @GRL_SW_TYPE_UNSIGNED_BYTE: 8-bit unsigned integer (SW_UNSIGNED_BYTE)
 * @GRL_SW_TYPE_BYTE: 8-bit signed integer (SW_BYTE)
 * @GRL_SW_TYPE_UNSIGNED_SHORT: 16-bit unsigned integer (SW_UNSIGNED_SHORT)
 * @GRL_SW_TYPE_SHORT: 16-bit signed integer (SW_SHORT)
 * @GRL_SW_TYPE_UNSIGNED_INT: 32-bit unsigned integer (SW_UNSIGNED_INT)
 * @GRL_SW_TYPE_INT: 32-bit signed integer (SW_INT)
 * @GRL_SW_TYPE_FLOAT: 32-bit float (SW_FLOAT)
 *
 * Per-component data type, used by grl_software_renderer_tex_image_2d() and
 * the pixel-readback functions.
 */
typedef enum
{
    GRL_SW_TYPE_BYTE           = 0x1400,
    GRL_SW_TYPE_UNSIGNED_BYTE  = 0x1401,
    GRL_SW_TYPE_SHORT          = 0x1402,
    GRL_SW_TYPE_UNSIGNED_SHORT = 0x1403,
    GRL_SW_TYPE_INT            = 0x1404,
    GRL_SW_TYPE_UNSIGNED_INT   = 0x1405,
    GRL_SW_TYPE_FLOAT          = 0x1406
} GrlSwType;

GRL_AVAILABLE_IN_ALL
GType grl_sw_type_get_type (void) G_GNUC_CONST;
#define GRL_TYPE_SW_TYPE (grl_sw_type_get_type ())

/**
 * GrlSwTexParam:
 * @GRL_SW_TEX_PARAM_MAG_FILTER: Magnification filter (SW_TEXTURE_MAG_FILTER)
 * @GRL_SW_TEX_PARAM_MIN_FILTER: Minification filter (SW_TEXTURE_MIN_FILTER)
 * @GRL_SW_TEX_PARAM_WRAP_S: Horizontal wrap mode (SW_TEXTURE_WRAP_S)
 * @GRL_SW_TEX_PARAM_WRAP_T: Vertical wrap mode (SW_TEXTURE_WRAP_T)
 *
 * Texture parameter names accepted by grl_software_renderer_tex_parameteri().
 */
typedef enum
{
    GRL_SW_TEX_PARAM_MAG_FILTER = 0x2800,
    GRL_SW_TEX_PARAM_MIN_FILTER = 0x2801,
    GRL_SW_TEX_PARAM_WRAP_S     = 0x2802,
    GRL_SW_TEX_PARAM_WRAP_T     = 0x2803
} GrlSwTexParam;

GRL_AVAILABLE_IN_ALL
GType grl_sw_tex_param_get_type (void) G_GNUC_CONST;
#define GRL_TYPE_SW_TEX_PARAM (grl_sw_tex_param_get_type ())

/*
 * =============================================================================
 * Type
 * =============================================================================
 */

#define GRL_TYPE_SOFTWARE_RENDERER (grl_software_renderer_get_type ())

GRL_AVAILABLE_IN_ALL
G_DECLARE_FINAL_TYPE (GrlSoftwareRenderer, grl_software_renderer, GRL, SOFTWARE_RENDERER, GObject)

/*
 * =============================================================================
 * Constructors / lifecycle
 * =============================================================================
 */

/**
 * grl_software_renderer_new:
 * @width: Framebuffer width in pixels (must be > 0)
 * @height: Framebuffer height in pixels (must be > 0)
 *
 * Creates a new #GrlSoftwareRenderer and initializes the rlsw software
 * rasterizer with a framebuffer of the given dimensions.
 *
 * Because rlsw maintains a single, process-global rasterizer context (its
 * init/shutdown entry points take no handle), at most one
 * #GrlSoftwareRenderer instance may be alive at a time. Calling this function
 * while another instance is live emits a #GWarning and returns %NULL. The
 * global context is released when the live instance is finalized.
 *
 * Returns: (transfer full) (nullable): A new #GrlSoftwareRenderer, or %NULL if
 *   an instance is already live or initialization failed
 */
GRL_AVAILABLE_IN_ALL
GrlSoftwareRenderer * grl_software_renderer_new        (gint                 width,
                                                        gint                 height);

/**
 * grl_software_renderer_resize:
 * @self: A #GrlSoftwareRenderer
 * @width: New framebuffer width in pixels (must be > 0)
 * @height: New framebuffer height in pixels (must be > 0)
 *
 * Resizes the internal framebuffer.
 *
 * Returns: %TRUE on success, %FALSE on failure (e.g. out of memory)
 */
GRL_AVAILABLE_IN_ALL
gboolean              grl_software_renderer_resize     (GrlSoftwareRenderer *self,
                                                        gint                 width,
                                                        gint                 height);

/**
 * grl_software_renderer_get_width:
 * @self: A #GrlSoftwareRenderer
 *
 * Gets the current framebuffer width.
 *
 * Returns: Framebuffer width in pixels
 */
GRL_AVAILABLE_IN_ALL
gint                  grl_software_renderer_get_width  (GrlSoftwareRenderer *self);

/**
 * grl_software_renderer_get_height:
 * @self: A #GrlSoftwareRenderer
 *
 * Gets the current framebuffer height.
 *
 * Returns: Framebuffer height in pixels
 */
GRL_AVAILABLE_IN_ALL
gint                  grl_software_renderer_get_height (GrlSoftwareRenderer *self);

/*
 * =============================================================================
 * Framebuffer state
 * =============================================================================
 */

/**
 * grl_software_renderer_enable:
 * @self: A #GrlSoftwareRenderer
 * @state: The #GrlSwState capability to enable
 *
 * Enables a rasterizer state capability (scissor test, texturing, depth test,
 * face culling, blending).
 */
GRL_AVAILABLE_IN_ALL
void                  grl_software_renderer_enable     (GrlSoftwareRenderer *self,
                                                        GrlSwState           state);

/**
 * grl_software_renderer_disable:
 * @self: A #GrlSoftwareRenderer
 * @state: The #GrlSwState capability to disable
 *
 * Disables a rasterizer state capability.
 */
GRL_AVAILABLE_IN_ALL
void                  grl_software_renderer_disable    (GrlSoftwareRenderer *self,
                                                        GrlSwState           state);

/**
 * grl_software_renderer_viewport:
 * @self: A #GrlSoftwareRenderer
 * @x: Lower-left viewport X origin
 * @y: Lower-left viewport Y origin
 * @width: Viewport width
 * @height: Viewport height
 *
 * Sets the viewport transform used to map normalized device coordinates to
 * framebuffer pixels.
 */
GRL_AVAILABLE_IN_ALL
void                  grl_software_renderer_viewport   (GrlSoftwareRenderer *self,
                                                        gint                 x,
                                                        gint                 y,
                                                        gint                 width,
                                                        gint                 height);

/**
 * grl_software_renderer_scissor:
 * @self: A #GrlSoftwareRenderer
 * @x: Lower-left scissor X origin
 * @y: Lower-left scissor Y origin
 * @width: Scissor rectangle width
 * @height: Scissor rectangle height
 *
 * Sets the scissor rectangle. Pixels outside this rectangle are discarded
 * when the scissor test is enabled.
 */
GRL_AVAILABLE_IN_ALL
void                  grl_software_renderer_scissor    (GrlSoftwareRenderer *self,
                                                        gint                 x,
                                                        gint                 y,
                                                        gint                 width,
                                                        gint                 height);

/**
 * grl_software_renderer_clear_color:
 * @self: A #GrlSoftwareRenderer
 * @r: Red component in [0, 1]
 * @g: Green component in [0, 1]
 * @b: Blue component in [0, 1]
 * @a: Alpha component in [0, 1]
 *
 * Sets the color the framebuffer is cleared to.
 */
GRL_AVAILABLE_IN_ALL
void                  grl_software_renderer_clear_color (GrlSoftwareRenderer *self,
                                                         gfloat               r,
                                                         gfloat               g,
                                                         gfloat               b,
                                                         gfloat               a);

/**
 * grl_software_renderer_clear_depth:
 * @self: A #GrlSoftwareRenderer
 * @depth: Depth value in [0, 1] the depth buffer is cleared to
 *
 * Sets the depth value the depth buffer is cleared to.
 */
GRL_AVAILABLE_IN_ALL
void                  grl_software_renderer_clear_depth (GrlSoftwareRenderer *self,
                                                         gfloat               depth);

/**
 * grl_software_renderer_clear:
 * @self: A #GrlSoftwareRenderer
 * @mask: A #GrlSwBuffer bitmask selecting which buffers to clear
 *
 * Clears the selected framebuffer buffers to their configured clear values.
 */
GRL_AVAILABLE_IN_ALL
void                  grl_software_renderer_clear      (GrlSoftwareRenderer *self,
                                                        GrlSwBuffer          mask);

/**
 * grl_software_renderer_blend_func:
 * @self: A #GrlSoftwareRenderer
 * @src_factor: The source #GrlSwFactor
 * @dst_factor: The destination #GrlSwFactor
 *
 * Sets the blend factors used when blending is enabled.
 */
GRL_AVAILABLE_IN_ALL
void                  grl_software_renderer_blend_func  (GrlSoftwareRenderer *self,
                                                         GrlSwFactor          src_factor,
                                                         GrlSwFactor          dst_factor);

/**
 * grl_software_renderer_polygon_mode:
 * @self: A #GrlSoftwareRenderer
 * @mode: The #GrlSwPolygonMode rasterization mode
 *
 * Sets how polygons are rasterized (points, wireframe, or filled).
 */
GRL_AVAILABLE_IN_ALL
void                  grl_software_renderer_polygon_mode (GrlSoftwareRenderer *self,
                                                          GrlSwPolygonMode     mode);

/**
 * grl_software_renderer_cull_face:
 * @self: A #GrlSoftwareRenderer
 * @face: The #GrlSwFace to cull
 *
 * Selects which polygon faces are culled when face culling is enabled.
 */
GRL_AVAILABLE_IN_ALL
void                  grl_software_renderer_cull_face   (GrlSoftwareRenderer *self,
                                                         GrlSwFace            face);

/**
 * grl_software_renderer_point_size:
 * @self: A #GrlSoftwareRenderer
 * @size: Point diameter in pixels
 *
 * Sets the rasterized size of points.
 */
GRL_AVAILABLE_IN_ALL
void                  grl_software_renderer_point_size  (GrlSoftwareRenderer *self,
                                                         gfloat               size);

/**
 * grl_software_renderer_line_width:
 * @self: A #GrlSoftwareRenderer
 * @width: Line width in pixels
 *
 * Sets the rasterized width of lines.
 */
GRL_AVAILABLE_IN_ALL
void                  grl_software_renderer_line_width  (GrlSoftwareRenderer *self,
                                                         gfloat               width);

/*
 * =============================================================================
 * Matrix stack
 * =============================================================================
 */

/**
 * grl_software_renderer_matrix_mode:
 * @self: A #GrlSoftwareRenderer
 * @mode: The #GrlSwMatrixMode stack to make current
 *
 * Selects which matrix stack subsequent matrix operations affect.
 */
GRL_AVAILABLE_IN_ALL
void                  grl_software_renderer_matrix_mode (GrlSoftwareRenderer *self,
                                                         GrlSwMatrixMode      mode);

/**
 * grl_software_renderer_push_matrix:
 * @self: A #GrlSoftwareRenderer
 *
 * Pushes the current matrix onto the active matrix stack.
 */
GRL_AVAILABLE_IN_ALL
void                  grl_software_renderer_push_matrix (GrlSoftwareRenderer *self);

/**
 * grl_software_renderer_pop_matrix:
 * @self: A #GrlSoftwareRenderer
 *
 * Pops the active matrix stack, restoring the previously pushed matrix.
 */
GRL_AVAILABLE_IN_ALL
void                  grl_software_renderer_pop_matrix  (GrlSoftwareRenderer *self);

/**
 * grl_software_renderer_load_identity:
 * @self: A #GrlSoftwareRenderer
 *
 * Replaces the current matrix with the identity matrix.
 */
GRL_AVAILABLE_IN_ALL
void                  grl_software_renderer_load_identity (GrlSoftwareRenderer *self);

/**
 * grl_software_renderer_translate:
 * @self: A #GrlSoftwareRenderer
 * @x: X translation
 * @y: Y translation
 * @z: Z translation
 *
 * Multiplies the current matrix by a translation matrix.
 */
GRL_AVAILABLE_IN_ALL
void                  grl_software_renderer_translate   (GrlSoftwareRenderer *self,
                                                         gfloat               x,
                                                         gfloat               y,
                                                         gfloat               z);

/**
 * grl_software_renderer_rotate:
 * @self: A #GrlSoftwareRenderer
 * @angle: Rotation angle in degrees
 * @x: X component of the rotation axis
 * @y: Y component of the rotation axis
 * @z: Z component of the rotation axis
 *
 * Multiplies the current matrix by a rotation matrix.
 */
GRL_AVAILABLE_IN_ALL
void                  grl_software_renderer_rotate      (GrlSoftwareRenderer *self,
                                                         gfloat               angle,
                                                         gfloat               x,
                                                         gfloat               y,
                                                         gfloat               z);

/**
 * grl_software_renderer_scale:
 * @self: A #GrlSoftwareRenderer
 * @x: X scale factor
 * @y: Y scale factor
 * @z: Z scale factor
 *
 * Multiplies the current matrix by a scaling matrix.
 */
GRL_AVAILABLE_IN_ALL
void                  grl_software_renderer_scale       (GrlSoftwareRenderer *self,
                                                         gfloat               x,
                                                         gfloat               y,
                                                         gfloat               z);

/**
 * grl_software_renderer_mult_matrix:
 * @self: A #GrlSoftwareRenderer
 * @matrix: (array fixed-size=16) (element-type gfloat): 16 floats, column-major
 *
 * Multiplies the current matrix by the given 4x4 matrix (16 floats, in
 * column-major order).
 */
GRL_AVAILABLE_IN_ALL
void                  grl_software_renderer_mult_matrix (GrlSoftwareRenderer *self,
                                                         const gfloat        *matrix);

/**
 * grl_software_renderer_frustum:
 * @self: A #GrlSoftwareRenderer
 * @left: Left clipping plane
 * @right: Right clipping plane
 * @bottom: Bottom clipping plane
 * @top: Top clipping plane
 * @znear: Near clipping plane distance (> 0)
 * @zfar: Far clipping plane distance (> @znear)
 *
 * Multiplies the current matrix by a perspective projection frustum.
 */
GRL_AVAILABLE_IN_ALL
void                  grl_software_renderer_frustum     (GrlSoftwareRenderer *self,
                                                         gdouble              left,
                                                         gdouble              right,
                                                         gdouble              bottom,
                                                         gdouble              top,
                                                         gdouble              znear,
                                                         gdouble              zfar);

/**
 * grl_software_renderer_ortho:
 * @self: A #GrlSoftwareRenderer
 * @left: Left clipping plane
 * @right: Right clipping plane
 * @bottom: Bottom clipping plane
 * @top: Top clipping plane
 * @znear: Near clipping plane distance
 * @zfar: Far clipping plane distance
 *
 * Multiplies the current matrix by an orthographic projection.
 */
GRL_AVAILABLE_IN_ALL
void                  grl_software_renderer_ortho       (GrlSoftwareRenderer *self,
                                                         gdouble              left,
                                                         gdouble              right,
                                                         gdouble              bottom,
                                                         gdouble              top,
                                                         gdouble              znear,
                                                         gdouble              zfar);

/*
 * =============================================================================
 * Immediate-mode geometry
 * =============================================================================
 */

/**
 * grl_software_renderer_begin:
 * @self: A #GrlSoftwareRenderer
 * @mode: The #GrlSwDrawMode primitive type to assemble
 *
 * Begins specifying a primitive. Vertices supplied between this call and
 * grl_software_renderer_end() are assembled into primitives of @mode.
 */
GRL_AVAILABLE_IN_ALL
void                  grl_software_renderer_begin       (GrlSoftwareRenderer *self,
                                                         GrlSwDrawMode        mode);

/**
 * grl_software_renderer_end:
 * @self: A #GrlSoftwareRenderer
 *
 * Ends the primitive started with grl_software_renderer_begin().
 */
GRL_AVAILABLE_IN_ALL
void                  grl_software_renderer_end         (GrlSoftwareRenderer *self);

/**
 * grl_software_renderer_vertex2f:
 * @self: A #GrlSoftwareRenderer
 * @x: X coordinate
 * @y: Y coordinate
 *
 * Emits a 2D vertex (z = 0).
 */
GRL_AVAILABLE_IN_ALL
void                  grl_software_renderer_vertex2f    (GrlSoftwareRenderer *self,
                                                         gfloat               x,
                                                         gfloat               y);

/**
 * grl_software_renderer_vertex3f:
 * @self: A #GrlSoftwareRenderer
 * @x: X coordinate
 * @y: Y coordinate
 * @z: Z coordinate
 *
 * Emits a 3D vertex.
 */
GRL_AVAILABLE_IN_ALL
void                  grl_software_renderer_vertex3f    (GrlSoftwareRenderer *self,
                                                         gfloat               x,
                                                         gfloat               y,
                                                         gfloat               z);

/**
 * grl_software_renderer_color3ub:
 * @self: A #GrlSoftwareRenderer
 * @r: Red component in [0, 255]
 * @g: Green component in [0, 255]
 * @b: Blue component in [0, 255]
 *
 * Sets the current vertex color from 8-bit RGB components (alpha = 255).
 */
GRL_AVAILABLE_IN_ALL
void                  grl_software_renderer_color3ub    (GrlSoftwareRenderer *self,
                                                         guint8               r,
                                                         guint8               g,
                                                         guint8               b);

/**
 * grl_software_renderer_color4ub:
 * @self: A #GrlSoftwareRenderer
 * @r: Red component in [0, 255]
 * @g: Green component in [0, 255]
 * @b: Blue component in [0, 255]
 * @a: Alpha component in [0, 255]
 *
 * Sets the current vertex color from 8-bit RGBA components.
 */
GRL_AVAILABLE_IN_ALL
void                  grl_software_renderer_color4ub    (GrlSoftwareRenderer *self,
                                                         guint8               r,
                                                         guint8               g,
                                                         guint8               b,
                                                         guint8               a);

/**
 * grl_software_renderer_color3f:
 * @self: A #GrlSoftwareRenderer
 * @r: Red component in [0, 1]
 * @g: Green component in [0, 1]
 * @b: Blue component in [0, 1]
 *
 * Sets the current vertex color from float RGB components (alpha = 1).
 */
GRL_AVAILABLE_IN_ALL
void                  grl_software_renderer_color3f     (GrlSoftwareRenderer *self,
                                                         gfloat               r,
                                                         gfloat               g,
                                                         gfloat               b);

/**
 * grl_software_renderer_color4f:
 * @self: A #GrlSoftwareRenderer
 * @r: Red component in [0, 1]
 * @g: Green component in [0, 1]
 * @b: Blue component in [0, 1]
 * @a: Alpha component in [0, 1]
 *
 * Sets the current vertex color from float RGBA components.
 */
GRL_AVAILABLE_IN_ALL
void                  grl_software_renderer_color4f     (GrlSoftwareRenderer *self,
                                                         gfloat               r,
                                                         gfloat               g,
                                                         gfloat               b,
                                                         gfloat               a);

/**
 * grl_software_renderer_tex_coord2f:
 * @self: A #GrlSoftwareRenderer
 * @u: Horizontal texture coordinate
 * @v: Vertical texture coordinate
 *
 * Sets the current vertex texture coordinate.
 */
GRL_AVAILABLE_IN_ALL
void                  grl_software_renderer_tex_coord2f (GrlSoftwareRenderer *self,
                                                         gfloat               u,
                                                         gfloat               v);

/*
 * =============================================================================
 * Textures
 * =============================================================================
 */

/**
 * grl_software_renderer_gen_texture:
 * @self: A #GrlSoftwareRenderer
 *
 * Generates a single texture object and returns its name (id).
 *
 * Returns: The new texture name (id)
 */
GRL_AVAILABLE_IN_ALL
guint32               grl_software_renderer_gen_texture (GrlSoftwareRenderer *self);

/**
 * grl_software_renderer_delete_texture:
 * @self: A #GrlSoftwareRenderer
 * @texture: The texture name (id) to delete
 *
 * Deletes a single texture object.
 */
GRL_AVAILABLE_IN_ALL
void                  grl_software_renderer_delete_texture (GrlSoftwareRenderer *self,
                                                            guint32              texture);

/**
 * grl_software_renderer_bind_texture:
 * @self: A #GrlSoftwareRenderer
 * @texture: The texture name (id) to bind, or 0 to unbind
 *
 * Binds a texture object as the active 2D texture.
 */
GRL_AVAILABLE_IN_ALL
void                  grl_software_renderer_bind_texture (GrlSoftwareRenderer *self,
                                                          guint32              texture);

/**
 * grl_software_renderer_tex_image_2d:
 * @self: A #GrlSoftwareRenderer
 * @width: Texture width in pixels
 * @height: Texture height in pixels
 * @format: The #GrlSwFormat pixel layout of @data
 * @type: The #GrlSwType per-component data type of @data
 * @data: (array) (nullable) (element-type guint8): Raw pixel data, or %NULL
 *
 * Uploads pixel data to the currently bound texture.
 */
GRL_AVAILABLE_IN_ALL
void                  grl_software_renderer_tex_image_2d (GrlSoftwareRenderer *self,
                                                          gint                 width,
                                                          gint                 height,
                                                          GrlSwFormat          format,
                                                          GrlSwType            type,
                                                          const guint8        *data);

/**
 * grl_software_renderer_tex_parameteri:
 * @self: A #GrlSoftwareRenderer
 * @param: The #GrlSwTexParam parameter name
 * @value: The integer parameter value (e.g. SW_NEAREST/SW_LINEAR token)
 *
 * Sets an integer texture parameter on the currently bound texture.
 */
GRL_AVAILABLE_IN_ALL
void                  grl_software_renderer_tex_parameteri (GrlSoftwareRenderer *self,
                                                            GrlSwTexParam        param,
                                                            gint                 value);

/*
 * =============================================================================
 * Pixel readback
 * =============================================================================
 */

/**
 * grl_software_renderer_read_to_image:
 * @self: A #GrlSoftwareRenderer
 * @x: Lower-left X origin of the region to read
 * @y: Lower-left Y origin of the region to read
 * @width: Width of the region to read (must be > 0)
 * @height: Height of the region to read (must be > 0)
 *
 * Reads back an RGBA region of the framebuffer into a new CPU-side
 * #GrlImage in %GRL_PIXELFORMAT_UNCOMPRESSED_R8G8B8A8 format. This is the
 * primary way to obtain rendered output from the headless rasterizer.
 *
 * Returns: (transfer full) (nullable): A new #GrlImage holding the pixels, or
 *   %NULL on error
 */
GRL_AVAILABLE_IN_ALL
GrlImage *            grl_software_renderer_read_to_image (GrlSoftwareRenderer *self,
                                                           gint                 x,
                                                           gint                 y,
                                                           gint                 width,
                                                           gint                 height);

G_END_DECLS
