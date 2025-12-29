/* test-resource-pack.c
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Unit tests for GrlResourceChunkInfo and GrlResourcePack.
 *
 * Note: Tests that require file I/O are skipped (no test assets).
 * These tests focus on the GrlResourceChunkInfo GBoxed type and
 * enum registrations. GrlResourcePack tests require rres files.
 */

#include <glib.h>
#include <string.h>
#include "src/resources/grl-resource-pack.h"
#include "src/resources/grl-resource-chunk-info.h"
#include "src/resources/grl-resource-enums.h"

/*
 * =============================================================================
 * GrlResourceChunkInfo Tests
 * =============================================================================
 */

static void
test_resource_chunk_info_type (void)
{
    GType type = grl_resource_chunk_info_get_type ();

    g_assert_true (type != G_TYPE_INVALID);
    g_assert_cmpstr (g_type_name (type), ==, "GrlResourceChunkInfo");
}

static void
test_resource_chunk_info_new (void)
{
    g_autoptr(GrlResourceChunkInfo) info = NULL;

    info = grl_resource_chunk_info_new ();

    g_assert_nonnull (info);
}

static void
test_resource_chunk_info_copy (void)
{
    g_autoptr(GrlResourceChunkInfo) info1 = NULL;
    g_autoptr(GrlResourceChunkInfo) info2 = NULL;

    info1 = grl_resource_chunk_info_new ();
    info1->id = 12345;
    info1->packed_size = 1024;
    info1->base_size = 2048;
    info1->compression_type = GRL_RESOURCE_COMP_DEFLATE;
    info1->cipher_type = GRL_RESOURCE_CIPHER_NONE;

    info2 = grl_resource_chunk_info_copy (info1);

    g_assert_nonnull (info2);
    g_assert_true (info1 != info2);
    g_assert_cmpuint (info1->id, ==, info2->id);
    g_assert_cmpuint (info1->packed_size, ==, info2->packed_size);
    g_assert_cmpuint (info1->base_size, ==, info2->base_size);
}

static void
test_resource_chunk_info_free_null (void)
{
    /* Should not crash when passed NULL */
    grl_resource_chunk_info_free (NULL);
}

static void
test_resource_chunk_info_get_type_string (void)
{
    g_autoptr(GrlResourceChunkInfo) info = NULL;
    g_autofree gchar *type_string = NULL;

    info = grl_resource_chunk_info_new ();
    info->type[0] = 'I';
    info->type[1] = 'M';
    info->type[2] = 'G';
    info->type[3] = 'D';

    type_string = grl_resource_chunk_info_get_type_string (info);

    g_assert_nonnull (type_string);
    g_assert_cmpstr (type_string, ==, "IMGD");
}

static void
test_resource_chunk_info_get_data_type (void)
{
    g_autoptr(GrlResourceChunkInfo) info = NULL;
    GrlResourceDataType data_type;

    info = grl_resource_chunk_info_new ();
    /* Default newly-created chunk info should return a valid data type */
    data_type = grl_resource_chunk_info_get_data_type (info);

    /* Just verify the function is callable and returns a valid enum value */
    g_assert_true (data_type >= GRL_RESOURCE_DATA_NULL);
}

static void
test_resource_chunk_info_get_compression_none (void)
{
    g_autoptr(GrlResourceChunkInfo) info = NULL;
    GrlResourceCompressionType comp;

    info = grl_resource_chunk_info_new ();
    info->compression_type = GRL_RESOURCE_COMP_NONE;

    comp = grl_resource_chunk_info_get_compression (info);

    g_assert_cmpint (comp, ==, GRL_RESOURCE_COMP_NONE);
}

static void
test_resource_chunk_info_get_compression_deflate (void)
{
    g_autoptr(GrlResourceChunkInfo) info = NULL;
    GrlResourceCompressionType comp;

    info = grl_resource_chunk_info_new ();
    info->compression_type = GRL_RESOURCE_COMP_DEFLATE;

    comp = grl_resource_chunk_info_get_compression (info);

    g_assert_cmpint (comp, ==, GRL_RESOURCE_COMP_DEFLATE);
}

static void
test_resource_chunk_info_get_compression_lz4 (void)
{
    g_autoptr(GrlResourceChunkInfo) info = NULL;
    GrlResourceCompressionType comp;

    info = grl_resource_chunk_info_new ();
    info->compression_type = GRL_RESOURCE_COMP_LZ4;

    comp = grl_resource_chunk_info_get_compression (info);

    g_assert_cmpint (comp, ==, GRL_RESOURCE_COMP_LZ4);
}

static void
test_resource_chunk_info_get_cipher_none (void)
{
    g_autoptr(GrlResourceChunkInfo) info = NULL;
    GrlResourceCipherType cipher;

    info = grl_resource_chunk_info_new ();
    info->cipher_type = GRL_RESOURCE_CIPHER_NONE;

    cipher = grl_resource_chunk_info_get_cipher (info);

    g_assert_cmpint (cipher, ==, GRL_RESOURCE_CIPHER_NONE);
}

static void
test_resource_chunk_info_get_cipher_aes (void)
{
    g_autoptr(GrlResourceChunkInfo) info = NULL;
    GrlResourceCipherType cipher;

    info = grl_resource_chunk_info_new ();
    info->cipher_type = GRL_RESOURCE_CIPHER_AES;

    cipher = grl_resource_chunk_info_get_cipher (info);

    g_assert_cmpint (cipher, ==, GRL_RESOURCE_CIPHER_AES);
}

static void
test_resource_chunk_info_has_next_false (void)
{
    g_autoptr(GrlResourceChunkInfo) info = NULL;

    info = grl_resource_chunk_info_new ();
    info->next_offset = 0;

    g_assert_false (grl_resource_chunk_info_has_next (info));
}

static void
test_resource_chunk_info_has_next_true (void)
{
    g_autoptr(GrlResourceChunkInfo) info = NULL;

    info = grl_resource_chunk_info_new ();
    info->next_offset = 1024;

    g_assert_true (grl_resource_chunk_info_has_next (info));
}

static void
test_resource_chunk_info_is_compressed_false (void)
{
    g_autoptr(GrlResourceChunkInfo) info = NULL;

    info = grl_resource_chunk_info_new ();
    info->compression_type = GRL_RESOURCE_COMP_NONE;

    g_assert_false (grl_resource_chunk_info_is_compressed (info));
}

static void
test_resource_chunk_info_is_compressed_true (void)
{
    g_autoptr(GrlResourceChunkInfo) info = NULL;

    info = grl_resource_chunk_info_new ();
    info->compression_type = GRL_RESOURCE_COMP_DEFLATE;

    g_assert_true (grl_resource_chunk_info_is_compressed (info));
}

static void
test_resource_chunk_info_is_encrypted_false (void)
{
    g_autoptr(GrlResourceChunkInfo) info = NULL;

    info = grl_resource_chunk_info_new ();
    info->cipher_type = GRL_RESOURCE_CIPHER_NONE;

    g_assert_false (grl_resource_chunk_info_is_encrypted (info));
}

static void
test_resource_chunk_info_is_encrypted_true (void)
{
    g_autoptr(GrlResourceChunkInfo) info = NULL;

    info = grl_resource_chunk_info_new ();
    info->cipher_type = GRL_RESOURCE_CIPHER_XCHACHA20;

    g_assert_true (grl_resource_chunk_info_is_encrypted (info));
}

/*
 * =============================================================================
 * Enum GType Tests
 * =============================================================================
 */

static void
test_resource_data_type_enum (void)
{
    GType type = grl_resource_data_type_get_type ();

    g_assert_true (type != G_TYPE_INVALID);
    g_assert_cmpstr (g_type_name (type), ==, "GrlResourceDataType");
}

static void
test_resource_compression_type_enum (void)
{
    GType type = grl_resource_compression_type_get_type ();

    g_assert_true (type != G_TYPE_INVALID);
    g_assert_cmpstr (g_type_name (type), ==, "GrlResourceCompressionType");
}

static void
test_resource_cipher_type_enum (void)
{
    GType type = grl_resource_cipher_type_get_type ();

    g_assert_true (type != G_TYPE_INVALID);
    g_assert_cmpstr (g_type_name (type), ==, "GrlResourceCipherType");
}

/*
 * =============================================================================
 * GrlResourcePack GType Test
 * =============================================================================
 */

static void
test_resource_pack_type (void)
{
    GType type = grl_resource_pack_get_type ();

    g_assert_true (type != G_TYPE_INVALID);
    g_assert_cmpstr (g_type_name (type), ==, "GrlResourcePack");
}

static void
test_resource_pack_type_is_object (void)
{
    GType type = grl_resource_pack_get_type ();

    g_assert_true (g_type_is_a (type, G_TYPE_OBJECT));
}

static void
test_resource_pack_type_not_abstract (void)
{
    GType type = grl_resource_pack_get_type ();

    /* GrlResourcePack can be instantiated (not abstract) */
    g_assert_false (G_TYPE_IS_ABSTRACT (type));
}

static void
test_resource_pack_error_quark (void)
{
    GQuark quark = grl_resource_pack_error_quark ();

    g_assert_true (quark != 0);
}

/*
 * =============================================================================
 * CRC32 Utility Test
 * =============================================================================
 */

static void
test_resource_pack_compute_crc32 (void)
{
    const guint8 data[] = { 0x48, 0x65, 0x6C, 0x6C, 0x6F }; /* "Hello" */
    guint32 crc;

    crc = grl_resource_pack_compute_crc32 (data, 5);

    /* CRC32 should be non-zero for non-empty data */
    g_assert_cmpuint (crc, !=, 0);
}

static void
test_resource_pack_compute_crc32_same_data (void)
{
    const guint8 data1[] = { 0x01, 0x02, 0x03, 0x04 };
    const guint8 data2[] = { 0x01, 0x02, 0x03, 0x04 };
    guint32 crc1, crc2;

    crc1 = grl_resource_pack_compute_crc32 (data1, 4);
    crc2 = grl_resource_pack_compute_crc32 (data2, 4);

    g_assert_cmpuint (crc1, ==, crc2);
}

static void
test_resource_pack_compute_crc32_different_data (void)
{
    const guint8 data1[] = { 0x01, 0x02, 0x03, 0x04 };
    const guint8 data2[] = { 0x04, 0x03, 0x02, 0x01 };
    guint32 crc1, crc2;

    crc1 = grl_resource_pack_compute_crc32 (data1, 4);
    crc2 = grl_resource_pack_compute_crc32 (data2, 4);

    g_assert_cmpuint (crc1, !=, crc2);
}

/*
 * Test registration
 */

int
main (int   argc,
      char *argv[])
{
    g_test_init (&argc, &argv, NULL);

    /* GrlResourceChunkInfo */
    g_test_add_func ("/resource-chunk-info/type", test_resource_chunk_info_type);
    g_test_add_func ("/resource-chunk-info/new", test_resource_chunk_info_new);
    g_test_add_func ("/resource-chunk-info/copy", test_resource_chunk_info_copy);
    g_test_add_func ("/resource-chunk-info/free-null", test_resource_chunk_info_free_null);
    g_test_add_func ("/resource-chunk-info/get-type-string", test_resource_chunk_info_get_type_string);
    g_test_add_func ("/resource-chunk-info/get-data-type", test_resource_chunk_info_get_data_type);
    g_test_add_func ("/resource-chunk-info/get-compression-none", test_resource_chunk_info_get_compression_none);
    g_test_add_func ("/resource-chunk-info/get-compression-deflate", test_resource_chunk_info_get_compression_deflate);
    g_test_add_func ("/resource-chunk-info/get-compression-lz4", test_resource_chunk_info_get_compression_lz4);
    g_test_add_func ("/resource-chunk-info/get-cipher-none", test_resource_chunk_info_get_cipher_none);
    g_test_add_func ("/resource-chunk-info/get-cipher-aes", test_resource_chunk_info_get_cipher_aes);
    g_test_add_func ("/resource-chunk-info/has-next-false", test_resource_chunk_info_has_next_false);
    g_test_add_func ("/resource-chunk-info/has-next-true", test_resource_chunk_info_has_next_true);
    g_test_add_func ("/resource-chunk-info/is-compressed-false", test_resource_chunk_info_is_compressed_false);
    g_test_add_func ("/resource-chunk-info/is-compressed-true", test_resource_chunk_info_is_compressed_true);
    g_test_add_func ("/resource-chunk-info/is-encrypted-false", test_resource_chunk_info_is_encrypted_false);
    g_test_add_func ("/resource-chunk-info/is-encrypted-true", test_resource_chunk_info_is_encrypted_true);

    /* Enum types */
    g_test_add_func ("/resource-enums/data-type", test_resource_data_type_enum);
    g_test_add_func ("/resource-enums/compression-type", test_resource_compression_type_enum);
    g_test_add_func ("/resource-enums/cipher-type", test_resource_cipher_type_enum);

    /* GrlResourcePack GType */
    g_test_add_func ("/resource-pack/type", test_resource_pack_type);
    g_test_add_func ("/resource-pack/type-is-object", test_resource_pack_type_is_object);
    g_test_add_func ("/resource-pack/type-not-abstract", test_resource_pack_type_not_abstract);
    g_test_add_func ("/resource-pack/error-quark", test_resource_pack_error_quark);

    /* CRC32 utility */
    g_test_add_func ("/resource-pack/compute-crc32", test_resource_pack_compute_crc32);
    g_test_add_func ("/resource-pack/compute-crc32-same-data", test_resource_pack_compute_crc32_same_data);
    g_test_add_func ("/resource-pack/compute-crc32-different-data", test_resource_pack_compute_crc32_different_data);

    return g_test_run ();
}
