/* grl-resource-chunk-info.c
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#include "config.h"
#include "grl-resource-chunk-info.h"
#include <string.h>

G_DEFINE_BOXED_TYPE (GrlResourceChunkInfo, grl_resource_chunk_info,
                     grl_resource_chunk_info_copy, grl_resource_chunk_info_free)

/**
 * grl_resource_chunk_info_new:
 *
 * Creates a new empty #GrlResourceChunkInfo.
 *
 * Returns: (transfer full): A new #GrlResourceChunkInfo
 */
GrlResourceChunkInfo *
grl_resource_chunk_info_new (void)
{
    GrlResourceChunkInfo *self;

    self = g_new0 (GrlResourceChunkInfo, 1);

    return self;
}

/**
 * grl_resource_chunk_info_copy:
 * @self: A #GrlResourceChunkInfo
 *
 * Creates a copy of a resource chunk info.
 *
 * Returns: (transfer full): A copy of @self
 */
GrlResourceChunkInfo *
grl_resource_chunk_info_copy (const GrlResourceChunkInfo *self)
{
    GrlResourceChunkInfo *copy;

    g_return_val_if_fail (self != NULL, NULL);

    copy = g_new (GrlResourceChunkInfo, 1);
    memcpy (copy, self, sizeof (GrlResourceChunkInfo));

    return copy;
}

/**
 * grl_resource_chunk_info_free:
 * @self: A #GrlResourceChunkInfo
 *
 * Frees a resource chunk info.
 */
void
grl_resource_chunk_info_free (GrlResourceChunkInfo *self)
{
    if (self == NULL)
        return;

    g_free (self);
}

/**
 * grl_resource_chunk_info_get_type_string:
 * @self: A #GrlResourceChunkInfo
 *
 * Gets the chunk type as a null-terminated string.
 *
 * Returns: (transfer full): The chunk type string (FourCC)
 */
gchar *
grl_resource_chunk_info_get_type_string (GrlResourceChunkInfo *self)
{
    gchar *type_str;

    g_return_val_if_fail (self != NULL, NULL);

    type_str = g_new (gchar, 5);
    memcpy (type_str, self->type, 4);
    type_str[4] = '\0';

    return type_str;
}

/**
 * grl_resource_chunk_info_get_data_type:
 * @self: A #GrlResourceChunkInfo
 *
 * Gets the resource data type from the FourCC code.
 *
 * Returns: The #GrlResourceDataType
 */
GrlResourceDataType
grl_resource_chunk_info_get_data_type (GrlResourceChunkInfo *self)
{
    g_return_val_if_fail (self != NULL, GRL_RESOURCE_DATA_NULL);

    if (memcmp (self->type, "NULL", 4) == 0)
        return GRL_RESOURCE_DATA_NULL;
    else if (memcmp (self->type, "RAWD", 4) == 0)
        return GRL_RESOURCE_DATA_RAW;
    else if (memcmp (self->type, "TEXT", 4) == 0)
        return GRL_RESOURCE_DATA_TEXT;
    else if (memcmp (self->type, "IMGE", 4) == 0)
        return GRL_RESOURCE_DATA_IMAGE;
    else if (memcmp (self->type, "WAVE", 4) == 0)
        return GRL_RESOURCE_DATA_WAVE;
    else if (memcmp (self->type, "VRTX", 4) == 0)
        return GRL_RESOURCE_DATA_VERTEX;
    else if (memcmp (self->type, "FNTG", 4) == 0)
        return GRL_RESOURCE_DATA_FONT_GLYPHS;
    else if (memcmp (self->type, "LINK", 4) == 0)
        return GRL_RESOURCE_DATA_LINK;
    else if (memcmp (self->type, "CDIR", 4) == 0)
        return GRL_RESOURCE_DATA_DIRECTORY;

    return GRL_RESOURCE_DATA_NULL;
}

/**
 * grl_resource_chunk_info_get_compression:
 * @self: A #GrlResourceChunkInfo
 *
 * Gets the compression type.
 *
 * Returns: The #GrlResourceCompressionType
 */
GrlResourceCompressionType
grl_resource_chunk_info_get_compression (GrlResourceChunkInfo *self)
{
    g_return_val_if_fail (self != NULL, GRL_RESOURCE_COMP_NONE);

    return (GrlResourceCompressionType)self->compression_type;
}

/**
 * grl_resource_chunk_info_get_cipher:
 * @self: A #GrlResourceChunkInfo
 *
 * Gets the cipher (encryption) type.
 *
 * Returns: The #GrlResourceCipherType
 */
GrlResourceCipherType
grl_resource_chunk_info_get_cipher (GrlResourceChunkInfo *self)
{
    g_return_val_if_fail (self != NULL, GRL_RESOURCE_CIPHER_NONE);

    return (GrlResourceCipherType)self->cipher_type;
}

/**
 * grl_resource_chunk_info_has_next:
 * @self: A #GrlResourceChunkInfo
 *
 * Checks if there is another linked chunk.
 *
 * Returns: %TRUE if there is a linked chunk
 */
gboolean
grl_resource_chunk_info_has_next (GrlResourceChunkInfo *self)
{
    g_return_val_if_fail (self != NULL, FALSE);

    return self->next_offset > 0;
}

/**
 * grl_resource_chunk_info_is_compressed:
 * @self: A #GrlResourceChunkInfo
 *
 * Checks if the chunk data is compressed.
 *
 * Returns: %TRUE if data is compressed
 */
gboolean
grl_resource_chunk_info_is_compressed (GrlResourceChunkInfo *self)
{
    g_return_val_if_fail (self != NULL, FALSE);

    return self->compression_type != GRL_RESOURCE_COMP_NONE;
}

/**
 * grl_resource_chunk_info_is_encrypted:
 * @self: A #GrlResourceChunkInfo
 *
 * Checks if the chunk data is encrypted.
 *
 * Returns: %TRUE if data is encrypted
 */
gboolean
grl_resource_chunk_info_is_encrypted (GrlResourceChunkInfo *self)
{
    g_return_val_if_fail (self != NULL, FALSE);

    return self->cipher_type != GRL_RESOURCE_CIPHER_NONE;
}
