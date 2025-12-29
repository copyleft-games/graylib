/* grl-resource-enums.h
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Resource pack enumerations for rres file format.
 */

#pragma once

#if !defined(GRAYLIB_INSIDE) && !defined(GRAYLIB_COMPILATION)
#error "Only <graylib.h> can be included directly."
#endif

#include <glib-object.h>
#include "../grl-version.h"

G_BEGIN_DECLS

/**
 * GrlResourceDataType:
 * @GRL_RESOURCE_DATA_NULL: Reserved for empty chunks, no props/data
 * @GRL_RESOURCE_DATA_RAW: Raw file data, unprocessed
 * @GRL_RESOURCE_DATA_TEXT: Text file data
 * @GRL_RESOURCE_DATA_IMAGE: Image pixel data
 * @GRL_RESOURCE_DATA_WAVE: Audio sample data
 * @GRL_RESOURCE_DATA_VERTEX: Vertex mesh data
 * @GRL_RESOURCE_DATA_FONT_GLYPHS: Font glyph info data
 * @GRL_RESOURCE_DATA_LINK: External linked file path
 * @GRL_RESOURCE_DATA_DIRECTORY: Central directory chunk
 *
 * Types of resource data that can be stored in rres chunks.
 */
typedef enum
{
    GRL_RESOURCE_DATA_NULL        = 0,
    GRL_RESOURCE_DATA_RAW         = 1,
    GRL_RESOURCE_DATA_TEXT        = 2,
    GRL_RESOURCE_DATA_IMAGE       = 3,
    GRL_RESOURCE_DATA_WAVE        = 4,
    GRL_RESOURCE_DATA_VERTEX      = 5,
    GRL_RESOURCE_DATA_FONT_GLYPHS = 6,
    GRL_RESOURCE_DATA_LINK        = 99,
    GRL_RESOURCE_DATA_DIRECTORY   = 100
} GrlResourceDataType;

GRL_AVAILABLE_IN_ALL
GType grl_resource_data_type_get_type (void) G_GNUC_CONST;
#define GRL_TYPE_RESOURCE_DATA_TYPE (grl_resource_data_type_get_type ())

/**
 * GrlResourceCompressionType:
 * @GRL_RESOURCE_COMP_NONE: No compression
 * @GRL_RESOURCE_COMP_RLE: RLE compression
 * @GRL_RESOURCE_COMP_DEFLATE: DEFLATE compression
 * @GRL_RESOURCE_COMP_LZ4: LZ4 compression
 * @GRL_RESOURCE_COMP_LZMA2: LZMA2 compression
 * @GRL_RESOURCE_COMP_QOI: QOI compression (for RGB(A) images)
 *
 * Compression algorithms for resource data.
 */
typedef enum
{
    GRL_RESOURCE_COMP_NONE    = 0,
    GRL_RESOURCE_COMP_RLE     = 1,
    GRL_RESOURCE_COMP_DEFLATE = 10,
    GRL_RESOURCE_COMP_LZ4     = 20,
    GRL_RESOURCE_COMP_LZMA2   = 30,
    GRL_RESOURCE_COMP_QOI     = 40
} GrlResourceCompressionType;

GRL_AVAILABLE_IN_ALL
GType grl_resource_compression_type_get_type (void) G_GNUC_CONST;
#define GRL_TYPE_RESOURCE_COMPRESSION_TYPE (grl_resource_compression_type_get_type ())

/**
 * GrlResourceCipherType:
 * @GRL_RESOURCE_CIPHER_NONE: No encryption
 * @GRL_RESOURCE_CIPHER_XOR: XOR encryption with 128-bit key
 * @GRL_RESOURCE_CIPHER_DES: DES encryption
 * @GRL_RESOURCE_CIPHER_TDES: Triple DES encryption
 * @GRL_RESOURCE_CIPHER_IDEA: IDEA encryption
 * @GRL_RESOURCE_CIPHER_AES: AES (128/256 bit) encryption
 * @GRL_RESOURCE_CIPHER_AES_GCM: AES Galois/Counter Mode
 * @GRL_RESOURCE_CIPHER_XTEA: XTEA encryption
 * @GRL_RESOURCE_CIPHER_BLOWFISH: Blowfish encryption
 * @GRL_RESOURCE_CIPHER_RSA: RSA asymmetric encryption
 * @GRL_RESOURCE_CIPHER_SALSA20: SALSA20 encryption
 * @GRL_RESOURCE_CIPHER_CHACHA20: CHACHA20 encryption
 * @GRL_RESOURCE_CIPHER_XCHACHA20: XCHACHA20 encryption
 * @GRL_RESOURCE_CIPHER_XCHACHA20_POLY1305: XCHACHA20 with POLY1305 MAC
 *
 * Encryption algorithms for resource data.
 */
typedef enum
{
    GRL_RESOURCE_CIPHER_NONE              = 0,
    GRL_RESOURCE_CIPHER_XOR               = 1,
    GRL_RESOURCE_CIPHER_DES               = 10,
    GRL_RESOURCE_CIPHER_TDES              = 11,
    GRL_RESOURCE_CIPHER_IDEA              = 20,
    GRL_RESOURCE_CIPHER_AES               = 30,
    GRL_RESOURCE_CIPHER_AES_GCM           = 31,
    GRL_RESOURCE_CIPHER_XTEA              = 40,
    GRL_RESOURCE_CIPHER_BLOWFISH          = 50,
    GRL_RESOURCE_CIPHER_RSA               = 60,
    GRL_RESOURCE_CIPHER_SALSA20           = 70,
    GRL_RESOURCE_CIPHER_CHACHA20          = 71,
    GRL_RESOURCE_CIPHER_XCHACHA20         = 72,
    GRL_RESOURCE_CIPHER_XCHACHA20_POLY1305 = 73
} GrlResourceCipherType;

GRL_AVAILABLE_IN_ALL
GType grl_resource_cipher_type_get_type (void) G_GNUC_CONST;
#define GRL_TYPE_RESOURCE_CIPHER_TYPE (grl_resource_cipher_type_get_type ())

G_END_DECLS
