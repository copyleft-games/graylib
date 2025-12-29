/* grl-resource-chunk-info.h
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Resource chunk info for rres file format.
 */

#pragma once

#if !defined(GRAYLIB_INSIDE) && !defined(GRAYLIB_COMPILATION)
#error "Only <graylib.h> can be included directly."
#endif

#include <glib-object.h>
#include "../grl-version.h"
#include "grl-resource-enums.h"

G_BEGIN_DECLS

#define GRL_TYPE_RESOURCE_CHUNK_INFO (grl_resource_chunk_info_get_type ())

/**
 * GrlResourceChunkInfo:
 * @type: Resource chunk type as FourCC (4 bytes)
 * @id: Resource chunk identifier (CRC32 hash of filename)
 * @compression_type: Data compression algorithm
 * @cipher_type: Data encryption algorithm
 * @flags: Data flags
 * @packed_size: Data size after compression/encryption
 * @base_size: Original data size
 * @next_offset: Offset to next linked chunk (0 if none)
 * @crc32: CRC32 checksum of chunk data
 *
 * Information about a resource chunk in an rres file.
 *
 * This is a GBoxed type.
 */
typedef struct _GrlResourceChunkInfo GrlResourceChunkInfo;

struct _GrlResourceChunkInfo
{
    gchar   type[4];            /* Resource chunk type FourCC */
    guint32 id;                 /* Resource identifier */
    guint8  compression_type;   /* Compression algorithm */
    guint8  cipher_type;        /* Encryption algorithm */
    guint16 flags;              /* Data flags */
    guint32 packed_size;        /* Compressed/encrypted size */
    guint32 base_size;          /* Original uncompressed size */
    guint32 next_offset;        /* Offset to next linked chunk */
    guint32 crc32;              /* Data CRC32 checksum */
};

GRL_AVAILABLE_IN_ALL
GType                   grl_resource_chunk_info_get_type (void) G_GNUC_CONST;

/**
 * grl_resource_chunk_info_new:
 *
 * Creates a new empty #GrlResourceChunkInfo.
 *
 * Returns: (transfer full): A new #GrlResourceChunkInfo
 */
GRL_AVAILABLE_IN_ALL
GrlResourceChunkInfo *  grl_resource_chunk_info_new             (void);

/**
 * grl_resource_chunk_info_copy:
 * @self: A #GrlResourceChunkInfo
 *
 * Creates a copy of a resource chunk info.
 *
 * Returns: (transfer full): A copy of @self
 */
GRL_AVAILABLE_IN_ALL
GrlResourceChunkInfo *  grl_resource_chunk_info_copy            (const GrlResourceChunkInfo *self);

/**
 * grl_resource_chunk_info_free:
 * @self: A #GrlResourceChunkInfo
 *
 * Frees a resource chunk info.
 */
GRL_AVAILABLE_IN_ALL
void                    grl_resource_chunk_info_free            (GrlResourceChunkInfo *self);

/**
 * grl_resource_chunk_info_get_type_string:
 * @self: A #GrlResourceChunkInfo
 *
 * Gets the chunk type as a null-terminated string.
 *
 * Returns: (transfer full): The chunk type string (FourCC)
 */
GRL_AVAILABLE_IN_ALL
gchar *                 grl_resource_chunk_info_get_type_string (GrlResourceChunkInfo *self);

/**
 * grl_resource_chunk_info_get_data_type:
 * @self: A #GrlResourceChunkInfo
 *
 * Gets the resource data type from the FourCC code.
 *
 * Returns: The #GrlResourceDataType
 */
GRL_AVAILABLE_IN_ALL
GrlResourceDataType     grl_resource_chunk_info_get_data_type   (GrlResourceChunkInfo *self);

/**
 * grl_resource_chunk_info_get_compression:
 * @self: A #GrlResourceChunkInfo
 *
 * Gets the compression type.
 *
 * Returns: The #GrlResourceCompressionType
 */
GRL_AVAILABLE_IN_ALL
GrlResourceCompressionType grl_resource_chunk_info_get_compression (GrlResourceChunkInfo *self);

/**
 * grl_resource_chunk_info_get_cipher:
 * @self: A #GrlResourceChunkInfo
 *
 * Gets the cipher (encryption) type.
 *
 * Returns: The #GrlResourceCipherType
 */
GRL_AVAILABLE_IN_ALL
GrlResourceCipherType   grl_resource_chunk_info_get_cipher      (GrlResourceChunkInfo *self);

/**
 * grl_resource_chunk_info_has_next:
 * @self: A #GrlResourceChunkInfo
 *
 * Checks if there is another linked chunk.
 *
 * Returns: %TRUE if there is a linked chunk
 */
GRL_AVAILABLE_IN_ALL
gboolean                grl_resource_chunk_info_has_next        (GrlResourceChunkInfo *self);

/**
 * grl_resource_chunk_info_is_compressed:
 * @self: A #GrlResourceChunkInfo
 *
 * Checks if the chunk data is compressed.
 *
 * Returns: %TRUE if data is compressed
 */
GRL_AVAILABLE_IN_ALL
gboolean                grl_resource_chunk_info_is_compressed   (GrlResourceChunkInfo *self);

/**
 * grl_resource_chunk_info_is_encrypted:
 * @self: A #GrlResourceChunkInfo
 *
 * Checks if the chunk data is encrypted.
 *
 * Returns: %TRUE if data is encrypted
 */
GRL_AVAILABLE_IN_ALL
gboolean                grl_resource_chunk_info_is_encrypted    (GrlResourceChunkInfo *self);

G_DEFINE_AUTOPTR_CLEANUP_FUNC (GrlResourceChunkInfo, grl_resource_chunk_info_free)

G_END_DECLS
