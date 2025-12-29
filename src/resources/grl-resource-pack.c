/* grl-resource-pack.c
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#include "config.h"
#include "grl-resource-pack.h"

/*
 * rres library - header-only implementation
 * Note: rres requires C99 for bool type
 */
#define RRES_IMPLEMENTATION
#include <rres.h>

#include <string.h>

struct _GrlResourcePack
{
    GObject          parent_instance;

    gchar           *filename;
    guint16          version;
    guint16          chunk_count;
    guint32          cd_offset;
    gboolean         has_central_dir;

    /* Central directory (optional) */
    rresCentralDir   central_dir;

    /* Cipher password for encrypted resources */
    gchar           *password;
};

G_DEFINE_TYPE (GrlResourcePack, grl_resource_pack, G_TYPE_OBJECT)

enum
{
    PROP_0,
    PROP_FILENAME,
    PROP_VERSION,
    PROP_CHUNK_COUNT,
    N_PROPS
};

static GParamSpec *properties[N_PROPS];

G_DEFINE_QUARK (grl-resource-pack-error-quark, grl_resource_pack_error)

static void
grl_resource_pack_finalize (GObject *object)
{
    GrlResourcePack *self = GRL_RESOURCE_PACK (object);

    g_clear_pointer (&self->filename, g_free);
    g_clear_pointer (&self->password, g_free);

    if (self->has_central_dir)
        rresUnloadCentralDirectory (self->central_dir);

    G_OBJECT_CLASS (grl_resource_pack_parent_class)->finalize (object);
}

static void
grl_resource_pack_get_property (GObject    *object,
                                guint       prop_id,
                                GValue     *value,
                                GParamSpec *pspec)
{
    GrlResourcePack *self = GRL_RESOURCE_PACK (object);

    switch (prop_id)
    {
    case PROP_FILENAME:
        g_value_set_string (value, self->filename);
        break;
    case PROP_VERSION:
        g_value_set_uint (value, self->version);
        break;
    case PROP_CHUNK_COUNT:
        g_value_set_uint (value, self->chunk_count);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
grl_resource_pack_class_init (GrlResourcePackClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->finalize = grl_resource_pack_finalize;
    object_class->get_property = grl_resource_pack_get_property;

    /**
     * GrlResourcePack:filename:
     *
     * The filename of the resource pack.
     */
    properties[PROP_FILENAME] =
        g_param_spec_string ("filename",
                             "Filename",
                             "The resource pack filename",
                             NULL,
                             G_PARAM_READABLE |
                             G_PARAM_STATIC_STRINGS);

    /**
     * GrlResourcePack:version:
     *
     * The rres format version.
     */
    properties[PROP_VERSION] =
        g_param_spec_uint ("version",
                           "Version",
                           "The rres format version",
                           0, G_MAXUINT16, 0,
                           G_PARAM_READABLE |
                           G_PARAM_STATIC_STRINGS);

    /**
     * GrlResourcePack:chunk-count:
     *
     * The number of resource chunks in the pack.
     */
    properties[PROP_CHUNK_COUNT] =
        g_param_spec_uint ("chunk-count",
                           "Chunk Count",
                           "Number of resource chunks",
                           0, G_MAXUINT16, 0,
                           G_PARAM_READABLE |
                           G_PARAM_STATIC_STRINGS);

    g_object_class_install_properties (object_class, N_PROPS, properties);
}

static void
grl_resource_pack_init (GrlResourcePack *self)
{
    self->filename = NULL;
    self->password = NULL;
    self->has_central_dir = FALSE;
    self->central_dir.count = 0;
    self->central_dir.entries = NULL;
}

/*
 * Helper to read rres file header
 */
static gboolean
read_rres_header (const gchar  *filename,
                  guint16      *version,
                  guint16      *chunk_count,
                  guint32      *cd_offset,
                  GError      **error)
{
    FILE *file;
    rresFileHeader header;
    gsize read_count;

    file = fopen (filename, "rb");
    if (file == NULL)
    {
        g_set_error (error,
                     GRL_RESOURCE_PACK_ERROR,
                     GRL_RESOURCE_PACK_ERROR_FILE_NOT_FOUND,
                     "Could not open file: %s", filename);
        return FALSE;
    }

    read_count = fread (&header, sizeof (rresFileHeader), 1, file);
    fclose (file);

    if (read_count != 1)
    {
        g_set_error (error,
                     GRL_RESOURCE_PACK_ERROR,
                     GRL_RESOURCE_PACK_ERROR_INVALID_FORMAT,
                     "Could not read file header");
        return FALSE;
    }

    /* Verify rres signature */
    if (header.id[0] != 'r' || header.id[1] != 'r' ||
        header.id[2] != 'e' || header.id[3] != 's')
    {
        g_set_error (error,
                     GRL_RESOURCE_PACK_ERROR,
                     GRL_RESOURCE_PACK_ERROR_INVALID_FORMAT,
                     "Invalid rres file signature");
        return FALSE;
    }

    /* Verify version (100 = 1.0) */
    if (header.version != 100)
    {
        g_set_error (error,
                     GRL_RESOURCE_PACK_ERROR,
                     GRL_RESOURCE_PACK_ERROR_INVALID_FORMAT,
                     "Unsupported rres version: %d", header.version);
        return FALSE;
    }

    *version = header.version;
    *chunk_count = header.chunkCount;
    *cd_offset = header.cdOffset;

    return TRUE;
}

/**
 * grl_resource_pack_new:
 * @filename: (type filename): Path to the rres file
 * @error: (nullable): Return location for error
 *
 * Opens an rres resource pack file.
 *
 * Returns: (transfer full) (nullable): A new #GrlResourcePack, or %NULL on error
 */
GrlResourcePack *
grl_resource_pack_new (const gchar  *filename,
                       GError      **error)
{
    GrlResourcePack *self;
    guint16 version;
    guint16 chunk_count;
    guint32 cd_offset;

    g_return_val_if_fail (filename != NULL, NULL);

    /* Read and validate file header */
    if (!read_rres_header (filename, &version, &chunk_count, &cd_offset, error))
        return NULL;

    self = g_object_new (GRL_TYPE_RESOURCE_PACK, NULL);

    self->filename = g_strdup (filename);
    self->version = version;
    self->chunk_count = chunk_count;
    self->cd_offset = cd_offset;

    /* Try to load central directory if available */
    if (cd_offset > 0)
    {
        self->central_dir = rresLoadCentralDirectory (filename);
        self->has_central_dir = (self->central_dir.count > 0);
    }

    return self;
}

/**
 * grl_resource_pack_get_filename:
 * @self: A #GrlResourcePack
 *
 * Gets the filename of the resource pack.
 *
 * Returns: (transfer none): The filename
 */
const gchar *
grl_resource_pack_get_filename (GrlResourcePack *self)
{
    g_return_val_if_fail (GRL_IS_RESOURCE_PACK (self), NULL);

    return self->filename;
}

/**
 * grl_resource_pack_get_version:
 * @self: A #GrlResourcePack
 *
 * Gets the rres format version.
 *
 * Returns: The version number (e.g., 100 for version 1.0)
 */
guint16
grl_resource_pack_get_version (GrlResourcePack *self)
{
    g_return_val_if_fail (GRL_IS_RESOURCE_PACK (self), 0);

    return self->version;
}

/**
 * grl_resource_pack_get_chunk_count:
 * @self: A #GrlResourcePack
 *
 * Gets the number of resource chunks in the pack.
 *
 * Returns: The chunk count
 */
guint
grl_resource_pack_get_chunk_count (GrlResourcePack *self)
{
    g_return_val_if_fail (GRL_IS_RESOURCE_PACK (self), 0);

    return self->chunk_count;
}

/**
 * grl_resource_pack_has_central_directory:
 * @self: A #GrlResourcePack
 *
 * Checks if the pack has a central directory.
 *
 * Returns: %TRUE if central directory is available
 */
gboolean
grl_resource_pack_has_central_directory (GrlResourcePack *self)
{
    g_return_val_if_fail (GRL_IS_RESOURCE_PACK (self), FALSE);

    return self->has_central_dir;
}

/**
 * grl_resource_pack_get_entry_count:
 * @self: A #GrlResourcePack
 *
 * Gets the number of entries in the central directory.
 * Returns 0 if no central directory is available.
 *
 * Returns: The entry count
 */
guint
grl_resource_pack_get_entry_count (GrlResourcePack *self)
{
    g_return_val_if_fail (GRL_IS_RESOURCE_PACK (self), 0);

    if (!self->has_central_dir)
        return 0;

    return self->central_dir.count;
}

/**
 * grl_resource_pack_get_resource_id:
 * @self: A #GrlResourcePack
 * @filename: The filename to look up
 *
 * Gets the resource ID for a filename from the central directory.
 *
 * Returns: The resource ID, or 0 if not found
 */
guint32
grl_resource_pack_get_resource_id (GrlResourcePack *self,
                                   const gchar     *filename)
{
    int id;

    g_return_val_if_fail (GRL_IS_RESOURCE_PACK (self), 0);
    g_return_val_if_fail (filename != NULL, 0);

    if (!self->has_central_dir)
        return 0;

    id = rresGetResourceId (self->central_dir, filename);

    return (guint32)id;
}

/**
 * grl_resource_pack_get_entry_filename:
 * @self: A #GrlResourcePack
 * @index: The entry index
 *
 * Gets the filename of an entry in the central directory.
 *
 * Returns: (transfer full) (nullable): The filename, or %NULL if out of range
 */
gchar *
grl_resource_pack_get_entry_filename (GrlResourcePack *self,
                                      guint            index)
{
    g_return_val_if_fail (GRL_IS_RESOURCE_PACK (self), NULL);

    if (!self->has_central_dir || index >= self->central_dir.count)
        return NULL;

    return g_strdup (self->central_dir.entries[index].fileName);
}

/**
 * grl_resource_pack_get_entry_id:
 * @self: A #GrlResourcePack
 * @index: The entry index
 *
 * Gets the resource ID of an entry in the central directory.
 *
 * Returns: The resource ID, or 0 if out of range
 */
guint32
grl_resource_pack_get_entry_id (GrlResourcePack *self,
                                guint            index)
{
    g_return_val_if_fail (GRL_IS_RESOURCE_PACK (self), 0);

    if (!self->has_central_dir || index >= self->central_dir.count)
        return 0;

    return self->central_dir.entries[index].id;
}

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
GrlResourceChunkInfo *
grl_resource_pack_get_chunk_info (GrlResourcePack *self,
                                  guint32          resource_id,
                                  GError         **error)
{
    rresResourceChunkInfo rres_info;
    GrlResourceChunkInfo *info;

    g_return_val_if_fail (GRL_IS_RESOURCE_PACK (self), NULL);

    rres_info = rresLoadResourceChunkInfo (self->filename, resource_id);

    /* Check if resource was found */
    if (rres_info.id != resource_id)
    {
        g_set_error (error,
                     GRL_RESOURCE_PACK_ERROR,
                     GRL_RESOURCE_PACK_ERROR_RESOURCE_NOT_FOUND,
                     "Resource not found: 0x%08x", resource_id);
        return NULL;
    }

    info = grl_resource_chunk_info_new ();
    memcpy (info->type, rres_info.type, 4);
    info->id = rres_info.id;
    info->compression_type = rres_info.compType;
    info->cipher_type = rres_info.cipherType;
    info->flags = rres_info.flags;
    info->packed_size = rres_info.packedSize;
    info->base_size = rres_info.baseSize;
    info->next_offset = rres_info.nextOffset;
    info->crc32 = rres_info.crc32;

    return info;
}

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
GrlResourceChunkInfo **
grl_resource_pack_get_all_chunk_info (GrlResourcePack *self,
                                      guint           *count,
                                      GError         **error)
{
    rresResourceChunkInfo *rres_infos;
    GrlResourceChunkInfo **infos;
    unsigned int rres_count;
    guint i;

    g_return_val_if_fail (GRL_IS_RESOURCE_PACK (self), NULL);
    g_return_val_if_fail (count != NULL, NULL);

    rres_infos = rresLoadResourceChunkInfoAll (self->filename, &rres_count);

    if (rres_infos == NULL || rres_count == 0)
    {
        *count = 0;
        return NULL;
    }

    infos = g_new (GrlResourceChunkInfo *, rres_count);

    for (i = 0; i < rres_count; i++)
    {
        infos[i] = grl_resource_chunk_info_new ();
        memcpy (infos[i]->type, rres_infos[i].type, 4);
        infos[i]->id = rres_infos[i].id;
        infos[i]->compression_type = rres_infos[i].compType;
        infos[i]->cipher_type = rres_infos[i].cipherType;
        infos[i]->flags = rres_infos[i].flags;
        infos[i]->packed_size = rres_infos[i].packedSize;
        infos[i]->base_size = rres_infos[i].baseSize;
        infos[i]->next_offset = rres_infos[i].nextOffset;
        infos[i]->crc32 = rres_infos[i].crc32;
    }

    /* Free rres array */
    RRES_FREE (rres_infos);

    *count = rres_count;
    return infos;
}

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
guint8 *
grl_resource_pack_load_raw (GrlResourcePack *self,
                            guint32          resource_id,
                            gsize           *size,
                            GError         **error)
{
    rresResourceChunk chunk;
    guint8 *data;
    gsize data_size;

    g_return_val_if_fail (GRL_IS_RESOURCE_PACK (self), NULL);
    g_return_val_if_fail (size != NULL, NULL);

    /* Set password if available */
    if (self->password != NULL)
        rresSetCipherPassword (self->password);

    chunk = rresLoadResourceChunk (self->filename, resource_id);

    /* Check if resource was found */
    if (chunk.info.id != resource_id)
    {
        g_set_error (error,
                     GRL_RESOURCE_PACK_ERROR,
                     GRL_RESOURCE_PACK_ERROR_RESOURCE_NOT_FOUND,
                     "Resource not found: 0x%08x", resource_id);
        *size = 0;
        return NULL;
    }

    /* Check if data is valid */
    if (chunk.data.raw == NULL)
    {
        g_set_error (error,
                     GRL_RESOURCE_PACK_ERROR,
                     GRL_RESOURCE_PACK_ERROR_CORRUPTED_DATA,
                     "Resource data corrupted: 0x%08x", resource_id);
        rresUnloadResourceChunk (chunk);
        *size = 0;
        return NULL;
    }

    /* Calculate raw data size and copy */
    if (chunk.info.compType == RRES_COMP_NONE &&
        chunk.info.cipherType == RRES_CIPHER_NONE)
    {
        /* Uncompressed/unencrypted: size is baseSize minus props header */
        data_size = chunk.info.baseSize -
                    sizeof (unsigned int) -
                    (chunk.data.propCount * sizeof (unsigned int));
    }
    else
    {
        /* Compressed/encrypted: return packed data for user to process */
        data_size = chunk.info.packedSize;
    }

    data = g_memdup2 (chunk.data.raw, data_size);
    *size = data_size;

    rresUnloadResourceChunk (chunk);

    return data;
}

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
guint8 *
grl_resource_pack_load_raw_by_name (GrlResourcePack *self,
                                    const gchar     *filename,
                                    gsize           *size,
                                    GError         **error)
{
    guint32 resource_id;

    g_return_val_if_fail (GRL_IS_RESOURCE_PACK (self), NULL);
    g_return_val_if_fail (filename != NULL, NULL);
    g_return_val_if_fail (size != NULL, NULL);

    if (!self->has_central_dir)
    {
        g_set_error (error,
                     GRL_RESOURCE_PACK_ERROR,
                     GRL_RESOURCE_PACK_ERROR_NO_CENTRAL_DIR,
                     "No central directory available");
        *size = 0;
        return NULL;
    }

    resource_id = grl_resource_pack_get_resource_id (self, filename);

    if (resource_id == 0)
    {
        g_set_error (error,
                     GRL_RESOURCE_PACK_ERROR,
                     GRL_RESOURCE_PACK_ERROR_RESOURCE_NOT_FOUND,
                     "Resource not found: %s", filename);
        *size = 0;
        return NULL;
    }

    return grl_resource_pack_load_raw (self, resource_id, size, error);
}

/**
 * grl_resource_pack_set_cipher_password:
 * @self: A #GrlResourcePack
 * @password: (nullable): The password for encrypted resources
 *
 * Sets the password for decrypting encrypted resources.
 * The password is stored internally and used for subsequent load operations.
 */
void
grl_resource_pack_set_cipher_password (GrlResourcePack *self,
                                       const gchar     *password)
{
    g_return_if_fail (GRL_IS_RESOURCE_PACK (self));

    g_clear_pointer (&self->password, g_free);

    if (password != NULL)
        self->password = g_strdup (password);
}

/**
 * grl_resource_pack_compute_crc32:
 * @data: (array length=size): Data to compute CRC32 for
 * @size: Size of data in bytes
 *
 * Computes CRC32 checksum for data.
 *
 * Returns: The CRC32 checksum
 */
guint32
grl_resource_pack_compute_crc32 (const guint8 *data,
                                 gsize         size)
{
    g_return_val_if_fail (data != NULL, 0);

    return rresComputeCRC32 (data, (int)size);
}
