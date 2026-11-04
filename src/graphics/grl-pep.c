/* grl-pep.c
 *
 * Copyright 2026 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * graylib's .pep codec: raylib's external/pep.h compiled privately. Every
 * public pep_* symbol is renamed so this copy never collides with a raylib
 * that was built with SUPPORT_FILEFORMAT_PEP. Compiled as gnu99 (pep.h
 * uses C99 declarations).
 */

#include "config.h"
#include "grl-pep-private.h"

#include <string.h>

#define pep_compress     grl_pep_impl_compress
#define pep_decompress   grl_pep_impl_decompress
#define pep_free         grl_pep_impl_free
#define pep_serialize    grl_pep_impl_serialize
#define pep_deserialize  grl_pep_impl_deserialize
#define pep_save         grl_pep_impl_save
#define pep_load         grl_pep_impl_load
#define PEP_IMPLEMENTATION
#include <external/pep.h>

gboolean
grl_pep_is_file_type (const gchar *file_type)
{
    const gchar *dot;

    if (file_type == NULL)
        return FALSE;
    dot = strrchr (file_type, '.');
    return dot != NULL && g_ascii_strcasecmp (dot, ".pep") == 0;
}

gboolean
grl_pep_decode (const guint8 *data,
                gsize         size,
                Image        *out)
{
    pep       p;
    uint32_t *pixels;

    g_return_val_if_fail (out != NULL, FALSE);
    memset (out, 0, sizeof *out);

    if (data == NULL || size == 0 || size > G_MAXUINT32)
        return FALSE;

    /* the deserializer bounds-checks the header and the palette */
    p = pep_deserialize (data, (uint32_t)size);
    if (p.bytes == NULL || p.width == 0 || p.height == 0)
    {
        pep_free (&p);
        return FALSE;
    }

    /* lossless: keep the first colour opaque, no premultiplied alpha
     * (raylib 6.2 forces both on) */
    pixels = pep_decompress (&p, pep_rgba, 0, 0);
    if (pixels == NULL)
    {
        pep_free (&p);
        return FALSE;
    }

    out->data = pixels;
    out->width = p.width;
    out->height = p.height;
    out->mipmaps = 1;
    out->format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;

    pep_free (&p);
    return TRUE;
}

/* count_colors:
 * Distinct colours of @pixels, stopping once it passes @limit. */
static guint
count_colors (const uint32_t *pixels,
              gsize           n,
              guint           limit)
{
    g_autoptr(GHashTable) seen = g_hash_table_new (g_direct_hash, g_direct_equal);
    gsize                 i;

    for (i = 0; i < n; i++)
    {
        g_hash_table_add (seen, GUINT_TO_POINTER (pixels[i]));
        if (g_hash_table_size (seen) > limit)
            break;
    }
    return g_hash_table_size (seen);
}

guint8 *
grl_pep_encode (const Image *image,
                gsize       *out_size)
{
    Image     rgba;
    pep       p;
    uint8_t  *bytes;
    uint32_t  bytes_size;
    guint8   *result;
    guint     colors;

    g_return_val_if_fail (image != NULL, NULL);
    g_return_val_if_fail (out_size != NULL, NULL);
    *out_size = 0;

    if (image->data == NULL || image->width <= 0 || image->height <= 0)
        return NULL;
    if (image->width > GRL_PEP_MAX_SIZE || image->height > GRL_PEP_MAX_SIZE)
    {
        g_warning ("Cannot encode a %dx%d image as .pep (at most %d per side)",
                   image->width, image->height, GRL_PEP_MAX_SIZE);
        return NULL;
    }

    /* pep packs 32-bit RGBA pixels in memory order */
    rgba = ImageCopy (*image);
    if (rgba.format != PIXELFORMAT_UNCOMPRESSED_R8G8B8A8)
        ImageFormat (&rgba, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
    if (rgba.data == NULL || rgba.format != PIXELFORMAT_UNCOMPRESSED_R8G8B8A8)
    {
        UnloadImage (rgba);
        return NULL;
    }

    /* pep_compress() maps every colour past the 256th to index 0 */
    colors = count_colors ((const uint32_t *)rgba.data,
                           (gsize)rgba.width * (gsize)rgba.height,
                           GRL_PEP_MAX_COLORS);
    if (colors > GRL_PEP_MAX_COLORS)
    {
        g_warning ("Cannot encode an image with more than %d colours as .pep",
                   GRL_PEP_MAX_COLORS);
        UnloadImage (rgba);
        return NULL;
    }

    p = pep_compress ((const uint32_t *)rgba.data, (uint16_t)rgba.width,
                      (uint16_t)rgba.height, pep_rgba, pep_8bit);
    UnloadImage (rgba);
    if (p.bytes == NULL)
        return NULL;

    bytes_size = 0;
    bytes = pep_serialize (&p, &bytes_size);
    pep_free (&p);
    if (bytes == NULL || bytes_size == 0)
    {
        free (bytes);
        return NULL;
    }

    result = g_memdup2 (bytes, bytes_size);
    free (bytes);
    *out_size = bytes_size;
    return result;
}
