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

/*
 * Pixel access
 */

GRL_AVAILABLE_IN_ALL
GrlColor *          grl_image_get_pixel         (GrlImage           *self,
                                                 gint                x,
                                                 gint                y);

/*
 * Internal - get raylib Image handle
 */

gpointer            grl_image_get_handle        (GrlImage           *self);

G_END_DECLS
