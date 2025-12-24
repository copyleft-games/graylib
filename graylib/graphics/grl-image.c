/* grl-image.c
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * CPU-side image implementation.
 */

#include "config.h"
#include "grl-image.h"
#include "grl-graphics-private.h"
#include <raylib.h>

/**
 * SECTION:grl-image
 * @title: GrlImage
 * @short_description: CPU-side image for loading and manipulation
 *
 * #GrlImage represents an image stored in CPU memory (RAM). It can be
 * loaded from files, generated procedurally, or captured from the screen.
 *
 * Images can be manipulated (resized, cropped, color adjusted, etc.) and
 * then uploaded to the GPU as a #GrlTexture for rendering.
 *
 * Example:
 * |[<!-- language="C" -->
 * g_autoptr(GrlImage) img = grl_image_new_from_file ("sprite.png");
 *
 * // Manipulate the image
 * grl_image_resize (img, 64, 64);
 * grl_image_flip_vertical (img);
 *
 * // Create texture from image
 * g_autoptr(GrlTexture) tex = grl_texture_new_from_image (img);
 * ]|
 */

struct _GrlImage
{
    GObject parent_instance;

    Image handle;
    gboolean valid;
};

G_DEFINE_TYPE (GrlImage, grl_image, G_TYPE_OBJECT)

enum
{
    PROP_0,
    PROP_WIDTH,
    PROP_HEIGHT,
    PROP_FORMAT,
    PROP_MIPMAPS,
    PROP_VALID,
    N_PROPS
};

static GParamSpec *properties[N_PROPS];

/*
 * Helper macros
 */

#define GRL_TO_RAYLIB_COLOR(c) \
    ((Color){ .r = (c)->r, .g = (c)->g, .b = (c)->b, .a = (c)->a })

#define GRL_TO_RAYLIB_RECTANGLE(r) \
    ((Rectangle){ .x = (r)->x, .y = (r)->y, .width = (r)->width, .height = (r)->height })

/*
 * Private helper to create GrlImage from raylib Image
 */

static GrlImage *
grl_image_new_from_handle (Image handle)
{
    GrlImage *self;

    self = g_object_new (GRL_TYPE_IMAGE, NULL);
    self->handle = handle;
    self->valid = (handle.data != NULL);

    return self;
}

/*
 * Internal constructor for use by other graphics modules.
 * Takes ownership of the raylib Image handle.
 */

GrlImage *
grl_image_new_from_raw_handle (gpointer handle)
{
    Image *img;

    g_return_val_if_fail (handle != NULL, NULL);

    img = (Image *)handle;

    return grl_image_new_from_handle (*img);
}

/*
 * GObject virtual methods
 */

static void
grl_image_get_property (GObject    *object,
                        guint       prop_id,
                        GValue     *value,
                        GParamSpec *pspec)
{
    GrlImage *self = GRL_IMAGE (object);

    switch (prop_id)
    {
    case PROP_WIDTH:
        g_value_set_int (value, self->handle.width);
        break;

    case PROP_HEIGHT:
        g_value_set_int (value, self->handle.height);
        break;

    case PROP_FORMAT:
        g_value_set_enum (value, (GrlPixelFormat)self->handle.format);
        break;

    case PROP_MIPMAPS:
        g_value_set_int (value, self->handle.mipmaps);
        break;

    case PROP_VALID:
        g_value_set_boolean (value, self->valid);
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static void
grl_image_finalize (GObject *object)
{
    GrlImage *self = GRL_IMAGE (object);

    if (self->valid && self->handle.data != NULL)
    {
        UnloadImage (self->handle);
        self->handle.data = NULL;
        self->valid = FALSE;
    }

    G_OBJECT_CLASS (grl_image_parent_class)->finalize (object);
}

static void
grl_image_class_init (GrlImageClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->get_property = grl_image_get_property;
    object_class->finalize = grl_image_finalize;

    /**
     * GrlImage:width:
     *
     * Width of the image in pixels.
     */
    properties[PROP_WIDTH] =
        g_param_spec_int ("width",
                          "Width",
                          "Image width in pixels",
                          0, G_MAXINT, 0,
                          G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

    /**
     * GrlImage:height:
     *
     * Height of the image in pixels.
     */
    properties[PROP_HEIGHT] =
        g_param_spec_int ("height",
                          "Height",
                          "Image height in pixels",
                          0, G_MAXINT, 0,
                          G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

    /**
     * GrlImage:format:
     *
     * Pixel format of the image.
     */
    properties[PROP_FORMAT] =
        g_param_spec_enum ("format",
                           "Format",
                           "Pixel format",
                           GRL_TYPE_PIXEL_FORMAT,
                           GRL_PIXELFORMAT_UNCOMPRESSED_R8G8B8A8,
                           G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

    /**
     * GrlImage:mipmaps:
     *
     * Number of mipmap levels.
     */
    properties[PROP_MIPMAPS] =
        g_param_spec_int ("mipmaps",
                          "Mipmaps",
                          "Number of mipmap levels",
                          0, G_MAXINT, 1,
                          G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

    /**
     * GrlImage:valid:
     *
     * Whether the image data is valid.
     */
    properties[PROP_VALID] =
        g_param_spec_boolean ("valid",
                              "Valid",
                              "Whether the image is valid",
                              FALSE,
                              G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

    g_object_class_install_properties (object_class, N_PROPS, properties);
}

static void
grl_image_init (GrlImage *self)
{
    self->handle.data = NULL;
    self->handle.width = 0;
    self->handle.height = 0;
    self->handle.mipmaps = 1;
    self->handle.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;
    self->valid = FALSE;
}

/*
 * Public API - Loading
 */

/**
 * grl_image_new_from_file:
 * @filename: Path to the image file.
 *
 * Loads an image from a file into CPU memory.
 * Supported formats: PNG, BMP, TGA, JPG, GIF, QOI, PSD, DDS, HDR, KTX,
 * ASTC, PKM, PVR.
 *
 * Returns: (transfer full): A new #GrlImage, or an invalid image if loading failed.
 */
GrlImage *
grl_image_new_from_file (const gchar *filename)
{
    Image handle;

    g_return_val_if_fail (filename != NULL, NULL);

    handle = LoadImage (filename);
    return grl_image_new_from_handle (handle);
}

/**
 * grl_image_new_from_memory:
 * @file_type: File type extension (e.g., ".png", ".jpg").
 * @data: (array length=data_size): Image data in memory.
 * @data_size: Size of the data in bytes.
 *
 * Loads an image from a memory buffer.
 *
 * Returns: (transfer full): A new #GrlImage, or an invalid image if loading failed.
 */
GrlImage *
grl_image_new_from_memory (const gchar  *file_type,
                           const guint8 *data,
                           gsize         data_size)
{
    Image handle;

    g_return_val_if_fail (file_type != NULL, NULL);
    g_return_val_if_fail (data != NULL, NULL);

    handle = LoadImageFromMemory (file_type, data, (int)data_size);
    return grl_image_new_from_handle (handle);
}

/**
 * grl_image_new_from_screen:
 *
 * Captures the current screen contents as an image.
 * Must be called after grl_window_begin_drawing().
 *
 * Returns: (transfer full): A new #GrlImage with screen contents.
 */
GrlImage *
grl_image_new_from_screen (void)
{
    Image handle;

    handle = LoadImageFromScreen ();
    return grl_image_new_from_handle (handle);
}

/*
 * Public API - Generation
 */

/**
 * grl_image_new_color:
 * @width: Image width.
 * @height: Image height.
 * @color: Fill color.
 *
 * Generates a solid color image.
 *
 * Returns: (transfer full): A new #GrlImage.
 */
GrlImage *
grl_image_new_color (gint            width,
                     gint            height,
                     const GrlColor *color)
{
    Image handle;

    g_return_val_if_fail (width > 0, NULL);
    g_return_val_if_fail (height > 0, NULL);
    g_return_val_if_fail (color != NULL, NULL);

    handle = GenImageColor (width, height, GRL_TO_RAYLIB_COLOR (color));
    return grl_image_new_from_handle (handle);
}

/**
 * grl_image_new_gradient_linear:
 * @width: Image width.
 * @height: Image height.
 * @direction: Gradient direction in degrees (0 = vertical).
 * @start: Start color.
 * @end: End color.
 *
 * Generates a linear gradient image.
 *
 * Returns: (transfer full): A new #GrlImage.
 */
GrlImage *
grl_image_new_gradient_linear (gint            width,
                               gint            height,
                               gint            direction,
                               const GrlColor *start,
                               const GrlColor *end)
{
    Image handle;

    g_return_val_if_fail (width > 0, NULL);
    g_return_val_if_fail (height > 0, NULL);
    g_return_val_if_fail (start != NULL, NULL);
    g_return_val_if_fail (end != NULL, NULL);

    handle = GenImageGradientLinear (width, height, direction,
                                     GRL_TO_RAYLIB_COLOR (start),
                                     GRL_TO_RAYLIB_COLOR (end));
    return grl_image_new_from_handle (handle);
}

/**
 * grl_image_new_gradient_radial:
 * @width: Image width.
 * @height: Image height.
 * @density: Gradient density factor.
 * @inner: Inner (center) color.
 * @outer: Outer (edge) color.
 *
 * Generates a radial gradient image.
 *
 * Returns: (transfer full): A new #GrlImage.
 */
GrlImage *
grl_image_new_gradient_radial (gint            width,
                               gint            height,
                               gfloat          density,
                               const GrlColor *inner,
                               const GrlColor *outer)
{
    Image handle;

    g_return_val_if_fail (width > 0, NULL);
    g_return_val_if_fail (height > 0, NULL);
    g_return_val_if_fail (inner != NULL, NULL);
    g_return_val_if_fail (outer != NULL, NULL);

    handle = GenImageGradientRadial (width, height, density,
                                     GRL_TO_RAYLIB_COLOR (inner),
                                     GRL_TO_RAYLIB_COLOR (outer));
    return grl_image_new_from_handle (handle);
}

/**
 * grl_image_new_checked:
 * @width: Image width.
 * @height: Image height.
 * @checks_x: Number of checks horizontally.
 * @checks_y: Number of checks vertically.
 * @color1: First check color.
 * @color2: Second check color.
 *
 * Generates a checkerboard pattern image.
 *
 * Returns: (transfer full): A new #GrlImage.
 */
GrlImage *
grl_image_new_checked (gint            width,
                       gint            height,
                       gint            checks_x,
                       gint            checks_y,
                       const GrlColor *color1,
                       const GrlColor *color2)
{
    Image handle;

    g_return_val_if_fail (width > 0, NULL);
    g_return_val_if_fail (height > 0, NULL);
    g_return_val_if_fail (color1 != NULL, NULL);
    g_return_val_if_fail (color2 != NULL, NULL);

    handle = GenImageChecked (width, height, checks_x, checks_y,
                              GRL_TO_RAYLIB_COLOR (color1),
                              GRL_TO_RAYLIB_COLOR (color2));
    return grl_image_new_from_handle (handle);
}

/**
 * grl_image_new_white_noise:
 * @width: Image width.
 * @height: Image height.
 * @factor: Noise factor.
 *
 * Generates a white noise image.
 *
 * Returns: (transfer full): A new #GrlImage.
 */
GrlImage *
grl_image_new_white_noise (gint   width,
                           gint   height,
                           gfloat factor)
{
    Image handle;

    g_return_val_if_fail (width > 0, NULL);
    g_return_val_if_fail (height > 0, NULL);

    handle = GenImageWhiteNoise (width, height, factor);
    return grl_image_new_from_handle (handle);
}

/**
 * grl_image_new_perlin_noise:
 * @width: Image width.
 * @height: Image height.
 * @offset_x: X offset in noise space.
 * @offset_y: Y offset in noise space.
 * @scale: Noise scale.
 *
 * Generates a Perlin noise image.
 *
 * Returns: (transfer full): A new #GrlImage.
 */
GrlImage *
grl_image_new_perlin_noise (gint   width,
                            gint   height,
                            gint   offset_x,
                            gint   offset_y,
                            gfloat scale)
{
    Image handle;

    g_return_val_if_fail (width > 0, NULL);
    g_return_val_if_fail (height > 0, NULL);

    handle = GenImagePerlinNoise (width, height, offset_x, offset_y, scale);
    return grl_image_new_from_handle (handle);
}

/**
 * grl_image_new_cellular:
 * @width: Image width.
 * @height: Image height.
 * @tile_size: Size of each cell.
 *
 * Generates a cellular (Voronoi-like) pattern image.
 *
 * Returns: (transfer full): A new #GrlImage.
 */
GrlImage *
grl_image_new_cellular (gint width,
                        gint height,
                        gint tile_size)
{
    Image handle;

    g_return_val_if_fail (width > 0, NULL);
    g_return_val_if_fail (height > 0, NULL);
    g_return_val_if_fail (tile_size > 0, NULL);

    handle = GenImageCellular (width, height, tile_size);
    return grl_image_new_from_handle (handle);
}

/*
 * Public API - Properties
 */

/**
 * grl_image_get_width:
 * @self: A #GrlImage.
 *
 * Gets the image width.
 *
 * Returns: Width in pixels.
 */
gint
grl_image_get_width (GrlImage *self)
{
    g_return_val_if_fail (GRL_IS_IMAGE (self), 0);

    return self->handle.width;
}

/**
 * grl_image_get_height:
 * @self: A #GrlImage.
 *
 * Gets the image height.
 *
 * Returns: Height in pixels.
 */
gint
grl_image_get_height (GrlImage *self)
{
    g_return_val_if_fail (GRL_IS_IMAGE (self), 0);

    return self->handle.height;
}

/**
 * grl_image_get_mipmaps:
 * @self: A #GrlImage.
 *
 * Gets the number of mipmap levels.
 *
 * Returns: Number of mipmaps.
 */
gint
grl_image_get_mipmaps (GrlImage *self)
{
    g_return_val_if_fail (GRL_IS_IMAGE (self), 0);

    return self->handle.mipmaps;
}

/**
 * grl_image_get_format:
 * @self: A #GrlImage.
 *
 * Gets the pixel format.
 *
 * Returns: The #GrlPixelFormat.
 */
GrlPixelFormat
grl_image_get_format (GrlImage *self)
{
    g_return_val_if_fail (GRL_IS_IMAGE (self), GRL_PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);

    return (GrlPixelFormat)self->handle.format;
}

/**
 * grl_image_is_valid:
 * @self: A #GrlImage.
 *
 * Checks if the image data is valid.
 *
 * Returns: %TRUE if the image is valid.
 */
gboolean
grl_image_is_valid (GrlImage *self)
{
    g_return_val_if_fail (GRL_IS_IMAGE (self), FALSE);

    return self->valid && self->handle.data != NULL;
}

/*
 * Public API - Copy/Extract
 */

/**
 * grl_image_copy:
 * @self: A #GrlImage.
 *
 * Creates a copy of the image.
 *
 * Returns: (transfer full): A new #GrlImage copy.
 */
GrlImage *
grl_image_copy (GrlImage *self)
{
    Image handle;

    g_return_val_if_fail (GRL_IS_IMAGE (self), NULL);

    handle = ImageCopy (self->handle);
    return grl_image_new_from_handle (handle);
}

/**
 * grl_image_from_region:
 * @self: A #GrlImage.
 * @region: Region to extract.
 *
 * Creates a new image from a region of this image.
 *
 * Returns: (transfer full): A new #GrlImage with the extracted region.
 */
GrlImage *
grl_image_from_region (GrlImage           *self,
                       const GrlRectangle *region)
{
    Image handle;

    g_return_val_if_fail (GRL_IS_IMAGE (self), NULL);
    g_return_val_if_fail (region != NULL, NULL);

    handle = ImageFromImage (self->handle, GRL_TO_RAYLIB_RECTANGLE (region));
    return grl_image_new_from_handle (handle);
}

/*
 * Public API - Export
 */

/**
 * grl_image_export:
 * @self: A #GrlImage.
 * @filename: Output file path.
 *
 * Exports the image to a file. Format is determined by extension.
 *
 * Returns: %TRUE on success.
 */
gboolean
grl_image_export (GrlImage    *self,
                  const gchar *filename)
{
    g_return_val_if_fail (GRL_IS_IMAGE (self), FALSE);
    g_return_val_if_fail (filename != NULL, FALSE);

    return ExportImage (self->handle, filename);
}

/**
 * grl_image_export_to_memory:
 * @self: A #GrlImage.
 * @file_type: Output format (e.g., ".png").
 * @out_size: (out): Output size in bytes.
 *
 * Exports the image to a memory buffer.
 *
 * Returns: (transfer full) (array length=out_size): The exported data, or %NULL on failure.
 *          Free with g_free().
 */
guint8 *
grl_image_export_to_memory (GrlImage    *self,
                            const gchar *file_type,
                            gsize       *out_size)
{
    unsigned char *data;
    guint8        *result;
    int            size;

    g_return_val_if_fail (GRL_IS_IMAGE (self), NULL);
    g_return_val_if_fail (file_type != NULL, NULL);
    g_return_val_if_fail (out_size != NULL, NULL);

    data = ExportImageToMemory (self->handle, file_type, &size);
    if (data != NULL)
    {
        *out_size = (gsize)size;
        /* ExportImageToMemory uses RL_MALLOC, need to copy to g_malloc */
        result = g_memdup2 (data, size);
        RL_FREE (data);
        return result;
    }

    *out_size = 0;
    return NULL;
}

/*
 * Public API - Transformations
 */

/**
 * grl_image_resize:
 * @self: A #GrlImage.
 * @new_width: New width.
 * @new_height: New height.
 *
 * Resizes the image using bicubic scaling.
 */
void
grl_image_resize (GrlImage *self,
                  gint      new_width,
                  gint      new_height)
{
    g_return_if_fail (GRL_IS_IMAGE (self));
    g_return_if_fail (new_width > 0);
    g_return_if_fail (new_height > 0);

    ImageResize (&self->handle, new_width, new_height);
}

/**
 * grl_image_resize_nearest:
 * @self: A #GrlImage.
 * @new_width: New width.
 * @new_height: New height.
 *
 * Resizes the image using nearest-neighbor scaling.
 * Better for pixel art.
 */
void
grl_image_resize_nearest (GrlImage *self,
                          gint      new_width,
                          gint      new_height)
{
    g_return_if_fail (GRL_IS_IMAGE (self));
    g_return_if_fail (new_width > 0);
    g_return_if_fail (new_height > 0);

    ImageResizeNN (&self->handle, new_width, new_height);
}

/**
 * grl_image_resize_canvas:
 * @self: A #GrlImage.
 * @new_width: New canvas width.
 * @new_height: New canvas height.
 * @offset_x: X offset for the original image.
 * @offset_y: Y offset for the original image.
 * @fill: Fill color for new pixels.
 *
 * Resizes the canvas without scaling the image.
 */
void
grl_image_resize_canvas (GrlImage       *self,
                         gint            new_width,
                         gint            new_height,
                         gint            offset_x,
                         gint            offset_y,
                         const GrlColor *fill)
{
    g_return_if_fail (GRL_IS_IMAGE (self));
    g_return_if_fail (fill != NULL);

    ImageResizeCanvas (&self->handle, new_width, new_height,
                       offset_x, offset_y, GRL_TO_RAYLIB_COLOR (fill));
}

/**
 * grl_image_crop:
 * @self: A #GrlImage.
 * @crop: Crop region.
 *
 * Crops the image to the specified region.
 */
void
grl_image_crop (GrlImage           *self,
                const GrlRectangle *crop)
{
    g_return_if_fail (GRL_IS_IMAGE (self));
    g_return_if_fail (crop != NULL);

    ImageCrop (&self->handle, GRL_TO_RAYLIB_RECTANGLE (crop));
}

/**
 * grl_image_flip_vertical:
 * @self: A #GrlImage.
 *
 * Flips the image vertically.
 */
void
grl_image_flip_vertical (GrlImage *self)
{
    g_return_if_fail (GRL_IS_IMAGE (self));

    ImageFlipVertical (&self->handle);
}

/**
 * grl_image_flip_horizontal:
 * @self: A #GrlImage.
 *
 * Flips the image horizontally.
 */
void
grl_image_flip_horizontal (GrlImage *self)
{
    g_return_if_fail (GRL_IS_IMAGE (self));

    ImageFlipHorizontal (&self->handle);
}

/**
 * grl_image_rotate:
 * @self: A #GrlImage.
 * @degrees: Rotation angle (90, 180, or 270).
 *
 * Rotates the image by the specified degrees.
 */
void
grl_image_rotate (GrlImage *self,
                  gint      degrees)
{
    g_return_if_fail (GRL_IS_IMAGE (self));

    switch (degrees)
    {
    case 90:
        ImageRotateCW (&self->handle);
        break;
    case 180:
        ImageRotateCW (&self->handle);
        ImageRotateCW (&self->handle);
        break;
    case 270:
        ImageRotateCCW (&self->handle);
        break;
    default:
        g_warning ("grl_image_rotate: Only 90, 180, 270 degrees supported");
        break;
    }
}

/**
 * grl_image_rotate_cw:
 * @self: A #GrlImage.
 *
 * Rotates the image 90 degrees clockwise.
 */
void
grl_image_rotate_cw (GrlImage *self)
{
    g_return_if_fail (GRL_IS_IMAGE (self));

    ImageRotateCW (&self->handle);
}

/**
 * grl_image_rotate_ccw:
 * @self: A #GrlImage.
 *
 * Rotates the image 90 degrees counter-clockwise.
 */
void
grl_image_rotate_ccw (GrlImage *self)
{
    g_return_if_fail (GRL_IS_IMAGE (self));

    ImageRotateCCW (&self->handle);
}

/*
 * Public API - Color operations
 */

/**
 * grl_image_color_tint:
 * @self: A #GrlImage.
 * @tint: Tint color.
 *
 * Applies a color tint to the image.
 */
void
grl_image_color_tint (GrlImage       *self,
                      const GrlColor *tint)
{
    g_return_if_fail (GRL_IS_IMAGE (self));
    g_return_if_fail (tint != NULL);

    ImageColorTint (&self->handle, GRL_TO_RAYLIB_COLOR (tint));
}

/**
 * grl_image_color_invert:
 * @self: A #GrlImage.
 *
 * Inverts the image colors.
 */
void
grl_image_color_invert (GrlImage *self)
{
    g_return_if_fail (GRL_IS_IMAGE (self));

    ImageColorInvert (&self->handle);
}

/**
 * grl_image_color_grayscale:
 * @self: A #GrlImage.
 *
 * Converts the image to grayscale.
 */
void
grl_image_color_grayscale (GrlImage *self)
{
    g_return_if_fail (GRL_IS_IMAGE (self));

    ImageColorGrayscale (&self->handle);
}

/**
 * grl_image_color_contrast:
 * @self: A #GrlImage.
 * @contrast: Contrast adjustment (-100 to 100).
 *
 * Adjusts image contrast.
 */
void
grl_image_color_contrast (GrlImage *self,
                          gfloat    contrast)
{
    g_return_if_fail (GRL_IS_IMAGE (self));

    ImageColorContrast (&self->handle, contrast);
}

/**
 * grl_image_color_brightness:
 * @self: A #GrlImage.
 * @brightness: Brightness adjustment (-255 to 255).
 *
 * Adjusts image brightness.
 */
void
grl_image_color_brightness (GrlImage *self,
                            gint      brightness)
{
    g_return_if_fail (GRL_IS_IMAGE (self));

    ImageColorBrightness (&self->handle, brightness);
}

/**
 * grl_image_color_replace:
 * @self: A #GrlImage.
 * @color: Color to find.
 * @replace: Replacement color.
 *
 * Replaces a color with another.
 */
void
grl_image_color_replace (GrlImage       *self,
                         const GrlColor *color,
                         const GrlColor *replace)
{
    g_return_if_fail (GRL_IS_IMAGE (self));
    g_return_if_fail (color != NULL);
    g_return_if_fail (replace != NULL);

    ImageColorReplace (&self->handle,
                       GRL_TO_RAYLIB_COLOR (color),
                       GRL_TO_RAYLIB_COLOR (replace));
}

/*
 * Public API - Mipmaps and format
 */

/**
 * grl_image_gen_mipmaps:
 * @self: A #GrlImage.
 *
 * Generates mipmap levels for the image.
 */
void
grl_image_gen_mipmaps (GrlImage *self)
{
    g_return_if_fail (GRL_IS_IMAGE (self));

    ImageMipmaps (&self->handle);
}

/**
 * grl_image_set_format:
 * @self: A #GrlImage.
 * @format: New pixel format.
 *
 * Converts the image to a new pixel format.
 */
void
grl_image_set_format (GrlImage      *self,
                      GrlPixelFormat format)
{
    g_return_if_fail (GRL_IS_IMAGE (self));

    ImageFormat (&self->handle, (int)format);
}

/**
 * grl_image_alpha_premultiply:
 * @self: A #GrlImage.
 *
 * Premultiplies alpha into color channels.
 */
void
grl_image_alpha_premultiply (GrlImage *self)
{
    g_return_if_fail (GRL_IS_IMAGE (self));

    ImageAlphaPremultiply (&self->handle);
}

/**
 * grl_image_alpha_crop:
 * @self: A #GrlImage.
 * @threshold: Alpha threshold.
 *
 * Crops the image based on alpha channel.
 */
void
grl_image_alpha_crop (GrlImage *self,
                      gfloat    threshold)
{
    g_return_if_fail (GRL_IS_IMAGE (self));

    ImageAlphaCrop (&self->handle, threshold);
}

/**
 * grl_image_alpha_clear:
 * @self: A #GrlImage.
 * @color: Color to make transparent.
 * @threshold: Similarity threshold.
 *
 * Clears alpha for pixels matching the color.
 */
void
grl_image_alpha_clear (GrlImage       *self,
                       const GrlColor *color,
                       gfloat          threshold)
{
    g_return_if_fail (GRL_IS_IMAGE (self));
    g_return_if_fail (color != NULL);

    ImageAlphaClear (&self->handle, GRL_TO_RAYLIB_COLOR (color), threshold);
}

/**
 * grl_image_alpha_mask:
 * @self: A #GrlImage.
 * @alpha_mask: Image to use as alpha mask.
 *
 * Applies an alpha mask to the image.
 */
void
grl_image_alpha_mask (GrlImage *self,
                      GrlImage *alpha_mask)
{
    g_return_if_fail (GRL_IS_IMAGE (self));
    g_return_if_fail (GRL_IS_IMAGE (alpha_mask));

    ImageAlphaMask (&self->handle, alpha_mask->handle);
}

/*
 * Public API - Drawing on image
 */

/**
 * grl_image_clear_background:
 * @self: A #GrlImage.
 * @color: Clear color.
 *
 * Clears the image with the specified color.
 */
void
grl_image_clear_background (GrlImage       *self,
                            const GrlColor *color)
{
    g_return_if_fail (GRL_IS_IMAGE (self));
    g_return_if_fail (color != NULL);

    ImageClearBackground (&self->handle, GRL_TO_RAYLIB_COLOR (color));
}

/**
 * grl_image_draw_pixel:
 * @self: A #GrlImage.
 * @x: X position.
 * @y: Y position.
 * @color: Pixel color.
 *
 * Draws a pixel on the image.
 */
void
grl_image_draw_pixel (GrlImage       *self,
                      gint            x,
                      gint            y,
                      const GrlColor *color)
{
    g_return_if_fail (GRL_IS_IMAGE (self));
    g_return_if_fail (color != NULL);

    ImageDrawPixel (&self->handle, x, y, GRL_TO_RAYLIB_COLOR (color));
}

/**
 * grl_image_draw_line:
 * @self: A #GrlImage.
 * @start_x: Start X position.
 * @start_y: Start Y position.
 * @end_x: End X position.
 * @end_y: End Y position.
 * @color: Line color.
 *
 * Draws a line on the image.
 */
void
grl_image_draw_line (GrlImage       *self,
                     gint            start_x,
                     gint            start_y,
                     gint            end_x,
                     gint            end_y,
                     const GrlColor *color)
{
    g_return_if_fail (GRL_IS_IMAGE (self));
    g_return_if_fail (color != NULL);

    ImageDrawLine (&self->handle, start_x, start_y, end_x, end_y,
                   GRL_TO_RAYLIB_COLOR (color));
}

/**
 * grl_image_draw_circle:
 * @self: A #GrlImage.
 * @center_x: Center X position.
 * @center_y: Center Y position.
 * @radius: Circle radius.
 * @color: Circle color.
 *
 * Draws a filled circle on the image.
 */
void
grl_image_draw_circle (GrlImage       *self,
                       gint            center_x,
                       gint            center_y,
                       gint            radius,
                       const GrlColor *color)
{
    g_return_if_fail (GRL_IS_IMAGE (self));
    g_return_if_fail (color != NULL);

    ImageDrawCircle (&self->handle, center_x, center_y, radius,
                     GRL_TO_RAYLIB_COLOR (color));
}

/**
 * grl_image_draw_rectangle:
 * @self: A #GrlImage.
 * @rect: Rectangle to draw.
 * @color: Rectangle color.
 *
 * Draws a filled rectangle on the image.
 */
void
grl_image_draw_rectangle (GrlImage           *self,
                          const GrlRectangle *rect,
                          const GrlColor     *color)
{
    g_return_if_fail (GRL_IS_IMAGE (self));
    g_return_if_fail (rect != NULL);
    g_return_if_fail (color != NULL);

    ImageDrawRectangleRec (&self->handle,
                           GRL_TO_RAYLIB_RECTANGLE (rect),
                           GRL_TO_RAYLIB_COLOR (color));
}

/**
 * grl_image_draw_rectangle_lines:
 * @self: A #GrlImage.
 * @rect: Rectangle to draw.
 * @thickness: Line thickness.
 * @color: Line color.
 *
 * Draws a rectangle outline on the image.
 */
void
grl_image_draw_rectangle_lines (GrlImage           *self,
                                const GrlRectangle *rect,
                                gint                thickness,
                                const GrlColor     *color)
{
    g_return_if_fail (GRL_IS_IMAGE (self));
    g_return_if_fail (rect != NULL);
    g_return_if_fail (color != NULL);

    ImageDrawRectangleLines (&self->handle,
                             GRL_TO_RAYLIB_RECTANGLE (rect),
                             thickness,
                             GRL_TO_RAYLIB_COLOR (color));
}

/**
 * grl_image_draw_image:
 * @self: A #GrlImage.
 * @src: Source image.
 * @src_rect: Source region (or NULL for full image).
 * @dst_rect: Destination region.
 * @tint: Tint color.
 *
 * Draws another image onto this image.
 */
void
grl_image_draw_image (GrlImage           *self,
                      GrlImage           *src,
                      const GrlRectangle *src_rect,
                      const GrlRectangle *dst_rect,
                      const GrlColor     *tint)
{
    Rectangle src_rec, dst_rec;

    g_return_if_fail (GRL_IS_IMAGE (self));
    g_return_if_fail (GRL_IS_IMAGE (src));
    g_return_if_fail (dst_rect != NULL);
    g_return_if_fail (tint != NULL);

    if (src_rect != NULL)
    {
        src_rec = GRL_TO_RAYLIB_RECTANGLE (src_rect);
    }
    else
    {
        src_rec = (Rectangle){
            0, 0,
            (float)src->handle.width,
            (float)src->handle.height
        };
    }

    dst_rec = GRL_TO_RAYLIB_RECTANGLE (dst_rect);

    ImageDraw (&self->handle, src->handle, src_rec, dst_rec,
               GRL_TO_RAYLIB_COLOR (tint));
}

/**
 * grl_image_draw_text:
 * @self: A #GrlImage.
 * @text: Text to draw.
 * @x: X position.
 * @y: Y position.
 * @font_size: Font size.
 * @color: Text color.
 *
 * Draws text on the image using the default font.
 */
void
grl_image_draw_text (GrlImage       *self,
                     const gchar    *text,
                     gint            x,
                     gint            y,
                     gint            font_size,
                     const GrlColor *color)
{
    g_return_if_fail (GRL_IS_IMAGE (self));
    g_return_if_fail (text != NULL);
    g_return_if_fail (color != NULL);

    ImageDrawText (&self->handle, text, x, y, font_size,
                   GRL_TO_RAYLIB_COLOR (color));
}

/*
 * Public API - Pixel access
 */

/**
 * grl_image_get_pixel:
 * @self: A #GrlImage.
 * @x: X position.
 * @y: Y position.
 *
 * Gets the color of a pixel.
 *
 * Returns: (transfer full): A new #GrlColor with the pixel color.
 */
GrlColor *
grl_image_get_pixel (GrlImage *self,
                     gint      x,
                     gint      y)
{
    Color color;

    g_return_val_if_fail (GRL_IS_IMAGE (self), NULL);

    color = GetImageColor (self->handle, x, y);
    return grl_color_new (color.r, color.g, color.b, color.a);
}

/*
 * Internal API
 */

/**
 * grl_image_get_handle:
 * @self: A #GrlImage.
 *
 * Gets the internal raylib Image handle.
 * For internal use only.
 *
 * Returns: Pointer to the internal Image structure.
 */
gpointer
grl_image_get_handle (GrlImage *self)
{
    g_return_val_if_fail (GRL_IS_IMAGE (self), NULL);

    return &self->handle;
}
