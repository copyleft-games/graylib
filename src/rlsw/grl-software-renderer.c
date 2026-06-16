/* grl-software-renderer.c
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Headless, no-GPU software OpenGL-1.1 rasterizer implementation.
 */

#include "config.h"
#include "grl-software-renderer.h"

/*
 * rlsw is a header-only library: defining RLSW_IMPLEMENTATION before the
 * include compiles the implementation into this translation unit. The
 * <external/rlsw.h> spelling resolves because the build adds raylib's src
 * directory to the system include path (-isystem .../raylib/src).
 *
 * NOTE: this TU is compiled as gnu99 (rlsw uses C99 constructs); the rest of
 * graylib is gnu89. We still follow graylib's declaration-at-top-of-block
 * style for consistency.
 */
#define RLSW_IMPLEMENTATION
/* rlsw defaults SW_FRAMEBUFFER_OUTPUT_BGRA to true (for direct blits to native
 * window framebuffers), which makes swReadPixels() emit BGRA-ordered bytes.
 * graylib reads the framebuffer into an R8G8B8A8 GrlImage, so force RGBA output
 * to keep channels correct (a red clear must read back as red, not blue). */
#define SW_FRAMEBUFFER_OUTPUT_BGRA 0
#include <external/rlsw.h>

/**
 * SECTION:grl-software-renderer
 * @title: GrlSoftwareRenderer
 * @short_description: Headless CPU software rasterizer
 *
 * #GrlSoftwareRenderer wraps raylib's standalone, header-only software
 * rasterizer ("rlsw"), which implements an OpenGL-1.1-style immediate-mode
 * pipeline entirely on the CPU. It needs no GPU, no GL context and no window,
 * making it suitable for genuinely headless rendering (CI, asset bakers,
 * servers).
 *
 * Unlike #GrlTexture / #GrlRenderTexture (which require a live GL context),
 * #GrlSoftwareRenderer renders into an internal CPU framebuffer. Rendered
 * output is obtained with grl_software_renderer_read_to_image(), which returns
 * a CPU-side #GrlImage.
 *
 * # Singleton constraint
 *
 * rlsw keeps a single, process-global rasterizer context (its swInit()/
 * swClose() entry points take no handle). Consequently, at most one
 * #GrlSoftwareRenderer may be alive at a time: grl_software_renderer_new()
 * returns %NULL (with a #GWarning) if another instance is still live. The
 * global context is torn down when the live instance is finalized.
 *
 * |[<!-- language="C" -->
 * g_autoptr(GrlSoftwareRenderer) sw = grl_software_renderer_new (256, 256);
 *
 * grl_software_renderer_clear_color (sw, 0.0f, 0.0f, 0.0f, 1.0f);
 * grl_software_renderer_clear (sw, GRL_SW_BUFFER_COLOR | GRL_SW_BUFFER_DEPTH);
 *
 * grl_software_renderer_begin (sw, GRL_SW_DRAW_MODE_TRIANGLES);
 * grl_software_renderer_color3ub (sw, 255, 0, 0);
 * grl_software_renderer_vertex2f (sw, -0.5f, -0.5f);
 * grl_software_renderer_vertex2f (sw,  0.5f, -0.5f);
 * grl_software_renderer_vertex2f (sw,  0.0f,  0.5f);
 * grl_software_renderer_end (sw);
 *
 * g_autoptr(GrlImage) img = grl_software_renderer_read_to_image (sw, 0, 0, 256, 256);
 * ]|
 */

struct _GrlSoftwareRenderer
{
    GObject parent_instance;

    gint width;
    gint height;
};

G_DEFINE_TYPE (GrlSoftwareRenderer, grl_software_renderer, G_TYPE_OBJECT)

/*
 * rlsw maintains a single global context, so only one live instance is
 * permitted at a time.
 */
static gboolean grl_software_renderer_live = FALSE;

/*
 * =============================================================================
 * Enumeration GType registration (mirrors src/grl-enums.c style)
 * =============================================================================
 */

GType
grl_sw_state_get_type (void)
{
    static volatile gsize g_define_type_id__volatile = 0;

    if (g_once_init_enter (&g_define_type_id__volatile))
    {
        static const GEnumValue values[] = {
            { GRL_SW_STATE_SCISSOR_TEST, "GRL_SW_STATE_SCISSOR_TEST", "scissor-test" },
            { GRL_SW_STATE_TEXTURE_2D, "GRL_SW_STATE_TEXTURE_2D", "texture-2d" },
            { GRL_SW_STATE_DEPTH_TEST, "GRL_SW_STATE_DEPTH_TEST", "depth-test" },
            { GRL_SW_STATE_CULL_FACE, "GRL_SW_STATE_CULL_FACE", "cull-face" },
            { GRL_SW_STATE_BLEND, "GRL_SW_STATE_BLEND", "blend" },
            { 0, NULL, NULL }
        };
        GType g_define_type_id = g_enum_register_static ("GrlSwState", values);
        g_once_init_leave (&g_define_type_id__volatile, g_define_type_id);
    }

    return g_define_type_id__volatile;
}

GType
grl_sw_buffer_get_type (void)
{
    static volatile gsize g_define_type_id__volatile = 0;

    if (g_once_init_enter (&g_define_type_id__volatile))
    {
        static const GFlagsValue values[] = {
            { GRL_SW_BUFFER_COLOR, "GRL_SW_BUFFER_COLOR", "color" },
            { GRL_SW_BUFFER_DEPTH, "GRL_SW_BUFFER_DEPTH", "depth" },
            { 0, NULL, NULL }
        };
        GType g_define_type_id = g_flags_register_static ("GrlSwBuffer", values);
        g_once_init_leave (&g_define_type_id__volatile, g_define_type_id);
    }

    return g_define_type_id__volatile;
}

GType
grl_sw_matrix_mode_get_type (void)
{
    static volatile gsize g_define_type_id__volatile = 0;

    if (g_once_init_enter (&g_define_type_id__volatile))
    {
        static const GEnumValue values[] = {
            { GRL_SW_MATRIX_MODE_MODELVIEW, "GRL_SW_MATRIX_MODE_MODELVIEW", "modelview" },
            { GRL_SW_MATRIX_MODE_PROJECTION, "GRL_SW_MATRIX_MODE_PROJECTION", "projection" },
            { GRL_SW_MATRIX_MODE_TEXTURE, "GRL_SW_MATRIX_MODE_TEXTURE", "texture" },
            { 0, NULL, NULL }
        };
        GType g_define_type_id = g_enum_register_static ("GrlSwMatrixMode", values);
        g_once_init_leave (&g_define_type_id__volatile, g_define_type_id);
    }

    return g_define_type_id__volatile;
}

GType
grl_sw_draw_mode_get_type (void)
{
    static volatile gsize g_define_type_id__volatile = 0;

    if (g_once_init_enter (&g_define_type_id__volatile))
    {
        static const GEnumValue values[] = {
            { GRL_SW_DRAW_MODE_POINTS, "GRL_SW_DRAW_MODE_POINTS", "points" },
            { GRL_SW_DRAW_MODE_LINES, "GRL_SW_DRAW_MODE_LINES", "lines" },
            { GRL_SW_DRAW_MODE_TRIANGLES, "GRL_SW_DRAW_MODE_TRIANGLES", "triangles" },
            { GRL_SW_DRAW_MODE_QUADS, "GRL_SW_DRAW_MODE_QUADS", "quads" },
            { 0, NULL, NULL }
        };
        GType g_define_type_id = g_enum_register_static ("GrlSwDrawMode", values);
        g_once_init_leave (&g_define_type_id__volatile, g_define_type_id);
    }

    return g_define_type_id__volatile;
}

GType
grl_sw_polygon_mode_get_type (void)
{
    static volatile gsize g_define_type_id__volatile = 0;

    if (g_once_init_enter (&g_define_type_id__volatile))
    {
        static const GEnumValue values[] = {
            { GRL_SW_POLYGON_MODE_POINT, "GRL_SW_POLYGON_MODE_POINT", "point" },
            { GRL_SW_POLYGON_MODE_LINE, "GRL_SW_POLYGON_MODE_LINE", "line" },
            { GRL_SW_POLYGON_MODE_FILL, "GRL_SW_POLYGON_MODE_FILL", "fill" },
            { 0, NULL, NULL }
        };
        GType g_define_type_id = g_enum_register_static ("GrlSwPolygonMode", values);
        g_once_init_leave (&g_define_type_id__volatile, g_define_type_id);
    }

    return g_define_type_id__volatile;
}

GType
grl_sw_face_get_type (void)
{
    static volatile gsize g_define_type_id__volatile = 0;

    if (g_once_init_enter (&g_define_type_id__volatile))
    {
        static const GEnumValue values[] = {
            { GRL_SW_FACE_FRONT, "GRL_SW_FACE_FRONT", "front" },
            { GRL_SW_FACE_BACK, "GRL_SW_FACE_BACK", "back" },
            { 0, NULL, NULL }
        };
        GType g_define_type_id = g_enum_register_static ("GrlSwFace", values);
        g_once_init_leave (&g_define_type_id__volatile, g_define_type_id);
    }

    return g_define_type_id__volatile;
}

GType
grl_sw_factor_get_type (void)
{
    static volatile gsize g_define_type_id__volatile = 0;

    if (g_once_init_enter (&g_define_type_id__volatile))
    {
        static const GEnumValue values[] = {
            { GRL_SW_FACTOR_ZERO, "GRL_SW_FACTOR_ZERO", "zero" },
            { GRL_SW_FACTOR_ONE, "GRL_SW_FACTOR_ONE", "one" },
            { GRL_SW_FACTOR_SRC_COLOR, "GRL_SW_FACTOR_SRC_COLOR", "src-color" },
            { GRL_SW_FACTOR_ONE_MINUS_SRC_COLOR, "GRL_SW_FACTOR_ONE_MINUS_SRC_COLOR", "one-minus-src-color" },
            { GRL_SW_FACTOR_SRC_ALPHA, "GRL_SW_FACTOR_SRC_ALPHA", "src-alpha" },
            { GRL_SW_FACTOR_ONE_MINUS_SRC_ALPHA, "GRL_SW_FACTOR_ONE_MINUS_SRC_ALPHA", "one-minus-src-alpha" },
            { GRL_SW_FACTOR_DST_ALPHA, "GRL_SW_FACTOR_DST_ALPHA", "dst-alpha" },
            { GRL_SW_FACTOR_ONE_MINUS_DST_ALPHA, "GRL_SW_FACTOR_ONE_MINUS_DST_ALPHA", "one-minus-dst-alpha" },
            { GRL_SW_FACTOR_DST_COLOR, "GRL_SW_FACTOR_DST_COLOR", "dst-color" },
            { GRL_SW_FACTOR_ONE_MINUS_DST_COLOR, "GRL_SW_FACTOR_ONE_MINUS_DST_COLOR", "one-minus-dst-color" },
            { GRL_SW_FACTOR_SRC_ALPHA_SATURATE, "GRL_SW_FACTOR_SRC_ALPHA_SATURATE", "src-alpha-saturate" },
            { 0, NULL, NULL }
        };
        GType g_define_type_id = g_enum_register_static ("GrlSwFactor", values);
        g_once_init_leave (&g_define_type_id__volatile, g_define_type_id);
    }

    return g_define_type_id__volatile;
}

GType
grl_sw_format_get_type (void)
{
    static volatile gsize g_define_type_id__volatile = 0;

    if (g_once_init_enter (&g_define_type_id__volatile))
    {
        static const GEnumValue values[] = {
            { GRL_SW_FORMAT_LUMINANCE, "GRL_SW_FORMAT_LUMINANCE", "luminance" },
            { GRL_SW_FORMAT_LUMINANCE_ALPHA, "GRL_SW_FORMAT_LUMINANCE_ALPHA", "luminance-alpha" },
            { GRL_SW_FORMAT_RGB, "GRL_SW_FORMAT_RGB", "rgb" },
            { GRL_SW_FORMAT_RGBA, "GRL_SW_FORMAT_RGBA", "rgba" },
            { GRL_SW_FORMAT_DEPTH_COMPONENT, "GRL_SW_FORMAT_DEPTH_COMPONENT", "depth-component" },
            { 0, NULL, NULL }
        };
        GType g_define_type_id = g_enum_register_static ("GrlSwFormat", values);
        g_once_init_leave (&g_define_type_id__volatile, g_define_type_id);
    }

    return g_define_type_id__volatile;
}

GType
grl_sw_type_get_type (void)
{
    static volatile gsize g_define_type_id__volatile = 0;

    if (g_once_init_enter (&g_define_type_id__volatile))
    {
        static const GEnumValue values[] = {
            { GRL_SW_TYPE_BYTE, "GRL_SW_TYPE_BYTE", "byte" },
            { GRL_SW_TYPE_UNSIGNED_BYTE, "GRL_SW_TYPE_UNSIGNED_BYTE", "unsigned-byte" },
            { GRL_SW_TYPE_SHORT, "GRL_SW_TYPE_SHORT", "short" },
            { GRL_SW_TYPE_UNSIGNED_SHORT, "GRL_SW_TYPE_UNSIGNED_SHORT", "unsigned-short" },
            { GRL_SW_TYPE_INT, "GRL_SW_TYPE_INT", "int" },
            { GRL_SW_TYPE_UNSIGNED_INT, "GRL_SW_TYPE_UNSIGNED_INT", "unsigned-int" },
            { GRL_SW_TYPE_FLOAT, "GRL_SW_TYPE_FLOAT", "float" },
            { 0, NULL, NULL }
        };
        GType g_define_type_id = g_enum_register_static ("GrlSwType", values);
        g_once_init_leave (&g_define_type_id__volatile, g_define_type_id);
    }

    return g_define_type_id__volatile;
}

GType
grl_sw_tex_param_get_type (void)
{
    static volatile gsize g_define_type_id__volatile = 0;

    if (g_once_init_enter (&g_define_type_id__volatile))
    {
        static const GEnumValue values[] = {
            { GRL_SW_TEX_PARAM_MAG_FILTER, "GRL_SW_TEX_PARAM_MAG_FILTER", "mag-filter" },
            { GRL_SW_TEX_PARAM_MIN_FILTER, "GRL_SW_TEX_PARAM_MIN_FILTER", "min-filter" },
            { GRL_SW_TEX_PARAM_WRAP_S, "GRL_SW_TEX_PARAM_WRAP_S", "wrap-s" },
            { GRL_SW_TEX_PARAM_WRAP_T, "GRL_SW_TEX_PARAM_WRAP_T", "wrap-t" },
            { 0, NULL, NULL }
        };
        GType g_define_type_id = g_enum_register_static ("GrlSwTexParam", values);
        g_once_init_leave (&g_define_type_id__volatile, g_define_type_id);
    }

    return g_define_type_id__volatile;
}

/*
 * =============================================================================
 * GObject virtual methods
 * =============================================================================
 */

static void
grl_software_renderer_finalize (GObject *object)
{
    GrlSoftwareRenderer *self = GRL_SOFTWARE_RENDERER (object);

    /* Tear down the global rlsw context and release the singleton slot. */
    swClose ();
    self->width = 0;
    self->height = 0;
    grl_software_renderer_live = FALSE;

    G_OBJECT_CLASS (grl_software_renderer_parent_class)->finalize (object);
}

static void
grl_software_renderer_class_init (GrlSoftwareRendererClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->finalize = grl_software_renderer_finalize;
}

static void
grl_software_renderer_init (GrlSoftwareRenderer *self)
{
    self->width = 0;
    self->height = 0;
}

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
 * Creates a new #GrlSoftwareRenderer and initializes rlsw with a framebuffer
 * of the given dimensions. At most one instance may be live at a time; see
 * the singleton note in the type documentation.
 *
 * Returns: (transfer full) (nullable): A new #GrlSoftwareRenderer, or %NULL
 */
GrlSoftwareRenderer *
grl_software_renderer_new (gint width,
                           gint height)
{
    GrlSoftwareRenderer *self;
    unsigned char        ok;

    g_return_val_if_fail (width > 0, NULL);
    g_return_val_if_fail (height > 0, NULL);

    if (grl_software_renderer_live)
    {
        g_warning ("grl_software_renderer_new: rlsw uses a single global "
                   "context; only one GrlSoftwareRenderer may be live at a "
                   "time. Returning NULL.");
        return NULL;
    }

    /* bool-ABI gotcha: go through an unsigned char intermediate. */
    ok = swInit ((int)width, (int)height);
    if (ok == 0)
    {
        g_warning ("grl_software_renderer_new: swInit (%d x %d) failed",
                   width, height);
        return NULL;
    }

    self = g_object_new (GRL_TYPE_SOFTWARE_RENDERER, NULL);
    self->width = width;
    self->height = height;
    grl_software_renderer_live = TRUE;

    return self;
}

/**
 * grl_software_renderer_resize:
 * @self: A #GrlSoftwareRenderer
 * @width: New framebuffer width in pixels (must be > 0)
 * @height: New framebuffer height in pixels (must be > 0)
 *
 * Resizes the internal framebuffer.
 *
 * Returns: %TRUE on success, %FALSE on failure
 */
gboolean
grl_software_renderer_resize (GrlSoftwareRenderer *self,
                              gint                 width,
                              gint                 height)
{
    unsigned char ok;

    g_return_val_if_fail (GRL_IS_SOFTWARE_RENDERER (self), FALSE);
    g_return_val_if_fail (width > 0, FALSE);
    g_return_val_if_fail (height > 0, FALSE);

    /* bool-ABI gotcha: go through an unsigned char intermediate. */
    ok = swResize ((int)width, (int)height);
    if (ok == 0)
        return FALSE;

    self->width = width;
    self->height = height;

    return TRUE;
}

/**
 * grl_software_renderer_get_width:
 * @self: A #GrlSoftwareRenderer
 *
 * Gets the current framebuffer width.
 *
 * Returns: Framebuffer width in pixels
 */
gint
grl_software_renderer_get_width (GrlSoftwareRenderer *self)
{
    g_return_val_if_fail (GRL_IS_SOFTWARE_RENDERER (self), 0);

    return self->width;
}

/**
 * grl_software_renderer_get_height:
 * @self: A #GrlSoftwareRenderer
 *
 * Gets the current framebuffer height.
 *
 * Returns: Framebuffer height in pixels
 */
gint
grl_software_renderer_get_height (GrlSoftwareRenderer *self)
{
    g_return_val_if_fail (GRL_IS_SOFTWARE_RENDERER (self), 0);

    return self->height;
}

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
 * Enables a rasterizer state capability.
 */
void
grl_software_renderer_enable (GrlSoftwareRenderer *self,
                              GrlSwState           state)
{
    g_return_if_fail (GRL_IS_SOFTWARE_RENDERER (self));

    swEnable ((SWstate)state);
}

/**
 * grl_software_renderer_disable:
 * @self: A #GrlSoftwareRenderer
 * @state: The #GrlSwState capability to disable
 *
 * Disables a rasterizer state capability.
 */
void
grl_software_renderer_disable (GrlSoftwareRenderer *self,
                               GrlSwState           state)
{
    g_return_if_fail (GRL_IS_SOFTWARE_RENDERER (self));

    swDisable ((SWstate)state);
}

/**
 * grl_software_renderer_viewport:
 * @self: A #GrlSoftwareRenderer
 * @x: Lower-left viewport X origin
 * @y: Lower-left viewport Y origin
 * @width: Viewport width
 * @height: Viewport height
 *
 * Sets the viewport transform.
 */
void
grl_software_renderer_viewport (GrlSoftwareRenderer *self,
                                gint                 x,
                                gint                 y,
                                gint                 width,
                                gint                 height)
{
    g_return_if_fail (GRL_IS_SOFTWARE_RENDERER (self));

    swViewport ((int)x, (int)y, (int)width, (int)height);
}

/**
 * grl_software_renderer_scissor:
 * @self: A #GrlSoftwareRenderer
 * @x: Lower-left scissor X origin
 * @y: Lower-left scissor Y origin
 * @width: Scissor rectangle width
 * @height: Scissor rectangle height
 *
 * Sets the scissor rectangle.
 */
void
grl_software_renderer_scissor (GrlSoftwareRenderer *self,
                               gint                 x,
                               gint                 y,
                               gint                 width,
                               gint                 height)
{
    g_return_if_fail (GRL_IS_SOFTWARE_RENDERER (self));

    swScissor ((int)x, (int)y, (int)width, (int)height);
}

/**
 * grl_software_renderer_clear_color:
 * @self: A #GrlSoftwareRenderer
 * @r: Red component in [0, 1]
 * @g: Green component in [0, 1]
 * @b: Blue component in [0, 1]
 * @a: Alpha component in [0, 1]
 *
 * Sets the framebuffer clear color.
 */
void
grl_software_renderer_clear_color (GrlSoftwareRenderer *self,
                                   gfloat               r,
                                   gfloat               g,
                                   gfloat               b,
                                   gfloat               a)
{
    g_return_if_fail (GRL_IS_SOFTWARE_RENDERER (self));

    swClearColor ((float)r, (float)g, (float)b, (float)a);
}

/**
 * grl_software_renderer_clear_depth:
 * @self: A #GrlSoftwareRenderer
 * @depth: Depth value in [0, 1]
 *
 * Sets the depth-buffer clear value.
 */
void
grl_software_renderer_clear_depth (GrlSoftwareRenderer *self,
                                   gfloat               depth)
{
    g_return_if_fail (GRL_IS_SOFTWARE_RENDERER (self));

    swClearDepth ((float)depth);
}

/**
 * grl_software_renderer_clear:
 * @self: A #GrlSoftwareRenderer
 * @mask: A #GrlSwBuffer bitmask
 *
 * Clears the selected framebuffer buffers.
 */
void
grl_software_renderer_clear (GrlSoftwareRenderer *self,
                             GrlSwBuffer          mask)
{
    g_return_if_fail (GRL_IS_SOFTWARE_RENDERER (self));

    swClear ((uint32_t)mask);
}

/**
 * grl_software_renderer_blend_func:
 * @self: A #GrlSoftwareRenderer
 * @src_factor: The source #GrlSwFactor
 * @dst_factor: The destination #GrlSwFactor
 *
 * Sets the blend factors.
 */
void
grl_software_renderer_blend_func (GrlSoftwareRenderer *self,
                                  GrlSwFactor          src_factor,
                                  GrlSwFactor          dst_factor)
{
    g_return_if_fail (GRL_IS_SOFTWARE_RENDERER (self));

    swBlendFunc ((SWfactor)src_factor, (SWfactor)dst_factor);
}

/**
 * grl_software_renderer_polygon_mode:
 * @self: A #GrlSoftwareRenderer
 * @mode: The #GrlSwPolygonMode rasterization mode
 *
 * Sets how polygons are rasterized.
 */
void
grl_software_renderer_polygon_mode (GrlSoftwareRenderer *self,
                                    GrlSwPolygonMode     mode)
{
    g_return_if_fail (GRL_IS_SOFTWARE_RENDERER (self));

    swPolygonMode ((SWpoly)mode);
}

/**
 * grl_software_renderer_cull_face:
 * @self: A #GrlSoftwareRenderer
 * @face: The #GrlSwFace to cull
 *
 * Selects which polygon faces are culled.
 */
void
grl_software_renderer_cull_face (GrlSoftwareRenderer *self,
                                 GrlSwFace            face)
{
    g_return_if_fail (GRL_IS_SOFTWARE_RENDERER (self));

    swCullFace ((SWface)face);
}

/**
 * grl_software_renderer_point_size:
 * @self: A #GrlSoftwareRenderer
 * @size: Point diameter in pixels
 *
 * Sets the rasterized size of points.
 */
void
grl_software_renderer_point_size (GrlSoftwareRenderer *self,
                                  gfloat               size)
{
    g_return_if_fail (GRL_IS_SOFTWARE_RENDERER (self));

    swPointSize ((float)size);
}

/**
 * grl_software_renderer_line_width:
 * @self: A #GrlSoftwareRenderer
 * @width: Line width in pixels
 *
 * Sets the rasterized width of lines.
 */
void
grl_software_renderer_line_width (GrlSoftwareRenderer *self,
                                  gfloat               width)
{
    g_return_if_fail (GRL_IS_SOFTWARE_RENDERER (self));

    swLineWidth ((float)width);
}

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
 * Selects which matrix stack subsequent operations affect.
 */
void
grl_software_renderer_matrix_mode (GrlSoftwareRenderer *self,
                                   GrlSwMatrixMode      mode)
{
    g_return_if_fail (GRL_IS_SOFTWARE_RENDERER (self));

    swMatrixMode ((SWmatrix)mode);
}

/**
 * grl_software_renderer_push_matrix:
 * @self: A #GrlSoftwareRenderer
 *
 * Pushes the current matrix onto the active stack.
 */
void
grl_software_renderer_push_matrix (GrlSoftwareRenderer *self)
{
    g_return_if_fail (GRL_IS_SOFTWARE_RENDERER (self));

    swPushMatrix ();
}

/**
 * grl_software_renderer_pop_matrix:
 * @self: A #GrlSoftwareRenderer
 *
 * Pops the active matrix stack.
 */
void
grl_software_renderer_pop_matrix (GrlSoftwareRenderer *self)
{
    g_return_if_fail (GRL_IS_SOFTWARE_RENDERER (self));

    swPopMatrix ();
}

/**
 * grl_software_renderer_load_identity:
 * @self: A #GrlSoftwareRenderer
 *
 * Replaces the current matrix with the identity matrix.
 */
void
grl_software_renderer_load_identity (GrlSoftwareRenderer *self)
{
    g_return_if_fail (GRL_IS_SOFTWARE_RENDERER (self));

    swLoadIdentity ();
}

/**
 * grl_software_renderer_translate:
 * @self: A #GrlSoftwareRenderer
 * @x: X translation
 * @y: Y translation
 * @z: Z translation
 *
 * Multiplies the current matrix by a translation matrix.
 */
void
grl_software_renderer_translate (GrlSoftwareRenderer *self,
                                 gfloat               x,
                                 gfloat               y,
                                 gfloat               z)
{
    g_return_if_fail (GRL_IS_SOFTWARE_RENDERER (self));

    swTranslatef ((float)x, (float)y, (float)z);
}

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
void
grl_software_renderer_rotate (GrlSoftwareRenderer *self,
                              gfloat               angle,
                              gfloat               x,
                              gfloat               y,
                              gfloat               z)
{
    g_return_if_fail (GRL_IS_SOFTWARE_RENDERER (self));

    swRotatef ((float)angle, (float)x, (float)y, (float)z);
}

/**
 * grl_software_renderer_scale:
 * @self: A #GrlSoftwareRenderer
 * @x: X scale factor
 * @y: Y scale factor
 * @z: Z scale factor
 *
 * Multiplies the current matrix by a scaling matrix.
 */
void
grl_software_renderer_scale (GrlSoftwareRenderer *self,
                             gfloat               x,
                             gfloat               y,
                             gfloat               z)
{
    g_return_if_fail (GRL_IS_SOFTWARE_RENDERER (self));

    swScalef ((float)x, (float)y, (float)z);
}

/**
 * grl_software_renderer_mult_matrix:
 * @self: A #GrlSoftwareRenderer
 * @matrix: (array fixed-size=16) (element-type gfloat): 16 floats, column-major
 *
 * Multiplies the current matrix by the given 4x4 matrix.
 */
void
grl_software_renderer_mult_matrix (GrlSoftwareRenderer *self,
                                   const gfloat        *matrix)
{
    g_return_if_fail (GRL_IS_SOFTWARE_RENDERER (self));
    g_return_if_fail (matrix != NULL);

    swMultMatrixf ((const float *)matrix);
}

/**
 * grl_software_renderer_frustum:
 * @self: A #GrlSoftwareRenderer
 * @left: Left clipping plane
 * @right: Right clipping plane
 * @bottom: Bottom clipping plane
 * @top: Top clipping plane
 * @znear: Near clipping plane distance
 * @zfar: Far clipping plane distance
 *
 * Multiplies the current matrix by a perspective frustum.
 */
void
grl_software_renderer_frustum (GrlSoftwareRenderer *self,
                               gdouble              left,
                               gdouble              right,
                               gdouble              bottom,
                               gdouble              top,
                               gdouble              znear,
                               gdouble              zfar)
{
    g_return_if_fail (GRL_IS_SOFTWARE_RENDERER (self));

    swFrustum ((double)left, (double)right, (double)bottom, (double)top,
               (double)znear, (double)zfar);
}

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
void
grl_software_renderer_ortho (GrlSoftwareRenderer *self,
                             gdouble              left,
                             gdouble              right,
                             gdouble              bottom,
                             gdouble              top,
                             gdouble              znear,
                             gdouble              zfar)
{
    g_return_if_fail (GRL_IS_SOFTWARE_RENDERER (self));

    swOrtho ((double)left, (double)right, (double)bottom, (double)top,
             (double)znear, (double)zfar);
}

/*
 * =============================================================================
 * Immediate-mode geometry
 * =============================================================================
 */

/**
 * grl_software_renderer_begin:
 * @self: A #GrlSoftwareRenderer
 * @mode: The #GrlSwDrawMode primitive type
 *
 * Begins specifying a primitive.
 */
void
grl_software_renderer_begin (GrlSoftwareRenderer *self,
                             GrlSwDrawMode        mode)
{
    g_return_if_fail (GRL_IS_SOFTWARE_RENDERER (self));

    swBegin ((SWdraw)mode);
}

/**
 * grl_software_renderer_end:
 * @self: A #GrlSoftwareRenderer
 *
 * Ends the current primitive.
 */
void
grl_software_renderer_end (GrlSoftwareRenderer *self)
{
    g_return_if_fail (GRL_IS_SOFTWARE_RENDERER (self));

    swEnd ();
}

/**
 * grl_software_renderer_vertex2f:
 * @self: A #GrlSoftwareRenderer
 * @x: X coordinate
 * @y: Y coordinate
 *
 * Emits a 2D vertex.
 */
void
grl_software_renderer_vertex2f (GrlSoftwareRenderer *self,
                                gfloat               x,
                                gfloat               y)
{
    g_return_if_fail (GRL_IS_SOFTWARE_RENDERER (self));

    swVertex2f ((float)x, (float)y);
}

/**
 * grl_software_renderer_vertex3f:
 * @self: A #GrlSoftwareRenderer
 * @x: X coordinate
 * @y: Y coordinate
 * @z: Z coordinate
 *
 * Emits a 3D vertex.
 */
void
grl_software_renderer_vertex3f (GrlSoftwareRenderer *self,
                                gfloat               x,
                                gfloat               y,
                                gfloat               z)
{
    g_return_if_fail (GRL_IS_SOFTWARE_RENDERER (self));

    swVertex3f ((float)x, (float)y, (float)z);
}

/**
 * grl_software_renderer_color3ub:
 * @self: A #GrlSoftwareRenderer
 * @r: Red component in [0, 255]
 * @g: Green component in [0, 255]
 * @b: Blue component in [0, 255]
 *
 * Sets the current vertex color from 8-bit RGB.
 */
void
grl_software_renderer_color3ub (GrlSoftwareRenderer *self,
                                guint8               r,
                                guint8               g,
                                guint8               b)
{
    g_return_if_fail (GRL_IS_SOFTWARE_RENDERER (self));

    swColor3ub ((uint8_t)r, (uint8_t)g, (uint8_t)b);
}

/**
 * grl_software_renderer_color4ub:
 * @self: A #GrlSoftwareRenderer
 * @r: Red component in [0, 255]
 * @g: Green component in [0, 255]
 * @b: Blue component in [0, 255]
 * @a: Alpha component in [0, 255]
 *
 * Sets the current vertex color from 8-bit RGBA.
 */
void
grl_software_renderer_color4ub (GrlSoftwareRenderer *self,
                                guint8               r,
                                guint8               g,
                                guint8               b,
                                guint8               a)
{
    g_return_if_fail (GRL_IS_SOFTWARE_RENDERER (self));

    swColor4ub ((uint8_t)r, (uint8_t)g, (uint8_t)b, (uint8_t)a);
}

/**
 * grl_software_renderer_color3f:
 * @self: A #GrlSoftwareRenderer
 * @r: Red component in [0, 1]
 * @g: Green component in [0, 1]
 * @b: Blue component in [0, 1]
 *
 * Sets the current vertex color from float RGB.
 */
void
grl_software_renderer_color3f (GrlSoftwareRenderer *self,
                               gfloat               r,
                               gfloat               g,
                               gfloat               b)
{
    g_return_if_fail (GRL_IS_SOFTWARE_RENDERER (self));

    swColor3f ((float)r, (float)g, (float)b);
}

/**
 * grl_software_renderer_color4f:
 * @self: A #GrlSoftwareRenderer
 * @r: Red component in [0, 1]
 * @g: Green component in [0, 1]
 * @b: Blue component in [0, 1]
 * @a: Alpha component in [0, 1]
 *
 * Sets the current vertex color from float RGBA.
 */
void
grl_software_renderer_color4f (GrlSoftwareRenderer *self,
                               gfloat               r,
                               gfloat               g,
                               gfloat               b,
                               gfloat               a)
{
    g_return_if_fail (GRL_IS_SOFTWARE_RENDERER (self));

    swColor4f ((float)r, (float)g, (float)b, (float)a);
}

/**
 * grl_software_renderer_tex_coord2f:
 * @self: A #GrlSoftwareRenderer
 * @u: Horizontal texture coordinate
 * @v: Vertical texture coordinate
 *
 * Sets the current vertex texture coordinate.
 */
void
grl_software_renderer_tex_coord2f (GrlSoftwareRenderer *self,
                                   gfloat               u,
                                   gfloat               v)
{
    g_return_if_fail (GRL_IS_SOFTWARE_RENDERER (self));

    swTexCoord2f ((float)u, (float)v);
}

/*
 * =============================================================================
 * Textures
 * =============================================================================
 */

/**
 * grl_software_renderer_gen_texture:
 * @self: A #GrlSoftwareRenderer
 *
 * Generates a single texture object.
 *
 * Returns: The new texture name (id)
 */
guint32
grl_software_renderer_gen_texture (GrlSoftwareRenderer *self)
{
    uint32_t id = 0;

    g_return_val_if_fail (GRL_IS_SOFTWARE_RENDERER (self), 0);

    swGenTextures (1, &id);

    return (guint32)id;
}

/**
 * grl_software_renderer_delete_texture:
 * @self: A #GrlSoftwareRenderer
 * @texture: The texture name (id) to delete
 *
 * Deletes a single texture object.
 */
void
grl_software_renderer_delete_texture (GrlSoftwareRenderer *self,
                                      guint32              texture)
{
    uint32_t id;

    g_return_if_fail (GRL_IS_SOFTWARE_RENDERER (self));

    id = (uint32_t)texture;
    swDeleteTextures (1, &id);
}

/**
 * grl_software_renderer_bind_texture:
 * @self: A #GrlSoftwareRenderer
 * @texture: The texture name (id) to bind, or 0 to unbind
 *
 * Binds a texture object as the active 2D texture.
 */
void
grl_software_renderer_bind_texture (GrlSoftwareRenderer *self,
                                    guint32              texture)
{
    g_return_if_fail (GRL_IS_SOFTWARE_RENDERER (self));

    swBindTexture ((uint32_t)texture);
}

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
void
grl_software_renderer_tex_image_2d (GrlSoftwareRenderer *self,
                                    gint                 width,
                                    gint                 height,
                                    GrlSwFormat          format,
                                    GrlSwType            type,
                                    const guint8        *data)
{
    g_return_if_fail (GRL_IS_SOFTWARE_RENDERER (self));

    swTexImage2D ((int)width, (int)height, (SWformat)format, (SWtype)type,
                  (const void *)data);
}

/**
 * grl_software_renderer_tex_parameteri:
 * @self: A #GrlSoftwareRenderer
 * @param: The #GrlSwTexParam parameter name
 * @value: The integer parameter value
 *
 * Sets an integer texture parameter on the currently bound texture.
 */
void
grl_software_renderer_tex_parameteri (GrlSoftwareRenderer *self,
                                      GrlSwTexParam        param,
                                      gint                 value)
{
    g_return_if_fail (GRL_IS_SOFTWARE_RENDERER (self));

    swTexParameteri ((int)param, (int)value);
}

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
 * Reads back an RGBA region of the framebuffer into a new CPU-side #GrlImage
 * in %GRL_PIXELFORMAT_UNCOMPRESSED_R8G8B8A8 format.
 *
 * Returns: (transfer full) (nullable): A new #GrlImage, or %NULL on error
 */
GrlImage *
grl_software_renderer_read_to_image (GrlSoftwareRenderer *self,
                                     gint                 x,
                                     gint                 y,
                                     gint                 width,
                                     gint                 height)
{
    GrlImage *image;
    GrlColor *clear;
    gpointer  handle;

    g_return_val_if_fail (GRL_IS_SOFTWARE_RENDERER (self), NULL);
    g_return_val_if_fail (width > 0, NULL);
    g_return_val_if_fail (height > 0, NULL);

    /*
     * Allocate a CPU image of the right size/format, then read the framebuffer
     * directly into its pixel buffer. grl_image_new_color() produces an
     * R8G8B8A8 image with a contiguous 4-byte-per-pixel data buffer.
     */
    clear = grl_color_new (0, 0, 0, 255);
    image = grl_image_new_color (width, height, clear);
    grl_color_free (clear);

    if (image == NULL)
        return NULL;

    /*
     * grl_image_get_handle() returns the underlying raylib Image*; its `data`
     * field is the pixel buffer. We avoid including <raylib.h> here (rlsw
     * shadows the GL token namespace), so reach the data pointer through the
     * documented struct layout: Image is { void *data; int width; int height;
     * int mipmaps; int format; } and data is the first member.
     */
    handle = grl_image_get_handle (image);
    if (handle == NULL)
    {
        g_object_unref (image);
        return NULL;
    }

    swReadPixels ((int)x, (int)y, (int)width, (int)height,
                  SW_RGBA, SW_UNSIGNED_BYTE,
                  *(void **)handle);

    return image;
}
