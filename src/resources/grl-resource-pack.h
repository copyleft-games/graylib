/* grl-resource-pack.h
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Resource pack for loading rres files.
 */

#pragma once

#if !defined(GRAYLIB_INSIDE) && !defined(GRAYLIB_COMPILATION)
#error "Only <graylib.h> can be included directly."
#endif

#include <glib-object.h>
#include "../grl-version.h"
#include "grl-resource-enums.h"
#include "grl-resource-chunk-info.h"

G_BEGIN_DECLS

#define GRL_TYPE_RESOURCE_PACK (grl_resource_pack_get_type ())
#define GRL_RESOURCE_PACK_ERROR (grl_resource_pack_error_quark ())

GRL_AVAILABLE_IN_ALL
G_DECLARE_FINAL_TYPE (GrlResourcePack, grl_resource_pack, GRL, RESOURCE_PACK, GObject)

/**
 * GrlResourcePackError:
 * @GRL_RESOURCE_PACK_ERROR_FILE_NOT_FOUND: File could not be opened
 * @GRL_RESOURCE_PACK_ERROR_INVALID_FORMAT: Invalid rres file format
 * @GRL_RESOURCE_PACK_ERROR_NO_CENTRAL_DIR: No central directory in file
 * @GRL_RESOURCE_PACK_ERROR_RESOURCE_NOT_FOUND: Resource ID not found
 * @GRL_RESOURCE_PACK_ERROR_CORRUPTED_DATA: Data CRC32 mismatch
 *
 * Error codes for #GrlResourcePack operations.
 */
typedef enum
{
    GRL_RESOURCE_PACK_ERROR_FILE_NOT_FOUND,
    GRL_RESOURCE_PACK_ERROR_INVALID_FORMAT,
    GRL_RESOURCE_PACK_ERROR_NO_CENTRAL_DIR,
    GRL_RESOURCE_PACK_ERROR_RESOURCE_NOT_FOUND,
    GRL_RESOURCE_PACK_ERROR_CORRUPTED_DATA
} GrlResourcePackError;

GRL_AVAILABLE_IN_ALL
GQuark grl_resource_pack_error_quark (void);

/*
 * Constructors
 */

/**
 * grl_resource_pack_new:
 * @filename: (type filename): Path to the rres file
 * @error: (nullable): Return location for error
 *
 * Opens an rres resource pack file.
 *
 * Returns: (transfer full) (nullable): A new #GrlResourcePack, or %NULL on error
 */
GRL_AVAILABLE_IN_ALL
GrlResourcePack *       grl_resource_pack_new                   (const gchar     *filename,
                                                                 GError         **error);

/*
 * File Info
 */

/**
 * grl_resource_pack_get_filename:
 * @self: A #GrlResourcePack
 *
 * Gets the filename of the resource pack.
 *
 * Returns: (transfer none): The filename
 */
GRL_AVAILABLE_IN_ALL
const gchar *           grl_resource_pack_get_filename          (GrlResourcePack *self);

/**
 * grl_resource_pack_get_version:
 * @self: A #GrlResourcePack
 *
 * Gets the rres format version.
 *
 * Returns: The version number (e.g., 100 for version 1.0)
 */
GRL_AVAILABLE_IN_ALL
guint16                 grl_resource_pack_get_version           (GrlResourcePack *self);

/**
 * grl_resource_pack_get_chunk_count:
 * @self: A #GrlResourcePack
 *
 * Gets the number of resource chunks in the pack.
 *
 * Returns: The chunk count
 */
GRL_AVAILABLE_IN_ALL
guint                   grl_resource_pack_get_chunk_count       (GrlResourcePack *self);

/**
 * grl_resource_pack_has_central_directory:
 * @self: A #GrlResourcePack
 *
 * Checks if the pack has a central directory.
 *
 * Returns: %TRUE if central directory is available
 */
GRL_AVAILABLE_IN_ALL
gboolean                grl_resource_pack_has_central_directory (GrlResourcePack *self);

/*
 * Central Directory Operations
 */

/**
 * grl_resource_pack_get_entry_count:
 * @self: A #GrlResourcePack
 *
 * Gets the number of entries in the central directory.
 * Returns 0 if no central directory is available.
 *
 * Returns: The entry count
 */
GRL_AVAILABLE_IN_ALL
guint                   grl_resource_pack_get_entry_count       (GrlResourcePack *self);

/**
 * grl_resource_pack_get_resource_id:
 * @self: A #GrlResourcePack
 * @filename: The filename to look up
 *
 * Gets the resource ID for a filename from the central directory.
 *
 * Returns: The resource ID, or 0 if not found
 */
GRL_AVAILABLE_IN_ALL
guint32                 grl_resource_pack_get_resource_id       (GrlResourcePack *self,
                                                                 const gchar     *filename);

/**
 * grl_resource_pack_get_entry_filename:
 * @self: A #GrlResourcePack
 * @index: The entry index
 *
 * Gets the filename of an entry in the central directory.
 *
 * Returns: (transfer full) (nullable): The filename, or %NULL if out of range
 */
GRL_AVAILABLE_IN_ALL
gchar *                 grl_resource_pack_get_entry_filename    (GrlResourcePack *self,
                                                                 guint            index);

/**
 * grl_resource_pack_get_entry_id:
 * @self: A #GrlResourcePack
 * @index: The entry index
 *
 * Gets the resource ID of an entry in the central directory.
 *
 * Returns: The resource ID, or 0 if out of range
 */
GRL_AVAILABLE_IN_ALL
guint32                 grl_resource_pack_get_entry_id          (GrlResourcePack *self,
                                                                 guint            index);

/*
 * Chunk Info Operations
 */

/**
 * grl_resource_pack_get_chunk_info:
 * @self: A #GrlResourcePack
 * @resource_id: The resource ID
 * @error: (nullable): Return location for error
 *
 * Gets info about a resource chunk by ID.
 *
 * Returns: (transfer full) (nullable): The chunk info, or %NULL on error
 */
GRL_AVAILABLE_IN_ALL
GrlResourceChunkInfo *  grl_resource_pack_get_chunk_info        (GrlResourcePack *self,
                                                                 guint32          resource_id,
                                                                 GError         **error);

/**
 * grl_resource_pack_get_all_chunk_info:
 * @self: A #GrlResourcePack
 * @count: (out): Return location for chunk count
 * @error: (nullable): Return location for error
 *
 * Gets info about all resource chunks in the pack.
 *
 * Returns: (transfer full) (array length=count) (nullable): Array of chunk infos
 */
GRL_AVAILABLE_IN_ALL
GrlResourceChunkInfo ** grl_resource_pack_get_all_chunk_info    (GrlResourcePack *self,
                                                                 guint           *count,
                                                                 GError         **error);

/*
 * Raw Data Loading
 */

/**
 * grl_resource_pack_load_raw:
 * @self: A #GrlResourcePack
 * @resource_id: The resource ID
 * @size: (out): Return location for data size
 * @error: (nullable): Return location for error
 *
 * Loads raw resource data by ID. The data may be compressed or encrypted;
 * use chunk info to determine processing needed.
 *
 * Returns: (transfer full) (array length=size) (nullable): The raw data bytes
 */
GRL_AVAILABLE_IN_ALL
guint8 *                grl_resource_pack_load_raw              (GrlResourcePack *self,
                                                                 guint32          resource_id,
                                                                 gsize           *size,
                                                                 GError         **error);

/**
 * grl_resource_pack_load_raw_by_name:
 * @self: A #GrlResourcePack
 * @filename: The filename to load
 * @size: (out): Return location for data size
 * @error: (nullable): Return location for error
 *
 * Loads raw resource data by filename (using central directory).
 *
 * Returns: (transfer full) (array length=size) (nullable): The raw data bytes
 */
GRL_AVAILABLE_IN_ALL
guint8 *                grl_resource_pack_load_raw_by_name      (GrlResourcePack *self,
                                                                 const gchar     *filename,
                                                                 gsize           *size,
                                                                 GError         **error);

/*
 * Encryption Support
 */

/**
 * grl_resource_pack_set_cipher_password:
 * @self: A #GrlResourcePack
 * @password: (nullable): The password for encrypted resources
 *
 * Sets the password for decrypting encrypted resources.
 * The password is stored internally and used for subsequent load operations.
 */
GRL_AVAILABLE_IN_ALL
void                    grl_resource_pack_set_cipher_password   (GrlResourcePack *self,
                                                                 const gchar     *password);

/*
 * Utility Functions
 */

/**
 * grl_resource_pack_compute_crc32:
 * @data: (array length=size): Data to compute CRC32 for
 * @size: Size of data in bytes
 *
 * Computes CRC32 checksum for data.
 *
 * Returns: The CRC32 checksum
 */
GRL_AVAILABLE_IN_ALL
guint32                 grl_resource_pack_compute_crc32         (const guint8    *data,
                                                                 gsize            size);

G_END_DECLS
