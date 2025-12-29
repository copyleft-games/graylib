/* grl-resource-enums.c
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#include "config.h"
#include "grl-resource-enums.h"

/*
 * GrlResourceDataType
 */
GType
grl_resource_data_type_get_type (void)
{
    static gsize g_define_type_id__volatile = 0;

    if (g_once_init_enter (&g_define_type_id__volatile))
    {
        static const GEnumValue values[] = {
            { GRL_RESOURCE_DATA_NULL, "GRL_RESOURCE_DATA_NULL", "null" },
            { GRL_RESOURCE_DATA_RAW, "GRL_RESOURCE_DATA_RAW", "raw" },
            { GRL_RESOURCE_DATA_TEXT, "GRL_RESOURCE_DATA_TEXT", "text" },
            { GRL_RESOURCE_DATA_IMAGE, "GRL_RESOURCE_DATA_IMAGE", "image" },
            { GRL_RESOURCE_DATA_WAVE, "GRL_RESOURCE_DATA_WAVE", "wave" },
            { GRL_RESOURCE_DATA_VERTEX, "GRL_RESOURCE_DATA_VERTEX", "vertex" },
            { GRL_RESOURCE_DATA_FONT_GLYPHS, "GRL_RESOURCE_DATA_FONT_GLYPHS", "font-glyphs" },
            { GRL_RESOURCE_DATA_LINK, "GRL_RESOURCE_DATA_LINK", "link" },
            { GRL_RESOURCE_DATA_DIRECTORY, "GRL_RESOURCE_DATA_DIRECTORY", "directory" },
            { 0, NULL, NULL }
        };

        GType g_define_type_id =
            g_enum_register_static (g_intern_static_string ("GrlResourceDataType"), values);

        g_once_init_leave (&g_define_type_id__volatile, g_define_type_id);
    }

    return g_define_type_id__volatile;
}

/*
 * GrlResourceCompressionType
 */
GType
grl_resource_compression_type_get_type (void)
{
    static gsize g_define_type_id__volatile = 0;

    if (g_once_init_enter (&g_define_type_id__volatile))
    {
        static const GEnumValue values[] = {
            { GRL_RESOURCE_COMP_NONE, "GRL_RESOURCE_COMP_NONE", "none" },
            { GRL_RESOURCE_COMP_RLE, "GRL_RESOURCE_COMP_RLE", "rle" },
            { GRL_RESOURCE_COMP_DEFLATE, "GRL_RESOURCE_COMP_DEFLATE", "deflate" },
            { GRL_RESOURCE_COMP_LZ4, "GRL_RESOURCE_COMP_LZ4", "lz4" },
            { GRL_RESOURCE_COMP_LZMA2, "GRL_RESOURCE_COMP_LZMA2", "lzma2" },
            { GRL_RESOURCE_COMP_QOI, "GRL_RESOURCE_COMP_QOI", "qoi" },
            { 0, NULL, NULL }
        };

        GType g_define_type_id =
            g_enum_register_static (g_intern_static_string ("GrlResourceCompressionType"), values);

        g_once_init_leave (&g_define_type_id__volatile, g_define_type_id);
    }

    return g_define_type_id__volatile;
}

/*
 * GrlResourceCipherType
 */
GType
grl_resource_cipher_type_get_type (void)
{
    static gsize g_define_type_id__volatile = 0;

    if (g_once_init_enter (&g_define_type_id__volatile))
    {
        static const GEnumValue values[] = {
            { GRL_RESOURCE_CIPHER_NONE, "GRL_RESOURCE_CIPHER_NONE", "none" },
            { GRL_RESOURCE_CIPHER_XOR, "GRL_RESOURCE_CIPHER_XOR", "xor" },
            { GRL_RESOURCE_CIPHER_DES, "GRL_RESOURCE_CIPHER_DES", "des" },
            { GRL_RESOURCE_CIPHER_TDES, "GRL_RESOURCE_CIPHER_TDES", "tdes" },
            { GRL_RESOURCE_CIPHER_IDEA, "GRL_RESOURCE_CIPHER_IDEA", "idea" },
            { GRL_RESOURCE_CIPHER_AES, "GRL_RESOURCE_CIPHER_AES", "aes" },
            { GRL_RESOURCE_CIPHER_AES_GCM, "GRL_RESOURCE_CIPHER_AES_GCM", "aes-gcm" },
            { GRL_RESOURCE_CIPHER_XTEA, "GRL_RESOURCE_CIPHER_XTEA", "xtea" },
            { GRL_RESOURCE_CIPHER_BLOWFISH, "GRL_RESOURCE_CIPHER_BLOWFISH", "blowfish" },
            { GRL_RESOURCE_CIPHER_RSA, "GRL_RESOURCE_CIPHER_RSA", "rsa" },
            { GRL_RESOURCE_CIPHER_SALSA20, "GRL_RESOURCE_CIPHER_SALSA20", "salsa20" },
            { GRL_RESOURCE_CIPHER_CHACHA20, "GRL_RESOURCE_CIPHER_CHACHA20", "chacha20" },
            { GRL_RESOURCE_CIPHER_XCHACHA20, "GRL_RESOURCE_CIPHER_XCHACHA20", "xchacha20" },
            { GRL_RESOURCE_CIPHER_XCHACHA20_POLY1305, "GRL_RESOURCE_CIPHER_XCHACHA20_POLY1305", "xchacha20-poly1305" },
            { 0, NULL, NULL }
        };

        GType g_define_type_id =
            g_enum_register_static (g_intern_static_string ("GrlResourceCipherType"), values);

        g_once_init_leave (&g_define_type_id__volatile, g_define_type_id);
    }

    return g_define_type_id__volatile;
}
