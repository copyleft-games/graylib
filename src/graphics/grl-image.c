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
#include "grl-png.h"
#include "grl-graphics-private.h"
#include "../resources/grl-resource-pack.h"
#include <raylib.h>
#include <rpng.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "grl-image-font-data.h"

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

    /* Drawing state (see grl_image_set_blend_mode / _set_clip_rect / _set_antialias) */
    GrlImageBlendMode blend_mode;   /* default GRL_IMAGE_BLEND_REPLACE */
    gboolean          has_clip;
    gint              clip_x;        /* integer, half-open [clip_x, clip_x + clip_w) */
    gint              clip_y;
    gint              clip_w;
    gint              clip_h;
    gboolean          antialias;     /* default FALSE */
    GrlImageColorSpace blend_space;  /* default GRL_IMAGE_COLOR_SPACE_GAMMA */
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
 * =============================================================================
 * Blend / clip / anti-alias aware software raster core
 * =============================================================================
 *
 * raylib's ImageDraw* primitives overwrite pixels (REPLACE) and honour no blend
 * mode or clip rectangle. To support GrlImageBlendMode, clipping and AA, the new
 * primitives plot through this core instead.
 *
 * Real blending requires read-modify-write of the destination. We only do that
 * for PIXELFORMAT_UNCOMPRESSED_R8G8B8A8 (inline 4-byte access, fast and lossless);
 * on any other format a non-REPLACE mode silently degrades to REPLACE so we never
 * round-trip through the lossy/quantising GetImageColor()/ImageDrawPixel() path.
 */

typedef struct
{
    Image            *img;
    GrlImageBlendMode mode;        /* effective mode (REPLACE if blending unsupported) */
    GrlImageColorSpace space;      /* effective blend colour space (GAMMA unless rgba)  */
    gboolean          rgba;        /* TRUE if R8G8B8A8 (inline fast path available)     */
    gboolean          antialias;   /* AA enabled (requires rgba)                        */
    gint              cx0, cy0;     /* clip box, half-open: [cx0, cx1) x [cy0, cy1)      */
    gint              cx1, cy1;
} GrlDrawCtx;

/*
 * sRGB <-> linear-light conversion for colour-correct compositing.
 *
 * grl_srgb_to_linear_lut decodes an 8-bit sRGB channel to a linear float in
 * [0, 1]; grl_linear_to_srgb_lut encodes a linear value (quantised to 4096
 * buckets to avoid banding) back to an 8-bit sRGB channel. Both are built once
 * via grl_image_blend_luts_init() the first time a LINEAR draw is set up.
 */
static gfloat grl_srgb_to_linear_lut[256];
static guint8 grl_linear_to_srgb_lut[4096];

static gfloat
grl_srgb_decode (gfloat c)
{
    if (c <= 0.04045f)
        return c / 12.92f;
    return powf ((c + 0.055f) / 1.055f, 2.4f);
}

static gfloat
grl_srgb_encode (gfloat c)
{
    if (c <= 0.0031308f)
        return c * 12.92f;
    return 1.055f * powf (c, 1.0f / 2.4f) - 0.055f;
}

static void
grl_image_blend_luts_init (void)
{
    static volatile gsize once = 0;

    if (g_once_init_enter (&once))
    {
        gint i;

        for (i = 0; i < 256; i++)
            grl_srgb_to_linear_lut[i] = grl_srgb_decode ((gfloat)i / 255.0f);

        for (i = 0; i < 4096; i++)
        {
            gfloat e = grl_srgb_encode ((gfloat)i / 4095.0f) * 255.0f + 0.5f;

            if (e < 0.0f) e = 0.0f;
            if (e > 255.0f) e = 255.0f;
            grl_linear_to_srgb_lut[i] = (guint8)e;
        }

        g_once_init_leave (&once, 1);
    }
}

/* Encode a linear-light channel value in [0, 1] to an 8-bit sRGB byte. */
static guint8
grl_lin_to_srgb_byte (gfloat lin)
{
    gint idx;

    if (lin <= 0.0f)
        return 0;
    if (lin >= 1.0f)
        return 255;

    idx = (gint)(lin * 4095.0f + 0.5f);
    return grl_linear_to_srgb_lut[idx];
}

/*
 * Composite one already-clipped, coverage-folded pixel in linear light.
 * @p points at the destination R8G8B8A8 pixel and @sa is the (coverage-folded)
 * source alpha. Only the true blending modes reach here; REPLACE and opaque
 * OVER are short-circuited by the fast paths in grl_image_plot(). Alpha is a
 * coverage value and is composited unchanged (it is never gamma-transformed).
 */
static void
grl_image_plot_linear (const GrlDrawCtx *ctx,
                       guint8           *p,
                       Color             c,
                       guint             sa)
{
    gfloat sr = grl_srgb_to_linear_lut[c.r];
    gfloat sg = grl_srgb_to_linear_lut[c.g];
    gfloat sb = grl_srgb_to_linear_lut[c.b];
    gfloat dr = grl_srgb_to_linear_lut[p[0]];
    gfloat dg = grl_srgb_to_linear_lut[p[1]];
    gfloat db = grl_srgb_to_linear_lut[p[2]];
    gfloat saf = (gfloat)sa / 255.0f;
    guint  da = p[3];
    gfloat daf = (gfloat)da / 255.0f;

    switch (ctx->mode)
    {
    case GRL_IMAGE_BLEND_OVER:
        {
            gfloat iaf = 1.0f - saf;
            gfloat oaf = saf + daf * iaf;

            if (oaf <= 0.0f)
            {
                p[0] = p[1] = p[2] = p[3] = 0;
            }
            else
            {
                p[0] = grl_lin_to_srgb_byte ((sr * saf + dr * daf * iaf) / oaf);
                p[1] = grl_lin_to_srgb_byte ((sg * saf + dg * daf * iaf) / oaf);
                p[2] = grl_lin_to_srgb_byte ((sb * saf + db * daf * iaf) / oaf);
                p[3] = (guint8)(oaf * 255.0f + 0.5f);
            }
        }
        break;

    case GRL_IMAGE_BLEND_ADD:
        {
            guint a = da + sa;

            p[0] = grl_lin_to_srgb_byte (dr + sr * saf);
            p[1] = grl_lin_to_srgb_byte (dg + sg * saf);
            p[2] = grl_lin_to_srgb_byte (db + sb * saf);
            p[3] = (guint8)(a > 255u ? 255u : a);
        }
        break;

    case GRL_IMAGE_BLEND_MULTIPLY:
        {
            gfloat iaf = 1.0f - saf;

            p[0] = grl_lin_to_srgb_byte (dr * sr * saf + dr * iaf);
            p[1] = grl_lin_to_srgb_byte (dg * sg * saf + dg * iaf);
            p[2] = grl_lin_to_srgb_byte (db * sb * saf + db * iaf);
        }
        break;

    case GRL_IMAGE_BLEND_SUBTRACT:
        {
            gfloat orr = dr - sr * saf;
            gfloat og  = dg - sg * saf;
            gfloat ob  = db - sb * saf;

            p[0] = grl_lin_to_srgb_byte (orr < 0.0f ? 0.0f : orr);
            p[1] = grl_lin_to_srgb_byte (og < 0.0f ? 0.0f : og);
            p[2] = grl_lin_to_srgb_byte (ob < 0.0f ? 0.0f : ob);
        }
        break;

    default:
        break;
    }
}

static void
grl_image_draw_ctx_init (GrlImage   *self,
                         GrlDrawCtx *ctx)
{
    ctx->img = &self->handle;
    ctx->rgba = (self->handle.format == PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
    ctx->mode = self->blend_mode;

    /* Blending and AA need a real RGBA buffer to read back from. */
    if (!ctx->rgba && ctx->mode != GRL_IMAGE_BLEND_REPLACE)
        ctx->mode = GRL_IMAGE_BLEND_REPLACE;
    ctx->antialias = (self->antialias && ctx->rgba);

    /* Linear-light blending also needs an RGBA buffer; it never affects
     * REPLACE (the fast path overwrites without any colour math). */
    ctx->space = ctx->rgba ? self->blend_space : GRL_IMAGE_COLOR_SPACE_GAMMA;
    if (ctx->space == GRL_IMAGE_COLOR_SPACE_LINEAR)
        grl_image_blend_luts_init ();

    ctx->cx0 = 0;
    ctx->cy0 = 0;
    ctx->cx1 = self->handle.width;
    ctx->cy1 = self->handle.height;

    if (self->has_clip)
    {
        gint x1 = self->clip_x + self->clip_w;
        gint y1 = self->clip_y + self->clip_h;

        if (self->clip_x > ctx->cx0) ctx->cx0 = self->clip_x;
        if (self->clip_y > ctx->cy0) ctx->cy0 = self->clip_y;
        if (x1 < ctx->cx1) ctx->cx1 = x1;
        if (y1 < ctx->cy1) ctx->cy1 = y1;
    }
}

/* Plot a single pixel with the given coverage (0-255) folded into source alpha. */
static void
grl_image_plot (const GrlDrawCtx *ctx,
                gint              x,
                gint              y,
                Color             c,
                guint             coverage)
{
    guint8 *p;
    guint   sa, da, ia;

    if (x < ctx->cx0 || x >= ctx->cx1 || y < ctx->cy0 || y >= ctx->cy1)
        return;

    if (coverage < 255)
        c.a = (guint8)(((guint)c.a * coverage) / 255u);

    /* Fast paths handle every pixel format via raylib's converter. */
    if (ctx->mode == GRL_IMAGE_BLEND_REPLACE ||
        (ctx->mode == GRL_IMAGE_BLEND_OVER && c.a == 255))
    {
        ImageDrawPixel (ctx->img, x, y, c);
        return;
    }

    /* Transparent source is a no-op for additive/over/subtract. */
    if (c.a == 0 && ctx->mode != GRL_IMAGE_BLEND_MULTIPLY)
        return;

    /* Blended path: R8G8B8A8 guaranteed here (mode forced to REPLACE otherwise). */
    p = (guint8 *)ctx->img->data + ((gsize)y * ctx->img->width + x) * 4;
    sa = c.a;

    /* Linear-light compositing: decode to linear, blend, re-encode to sRGB.
     * Fixes AA-edge darkening and hue shift at partial coverage. */
    if (ctx->space == GRL_IMAGE_COLOR_SPACE_LINEAR)
    {
        grl_image_plot_linear (ctx, p, c, sa);
        return;
    }

    switch (ctx->mode)
    {
    case GRL_IMAGE_BLEND_OVER:
        {
            guint oa;

            da = p[3];
            ia = (255u - sa);
            oa = sa + da * ia / 255u;

            if (oa == 0)
            {
                p[0] = p[1] = p[2] = p[3] = 0;
            }
            else
            {
                p[0] = (guint8)(((guint)c.r * sa + (guint)p[0] * da * ia / 255u) / oa);
                p[1] = (guint8)(((guint)c.g * sa + (guint)p[1] * da * ia / 255u) / oa);
                p[2] = (guint8)(((guint)c.b * sa + (guint)p[2] * da * ia / 255u) / oa);
                p[3] = (guint8)oa;
            }
        }
        break;

    case GRL_IMAGE_BLEND_ADD:
        {
            guint r = (guint)p[0] + (guint)c.r * sa / 255u;
            guint g = (guint)p[1] + (guint)c.g * sa / 255u;
            guint b = (guint)p[2] + (guint)c.b * sa / 255u;
            guint a = (guint)p[3] + sa;

            p[0] = (guint8)(r > 255u ? 255u : r);
            p[1] = (guint8)(g > 255u ? 255u : g);
            p[2] = (guint8)(b > 255u ? 255u : b);
            p[3] = (guint8)(a > 255u ? 255u : a);
        }
        break;

    case GRL_IMAGE_BLEND_MULTIPLY:
        {
            /* Multiply, weighted by source alpha so a==0 leaves the pixel unchanged. */
            guint mr = (guint)p[0] * c.r / 255u;
            guint mg = (guint)p[1] * c.g / 255u;
            guint mb = (guint)p[2] * c.b / 255u;

            ia = (255u - sa);
            p[0] = (guint8)((mr * sa + (guint)p[0] * ia) / 255u);
            p[1] = (guint8)((mg * sa + (guint)p[1] * ia) / 255u);
            p[2] = (guint8)((mb * sa + (guint)p[2] * ia) / 255u);
        }
        break;

    case GRL_IMAGE_BLEND_SUBTRACT:
        {
            gint r = (gint)p[0] - (gint)((guint)c.r * sa / 255u);
            gint g = (gint)p[1] - (gint)((guint)c.g * sa / 255u);
            gint b = (gint)p[2] - (gint)((guint)c.b * sa / 255u);

            p[0] = (guint8)(r < 0 ? 0 : r);
            p[1] = (guint8)(g < 0 ? 0 : g);
            p[2] = (guint8)(b < 0 ? 0 : b);
        }
        break;

    case GRL_IMAGE_BLEND_REPLACE:
    default:
        ImageDrawPixel (ctx->img, x, y, c);
        break;
    }
}

/* Fill a horizontal run [x0, x1) on row y at full coverage. */
static void
grl_image_span (const GrlDrawCtx *ctx,
                gint              y,
                gint              x0,
                gint              x1,
                Color             c)
{
    gint x;

    if (y < ctx->cy0 || y >= ctx->cy1)
        return;
    if (x0 < ctx->cx0) x0 = ctx->cx0;
    if (x1 > ctx->cx1) x1 = ctx->cx1;

    for (x = x0; x < x1; x++)
        grl_image_plot (ctx, x, y, c, 255);
}

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

    self->blend_mode = GRL_IMAGE_BLEND_REPLACE;
    self->has_clip = FALSE;
    self->clip_x = 0;
    self->clip_y = 0;
    self->clip_w = 0;
    self->clip_h = 0;
    self->antialias = FALSE;
    self->blend_space = GRL_IMAGE_COLOR_SPACE_GAMMA;
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
GrlImage *
grl_image_new_from_resource (GrlResourcePack *pack,
                             guint32          resource_id,
                             const gchar     *file_type,
                             GError         **error)
{
    GrlImage *image;
    guint8 *data;
    gsize size;
    const gchar *type_hint;

    g_return_val_if_fail (GRL_IS_RESOURCE_PACK (pack), NULL);
    g_return_val_if_fail (error == NULL || *error == NULL, NULL);

    /* Load raw data from resource pack */
    data = grl_resource_pack_load_raw (pack, resource_id, &size, error);
    if (data == NULL)
        return NULL;

    /* Use provided file type or try to detect from magic bytes */
    if (file_type != NULL)
    {
        type_hint = file_type;
    }
    else
    {
        /* Try to detect format from magic bytes */
        if (size >= 8 && data[0] == 0x89 && data[1] == 'P' && data[2] == 'N' && data[3] == 'G')
            type_hint = ".png";
        else if (size >= 3 && data[0] == 0xFF && data[1] == 0xD8 && data[2] == 0xFF)
            type_hint = ".jpg";
        else if (size >= 4 && data[0] == 'B' && data[1] == 'M')
            type_hint = ".bmp";
        else if (size >= 6 && (memcmp (data, "GIF87a", 6) == 0 || memcmp (data, "GIF89a", 6) == 0))
            type_hint = ".gif";
        else
            type_hint = ".png"; /* Default fallback */
    }

    /* Load image from memory */
    image = grl_image_new_from_memory (type_hint, data, size);
    g_free (data);

    if (image == NULL || !grl_image_is_valid (image))
    {
        g_clear_object (&image);
        g_set_error (error,
                     GRL_RESOURCE_PACK_ERROR,
                     GRL_RESOURCE_PACK_ERROR_CORRUPTED_DATA,
                     "Failed to load image from resource %u",
                     resource_id);
        return NULL;
    }

    return image;
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

/**
 * grl_image_resized:
 * @self: A #GrlImage.
 * @new_width: New width.
 * @new_height: New height.
 *
 * Returns a new image that is a bicubically scaled copy of this image. Unlike
 * grl_image_resize(), the original image is left unchanged.
 *
 * Returns: (transfer full): A new, resized #GrlImage.
 */
GrlImage *
grl_image_resized (GrlImage *self,
                   gint      new_width,
                   gint      new_height)
{
    Image handle;

    g_return_val_if_fail (GRL_IS_IMAGE (self), NULL);
    g_return_val_if_fail (new_width > 0, NULL);
    g_return_val_if_fail (new_height > 0, NULL);

    handle = ImageCopy (self->handle);
    ImageResize (&handle, new_width, new_height);
    return grl_image_new_from_handle (handle);
}

/**
 * grl_image_scaled_nearest:
 * @self: A #GrlImage.
 * @new_width: New width.
 * @new_height: New height.
 *
 * Returns a new image that is a nearest-neighbor scaled copy of this image
 * (pixel-perfect, ideal for pixel art). Unlike grl_image_resize_nearest(), the
 * original image is left unchanged.
 *
 * Returns: (transfer full): A new, resized #GrlImage.
 */
GrlImage *
grl_image_scaled_nearest (GrlImage *self,
                          gint      new_width,
                          gint      new_height)
{
    Image handle;

    g_return_val_if_fail (GRL_IS_IMAGE (self), NULL);
    g_return_val_if_fail (new_width > 0, NULL);
    g_return_val_if_fail (new_height > 0, NULL);

    handle = ImageCopy (self->handle);
    ImageResizeNN (&handle, new_width, new_height);
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
 * @degrees: Rotation angle in degrees, clockwise (-359 to 359).
 *
 * Rotates the image by an arbitrary angle. The 90, 180 and 270 degree cases use
 * exact lossless rotations; other angles use raylib's general rotation (which
 * keeps the image dimensions and may leave transparent/black corners).
 */
void
grl_image_rotate (GrlImage *self,
                  gint      degrees)
{
    g_return_if_fail (GRL_IS_IMAGE (self));

    switch (degrees)
    {
    case 0:
    case 360:
    case -360:
        break;
    case 90:
    case -270:
        ImageRotateCW (&self->handle);
        break;
    case 180:
    case -180:
        ImageRotateCW (&self->handle);
        ImageRotateCW (&self->handle);
        break;
    case 270:
    case -90:
        ImageRotateCCW (&self->handle);
        break;
    default:
        ImageRotate (&self->handle, degrees);
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
 * @src_rect: (nullable): Source region, or %NULL for the full source image.
 * @dst_rect: Destination region.
 * @tint: (nullable): Tint color multiplied into the source, or %NULL for white
 *   (no tint).
 *
 * Draws another image onto this image.
 *
 * Passing %NULL for @tint is equivalent to passing opaque white: the source is
 * drawn unmodified.
 */
void
grl_image_draw_image (GrlImage           *self,
                      GrlImage           *src,
                      const GrlRectangle *src_rect,
                      const GrlRectangle *dst_rect,
                      const GrlColor     *tint)
{
    Rectangle src_rec, dst_rec;
    Color     raylib_tint;

    g_return_if_fail (GRL_IS_IMAGE (self));
    g_return_if_fail (GRL_IS_IMAGE (src));
    g_return_if_fail (dst_rect != NULL);

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

    if (tint != NULL)
        raylib_tint = GRL_TO_RAYLIB_COLOR (tint);
    else
        raylib_tint = (Color){ 255, 255, 255, 255 };

    ImageDraw (&self->handle, src->handle, src_rec, dst_rec, raylib_tint);
}

/*
 * Embedded bitmap font (headless-safe text)
 *
 * grl_image_draw_text() defers to raylib's default font when a GL context is
 * available, but raylib's LoadFontDefault() crashes headless (it lays out the
 * glyph atlas against a zero texture width). We reconstruct the same font on the
 * CPU from grl-image-font-data.h and rasterise glyphs ourselves, so text drawing
 * never needs a window. The atlas is built once, lazily, and lives for the
 * lifetime of the process (intentional one-time static allocation).
 */

typedef struct
{
    gint x;
    gint y;
    gint w;
    gint h;
} GrlGlyphRec;

static guint8      grl_font_atlas[GRL_DEFAULT_FONT_ATLAS_SIZE *
                                  GRL_DEFAULT_FONT_ATLAS_SIZE];
static GrlGlyphRec grl_font_recs[GRL_DEFAULT_FONT_GLYPH_COUNT];

static void
grl_image_font_build (void)
{
    const gint size = GRL_DEFAULT_FONT_ATLAS_SIZE;
    const gint divisor = GRL_DEFAULT_FONT_CHAR_DIVISOR;
    const gint char_h = GRL_DEFAULT_FONT_CHAR_HEIGHT;
    gint i, j, counter;
    gint current_line, current_pos_x, test_pos_x;

    /* Unpack the 1-bit glyph atlas to per-pixel coverage (0 or 255). */
    counter = 0;
    for (i = 0; i < size * size; i += 32)
    {
        for (j = 31; j >= 0; j--)
        {
            if ((grl_default_font_data[counter] & (1u << j)) != 0)
                grl_font_atlas[i + j] = 255;
            else
                grl_font_atlas[i + j] = 0;
        }
        counter++;
    }

    /* Lay out glyph rectangles against the known atlas width (128). raylib wraps
     * on defaultFont.texture.width which is 0 headless -- the bug we avoid. */
    current_line = 0;
    current_pos_x = divisor;
    test_pos_x = divisor;
    for (i = 0; i < GRL_DEFAULT_FONT_GLYPH_COUNT; i++)
    {
        gint w = grl_default_font_widths[i];

        grl_font_recs[i].x = current_pos_x;
        grl_font_recs[i].y = divisor + current_line * (char_h + divisor);
        grl_font_recs[i].w = w;
        grl_font_recs[i].h = char_h;

        test_pos_x += w + divisor;
        if (test_pos_x >= size)
        {
            current_line++;
            current_pos_x = 2 * divisor + w;
            test_pos_x = current_pos_x;
            grl_font_recs[i].x = divisor;
            grl_font_recs[i].y = divisor + current_line * (char_h + divisor);
        }
        else
        {
            current_pos_x = test_pos_x;
        }
    }
}

static void
grl_image_font_ensure (void)
{
    static gsize once = 0;

    if (g_once_init_enter (&once))
    {
        grl_image_font_build ();
        g_once_init_leave (&once, 1);
    }
}

/* Map a byte to a glyph index, or -1 for "draw nothing" (control char). */
static gint
grl_image_font_index (guchar ch)
{
    if (ch < GRL_DEFAULT_FONT_FIRST_CHAR)
        return -1;
    return (gint)ch - GRL_DEFAULT_FONT_FIRST_CHAR;
}

/* Rasterise @text through the supplied draw context (CPU bitmap font). */
static void
grl_image_blit_text (const GrlDrawCtx *ctx,
                     const gchar      *text,
                     gint              x,
                     gint              y,
                     gfloat            scale,
                     Color             color)
{
    const gint size = GRL_DEFAULT_FONT_ATLAS_SIZE;
    const gint char_h = GRL_DEFAULT_FONT_CHAR_HEIGHT;
    const guchar *p = (const guchar *)text;
    gint pen_x = x;
    gint pen_y = y;
    gint line_advance = (gint)(((gfloat)(char_h + GRL_DEFAULT_FONT_CHAR_DIVISOR)) * scale + 0.5f);

    if (line_advance < 1)
        line_advance = 1;

    for (; *p != '\0'; p++)
    {
        gint index;
        const GrlGlyphRec *rec;
        gint dst_w, dst_h, dx, dy;

        if (*p == '\n')
        {
            pen_x = x;
            pen_y += line_advance;
            continue;
        }

        index = grl_image_font_index (*p);
        if (index < 0 || index >= GRL_DEFAULT_FONT_GLYPH_COUNT)
            continue;

        rec = &grl_font_recs[index];
        dst_w = (gint)((gfloat)rec->w * scale + 0.5f);
        dst_h = (gint)((gfloat)char_h * scale + 0.5f);

        for (dy = 0; dy < dst_h; dy++)
        {
            gint src_y = rec->y + (gint)((gfloat)dy / scale);

            if (src_y < 0 || src_y >= size)
                continue;

            for (dx = 0; dx < dst_w; dx++)
            {
                gint src_x = rec->x + (gint)((gfloat)dx / scale);
                guint cov;

                if (src_x < 0 || src_x >= size)
                    continue;

                cov = grl_font_atlas[src_y * size + src_x];
                if (cov != 0)
                    grl_image_plot (ctx, pen_x + dx, pen_y + dy, color, cov);
            }
        }

        pen_x += (gint)(((gfloat)(rec->w + GRL_DEFAULT_FONT_CHAR_DIVISOR)) * scale + 0.5f);
    }
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
 *
 * When a window/GL context is active this uses raylib's default font. When no
 * window has been created (a headless tool, an asset baker, a unit test) it
 * automatically falls back to graylib's embedded CPU bitmap font, which is what
 * grl_image_draw_text_bitmap() uses directly. This avoids the headless crash in
 * raylib's lazy default-font loader.
 *
 * The headless path honours the image's blend mode, clip rectangle and
 * anti-alias state; the windowed raylib path does not.
 */
void
grl_image_draw_text (GrlImage       *self,
                     const gchar    *text,
                     gint            x,
                     gint            y,
                     gint            font_size,
                     const GrlColor *color)
{
    unsigned char window_ready;

    g_return_if_fail (GRL_IS_IMAGE (self));
    g_return_if_fail (text != NULL);
    g_return_if_fail (color != NULL);

    /* bool -> gboolean ABI guard (see graylib CLAUDE.md). */
    window_ready = IsWindowReady ();
    if (window_ready == 0)
    {
        grl_image_draw_text_bitmap (self, text, x, y, font_size, color);
        return;
    }

    ImageDrawText (&self->handle, text, x, y, font_size,
                   GRL_TO_RAYLIB_COLOR (color));
}

/**
 * grl_image_draw_text_bitmap:
 * @self: A #GrlImage.
 * @text: Text to draw.
 * @x: X position of the top-left of the text.
 * @y: Y position of the top-left of the text.
 * @font_size: Target glyph height in pixels (the base font is 10px tall).
 * @color: Text color.
 *
 * Draws text using graylib's embedded CPU bitmap font.
 *
 * Unlike grl_image_draw_text(), this never touches raylib's font system or a GL
 * context, so it is always safe in headless environments and produces identical
 * output whether or not a window exists. It honours the image's blend mode, clip
 * rectangle and anti-alias state. Newlines start a new line.
 */
void
grl_image_draw_text_bitmap (GrlImage       *self,
                            const gchar    *text,
                            gint            x,
                            gint            y,
                            gint            font_size,
                            const GrlColor *color)
{
    GrlDrawCtx ctx;
    gfloat     scale;

    g_return_if_fail (GRL_IS_IMAGE (self));
    g_return_if_fail (text != NULL);
    g_return_if_fail (color != NULL);

    grl_image_font_ensure ();

    scale = (gfloat)font_size / (gfloat)GRL_DEFAULT_FONT_CHAR_HEIGHT;
    if (scale <= 0.0f)
        scale = 1.0f;

    grl_image_draw_ctx_init (self, &ctx);
    grl_image_blit_text (&ctx, text, x, y, scale, GRL_TO_RAYLIB_COLOR (color));
}

/**
 * grl_image_measure_text_bitmap:
 * @text: Text to measure.
 * @font_size: Target glyph height in pixels.
 *
 * Measures the pixel size of @text as it would be rendered by
 * grl_image_draw_text_bitmap(). The returned vector holds the width in @x and
 * the height in @y. Multi-line text (containing newlines) is measured across all
 * lines.
 *
 * Returns: (transfer full): A new #GrlVector2 with the text size in pixels.
 */
GrlVector2 *
grl_image_measure_text_bitmap (const gchar *text,
                               gint         font_size)
{
    const guchar *p;
    gfloat        scale;
    gfloat        line_w = 0.0f;
    gfloat        max_w = 0.0f;
    gint          lines = 1;

    g_return_val_if_fail (text != NULL, grl_vector2_new (0.0f, 0.0f));

    grl_image_font_ensure ();

    scale = (gfloat)font_size / (gfloat)GRL_DEFAULT_FONT_CHAR_HEIGHT;
    if (scale <= 0.0f)
        scale = 1.0f;

    for (p = (const guchar *)text; *p != '\0'; p++)
    {
        gint index;

        if (*p == '\n')
        {
            if (line_w > max_w)
                max_w = line_w;
            line_w = 0.0f;
            lines++;
            continue;
        }

        index = grl_image_font_index (*p);
        if (index < 0 || index >= GRL_DEFAULT_FONT_GLYPH_COUNT)
            continue;

        line_w += (gfloat)(grl_font_recs[index].w + GRL_DEFAULT_FONT_CHAR_DIVISOR) * scale;
    }

    if (line_w > max_w)
        max_w = line_w;

    return grl_vector2_new (max_w, (gfloat)lines * (gfloat)font_size);
}

/*
 * =============================================================================
 * Additional drawing primitives (blend / clip / AA aware)
 * =============================================================================
 */

/* Linear interpolation between two colors (straight, per-channel). */
static Color
grl_image_color_lerp (Color a,
                      Color b,
                      gfloat t)
{
    Color r;

    if (t < 0.0f) t = 0.0f;
    if (t > 1.0f) t = 1.0f;

    r.r = (guint8)((gfloat)a.r + ((gfloat)b.r - (gfloat)a.r) * t + 0.5f);
    r.g = (guint8)((gfloat)a.g + ((gfloat)b.g - (gfloat)a.g) * t + 0.5f);
    r.b = (guint8)((gfloat)a.b + ((gfloat)b.b - (gfloat)a.b) * t + 0.5f);
    r.a = (guint8)((gfloat)a.a + ((gfloat)b.a - (gfloat)a.a) * t + 0.5f);

    return r;
}

/* Round-capped thick segment via distance field; AA when enabled. */
static void
grl_image_thick_segment (const GrlDrawCtx *ctx,
                         gfloat            x0,
                         gfloat            y0,
                         gfloat            x1,
                         gfloat            y1,
                         gfloat            half,
                         Color             c)
{
    gfloat dx = x1 - x0;
    gfloat dy = y1 - y0;
    gfloat len2 = dx * dx + dy * dy;
    gint   ix0 = (gint)floorf (MIN (x0, x1) - half - 1.0f);
    gint   ix1 = (gint)ceilf (MAX (x0, x1) + half + 1.0f);
    gint   iy0 = (gint)floorf (MIN (y0, y1) - half - 1.0f);
    gint   iy1 = (gint)ceilf (MAX (y0, y1) + half + 1.0f);
    gint   px, py;

    if (half < 0.5f)
        half = 0.5f;

    for (py = iy0; py <= iy1; py++)
    {
        for (px = ix0; px <= ix1; px++)
        {
            gfloat fx = (gfloat)px + 0.5f;
            gfloat fy = (gfloat)py + 0.5f;
            gfloat cx, cy, dist;

            if (len2 <= 0.0001f)
            {
                cx = x0;
                cy = y0;
            }
            else
            {
                gfloat t = ((fx - x0) * dx + (fy - y0) * dy) / len2;

                if (t < 0.0f) t = 0.0f;
                if (t > 1.0f) t = 1.0f;
                cx = x0 + t * dx;
                cy = y0 + t * dy;
            }

            dist = sqrtf ((fx - cx) * (fx - cx) + (fy - cy) * (fy - cy));

            if (ctx->antialias)
            {
                gfloat cov = half + 0.5f - dist;

                if (cov <= 0.0f)
                    continue;
                if (cov > 1.0f)
                    cov = 1.0f;
                grl_image_plot (ctx, px, py, c, (guint)(cov * 255.0f + 0.5f));
            }
            else if (dist <= half)
            {
                grl_image_plot (ctx, px, py, c, 255);
            }
        }
    }
}

/* Annulus (ring) of the given radius and half-thickness; AA when enabled. */
static void
grl_image_ring (const GrlDrawCtx *ctx,
                gfloat            cx,
                gfloat            cy,
                gfloat            radius,
                gfloat            half,
                Color             c)
{
    gint ix0 = (gint)floorf (cx - radius - half - 1.0f);
    gint ix1 = (gint)ceilf (cx + radius + half + 1.0f);
    gint iy0 = (gint)floorf (cy - radius - half - 1.0f);
    gint iy1 = (gint)ceilf (cy + radius + half + 1.0f);
    gint px, py;

    if (half < 0.5f)
        half = 0.5f;

    for (py = iy0; py <= iy1; py++)
    {
        for (px = ix0; px <= ix1; px++)
        {
            gfloat fx = (gfloat)px + 0.5f;
            gfloat fy = (gfloat)py + 0.5f;
            gfloat dist = sqrtf ((fx - cx) * (fx - cx) + (fy - cy) * (fy - cy));
            gfloat edge = fabsf (dist - radius);

            if (ctx->antialias)
            {
                gfloat cov = half + 0.5f - edge;

                if (cov <= 0.0f)
                    continue;
                if (cov > 1.0f)
                    cov = 1.0f;
                grl_image_plot (ctx, px, py, c, (guint)(cov * 255.0f + 0.5f));
            }
            else if (edge <= half)
            {
                grl_image_plot (ctx, px, py, c, 255);
            }
        }
    }
}

/* Even-odd point-in-polygon test. */
static gboolean
grl_image_point_in_poly (const gfloat *xs,
                         const gfloat *ys,
                         gint          n,
                         gfloat        px,
                         gfloat        py)
{
    gboolean inside = FALSE;
    gint     i, j;

    for (i = 0, j = n - 1; i < n; j = i++)
    {
        if (((ys[i] > py) != (ys[j] > py)) &&
            (px < (xs[j] - xs[i]) * (py - ys[i]) / (ys[j] - ys[i]) + xs[i]))
            inside = !inside;
    }

    return inside;
}

/* Filled convex/concave polygon by even-odd rule; 2x2 supersampled when AA. */
static void
grl_image_fill_poly (const GrlDrawCtx *ctx,
                     const gfloat     *xs,
                     const gfloat     *ys,
                     gint              n,
                     Color             c)
{
    gfloat minx = xs[0], maxx = xs[0], miny = ys[0], maxy = ys[0];
    gint   ix0, ix1, iy0, iy1, px, py, i;

    for (i = 1; i < n; i++)
    {
        minx = MIN (minx, xs[i]);
        maxx = MAX (maxx, xs[i]);
        miny = MIN (miny, ys[i]);
        maxy = MAX (maxy, ys[i]);
    }

    ix0 = (gint)floorf (minx);
    ix1 = (gint)ceilf (maxx);
    iy0 = (gint)floorf (miny);
    iy1 = (gint)ceilf (maxy);

    for (py = iy0; py <= iy1; py++)
    {
        for (px = ix0; px <= ix1; px++)
        {
            if (ctx->antialias)
            {
                gint inside = 0, sx, sy;

                for (sy = 0; sy < 2; sy++)
                {
                    for (sx = 0; sx < 2; sx++)
                    {
                        gfloat fx = (gfloat)px + 0.25f + 0.5f * (gfloat)sx;
                        gfloat fy = (gfloat)py + 0.25f + 0.5f * (gfloat)sy;

                        if (grl_image_point_in_poly (xs, ys, n, fx, fy))
                            inside++;
                    }
                }

                if (inside > 0)
                    grl_image_plot (ctx, px, py, c, (guint)(inside * 255 / 4));
            }
            else if (grl_image_point_in_poly (xs, ys, n,
                                              (gfloat)px + 0.5f, (gfloat)py + 0.5f))
            {
                grl_image_plot (ctx, px, py, c, 255);
            }
        }
    }
}

/* Connected polyline of thick segments. */
static void
grl_image_stroke_path (const GrlDrawCtx *ctx,
                       const gfloat     *xs,
                       const gfloat     *ys,
                       gint              n,
                       gboolean          closed,
                       gfloat            half,
                       Color             c)
{
    gint i;

    for (i = 0; i + 1 < n; i++)
        grl_image_thick_segment (ctx, xs[i], ys[i], xs[i + 1], ys[i + 1], half, c);

    if (closed && n > 2)
        grl_image_thick_segment (ctx, xs[n - 1], ys[n - 1], xs[0], ys[0], half, c);
}

/**
 * grl_image_draw_line_ex:
 * @self: A #GrlImage.
 * @start: Start point.
 * @end: End point.
 * @thickness: Line thickness in pixels (minimum 1).
 * @color: Line color.
 *
 * Draws a line of the given thickness with round end caps. Honours the image's
 * blend mode, clip rectangle and anti-alias state.
 */
void
grl_image_draw_line_ex (GrlImage         *self,
                        const GrlVector2 *start,
                        const GrlVector2 *end,
                        gint              thickness,
                        const GrlColor   *color)
{
    GrlDrawCtx ctx;

    g_return_if_fail (GRL_IS_IMAGE (self));
    g_return_if_fail (start != NULL);
    g_return_if_fail (end != NULL);
    g_return_if_fail (color != NULL);

    if (thickness < 1)
        thickness = 1;

    grl_image_draw_ctx_init (self, &ctx);
    grl_image_thick_segment (&ctx, start->x, start->y, end->x, end->y,
                             (gfloat)thickness * 0.5f, GRL_TO_RAYLIB_COLOR (color));
}

/**
 * grl_image_draw_circle_lines:
 * @self: A #GrlImage.
 * @center_x: Center X position.
 * @center_y: Center Y position.
 * @radius: Circle radius.
 * @thickness: Outline thickness in pixels (minimum 1).
 * @color: Outline color.
 *
 * Draws a circle outline of the given thickness. Honours the image's blend
 * mode, clip rectangle and anti-alias state.
 */
void
grl_image_draw_circle_lines (GrlImage       *self,
                             gint            center_x,
                             gint            center_y,
                             gint            radius,
                             gint            thickness,
                             const GrlColor *color)
{
    GrlDrawCtx ctx;

    g_return_if_fail (GRL_IS_IMAGE (self));
    g_return_if_fail (color != NULL);

    if (thickness < 1)
        thickness = 1;

    grl_image_draw_ctx_init (self, &ctx);
    grl_image_ring (&ctx, (gfloat)center_x + 0.5f, (gfloat)center_y + 0.5f,
                    (gfloat)radius, (gfloat)thickness * 0.5f,
                    GRL_TO_RAYLIB_COLOR (color));
}

/**
 * grl_image_draw_ellipse:
 * @self: A #GrlImage.
 * @center_x: Center X position.
 * @center_y: Center Y position.
 * @radius_x: Horizontal radius.
 * @radius_y: Vertical radius.
 * @color: Fill color.
 *
 * Draws a filled ellipse. Honours the image's blend mode, clip rectangle and
 * anti-alias state.
 */
void
grl_image_draw_ellipse (GrlImage       *self,
                        gint            center_x,
                        gint            center_y,
                        gint            radius_x,
                        gint            radius_y,
                        const GrlColor *color)
{
    GrlDrawCtx ctx;
    Color      c;
    gfloat     cx, cy, rx, ry;
    gint       ix0, ix1, iy0, iy1, px, py;

    g_return_if_fail (GRL_IS_IMAGE (self));
    g_return_if_fail (color != NULL);

    if (radius_x <= 0 || radius_y <= 0)
        return;

    grl_image_draw_ctx_init (self, &ctx);
    c = GRL_TO_RAYLIB_COLOR (color);

    cx = (gfloat)center_x + 0.5f;
    cy = (gfloat)center_y + 0.5f;
    rx = (gfloat)radius_x;
    ry = (gfloat)radius_y;

    ix0 = (gint)floorf (cx - rx);
    ix1 = (gint)ceilf (cx + rx);
    iy0 = (gint)floorf (cy - ry);
    iy1 = (gint)ceilf (cy + ry);

    for (py = iy0; py <= iy1; py++)
    {
        for (px = ix0; px <= ix1; px++)
        {
            if (ctx.antialias)
            {
                gint inside = 0, sx, sy;

                for (sy = 0; sy < 2; sy++)
                {
                    for (sx = 0; sx < 2; sx++)
                    {
                        gfloat fx = (gfloat)px + 0.25f + 0.5f * (gfloat)sx;
                        gfloat fy = (gfloat)py + 0.25f + 0.5f * (gfloat)sy;
                        gfloat nx = (fx - cx) / rx;
                        gfloat ny = (fy - cy) / ry;

                        if (nx * nx + ny * ny <= 1.0f)
                            inside++;
                    }
                }

                if (inside > 0)
                    grl_image_plot (&ctx, px, py, c, (guint)(inside * 255 / 4));
            }
            else
            {
                gfloat fx = (gfloat)px + 0.5f;
                gfloat fy = (gfloat)py + 0.5f;
                gfloat nx = (fx - cx) / rx;
                gfloat ny = (fy - cy) / ry;

                if (nx * nx + ny * ny <= 1.0f)
                    grl_image_plot (&ctx, px, py, c, 255);
            }
        }
    }
}

/**
 * grl_image_draw_ellipse_lines:
 * @self: A #GrlImage.
 * @center_x: Center X position.
 * @center_y: Center Y position.
 * @radius_x: Horizontal radius.
 * @radius_y: Vertical radius.
 * @thickness: Outline thickness in pixels (minimum 1).
 * @color: Outline color.
 *
 * Draws an ellipse outline of the given thickness. Honours the image's blend
 * mode, clip rectangle and anti-alias state.
 */
void
grl_image_draw_ellipse_lines (GrlImage       *self,
                              gint            center_x,
                              gint            center_y,
                              gint            radius_x,
                              gint            radius_y,
                              gint            thickness,
                              const GrlColor *color)
{
    GrlDrawCtx ctx;
    Color      c;
    gfloat     cx, cy, rx, ry, half;
    gint       ix0, ix1, iy0, iy1, px, py;

    g_return_if_fail (GRL_IS_IMAGE (self));
    g_return_if_fail (color != NULL);

    if (radius_x <= 0 || radius_y <= 0)
        return;
    if (thickness < 1)
        thickness = 1;

    grl_image_draw_ctx_init (self, &ctx);
    c = GRL_TO_RAYLIB_COLOR (color);
    half = (gfloat)thickness * 0.5f;

    cx = (gfloat)center_x + 0.5f;
    cy = (gfloat)center_y + 0.5f;
    rx = (gfloat)radius_x;
    ry = (gfloat)radius_y;

    ix0 = (gint)floorf (cx - rx - half - 1.0f);
    ix1 = (gint)ceilf (cx + rx + half + 1.0f);
    iy0 = (gint)floorf (cy - ry - half - 1.0f);
    iy1 = (gint)ceilf (cy + ry + half + 1.0f);

    /* Approximate signed distance to the ellipse boundary; good enough for a
     * smooth, even-thickness outline at typical radii. */
    for (py = iy0; py <= iy1; py++)
    {
        for (px = ix0; px <= ix1; px++)
        {
            gfloat fx = (gfloat)px + 0.5f;
            gfloat fy = (gfloat)py + 0.5f;
            gfloat nx = (fx - cx) / rx;
            gfloat ny = (fy - cy) / ry;
            gfloat rr = sqrtf (nx * nx + ny * ny);
            gfloat grad, edge;

            if (rr <= 0.0001f)
                continue;

            /* Distance in pixels: (||p||/scale - 1) over the local gradient. */
            grad = sqrtf ((nx / rx) * (nx / rx) + (ny / ry) * (ny / ry));
            if (grad <= 0.0001f)
                continue;
            edge = fabsf (rr - 1.0f) / grad;

            if (ctx.antialias)
            {
                gfloat cov = half + 0.5f - edge;

                if (cov <= 0.0f)
                    continue;
                if (cov > 1.0f)
                    cov = 1.0f;
                grl_image_plot (&ctx, px, py, c, (guint)(cov * 255.0f + 0.5f));
            }
            else if (edge <= half)
            {
                grl_image_plot (&ctx, px, py, c, 255);
            }
        }
    }
}

/**
 * grl_image_draw_triangle:
 * @self: A #GrlImage.
 * @v1: First vertex.
 * @v2: Second vertex.
 * @v3: Third vertex.
 * @color: Fill color.
 *
 * Draws a filled triangle. Honours the image's blend mode, clip rectangle and
 * anti-alias state.
 */
void
grl_image_draw_triangle (GrlImage         *self,
                         const GrlVector2 *v1,
                         const GrlVector2 *v2,
                         const GrlVector2 *v3,
                         const GrlColor   *color)
{
    GrlDrawCtx ctx;
    gfloat     xs[3], ys[3];

    g_return_if_fail (GRL_IS_IMAGE (self));
    g_return_if_fail (v1 != NULL && v2 != NULL && v3 != NULL);
    g_return_if_fail (color != NULL);

    xs[0] = v1->x; ys[0] = v1->y;
    xs[1] = v2->x; ys[1] = v2->y;
    xs[2] = v3->x; ys[2] = v3->y;

    grl_image_draw_ctx_init (self, &ctx);
    grl_image_fill_poly (&ctx, xs, ys, 3, GRL_TO_RAYLIB_COLOR (color));
}

/**
 * grl_image_draw_triangle_lines:
 * @self: A #GrlImage.
 * @v1: First vertex.
 * @v2: Second vertex.
 * @v3: Third vertex.
 * @thickness: Outline thickness in pixels (minimum 1).
 * @color: Outline color.
 *
 * Draws a triangle outline of the given thickness. Honours the image's blend
 * mode, clip rectangle and anti-alias state.
 */
void
grl_image_draw_triangle_lines (GrlImage         *self,
                               const GrlVector2 *v1,
                               const GrlVector2 *v2,
                               const GrlVector2 *v3,
                               gint              thickness,
                               const GrlColor   *color)
{
    GrlDrawCtx ctx;
    gfloat     xs[3], ys[3];

    g_return_if_fail (GRL_IS_IMAGE (self));
    g_return_if_fail (v1 != NULL && v2 != NULL && v3 != NULL);
    g_return_if_fail (color != NULL);

    if (thickness < 1)
        thickness = 1;

    xs[0] = v1->x; ys[0] = v1->y;
    xs[1] = v2->x; ys[1] = v2->y;
    xs[2] = v3->x; ys[2] = v3->y;

    grl_image_draw_ctx_init (self, &ctx);
    grl_image_stroke_path (&ctx, xs, ys, 3, TRUE,
                           (gfloat)thickness * 0.5f, GRL_TO_RAYLIB_COLOR (color));
}

/**
 * grl_image_draw_polygon:
 * @self: A #GrlImage.
 * @points: (array length=point_count): Polygon vertices.
 * @point_count: Number of vertices (minimum 3).
 * @color: Fill color.
 *
 * Draws a filled polygon (even-odd rule, supports concave shapes). Honours the
 * image's blend mode, clip rectangle and anti-alias state.
 */
void
grl_image_draw_polygon (GrlImage         *self,
                        const GrlVector2 *points,
                        gint              point_count,
                        const GrlColor   *color)
{
    GrlDrawCtx  ctx;
    gfloat     *xs, *ys;
    gint        i;

    g_return_if_fail (GRL_IS_IMAGE (self));
    g_return_if_fail (points != NULL);
    g_return_if_fail (point_count >= 3);
    g_return_if_fail (color != NULL);

    xs = g_new (gfloat, point_count);
    ys = g_new (gfloat, point_count);
    for (i = 0; i < point_count; i++)
    {
        xs[i] = points[i].x;
        ys[i] = points[i].y;
    }

    grl_image_draw_ctx_init (self, &ctx);
    grl_image_fill_poly (&ctx, xs, ys, point_count, GRL_TO_RAYLIB_COLOR (color));

    g_free (xs);
    g_free (ys);
}

/**
 * grl_image_draw_polyline:
 * @self: A #GrlImage.
 * @points: (array length=point_count): Vertices.
 * @point_count: Number of vertices (minimum 2).
 * @closed: %TRUE to connect the last point back to the first.
 * @thickness: Line thickness in pixels (minimum 1).
 * @color: Line color.
 *
 * Draws a connected sequence of thick line segments. Honours the image's blend
 * mode, clip rectangle and anti-alias state.
 */
void
grl_image_draw_polyline (GrlImage         *self,
                         const GrlVector2 *points,
                         gint              point_count,
                         gboolean          closed,
                         gint              thickness,
                         const GrlColor   *color)
{
    GrlDrawCtx  ctx;
    gfloat     *xs, *ys;
    gint        i;

    g_return_if_fail (GRL_IS_IMAGE (self));
    g_return_if_fail (points != NULL);
    g_return_if_fail (point_count >= 2);
    g_return_if_fail (color != NULL);

    if (thickness < 1)
        thickness = 1;

    xs = g_new (gfloat, point_count);
    ys = g_new (gfloat, point_count);
    for (i = 0; i < point_count; i++)
    {
        xs[i] = points[i].x;
        ys[i] = points[i].y;
    }

    grl_image_draw_ctx_init (self, &ctx);
    grl_image_stroke_path (&ctx, xs, ys, point_count, closed,
                           (gfloat)thickness * 0.5f, GRL_TO_RAYLIB_COLOR (color));

    g_free (xs);
    g_free (ys);
}

/**
 * grl_image_draw_bezier:
 * @self: A #GrlImage.
 * @p0: Start point.
 * @p1: First control point.
 * @p2: Second control point.
 * @p3: End point.
 * @thickness: Curve thickness in pixels (minimum 1).
 * @color: Curve color.
 *
 * Draws a cubic Bezier curve as a thick polyline. The number of segments is
 * chosen from the control-point span. Honours the image's blend mode, clip
 * rectangle and anti-alias state.
 */
void
grl_image_draw_bezier (GrlImage         *self,
                       const GrlVector2 *p0,
                       const GrlVector2 *p1,
                       const GrlVector2 *p2,
                       const GrlVector2 *p3,
                       gint              thickness,
                       const GrlColor   *color)
{
    GrlDrawCtx  ctx;
    gfloat     *xs, *ys;
    gfloat      span;
    gint        segments, i;

    g_return_if_fail (GRL_IS_IMAGE (self));
    g_return_if_fail (p0 != NULL && p1 != NULL && p2 != NULL && p3 != NULL);
    g_return_if_fail (color != NULL);

    if (thickness < 1)
        thickness = 1;

    /* Rough chord length to pick a sensible tessellation. */
    span = fabsf (p1->x - p0->x) + fabsf (p1->y - p0->y) +
           fabsf (p2->x - p1->x) + fabsf (p2->y - p1->y) +
           fabsf (p3->x - p2->x) + fabsf (p3->y - p2->y);
    segments = (gint)(span / 4.0f);
    segments = CLAMP (segments, 8, 256);

    xs = g_new (gfloat, segments + 1);
    ys = g_new (gfloat, segments + 1);
    for (i = 0; i <= segments; i++)
    {
        gfloat t = (gfloat)i / (gfloat)segments;
        gfloat u = 1.0f - t;
        gfloat w0 = u * u * u;
        gfloat w1 = 3.0f * u * u * t;
        gfloat w2 = 3.0f * u * t * t;
        gfloat w3 = t * t * t;

        xs[i] = w0 * p0->x + w1 * p1->x + w2 * p2->x + w3 * p3->x;
        ys[i] = w0 * p0->y + w1 * p1->y + w2 * p2->y + w3 * p3->y;
    }

    grl_image_draw_ctx_init (self, &ctx);
    grl_image_stroke_path (&ctx, xs, ys, segments + 1, FALSE,
                           (gfloat)thickness * 0.5f, GRL_TO_RAYLIB_COLOR (color));

    g_free (xs);
    g_free (ys);
}

/**
 * grl_image_draw_gradient_rect:
 * @self: A #GrlImage.
 * @rect: Region to fill.
 * @color_a: Start color (left for horizontal, top for vertical).
 * @color_b: End color (right for horizontal, bottom for vertical).
 * @axis: Interpolation axis.
 *
 * Fills a rectangular region of the image with a linear gradient. Unlike
 * grl_image_new_gradient_linear(), this draws onto the existing image. Honours
 * the image's blend mode and clip rectangle.
 */
void
grl_image_draw_gradient_rect (GrlImage           *self,
                              const GrlRectangle *rect,
                              const GrlColor     *color_a,
                              const GrlColor     *color_b,
                              GrlGradientAxis     axis)
{
    GrlDrawCtx ctx;
    Color      ca, cb;
    gint       x0, y0, x1, y1, px, py;
    gfloat     denom;

    g_return_if_fail (GRL_IS_IMAGE (self));
    g_return_if_fail (rect != NULL);
    g_return_if_fail (color_a != NULL);
    g_return_if_fail (color_b != NULL);

    grl_image_draw_ctx_init (self, &ctx);
    ca = GRL_TO_RAYLIB_COLOR (color_a);
    cb = GRL_TO_RAYLIB_COLOR (color_b);

    x0 = (gint)rect->x;
    y0 = (gint)rect->y;
    x1 = x0 + (gint)rect->width;
    y1 = y0 + (gint)rect->height;

    if (axis == GRL_GRADIENT_AXIS_VERTICAL)
    {
        denom = (gfloat)(y1 - y0 - 1);
        if (denom < 1.0f)
            denom = 1.0f;
        for (py = y0; py < y1; py++)
        {
            Color c = grl_image_color_lerp (ca, cb, (gfloat)(py - y0) / denom);
            grl_image_span (&ctx, py, x0, x1, c);
        }
    }
    else
    {
        denom = (gfloat)(x1 - x0 - 1);
        if (denom < 1.0f)
            denom = 1.0f;
        for (px = x0; px < x1; px++)
        {
            Color c = grl_image_color_lerp (ca, cb, (gfloat)(px - x0) / denom);

            for (py = y0; py < y1; py++)
                grl_image_plot (&ctx, px, py, c, 255);
        }
    }
}

/**
 * grl_image_draw_gradient_radial:
 * @self: A #GrlImage.
 * @center_x: Center X position.
 * @center_y: Center Y position.
 * @radius: Outer radius.
 * @inner: Color at the center.
 * @outer: Color at the edge.
 *
 * Fills a disc with a radial gradient from @inner at the center to @outer at
 * @radius. Pixels beyond @radius are untouched. Ideal for glows and halos
 * (combine with %GRL_IMAGE_BLEND_ADD). Honours the image's blend mode, clip
 * rectangle and anti-alias state (soft outer edge).
 */
void
grl_image_draw_gradient_radial (GrlImage       *self,
                                gint            center_x,
                                gint            center_y,
                                gint            radius,
                                const GrlColor *inner,
                                const GrlColor *outer)
{
    GrlDrawCtx ctx;
    Color      ci, co;
    gfloat     cx, cy, r;
    gint       ix0, ix1, iy0, iy1, px, py;

    g_return_if_fail (GRL_IS_IMAGE (self));
    g_return_if_fail (inner != NULL);
    g_return_if_fail (outer != NULL);

    if (radius <= 0)
        return;

    grl_image_draw_ctx_init (self, &ctx);
    ci = GRL_TO_RAYLIB_COLOR (inner);
    co = GRL_TO_RAYLIB_COLOR (outer);

    cx = (gfloat)center_x + 0.5f;
    cy = (gfloat)center_y + 0.5f;
    r = (gfloat)radius;

    ix0 = center_x - radius;
    ix1 = center_x + radius;
    iy0 = center_y - radius;
    iy1 = center_y + radius;

    for (py = iy0; py <= iy1; py++)
    {
        for (px = ix0; px <= ix1; px++)
        {
            gfloat fx = (gfloat)px + 0.5f;
            gfloat fy = (gfloat)py + 0.5f;
            gfloat dist = sqrtf ((fx - cx) * (fx - cx) + (fy - cy) * (fy - cy));
            Color  c;

            if (dist > r)
                continue;

            c = grl_image_color_lerp (ci, co, dist / r);

            if (ctx.antialias && (r - dist) < 1.0f)
                grl_image_plot (&ctx, px, py, c, (guint)((r - dist) * 255.0f + 0.5f));
            else
                grl_image_plot (&ctx, px, py, c, 255);
        }
    }
}

/**
 * grl_image_flood_fill:
 * @self: A #GrlImage.
 * @x: Seed X position.
 * @y: Seed Y position.
 * @color: Replacement color.
 * @tolerance: Per-channel match tolerance (0-255) against the seed color.
 *
 * Performs a 4-connected scanline flood fill starting at (@x, @y), replacing
 * the contiguous region whose color matches the seed pixel within @tolerance.
 *
 * The fill overwrites matched pixels with @color (it does not blend), and is
 * constrained to the active clip rectangle. Requires the image to be readable
 * via pixel sampling; works on any pixel format.
 */
void
grl_image_flood_fill (GrlImage       *self,
                      gint            x,
                      gint            y,
                      const GrlColor *color,
                      gint            tolerance)
{
    GrlDrawCtx ctx;
    GArray    *stack;
    Color      fill, seed;
    gint64     packed;

    g_return_if_fail (GRL_IS_IMAGE (self));
    g_return_if_fail (color != NULL);

    grl_image_draw_ctx_init (self, &ctx);

    if (x < ctx.cx0 || x >= ctx.cx1 || y < ctx.cy0 || y >= ctx.cy1)
        return;

    fill = GRL_TO_RAYLIB_COLOR (color);
    seed = GetImageColor (self->handle, x, y);

    /* Nothing to do if the seed already matches the fill (also avoids loops). */
    if (abs ((gint)seed.r - (gint)fill.r) <= tolerance &&
        abs ((gint)seed.g - (gint)fill.g) <= tolerance &&
        abs ((gint)seed.b - (gint)fill.b) <= tolerance &&
        abs ((gint)seed.a - (gint)fill.a) <= tolerance)
        return;

    stack = g_array_new (FALSE, FALSE, sizeof (gint64));
    packed = ((gint64)x << 32) | (guint32)y;
    g_array_append_val (stack, packed);

    while (stack->len > 0)
    {
        gint64 v = g_array_index (stack, gint64, stack->len - 1);
        gint   cx = (gint)(v >> 32);
        gint   cy = (gint)(v & 0xffffffff);
        gint   lx = cx;
        gint   rx = cx;
        gint   px;
        gint   row;

        g_array_set_size (stack, stack->len - 1);

        /* Extend the span left and right while it matches the seed. */
        while (lx - 1 >= ctx.cx0)
        {
            Color s = GetImageColor (self->handle, lx - 1, cy);

            if (abs ((gint)s.r - (gint)seed.r) > tolerance ||
                abs ((gint)s.g - (gint)seed.g) > tolerance ||
                abs ((gint)s.b - (gint)seed.b) > tolerance ||
                abs ((gint)s.a - (gint)seed.a) > tolerance)
                break;
            lx--;
        }
        while (rx + 1 < ctx.cx1)
        {
            Color s = GetImageColor (self->handle, rx + 1, cy);

            if (abs ((gint)s.r - (gint)seed.r) > tolerance ||
                abs ((gint)s.g - (gint)seed.g) > tolerance ||
                abs ((gint)s.b - (gint)seed.b) > tolerance ||
                abs ((gint)s.a - (gint)seed.a) > tolerance)
                break;
            rx++;
        }

        for (px = lx; px <= rx; px++)
            ImageDrawPixel (&self->handle, px, cy, fill);

        /* Seed the rows above and below. */
        for (row = cy - 1; row <= cy + 1; row += 2)
        {
            if (row < ctx.cy0 || row >= ctx.cy1)
                continue;

            px = lx;
            while (px <= rx)
            {
                Color s = GetImageColor (self->handle, px, row);
                gboolean match =
                    (abs ((gint)s.r - (gint)seed.r) <= tolerance &&
                     abs ((gint)s.g - (gint)seed.g) <= tolerance &&
                     abs ((gint)s.b - (gint)seed.b) <= tolerance &&
                     abs ((gint)s.a - (gint)seed.a) <= tolerance);

                if (match)
                {
                    gint64 np = ((gint64)px << 32) | (guint32)row;

                    g_array_append_val (stack, np);
                    /* Skip the rest of this matching run; it will be filled
                     * when the seed we just pushed is processed. */
                    while (px <= rx)
                    {
                        Color s2 = GetImageColor (self->handle, px, row);

                        if (abs ((gint)s2.r - (gint)seed.r) > tolerance ||
                            abs ((gint)s2.g - (gint)seed.g) > tolerance ||
                            abs ((gint)s2.b - (gint)seed.b) > tolerance ||
                            abs ((gint)s2.a - (gint)seed.a) > tolerance)
                            break;
                        px++;
                    }
                }
                else
                {
                    px++;
                }
            }
        }
    }

    g_array_free (stack, TRUE);
}

/*
 * Public API - Drawing state
 */

/**
 * grl_image_set_blend_mode:
 * @self: A #GrlImage.
 * @mode: The blend mode to use for subsequent drawing.
 *
 * Sets the blend mode applied by every grl_image_draw_* primitive.
 *
 * The default, %GRL_IMAGE_BLEND_REPLACE, overwrites destination pixels and is
 * byte-identical to drawing with no blend mode set. Other modes blend the
 * source against the existing pixels (for example %GRL_IMAGE_BLEND_ADD gives
 * additive glow with per-channel saturation).
 *
 * Blend modes other than %GRL_IMAGE_BLEND_REPLACE require the image to be in
 * %GRL_PIXELFORMAT_UNCOMPRESSED_R8G8B8A8 format; on any other format drawing
 * silently falls back to %GRL_IMAGE_BLEND_REPLACE.
 */
void
grl_image_set_blend_mode (GrlImage          *self,
                          GrlImageBlendMode  mode)
{
    g_return_if_fail (GRL_IS_IMAGE (self));

    self->blend_mode = mode;
}

/**
 * grl_image_get_blend_mode:
 * @self: A #GrlImage.
 *
 * Gets the current blend mode.
 *
 * Returns: The current #GrlImageBlendMode.
 */
GrlImageBlendMode
grl_image_get_blend_mode (GrlImage *self)
{
    g_return_val_if_fail (GRL_IS_IMAGE (self), GRL_IMAGE_BLEND_REPLACE);

    return self->blend_mode;
}

/**
 * grl_image_set_blend_color_space:
 * @self: A #GrlImage.
 * @space: The #GrlImageColorSpace to composite in.
 *
 * Sets the colour space used for blended drawing onto @self.
 *
 * The default is %GRL_IMAGE_COLOR_SPACE_GAMMA, which blends directly on 8-bit
 * sRGB values and is byte-for-byte identical to the historical behaviour.
 * %GRL_IMAGE_COLOR_SPACE_LINEAR decodes pixels to linear light before blending
 * and re-encodes afterwards, so anti-aliased edges and
 * %GRL_IMAGE_BLEND_OVER / _ADD / _MULTIPLY / _SUBTRACT no longer darken or
 * shift hue at partial coverage.
 *
 * Linear blending requires an %GRL_PIXELFORMAT_UNCOMPRESSED_R8G8B8A8 image; on
 * any other format the draw silently falls back to gamma blending. The colour
 * space never affects %GRL_IMAGE_BLEND_REPLACE, which always overwrites.
 */
void
grl_image_set_blend_color_space (GrlImage           *self,
                                 GrlImageColorSpace  space)
{
    g_return_if_fail (GRL_IS_IMAGE (self));

    self->blend_space = space;
}

/**
 * grl_image_get_blend_color_space:
 * @self: A #GrlImage.
 *
 * Gets the colour space used for blended drawing.
 *
 * Returns: The current #GrlImageColorSpace.
 */
GrlImageColorSpace
grl_image_get_blend_color_space (GrlImage *self)
{
    g_return_val_if_fail (GRL_IS_IMAGE (self), GRL_IMAGE_COLOR_SPACE_GAMMA);

    return self->blend_space;
}

/**
 * grl_image_set_clip_rect:
 * @self: A #GrlImage.
 * @clip: (nullable): Clip rectangle in image space, or %NULL to clear clipping.
 *
 * Constrains subsequent grl_image_draw_* primitives to the given rectangle.
 * Pixels outside the (half-open) clip box are left untouched. Passing %NULL
 * removes the clip and drawing may again affect the whole image.
 *
 * The clip is intersected with the image bounds; fractional coordinates are
 * truncated to integer pixels.
 */
void
grl_image_set_clip_rect (GrlImage           *self,
                         const GrlRectangle *clip)
{
    g_return_if_fail (GRL_IS_IMAGE (self));

    if (clip == NULL)
    {
        self->has_clip = FALSE;
        self->clip_x = 0;
        self->clip_y = 0;
        self->clip_w = 0;
        self->clip_h = 0;
        return;
    }

    self->has_clip = TRUE;
    self->clip_x = (gint)clip->x;
    self->clip_y = (gint)clip->y;
    self->clip_w = (gint)clip->width;
    self->clip_h = (gint)clip->height;
}

/**
 * grl_image_get_clip_rect:
 * @self: A #GrlImage.
 * @out_clip: (out caller-allocates): Filled with the active clip rectangle.
 *
 * Retrieves the current clip rectangle, if any.
 *
 * Returns: %TRUE and fills @out_clip if clipping is active, %FALSE otherwise.
 */
gboolean
grl_image_get_clip_rect (GrlImage     *self,
                         GrlRectangle *out_clip)
{
    g_return_val_if_fail (GRL_IS_IMAGE (self), FALSE);

    if (!self->has_clip)
        return FALSE;

    if (out_clip != NULL)
    {
        out_clip->x = (gfloat)self->clip_x;
        out_clip->y = (gfloat)self->clip_y;
        out_clip->width = (gfloat)self->clip_w;
        out_clip->height = (gfloat)self->clip_h;
    }

    return TRUE;
}

/**
 * grl_image_set_antialias:
 * @self: A #GrlImage.
 * @enabled: %TRUE to anti-alias the edges of new primitives.
 *
 * Enables or disables edge anti-aliasing for grl_image_draw_* primitives that
 * support it (circle/ellipse outlines, thick lines, polylines and beziers).
 *
 * Anti-aliasing blends partially-covered edge pixels and therefore requires an
 * %GRL_PIXELFORMAT_UNCOMPRESSED_R8G8B8A8 image; it is ignored on other formats.
 * Edge pixels are blended even when the blend mode is %GRL_IMAGE_BLEND_REPLACE.
 */
void
grl_image_set_antialias (GrlImage *self,
                         gboolean  enabled)
{
    g_return_if_fail (GRL_IS_IMAGE (self));

    self->antialias = enabled;
}

/**
 * grl_image_get_antialias:
 * @self: A #GrlImage.
 *
 * Gets whether edge anti-aliasing is enabled.
 *
 * Returns: %TRUE if anti-aliasing is enabled.
 */
gboolean
grl_image_get_antialias (GrlImage *self)
{
    g_return_val_if_fail (GRL_IS_IMAGE (self), FALSE);

    return self->antialias;
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
 * Indexed PNG functions
 */

/**
 * grl_image_new_from_png_indexed:
 * @filename: (type filename): Path to indexed PNG file
 * @palette_out: (out) (optional) (transfer full): Return location for palette
 * @error: (nullable): Return location for error
 *
 * Loads an indexed PNG image.
 *
 * Returns: (transfer full) (nullable): A new #GrlImage, or %NULL on error
 */
GrlImage *
grl_image_new_from_png_indexed (const gchar    *filename,
                                GrlPngPalette **palette_out,
                                GError        **error)
{
    GrlImage      *self;
    Image          handle;
    char          *indexed_data;
    char          *rgba_data;
    rpng_palette   rpng_pal;
    int            width;
    int            height;
    int            i;

    g_return_val_if_fail (filename != NULL, NULL);
    g_return_val_if_fail (error == NULL || *error == NULL, NULL);

    if (!g_file_test (filename, G_FILE_TEST_EXISTS))
    {
        g_set_error (error,
                     G_FILE_ERROR,
                     G_FILE_ERROR_NOENT,
                     "PNG file not found: %s",
                     filename);
        return NULL;
    }

    /* Initialize palette structure */
    rpng_pal.color_count = 0;
    rpng_pal.colors = NULL;

    /* Load indexed data and palette */
    indexed_data = rpng_load_image_indexed (filename, &width, &height, &rpng_pal);

    if (indexed_data == NULL)
    {
        g_set_error (error,
                     G_FILE_ERROR,
                     G_FILE_ERROR_FAILED,
                     "Failed to load indexed PNG (not indexed or invalid): %s",
                     filename);
        return NULL;
    }

    /* Convert indexed data to RGBA for raylib Image */
    rgba_data = rpng_unindex_image_data (indexed_data, width, height, rpng_pal);

    if (rgba_data == NULL)
    {
        RPNG_FREE (indexed_data);
        if (rpng_pal.colors != NULL)
            RPNG_FREE (rpng_pal.colors);

        g_set_error (error,
                     G_FILE_ERROR,
                     G_FILE_ERROR_FAILED,
                     "Failed to convert indexed data to RGBA");
        return NULL;
    }

    /* Create raylib Image from RGBA data */
    handle.data = rgba_data;
    handle.width = width;
    handle.height = height;
    handle.mipmaps = 1;
    handle.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;

    /* Create output palette if requested */
    if (palette_out != NULL)
    {
        GrlPngPalette *palette;

        palette = grl_png_palette_new_empty (rpng_pal.color_count);

        for (i = 0; i < rpng_pal.color_count; i++)
        {
            GrlColor *color;

            color = grl_color_new (rpng_pal.colors[i].r,
                                   rpng_pal.colors[i].g,
                                   rpng_pal.colors[i].b,
                                   rpng_pal.colors[i].a);
            grl_png_palette_set_color (palette, i, color);
            grl_color_free (color);
        }

        *palette_out = palette;
    }

    /* Clean up rpng resources */
    RPNG_FREE (indexed_data);
    if (rpng_pal.colors != NULL)
        RPNG_FREE (rpng_pal.colors);

    /* Create GrlImage wrapper */
    self = g_object_new (GRL_TYPE_IMAGE, NULL);
    self->handle = handle;
    self->valid = TRUE;

    return self;
}

/**
 * grl_image_save_as_png_indexed:
 * @self: A #GrlImage
 * @filename: (type filename): Output filename
 * @palette: The color palette to use
 * @error: (nullable): Return location for error
 *
 * Saves an image as an indexed PNG.
 *
 * Returns: %TRUE on success
 */
gboolean
grl_image_save_as_png_indexed (GrlImage       *self,
                               const gchar    *filename,
                               GrlPngPalette  *palette,
                               GError        **error)
{
    rpng_palette   rpng_pal;
    char          *indexed_data;
    gint           width;
    gint           height;
    gint           pixel_count;
    guint8        *rgba_data;
    int            result;
    int            i;

    g_return_val_if_fail (GRL_IS_IMAGE (self), FALSE);
    g_return_val_if_fail (filename != NULL, FALSE);
    g_return_val_if_fail (palette != NULL, FALSE);
    g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

    if (!self->valid || self->handle.data == NULL)
    {
        g_set_error (error,
                     G_FILE_ERROR,
                     G_FILE_ERROR_FAILED,
                     "Invalid image");
        return FALSE;
    }

    width = self->handle.width;
    height = self->handle.height;
    pixel_count = width * height;

    /* Build rpng palette from GrlPngPalette */
    rpng_pal.color_count = palette->color_count;
    rpng_pal.colors = g_new (rpng_color, palette->color_count);

    for (i = 0; i < palette->color_count; i++)
    {
        if (palette->colors != NULL && i < palette->color_count)
        {
            rpng_pal.colors[i].r = palette->colors[i].r;
            rpng_pal.colors[i].g = palette->colors[i].g;
            rpng_pal.colors[i].b = palette->colors[i].b;
            rpng_pal.colors[i].a = palette->colors[i].a;
        }
    }

    /* Export image to RGBA if not already */
    rgba_data = (guint8 *)ExportImageToMemory (self->handle, ".raw", NULL);
    if (rgba_data == NULL)
    {
        /* Fallback: Use image data directly if RGBA */
        if (self->handle.format == PIXELFORMAT_UNCOMPRESSED_R8G8B8A8)
            rgba_data = (guint8 *)self->handle.data;
        else
        {
            g_free (rpng_pal.colors);
            g_set_error (error,
                         G_FILE_ERROR,
                         G_FILE_ERROR_FAILED,
                         "Cannot convert image to RGBA for indexed export");
            return FALSE;
        }
    }

    /* Create indexed data by finding closest palette color for each pixel */
    indexed_data = g_new (char, pixel_count);

    for (i = 0; i < pixel_count; i++)
    {
        guint8 r;
        guint8 g;
        guint8 b;
        guint8 a;
        int    best_idx;
        int    best_dist;
        int    j;

        r = rgba_data[i * 4 + 0];
        g = rgba_data[i * 4 + 1];
        b = rgba_data[i * 4 + 2];
        a = rgba_data[i * 4 + 3];

        /* Find closest palette color (simple Euclidean distance) */
        best_idx = 0;
        best_dist = G_MAXINT;

        for (j = 0; j < rpng_pal.color_count; j++)
        {
            int dr;
            int dg;
            int db;
            int da;
            int dist;

            dr = (int)r - (int)rpng_pal.colors[j].r;
            dg = (int)g - (int)rpng_pal.colors[j].g;
            db = (int)b - (int)rpng_pal.colors[j].b;
            da = (int)a - (int)rpng_pal.colors[j].a;

            dist = dr * dr + dg * dg + db * db + da * da;

            if (dist < best_dist)
            {
                best_dist = dist;
                best_idx = j;
            }
        }

        indexed_data[i] = (char)best_idx;
    }

    /* Save as indexed PNG */
    result = rpng_save_image_indexed (filename, indexed_data,
                                      width, height, rpng_pal);

    g_free (indexed_data);
    g_free (rpng_pal.colors);

    if (result == 0)
    {
        g_set_error (error,
                     G_FILE_ERROR,
                     G_FILE_ERROR_FAILED,
                     "Failed to save indexed PNG: %s",
                     filename);
        return FALSE;
    }

    return TRUE;
}

/*
 * =============================================================================
 * Porter-Duff compositing
 * =============================================================================
 */

/*
 * Compute Porter-Duff coverage factors (Fa, Fb) from the operator and the
 * source / destination alpha values (already in [0,1]).
 *
 * Standard table (Porter-Duff 1984):
 *   CLEAR:    Fa=0,    Fb=0
 *   SRC:      Fa=1,    Fb=0
 *   DST:      Fa=0,    Fb=1
 *   SRC_OVER: Fa=1,    Fb=1-sa
 *   DST_OVER: Fa=1-da, Fb=1
 *   SRC_IN:   Fa=da,   Fb=0
 *   DST_IN:   Fa=0,    Fb=sa
 *   SRC_OUT:  Fa=1-da, Fb=0
 *   DST_OUT:  Fa=0,    Fb=1-sa
 *   SRC_ATOP: Fa=da,   Fb=1-sa
 *   DST_ATOP: Fa=1-da, Fb=sa
 *   XOR:      Fa=1-da, Fb=1-sa
 */
static void
grl_pd_factors (GrlPorterDuffOp op,
                gfloat          sa,
                gfloat          da,
                gfloat         *fa,
                gfloat         *fb)
{
    switch (op)
    {
    case GRL_PORTER_DUFF_CLEAR:
        *fa = 0.0f; *fb = 0.0f; break;
    case GRL_PORTER_DUFF_SRC:
        *fa = 1.0f; *fb = 0.0f; break;
    case GRL_PORTER_DUFF_DST:
        *fa = 0.0f; *fb = 1.0f; break;
    case GRL_PORTER_DUFF_SRC_OVER:
        *fa = 1.0f; *fb = 1.0f - sa; break;
    case GRL_PORTER_DUFF_DST_OVER:
        *fa = 1.0f - da; *fb = 1.0f; break;
    case GRL_PORTER_DUFF_SRC_IN:
        *fa = da; *fb = 0.0f; break;
    case GRL_PORTER_DUFF_DST_IN:
        *fa = 0.0f; *fb = sa; break;
    case GRL_PORTER_DUFF_SRC_OUT:
        *fa = 1.0f - da; *fb = 0.0f; break;
    case GRL_PORTER_DUFF_DST_OUT:
        *fa = 0.0f; *fb = 1.0f - sa; break;
    case GRL_PORTER_DUFF_SRC_ATOP:
        *fa = da; *fb = 1.0f - sa; break;
    case GRL_PORTER_DUFF_DST_ATOP:
        *fa = 1.0f - da; *fb = sa; break;
    case GRL_PORTER_DUFF_XOR:
        *fa = 1.0f - da; *fb = 1.0f - sa; break;
    default:
        *fa = 1.0f; *fb = 1.0f - sa; break; /* default: SRC_OVER */
    }
}

/**
 * grl_image_composite:
 * @self: The destination #GrlImage (must be R8G8B8A8)
 * @src: The source #GrlImage (must be R8G8B8A8)
 * @op: A #GrlPorterDuffOp compositing operator
 * @dst_x: X offset in @self where the top-left of @src is placed
 * @dst_y: Y offset in @self where the top-left of @src is placed
 *
 * Composites @src onto @self at pixel offset (@dst_x, @dst_y) using the
 * standard Porter-Duff coverage algebra on premultiplied alpha. Graylib stores
 * straight (un-premultiplied) alpha; the conversion is performed internally.
 *
 * Both @self and @src must be in #GRL_PIXELFORMAT_UNCOMPRESSED_R8G8B8A8 format.
 * If either image is in a different format the function returns silently (no
 * crash, no modification).
 *
 * @self's clip rectangle (if set) is honoured: only pixels within the effective
 * clip box are written. Pixels of @self that lie outside the mapped area of @src
 * are left completely unchanged.
 *
 * If @self's blend colour space is #GRL_IMAGE_COLOR_SPACE_LINEAR, the RGB
 * compositing is performed in linear light using the sRGB lookup tables
 * (equivalent to what the blended draw-primitives do); alpha coverage values
 * are composited directly (they are never gamma-transformed). Otherwise
 * compositing is in gamma (8-bit sRGB) space.
 *
 * Note: #GRL_PORTER_DUFF_DST_OVER places @src *behind* @self and is the key
 * operator for drop-shadows — composite the blurred shadow image under the
 * original with DST_OVER. This is sometimes documented as DEST_OVER in other
 * APIs; the two names refer to the same operation.
 */
void
grl_image_composite (GrlImage        *self,
                     GrlImage        *src,
                     GrlPorterDuffOp  op,
                     gint             dst_x,
                     gint             dst_y)
{
    GrlDrawCtx ctx;
    gint       src_w, src_h;
    gint       x, y;
    gint       sx0, sy0, sx1, sy1;  /* src pixel region to composite */
    gint       dx0, dy0;            /* corresponding dst top-left */
    gboolean   use_linear;

    g_return_if_fail (GRL_IS_IMAGE (self));
    g_return_if_fail (GRL_IS_IMAGE (src));

    /* Both images must be RGBA8 */
    if (self->handle.format != PIXELFORMAT_UNCOMPRESSED_R8G8B8A8)
        return;
    if (src->handle.format != PIXELFORMAT_UNCOMPRESSED_R8G8B8A8)
        return;

    grl_image_draw_ctx_init (self, &ctx);

    src_w = src->handle.width;
    src_h = src->handle.height;

    /* Compute source rectangle: the region of src that maps into self. */
    sx0 = 0;
    sy0 = 0;
    sx1 = src_w;
    sy1 = src_h;

    /* Corresponding destination top-left for (sx0, sy0). */
    dx0 = dst_x;
    dy0 = dst_y;

    /* Clip: left/top of src may be off-canvas */
    if (dx0 < ctx.cx0)
    {
        sx0 += (ctx.cx0 - dx0);
        dx0  = ctx.cx0;
    }
    if (dy0 < ctx.cy0)
    {
        sy0 += (ctx.cy0 - dy0);
        dy0  = ctx.cy0;
    }

    /* Clip: right/bottom of src may be off-canvas */
    if (dst_x + sx1 > ctx.cx1)
        sx1 = ctx.cx1 - dst_x;
    if (dst_y + sy1 > ctx.cy1)
        sy1 = ctx.cy1 - dst_y;

    /* Nothing to do? */
    if (sx0 >= sx1 || sy0 >= sy1)
        return;

    use_linear = (self->blend_space == GRL_IMAGE_COLOR_SPACE_LINEAR);
    if (use_linear)
        grl_image_blend_luts_init ();

    for (y = sy0; y < sy1; y++)
    {
        gint dy = dy0 + (y - sy0);

        for (x = sx0; x < sx1; x++)
        {
            gint   dx = dx0 + (x - sx0);
            guint8 *dp = (guint8 *)self->handle.data +
                         ((gsize)dy * self->handle.width + dx) * 4;
            const guint8 *sp = (const guint8 *)src->handle.data +
                               ((gsize)y * src_w + x) * 4;

            gfloat sr, sg, sb, dr, dg, db;
            gfloat sa, da, fa, fb, oa;
            gfloat out_r, out_g, out_b;
            guint  out_a_byte;

            sa = (gfloat)sp[3] / 255.0f;
            da = (gfloat)dp[3] / 255.0f;

            grl_pd_factors (op, sa, da, &fa, &fb);

            oa = fa * sa + fb * da;

            if (use_linear)
            {
                /* Decode to linear */
                sr = grl_srgb_to_linear_lut[sp[0]];
                sg = grl_srgb_to_linear_lut[sp[1]];
                sb = grl_srgb_to_linear_lut[sp[2]];
                dr = grl_srgb_to_linear_lut[dp[0]];
                dg = grl_srgb_to_linear_lut[dp[1]];
                db = grl_srgb_to_linear_lut[dp[2]];

                /* Premultiplied compositing */
                out_r = fa * sr * sa + fb * dr * da;
                out_g = fa * sg * sa + fb * dg * da;
                out_b = fa * sb * sa + fb * db * da;

                if (oa > 1e-6f)
                {
                    out_r /= oa;
                    out_g /= oa;
                    out_b /= oa;
                }
                else
                {
                    out_r = out_g = out_b = 0.0f;
                }

                if (out_r > 1.0f) out_r = 1.0f;
                if (out_g > 1.0f) out_g = 1.0f;
                if (out_b > 1.0f) out_b = 1.0f;

                out_a_byte = (guint)(oa * 255.0f + 0.5f);
                if (out_a_byte > 255) out_a_byte = 255;

                dp[0] = grl_lin_to_srgb_byte (out_r);
                dp[1] = grl_lin_to_srgb_byte (out_g);
                dp[2] = grl_lin_to_srgb_byte (out_b);
                dp[3] = (guint8)out_a_byte;
            }
            else
            {
                /* Gamma (8-bit sRGB) space */
                sr = (gfloat)sp[0];
                sg = (gfloat)sp[1];
                sb = (gfloat)sp[2];
                dr = (gfloat)dp[0];
                dg = (gfloat)dp[1];
                db = (gfloat)dp[2];

                /* Premultiplied compositing in sRGB (approximate but fast) */
                out_r = fa * sr * sa + fb * dr * da;
                out_g = fa * sg * sa + fb * dg * da;
                out_b = fa * sb * sa + fb * db * da;

                if (oa > 1e-6f)
                {
                    out_r /= oa;
                    out_g /= oa;
                    out_b /= oa;
                }
                else
                {
                    out_r = out_g = out_b = 0.0f;
                }

                if (out_r > 255.0f) out_r = 255.0f;
                if (out_g > 255.0f) out_g = 255.0f;
                if (out_b > 255.0f) out_b = 255.0f;

                out_a_byte = (guint)(oa * 255.0f + 0.5f);
                if (out_a_byte > 255) out_a_byte = 255;

                dp[0] = (guint8)(out_r + 0.5f);
                dp[1] = (guint8)(out_g + 0.5f);
                dp[2] = (guint8)(out_b + 0.5f);
                dp[3] = (guint8)out_a_byte;
            }
        }
    }
}

/*
 * =============================================================================
 * First-class alpha mask
 * =============================================================================
 */

/**
 * grl_image_new_mask:
 * @width: Mask width in pixels (must be > 0)
 * @height: Mask height in pixels (must be > 0)
 *
 * Creates a new single-channel grayscale (1 byte/pixel,
 * #GRL_PIXELFORMAT_UNCOMPRESSED_GRAYSCALE) image initialised to zero (fully
 * transparent when used as a coverage mask).
 *
 * The mask can be drawn into with the normal grl_image_draw_* primitives. The
 * drawn luminance becomes coverage when the mask is later applied with
 * grl_image_apply_mask(): white (255) means keep the destination pixel at full
 * alpha; black (0) means cut it to alpha 0.
 *
 * Returns: (transfer full): A new #GrlImage mask, or %NULL on invalid arguments.
 */
GrlImage *
grl_image_new_mask (gint width,
                    gint height)
{
    Image handle;
    Color clear = { 0, 0, 0, 0 };

    g_return_val_if_fail (width > 0, NULL);
    g_return_val_if_fail (height > 0, NULL);

    /* Allocate through raylib (GenImageColor) so the buffer is owned by the
     * same allocator that grl_image_finalize()'s UnloadImage() frees with.
     * Convert to single-channel grayscale; an all-zero RGBA image maps to
     * luminance 0, i.e. a fully-transparent coverage mask. */
    handle = GenImageColor (width, height, clear);
    ImageFormat (&handle, PIXELFORMAT_UNCOMPRESSED_GRAYSCALE);

    return grl_image_new_from_handle (handle);
}

/**
 * grl_image_apply_mask:
 * @self: A #GrlImage (must be R8G8B8A8)
 * @mask: A grayscale mask image (typically created with grl_image_new_mask())
 * @offset_x: X offset: pixel (x, y) of @self samples mask at (x - @offset_x, y - @offset_y)
 * @offset_y: Y offset (see @offset_x)
 *
 * Multiplies the alpha channel of every pixel in @self by the corresponding
 * coverage value from @mask, sampled at (x - @offset_x, y - @offset_y):
 *
 * - Pixels that map to a valid mask location: new_alpha = old_alpha * mask_value / 255
 * - Pixels of @self that map **outside** the mask region: alpha is set to 0 (cut)
 *
 * The "cut outside" contract is the standard stencil behaviour — the mask
 * defines exactly which region of @self survives. This is consistent with
 * grl_image_alpha_mask() (raylib's ImageAlphaMask), which also zeros pixels
 * outside the mask bounds.
 *
 * @self must be #GRL_PIXELFORMAT_UNCOMPRESSED_R8G8B8A8; if not, returns
 * silently. The mask may be any grayscale format; GRAYSCALE (1 byte/pixel) is
 * the native format returned by grl_image_new_mask(). For RGBA masks the red
 * channel is used as coverage.
 *
 * @self's clip rectangle (if set) is honoured: only pixels within the clip box
 * are modified.
 */
void
grl_image_apply_mask (GrlImage *self,
                      GrlImage *mask,
                      gint      offset_x,
                      gint      offset_y)
{
    GrlDrawCtx ctx;
    gint       x, y;
    gint       mw, mh;
    gboolean   mask_is_gray;

    g_return_if_fail (GRL_IS_IMAGE (self));
    g_return_if_fail (GRL_IS_IMAGE (mask));

    if (self->handle.format != PIXELFORMAT_UNCOMPRESSED_R8G8B8A8)
        return;

    grl_image_draw_ctx_init (self, &ctx);

    mw = mask->handle.width;
    mh = mask->handle.height;
    mask_is_gray = (mask->handle.format == PIXELFORMAT_UNCOMPRESSED_GRAYSCALE);

    for (y = ctx.cy0; y < ctx.cy1; y++)
    {
        for (x = ctx.cx0; x < ctx.cx1; x++)
        {
            gint    mx = x - offset_x;
            gint    my = y - offset_y;
            guint8 *dp = (guint8 *)self->handle.data +
                         ((gsize)y * self->handle.width + x) * 4;
            guint8  coverage;

            if (mx < 0 || mx >= mw || my < 0 || my >= mh)
            {
                /* Outside mask: cut alpha to 0 */
                dp[3] = 0;
                continue;
            }

            if (mask_is_gray)
            {
                coverage = ((const guint8 *)mask->handle.data)[
                               (gsize)my * mw + mx];
            }
            else
            {
                /* Fallback: use GetImageColor red channel as coverage */
                Color mc = GetImageColor (mask->handle, mx, my);
                coverage = mc.r;
            }

            dp[3] = (guint8)(((guint)dp[3] * (guint)coverage) / 255u);
        }
    }
}

/*
 * =============================================================================
 * Box blur
 * =============================================================================
 */

/**
 * grl_image_blur_box:
 * @self: A #GrlImage (must be R8G8B8A8)
 * @radius: Box blur radius in pixels (0 or negative is a no-op)
 *
 * Applies a separable box blur (horizontal pass then vertical pass) with a
 * kernel of size (2 * @radius + 1). All four channels (R, G, B, A) are blurred
 * so the function works correctly on shadow silhouettes (you want a soft alpha
 * edge as well as soft colour).
 *
 * Sampling near the image edges is handled by clamping to the nearest valid
 * pixel (clamp-to-edge). The result is stored back into @self in-place using
 * a temporary buffer.
 *
 * @self must be #GRL_PIXELFORMAT_UNCOMPRESSED_R8G8B8A8; on other formats the
 * function returns silently.
 *
 * This is a fast integer accumulator implementation (O(width * height)
 * regardless of radius) using a sliding-window sum.
 */
void
grl_image_blur_box (GrlImage *self,
                    gint      radius)
{
    gint     w, h;
    guint8  *src_data;
    guint8  *tmp;
    gsize    row_stride;
    gint     x, y;
    gint     ksize; /* kernel size = 2*radius + 1 */

    g_return_if_fail (GRL_IS_IMAGE (self));

    if (radius <= 0)
        return;

    if (self->handle.format != PIXELFORMAT_UNCOMPRESSED_R8G8B8A8)
        return;

    w = self->handle.width;
    h = self->handle.height;
    src_data = (guint8 *)self->handle.data;
    row_stride = (gsize)w * 4;
    ksize = 2 * radius + 1;

    tmp = (guint8 *)g_malloc (row_stride * (gsize)h);

    /*
     * Horizontal pass: src_data -> tmp
     * For each row y, blur each pixel horizontally into tmp.
     */
    for (y = 0; y < h; y++)
    {
        guint  sum[4];
        gint   c;
        guint8 *row = src_data + (gsize)y * row_stride;
        guint8 *out = tmp      + (gsize)y * row_stride;

        /* Prime the accumulator with the leftmost kernel [-radius .. radius]
         * (clamped to [0, w-1]). */
        sum[0] = sum[1] = sum[2] = sum[3] = 0;
        for (c = -radius; c <= radius; c++)
        {
            gint sx = c;
            gint i;

            if (sx < 0)   sx = 0;
            if (sx >= w)  sx = w - 1;

            for (i = 0; i < 4; i++)
                sum[i] += row[sx * 4 + i];
        }

        for (x = 0; x < w; x++)
        {
            gint add_x = x + radius + 1;
            gint sub_x = x - radius;       /* element leaving the window at x+1 */
            gint i;

            /* Write the average for this pixel. */
            for (i = 0; i < 4; i++)
                out[x * 4 + i] = (guint8)(sum[i] / (guint)ksize);

            /* Slide the window: add the incoming column, remove the outgoing. */
            if (add_x >= w) add_x = w - 1;
            if (sub_x <  0) sub_x = 0;

            for (i = 0; i < 4; i++)
            {
                sum[i] += row[add_x * 4 + i];
                sum[i] -= row[sub_x * 4 + i];
            }
        }
    }

    /*
     * Vertical pass: tmp -> src_data
     */
    for (x = 0; x < w; x++)
    {
        guint  sum[4];
        gint   c;
        gint   i;

        sum[0] = sum[1] = sum[2] = sum[3] = 0;
        for (c = -radius; c <= radius; c++)
        {
            gint sy = c;

            if (sy < 0)   sy = 0;
            if (sy >= h)  sy = h - 1;

            for (i = 0; i < 4; i++)
                sum[i] += tmp[(gsize)sy * row_stride + x * 4 + i];
        }

        for (y = 0; y < h; y++)
        {
            gint add_y = y + radius + 1;
            gint sub_y = y - radius;       /* element leaving the window at y+1 */

            for (i = 0; i < 4; i++)
                src_data[(gsize)y * row_stride + x * 4 + i] =
                    (guint8)(sum[i] / (guint)ksize);

            if (add_y >= h) add_y = h - 1;
            if (sub_y <  0) sub_y = 0;

            for (i = 0; i < 4; i++)
            {
                sum[i] += tmp[(gsize)add_y * row_stride + x * 4 + i];
                sum[i] -= tmp[(gsize)sub_y * row_stride + x * 4 + i];
            }
        }
    }

    g_free (tmp);
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
