/* grl-image.h
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * CPU-side image type for loading and manipulating pixel data.
 */

#pragma once

#if !defined(GRAYLIB_INSIDE) && !defined(GRAYLIB_COMPILATION)
#error "Only <graylib.h> can be included directly."
#endif

#include <glib-object.h>
#include "../grl-version.h"
#include "../grl-enums.h"
#include "../grl-types.h"
#include "../math/grl-vector2.h"
#include "../math/grl-color.h"
#include "../math/grl-rectangle.h"

G_BEGIN_DECLS

#define GRL_TYPE_IMAGE (grl_image_get_type ())

GRL_AVAILABLE_IN_ALL
G_DECLARE_FINAL_TYPE (GrlImage, grl_image, GRL, IMAGE, GObject)

/*
 * Constructors - Loading
 */

GRL_AVAILABLE_IN_ALL
GrlImage *          grl_image_new_from_file     (const gchar        *filename);

GRL_AVAILABLE_IN_ALL
GrlImage *          grl_image_new_from_memory   (const gchar        *file_type,
                                                 const guint8       *data,
                                                 gsize               data_size);

GRL_AVAILABLE_IN_ALL
GrlImage *          grl_image_new_from_screen   (void);

/**
 * grl_image_new_from_resource:
 * @pack: A #GrlResourcePack
 * @resource_id: The resource ID to load
 * @file_type: (nullable): File type hint (e.g., ".png"), or %NULL to auto-detect
 * @error: (nullable): Return location for error, or %NULL
 *
 * Loads an image from a resource pack.
 *
 * If @file_type is %NULL, the function will attempt to auto-detect the
 * format from the resource data. Providing a hint improves reliability.
 *
 * Returns: (transfer full) (nullable): A new #GrlImage, or %NULL on error
 */
GRL_AVAILABLE_IN_ALL
GrlImage *          grl_image_new_from_resource (GrlResourcePack    *pack,
                                                 guint32             resource_id,
                                                 const gchar        *file_type,
                                                 GError            **error);

/*
 * Constructors - Generation
 */

GRL_AVAILABLE_IN_ALL
GrlImage *          grl_image_new_color         (gint                width,
                                                 gint                height,
                                                 const GrlColor     *color);

GRL_AVAILABLE_IN_ALL
GrlImage *          grl_image_new_gradient_linear (gint              width,
                                                   gint              height,
                                                   gint              direction,
                                                   const GrlColor   *start,
                                                   const GrlColor   *end);

GRL_AVAILABLE_IN_ALL
GrlImage *          grl_image_new_gradient_radial (gint              width,
                                                   gint              height,
                                                   gfloat            density,
                                                   const GrlColor   *inner,
                                                   const GrlColor   *outer);

GRL_AVAILABLE_IN_ALL
GrlImage *          grl_image_new_checked       (gint                width,
                                                 gint                height,
                                                 gint                checks_x,
                                                 gint                checks_y,
                                                 const GrlColor     *color1,
                                                 const GrlColor     *color2);

GRL_AVAILABLE_IN_ALL
GrlImage *          grl_image_new_white_noise   (gint                width,
                                                 gint                height,
                                                 gfloat              factor);

GRL_AVAILABLE_IN_ALL
GrlImage *          grl_image_new_perlin_noise  (gint                width,
                                                 gint                height,
                                                 gint                offset_x,
                                                 gint                offset_y,
                                                 gfloat              scale);

GRL_AVAILABLE_IN_ALL
GrlImage *          grl_image_new_cellular      (gint                width,
                                                 gint                height,
                                                 gint                tile_size);

/*
 * Properties
 */

GRL_AVAILABLE_IN_ALL
gint                grl_image_get_width         (GrlImage           *self);

GRL_AVAILABLE_IN_ALL
gint                grl_image_get_height        (GrlImage           *self);

GRL_AVAILABLE_IN_ALL
gint                grl_image_get_mipmaps       (GrlImage           *self);

GRL_AVAILABLE_IN_ALL
GrlPixelFormat      grl_image_get_format        (GrlImage           *self);

GRL_AVAILABLE_IN_ALL
gboolean            grl_image_is_valid          (GrlImage           *self);

/*
 * Copy/Extract
 */

GRL_AVAILABLE_IN_ALL
GrlImage *          grl_image_copy              (GrlImage           *self);

GRL_AVAILABLE_IN_ALL
GrlImage *          grl_image_from_region       (GrlImage           *self,
                                                 const GrlRectangle *region);

GRL_AVAILABLE_IN_ALL
GrlImage *          grl_image_resized           (GrlImage           *self,
                                                 gint                new_width,
                                                 gint                new_height);

GRL_AVAILABLE_IN_ALL
GrlImage *          grl_image_scaled_nearest    (GrlImage           *self,
                                                 gint                new_width,
                                                 gint                new_height);

/*
 * Export
 */

GRL_AVAILABLE_IN_ALL
gboolean            grl_image_export            (GrlImage           *self,
                                                 const gchar        *filename);

GRL_AVAILABLE_IN_ALL
guint8 *            grl_image_export_to_memory  (GrlImage           *self,
                                                 const gchar        *file_type,
                                                 gsize              *out_size);

/*
 * Indexed PNG (via rpng)
 */

/**
 * grl_image_new_from_png_indexed:
 * @filename: (type filename): Path to indexed PNG file
 * @palette_out: (out) (optional) (transfer full): Return location for palette, or %NULL
 * @error: (nullable): Return location for error, or %NULL
 *
 * Loads an indexed (palette-based) PNG image.
 *
 * Indexed PNG images use a color palette where each pixel is an 8-bit
 * index into the palette. This function loads such images and optionally
 * returns the palette used.
 *
 * If the PNG file is not indexed (e.g., RGB or RGBA), this function
 * returns %NULL and sets an error.
 *
 * Returns: (transfer full) (nullable): A new #GrlImage, or %NULL on error
 */
GRL_AVAILABLE_IN_ALL
GrlImage *          grl_image_new_from_png_indexed (const gchar      *filename,
                                                    GrlPngPalette   **palette_out,
                                                    GError          **error);

/**
 * grl_image_save_as_png_indexed:
 * @self: A #GrlImage
 * @filename: (type filename): Output filename
 * @palette: The color palette to use
 * @error: (nullable): Return location for error, or %NULL
 *
 * Saves an image as an indexed PNG using the provided palette.
 *
 * The image will be quantized to match the palette colors. The palette
 * should contain all colors needed to represent the image.
 *
 * Note: This function requires the image to already be 8-bit indexed data.
 * For RGBA images, you should first reduce colors to create the palette
 * and indexed data.
 *
 * Returns: %TRUE on success
 */
GRL_AVAILABLE_IN_ALL
gboolean            grl_image_save_as_png_indexed (GrlImage         *self,
                                                   const gchar      *filename,
                                                   GrlPngPalette    *palette,
                                                   GError          **error);

/*
 * Transformations (modify in place)
 */

GRL_AVAILABLE_IN_ALL
void                grl_image_resize            (GrlImage           *self,
                                                 gint                new_width,
                                                 gint                new_height);

GRL_AVAILABLE_IN_ALL
void                grl_image_resize_nearest    (GrlImage           *self,
                                                 gint                new_width,
                                                 gint                new_height);

GRL_AVAILABLE_IN_ALL
void                grl_image_resize_canvas     (GrlImage           *self,
                                                 gint                new_width,
                                                 gint                new_height,
                                                 gint                offset_x,
                                                 gint                offset_y,
                                                 const GrlColor     *fill);

GRL_AVAILABLE_IN_ALL
void                grl_image_crop              (GrlImage           *self,
                                                 const GrlRectangle *crop);

GRL_AVAILABLE_IN_ALL
void                grl_image_flip_vertical     (GrlImage           *self);

GRL_AVAILABLE_IN_ALL
void                grl_image_flip_horizontal   (GrlImage           *self);

GRL_AVAILABLE_IN_ALL
void                grl_image_rotate            (GrlImage           *self,
                                                 gint                degrees);

GRL_AVAILABLE_IN_ALL
void                grl_image_rotate_cw         (GrlImage           *self);

GRL_AVAILABLE_IN_ALL
void                grl_image_rotate_ccw        (GrlImage           *self);

/*
 * Color operations (modify in place)
 */

GRL_AVAILABLE_IN_ALL
void                grl_image_color_tint        (GrlImage           *self,
                                                 const GrlColor     *tint);

GRL_AVAILABLE_IN_ALL
void                grl_image_color_invert      (GrlImage           *self);

GRL_AVAILABLE_IN_ALL
void                grl_image_color_grayscale   (GrlImage           *self);

GRL_AVAILABLE_IN_ALL
void                grl_image_color_contrast    (GrlImage           *self,
                                                 gfloat              contrast);

GRL_AVAILABLE_IN_ALL
void                grl_image_color_brightness  (GrlImage           *self,
                                                 gint                brightness);

GRL_AVAILABLE_IN_ALL
void                grl_image_color_replace     (GrlImage           *self,
                                                 const GrlColor     *color,
                                                 const GrlColor     *replace);

/*
 * Mipmaps and format
 */

GRL_AVAILABLE_IN_ALL
void                grl_image_gen_mipmaps       (GrlImage           *self);

GRL_AVAILABLE_IN_ALL
void                grl_image_set_format        (GrlImage           *self,
                                                 GrlPixelFormat      format);

GRL_AVAILABLE_IN_ALL
void                grl_image_alpha_premultiply (GrlImage           *self);

GRL_AVAILABLE_IN_ALL
void                grl_image_alpha_crop        (GrlImage           *self,
                                                 gfloat              threshold);

GRL_AVAILABLE_IN_ALL
void                grl_image_alpha_clear       (GrlImage           *self,
                                                 const GrlColor     *color,
                                                 gfloat              threshold);

GRL_AVAILABLE_IN_ALL
void                grl_image_alpha_mask        (GrlImage           *self,
                                                 GrlImage           *alpha_mask);

/*
 * Drawing on image
 */

GRL_AVAILABLE_IN_ALL
void                grl_image_clear_background  (GrlImage           *self,
                                                 const GrlColor     *color);

GRL_AVAILABLE_IN_ALL
void                grl_image_draw_pixel        (GrlImage           *self,
                                                 gint                x,
                                                 gint                y,
                                                 const GrlColor     *color);

GRL_AVAILABLE_IN_ALL
void                grl_image_draw_line         (GrlImage           *self,
                                                 gint                start_x,
                                                 gint                start_y,
                                                 gint                end_x,
                                                 gint                end_y,
                                                 const GrlColor     *color);

GRL_AVAILABLE_IN_ALL
void                grl_image_draw_circle       (GrlImage           *self,
                                                 gint                center_x,
                                                 gint                center_y,
                                                 gint                radius,
                                                 const GrlColor     *color);

GRL_AVAILABLE_IN_ALL
void                grl_image_draw_rectangle    (GrlImage           *self,
                                                 const GrlRectangle *rect,
                                                 const GrlColor     *color);

GRL_AVAILABLE_IN_ALL
void                grl_image_draw_rectangle_lines (GrlImage        *self,
                                                    const GrlRectangle *rect,
                                                    gint             thickness,
                                                    const GrlColor  *color);

GRL_AVAILABLE_IN_ALL
void                grl_image_draw_image        (GrlImage           *self,
                                                 GrlImage           *src,
                                                 const GrlRectangle *src_rect,
                                                 const GrlRectangle *dst_rect,
                                                 const GrlColor     *tint);

GRL_AVAILABLE_IN_ALL
void                grl_image_draw_text         (GrlImage           *self,
                                                 const gchar        *text,
                                                 gint                x,
                                                 gint                y,
                                                 gint                font_size,
                                                 const GrlColor     *color);

GRL_AVAILABLE_IN_ALL
void                grl_image_draw_text_bitmap  (GrlImage           *self,
                                                 const gchar        *text,
                                                 gint                x,
                                                 gint                y,
                                                 gint                font_size,
                                                 const GrlColor     *color);

GRL_AVAILABLE_IN_ALL
GrlVector2 *        grl_image_measure_text_bitmap (const gchar      *text,
                                                   gint              font_size);

/*
 * Drawing state (blend mode, clipping, anti-aliasing)
 *
 * These apply to every grl_image_draw_* primitive. The default blend mode is
 * %GRL_IMAGE_BLEND_REPLACE, which is byte-identical to drawing with no state
 * set. Non-REPLACE blending and anti-aliasing require an R8G8B8A8 image.
 */

GRL_AVAILABLE_IN_ALL
void                grl_image_set_blend_mode    (GrlImage           *self,
                                                 GrlImageBlendMode   mode);

GRL_AVAILABLE_IN_ALL
GrlImageBlendMode   grl_image_get_blend_mode    (GrlImage           *self);

GRL_AVAILABLE_IN_ALL
void                grl_image_set_blend_color_space (GrlImage           *self,
                                                     GrlImageColorSpace  space);

GRL_AVAILABLE_IN_ALL
GrlImageColorSpace  grl_image_get_blend_color_space (GrlImage           *self);

GRL_AVAILABLE_IN_ALL
void                grl_image_push_matrix       (GrlImage           *self);

GRL_AVAILABLE_IN_ALL
void                grl_image_pop_matrix        (GrlImage           *self);

GRL_AVAILABLE_IN_ALL
void                grl_image_reset_matrix      (GrlImage           *self);

GRL_AVAILABLE_IN_ALL
void                grl_image_translate         (GrlImage           *self,
                                                 gfloat              x,
                                                 gfloat              y);

GRL_AVAILABLE_IN_ALL
void                grl_image_scale             (GrlImage           *self,
                                                 gfloat              sx,
                                                 gfloat              sy);

GRL_AVAILABLE_IN_ALL
void                grl_image_rotate_matrix     (GrlImage           *self,
                                                 gfloat              radians);

GRL_AVAILABLE_IN_ALL
void                grl_image_set_matrix        (GrlImage           *self,
                                                 const GrlMatrix    *matrix);

GRL_AVAILABLE_IN_ALL
GrlMatrix *         grl_image_get_matrix        (GrlImage           *self);

GRL_AVAILABLE_IN_ALL
void                grl_image_set_clip_rect     (GrlImage           *self,
                                                 const GrlRectangle *clip);

GRL_AVAILABLE_IN_ALL
gboolean            grl_image_get_clip_rect     (GrlImage           *self,
                                                 GrlRectangle       *out_clip);

GRL_AVAILABLE_IN_ALL
void                grl_image_set_antialias     (GrlImage           *self,
                                                 gboolean            enabled);

GRL_AVAILABLE_IN_ALL
gboolean            grl_image_get_antialias     (GrlImage           *self);

/*
 * Drawing on image - additional primitives
 */

GRL_AVAILABLE_IN_ALL
void                grl_image_draw_line_ex      (GrlImage           *self,
                                                 const GrlVector2   *start,
                                                 const GrlVector2   *end,
                                                 gint                thickness,
                                                 const GrlColor     *color);

GRL_AVAILABLE_IN_ALL
void                grl_image_draw_line_thin    (GrlImage           *self,
                                                 const GrlVector2   *start,
                                                 const GrlVector2   *end,
                                                 gfloat              thickness,
                                                 const GrlColor     *color);

GRL_AVAILABLE_IN_ALL
void                grl_image_draw_circle_lines (GrlImage           *self,
                                                 gint                center_x,
                                                 gint                center_y,
                                                 gint                radius,
                                                 gint                thickness,
                                                 const GrlColor     *color);

GRL_AVAILABLE_IN_ALL
void                grl_image_draw_ellipse      (GrlImage           *self,
                                                 gint                center_x,
                                                 gint                center_y,
                                                 gint                radius_x,
                                                 gint                radius_y,
                                                 const GrlColor     *color);

GRL_AVAILABLE_IN_ALL
void                grl_image_draw_ellipse_lines (GrlImage          *self,
                                                  gint               center_x,
                                                  gint               center_y,
                                                  gint               radius_x,
                                                  gint               radius_y,
                                                  gint               thickness,
                                                  const GrlColor    *color);

GRL_AVAILABLE_IN_ALL
void                grl_image_draw_triangle     (GrlImage           *self,
                                                 const GrlVector2   *v1,
                                                 const GrlVector2   *v2,
                                                 const GrlVector2   *v3,
                                                 const GrlColor     *color);

GRL_AVAILABLE_IN_ALL
void                grl_image_draw_triangle_lines (GrlImage         *self,
                                                   const GrlVector2 *v1,
                                                   const GrlVector2 *v2,
                                                   const GrlVector2 *v3,
                                                   gint              thickness,
                                                   const GrlColor   *color);

GRL_AVAILABLE_IN_ALL
void                grl_image_draw_polygon      (GrlImage           *self,
                                                 const GrlVector2   *points,
                                                 gint                point_count,
                                                 const GrlColor     *color);

GRL_AVAILABLE_IN_ALL
void                grl_image_draw_polyline     (GrlImage           *self,
                                                 const GrlVector2   *points,
                                                 gint                point_count,
                                                 gboolean            closed,
                                                 gint                thickness,
                                                 const GrlColor     *color);

GRL_AVAILABLE_IN_ALL
void                grl_image_draw_bezier       (GrlImage           *self,
                                                 const GrlVector2   *p0,
                                                 const GrlVector2   *p1,
                                                 const GrlVector2   *p2,
                                                 const GrlVector2   *p3,
                                                 gint                thickness,
                                                 const GrlColor     *color);

GRL_AVAILABLE_IN_ALL
void                grl_image_draw_gradient_rect (GrlImage          *self,
                                                  const GrlRectangle *rect,
                                                  const GrlColor    *color_a,
                                                  const GrlColor    *color_b,
                                                  GrlGradientAxis    axis);

GRL_AVAILABLE_IN_ALL
void                grl_image_draw_gradient_radial (GrlImage        *self,
                                                    gint             center_x,
                                                    gint             center_y,
                                                    gint             radius,
                                                    const GrlColor  *inner,
                                                    const GrlColor  *outer);

GRL_AVAILABLE_IN_ALL
void                grl_image_flood_fill        (GrlImage           *self,
                                                 gint                x,
                                                 gint                y,
                                                 const GrlColor     *color,
                                                 gint                tolerance);

/*
 * Pixel access
 */

GRL_AVAILABLE_IN_ALL
GrlColor *          grl_image_get_pixel         (GrlImage           *self,
                                                 gint                x,
                                                 gint                y);

/*
 * Porter-Duff compositing (whole-image)
 *
 * grl_image_composite() composites @src onto @self at offset (@dst_x, @dst_y)
 * using standard Porter-Duff coverage algebra on premultiplied alpha. Both
 * images must be R8G8B8A8; if either is not R8G8B8A8 the call returns silently.
 * @self's clip rectangle is honoured. If @self's blend colour space is LINEAR,
 * RGB compositing is performed in linear light using the sRGB LUTs; otherwise
 * compositing is in gamma (8-bit sRGB) space.
 *
 * Outside-mask / outside-src pixels: pixels of @self that lie outside the
 * mapped region of @src are left completely unchanged.
 */

GRL_AVAILABLE_IN_ALL
void                grl_image_composite         (GrlImage           *self,
                                                 GrlImage           *src,
                                                 GrlPorterDuffOp     op,
                                                 gint                dst_x,
                                                 gint                dst_y);

/*
 * First-class alpha mask
 *
 * grl_image_new_mask() returns a single-channel GRAYSCALE (1 byte/pixel) image
 * initialised to zero (fully transparent when used as a mask). Drawing
 * primitives may be used on the mask — the drawn luminance becomes coverage.
 *
 * grl_image_apply_mask() multiplies the alpha channel of @self (must be
 * R8G8B8A8) by the corresponding mask value at (x - @offset_x, y - @offset_y).
 * Pixels of @self that map outside the mask region are set to alpha 0 (cut).
 * @self's clip rectangle is honoured.
 *
 * Contract: pixels of @self outside the mask's mapped area have their alpha
 * zeroed (fully cut), not left unchanged. This is the standard stencil
 * contract: the mask defines exactly which pixels survive.
 */

GRL_AVAILABLE_IN_ALL
GrlImage *          grl_image_new_mask          (gint                width,
                                                 gint                height);

GRL_AVAILABLE_IN_ALL
void                grl_image_apply_mask        (GrlImage           *self,
                                                 GrlImage           *mask,
                                                 gint                offset_x,
                                                 gint                offset_y);

/*
 * Box blur
 *
 * Separable (horizontal then vertical) box blur with clamped-edge sampling.
 * Blurs all four channels (R, G, B, A) so it works on shadow silhouettes.
 * @radius <= 0 is a no-op. @self must be R8G8B8A8; on other formats returns
 * silently. Uses a temporary buffer; the original is replaced in-place.
 */

GRL_AVAILABLE_IN_ALL
void                grl_image_blur_box          (GrlImage           *self,
                                                 gint                radius);

/*
 * Vector path fill and stroke
 */

/**
 * grl_image_fill_path:
 * @self: A #GrlImage.
 * @path: The #GrlPath to fill.
 * @rule: Winding rule controlling which regions are considered "inside".
 * @color: Fill color.
 *
 * Fills @path onto @self using the specified winding @rule. Curves are
 * flattened to polylines with a tolerance of ~0.25 px before rasterization.
 * The image's current transform, blend mode, clip rectangle, and anti-alias
 * state are all honoured.
 *
 * Under %GRL_FILL_RULE_NONZERO, a region is filled if the signed winding
 * count of any ray through it is non-zero; this correctly leaves holes in
 * paths with oppositely-wound subpaths. Under %GRL_FILL_RULE_EVEN_ODD, a
 * region is filled if a ray crosses the path boundary an odd number of times.
 */
GRL_AVAILABLE_IN_ALL
void                grl_image_fill_path         (GrlImage           *self,
                                                 GrlPath            *path,
                                                 GrlFillRule         rule,
                                                 const GrlColor     *color);

/**
 * grl_image_stroke_path:
 * @self: A #GrlImage.
 * @path: The #GrlPath to stroke.
 * @thickness: Stroke thickness in pixels (minimum 1).
 * @color: Stroke color.
 *
 * Strokes @path onto @self with round end-caps. Each subpath is stroked
 * independently according to its closed flag. Curves are flattened before
 * rasterization. The image's current transform (thickness is scaled by the
 * matrix's area-preserving scale factor), blend mode, clip rectangle, and
 * anti-alias state are all honoured.
 */
GRL_AVAILABLE_IN_ALL
void                grl_image_stroke_path       (GrlImage           *self,
                                                 GrlPath            *path,
                                                 gint                thickness,
                                                 const GrlColor     *color);

/*
 * Internal - get raylib Image handle
 */

gpointer            grl_image_get_handle        (GrlImage           *self);

/* ==========================================================================
 * Paint engine — noise, bloom, stamp-along-path, and layer compositing
 * (salvaged from MR !2 feat/paint-engine-pro)
 * ========================================================================== */

/*
 * Noise overlay — applies pseudo-random per-pixel noise blended into
 * the image. Deterministic for a given seed.
 */
typedef enum {
    GRL_NOISE_BLEND_ADDITIVE = 0,
    GRL_NOISE_BLEND_MULTIPLY,
    GRL_NOISE_BLEND_OVERLAY
} GrlNoiseBlend;

GRL_AVAILABLE_IN_ALL
void                grl_image_apply_noise       (GrlImage           *self,
                                                 GrlNoiseBlend       blend,
                                                 gfloat              amplitude,
                                                 gfloat              frequency,
                                                 guint32             seed);

/*
 * Bloom — threshold + blur + additive composite back. Brightens
 * highlights with a soft halo, simulating HDR overexposure.
 */
GRL_AVAILABLE_IN_ALL
void                grl_image_apply_bloom       (GrlImage           *self,
                                                 guint8              threshold,
                                                 gint                blur_radius,
                                                 gfloat              intensity);

/*
 * Stamp brush along path — samples points along the path at `spacing`
 * pixel intervals and stamps the brush image (centered) at each point.
 * Brush is alpha-blended; transparent brush pixels don't overwrite dst.
 */
GRL_AVAILABLE_IN_ALL
void                grl_image_stamp_along_path  (GrlImage           *self,
                                                 GrlImage           *brush,
                                                 GrlPath            *path,
                                                 gfloat              spacing,
                                                 gfloat              jitter,
                                                 guint32             seed);

/*
 * Layer + blend modes — off-screen drawing buffers composited back
 * into a destination image with selectable blend math.
 */
typedef enum {
    GRL_LAYER_BLEND_NORMAL = 0,
    GRL_LAYER_BLEND_MULTIPLY,
    GRL_LAYER_BLEND_SCREEN,
    GRL_LAYER_BLEND_OVERLAY,
    GRL_LAYER_BLEND_SOFT_LIGHT,
    GRL_LAYER_BLEND_ADD,
    GRL_LAYER_BLEND_COLOR_DODGE,
    GRL_LAYER_BLEND_COLOR_BURN
} GrlLayerBlendMode;

#define GRL_TYPE_LAYER (grl_layer_get_type ())
typedef struct _GrlLayer GrlLayer;

GRL_AVAILABLE_IN_ALL
GType               grl_layer_get_type          (void) G_GNUC_CONST;

GRL_AVAILABLE_IN_ALL
GrlLayer *          grl_layer_new               (gint                width,
                                                 gint                height);

GRL_AVAILABLE_IN_ALL
GrlLayer *          grl_layer_ref               (GrlLayer           *self);

GRL_AVAILABLE_IN_ALL
void                grl_layer_unref             (GrlLayer           *self);

GRL_AVAILABLE_IN_ALL
GrlImage *          grl_layer_get_image         (GrlLayer           *self);

GRL_AVAILABLE_IN_ALL
void                grl_image_composite_layer   (GrlImage           *dst,
                                                 GrlLayer           *layer,
                                                 gint                x,
                                                 gint                y,
                                                 GrlLayerBlendMode   mode,
                                                 gfloat              opacity);

G_DEFINE_AUTOPTR_CLEANUP_FUNC (GrlLayer, grl_layer_unref)

G_END_DECLS
