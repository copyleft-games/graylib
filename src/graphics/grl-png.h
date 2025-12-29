/* grl-png.h
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * PNG chunk manipulation types and functions.
 * Wraps the rpng library for PNG metadata and indexed image handling.
 */

#pragma once

#if !defined(GRAYLIB_INSIDE) && !defined(GRAYLIB_COMPILATION)
#error "Only <graylib.h> can be included directly."
#endif

#include <glib-object.h>
#include "../grl-version.h"
#include "../math/grl-color.h"

G_BEGIN_DECLS

/*
 * =============================================================================
 * GrlPngChunk - PNG Chunk Data
 * =============================================================================
 */

#define GRL_TYPE_PNG_CHUNK (grl_png_chunk_get_type ())

/**
 * GrlPngChunk:
 *
 * A PNG chunk containing metadata or image data.
 * PNG files are structured as a series of chunks, each with a type,
 * length, data, and CRC checksum.
 *
 * This is a GBoxed type.
 */
typedef struct _GrlPngChunk GrlPngChunk;

struct _GrlPngChunk
{
    gint      length;       /* Data length */
    gchar     type[4];      /* Chunk type FOURCC (e.g., "tEXt", "gAMA") */
    guint8   *data;         /* Chunk data pointer (owned by this struct) */
    guint32   crc;          /* 32-bit CRC checksum */
};

GRL_AVAILABLE_IN_ALL
GType           grl_png_chunk_get_type      (void) G_GNUC_CONST;

/**
 * grl_png_chunk_new:
 * @type: Four-character chunk type (e.g., "tEXt")
 * @data: (array length=length): Chunk data
 * @length: Length of chunk data
 *
 * Creates a new PNG chunk. The CRC will be computed automatically.
 *
 * Returns: (transfer full): A new #GrlPngChunk
 */
GRL_AVAILABLE_IN_ALL
GrlPngChunk *   grl_png_chunk_new           (const gchar    *type,
                                             const guint8   *data,
                                             gint            length);

/**
 * grl_png_chunk_copy:
 * @self: A #GrlPngChunk
 *
 * Creates a copy of a PNG chunk.
 *
 * Returns: (transfer full): A copy of @self
 */
GRL_AVAILABLE_IN_ALL
GrlPngChunk *   grl_png_chunk_copy          (const GrlPngChunk *self);

/**
 * grl_png_chunk_free:
 * @self: A #GrlPngChunk
 *
 * Frees a PNG chunk and its data.
 */
GRL_AVAILABLE_IN_ALL
void            grl_png_chunk_free          (GrlPngChunk    *self);

/**
 * grl_png_chunk_get_type_string:
 * @self: A #GrlPngChunk
 *
 * Gets the chunk type as a null-terminated string.
 *
 * Returns: (transfer full): The chunk type string
 */
GRL_AVAILABLE_IN_ALL
gchar *         grl_png_chunk_get_type_string (GrlPngChunk  *self);

G_DEFINE_AUTOPTR_CLEANUP_FUNC (GrlPngChunk, grl_png_chunk_free)

/*
 * =============================================================================
 * GrlPngPalette - PNG Color Palette
 * =============================================================================
 */

#define GRL_TYPE_PNG_PALETTE (grl_png_palette_get_type ())

/**
 * GrlPngPalette:
 *
 * A color palette for indexed PNG images.
 * Contains an array of colors used to map 8-bit indices to RGBA colors.
 *
 * This is a GBoxed type.
 */
typedef struct _GrlPngPalette GrlPngPalette;

struct _GrlPngPalette
{
    gint       color_count;   /* Number of colors in palette */
    GrlColor  *colors;        /* Array of palette colors (owned by this struct) */
};

GRL_AVAILABLE_IN_ALL
GType           grl_png_palette_get_type    (void) G_GNUC_CONST;

/**
 * grl_png_palette_new:
 * @colors: (array length=count): Array of colors
 * @count: Number of colors
 *
 * Creates a new PNG palette.
 *
 * Returns: (transfer full): A new #GrlPngPalette
 */
GRL_AVAILABLE_IN_ALL
GrlPngPalette * grl_png_palette_new         (const GrlColor *colors,
                                             gint            count);

/**
 * grl_png_palette_new_empty:
 * @count: Number of palette entries to allocate
 *
 * Creates a new empty PNG palette with the specified number of entries.
 *
 * Returns: (transfer full): A new #GrlPngPalette
 */
GRL_AVAILABLE_IN_ALL
GrlPngPalette * grl_png_palette_new_empty   (gint            count);

/**
 * grl_png_palette_copy:
 * @self: A #GrlPngPalette
 *
 * Creates a copy of a PNG palette.
 *
 * Returns: (transfer full): A copy of @self
 */
GRL_AVAILABLE_IN_ALL
GrlPngPalette * grl_png_palette_copy        (const GrlPngPalette *self);

/**
 * grl_png_palette_free:
 * @self: A #GrlPngPalette
 *
 * Frees a PNG palette.
 */
GRL_AVAILABLE_IN_ALL
void            grl_png_palette_free        (GrlPngPalette  *self);

/**
 * grl_png_palette_get_color:
 * @self: A #GrlPngPalette
 * @index: Color index (0-255)
 *
 * Gets a color from the palette.
 *
 * Returns: (transfer full) (nullable): The color at @index, or %NULL if out of range
 */
GRL_AVAILABLE_IN_ALL
GrlColor *      grl_png_palette_get_color   (GrlPngPalette  *self,
                                             gint            index);

/**
 * grl_png_palette_set_color:
 * @self: A #GrlPngPalette
 * @index: Color index (0-255)
 * @color: The color to set
 *
 * Sets a color in the palette.
 */
GRL_AVAILABLE_IN_ALL
void            grl_png_palette_set_color   (GrlPngPalette  *self,
                                             gint            index,
                                             GrlColor       *color);

G_DEFINE_AUTOPTR_CLEANUP_FUNC (GrlPngPalette, grl_png_palette_free)

/*
 * =============================================================================
 * PNG Chunk Functions
 * =============================================================================
 */

/**
 * grl_png_chunk_count:
 * @filename: (type filename): Path to PNG file
 * @error: (nullable): Return location for error
 *
 * Counts the number of chunks in a PNG file.
 *
 * Returns: The number of chunks, or -1 on error
 */
GRL_AVAILABLE_IN_ALL
gint            grl_png_chunk_count         (const gchar    *filename,
                                             GError        **error);

/**
 * grl_png_chunk_read:
 * @filename: (type filename): Path to PNG file
 * @chunk_type: Four-character chunk type to read
 * @error: (nullable): Return location for error
 *
 * Reads a specific chunk type from a PNG file.
 *
 * Returns: (transfer full) (nullable): The chunk, or %NULL if not found or error
 */
GRL_AVAILABLE_IN_ALL
GrlPngChunk *   grl_png_chunk_read          (const gchar    *filename,
                                             const gchar    *chunk_type,
                                             GError        **error);

/**
 * grl_png_chunk_read_all:
 * @filename: (type filename): Path to PNG file
 * @count: (out): Number of chunks read
 * @error: (nullable): Return location for error
 *
 * Reads all chunks from a PNG file.
 *
 * Returns: (transfer full) (array length=count): Array of chunks, or %NULL on error
 */
GRL_AVAILABLE_IN_ALL
GrlPngChunk **  grl_png_chunk_read_all      (const gchar    *filename,
                                             gint           *count,
                                             GError        **error);

/**
 * grl_png_chunk_write:
 * @filename: (type filename): Path to PNG file
 * @chunk: The chunk to write
 * @error: (nullable): Return location for error
 *
 * Writes a chunk to a PNG file (after IHDR).
 *
 * Returns: %TRUE on success
 */
GRL_AVAILABLE_IN_ALL
gboolean        grl_png_chunk_write         (const gchar    *filename,
                                             GrlPngChunk    *chunk,
                                             GError        **error);

/**
 * grl_png_chunk_remove:
 * @filename: (type filename): Path to PNG file
 * @chunk_type: Four-character chunk type to remove
 * @error: (nullable): Return location for error
 *
 * Removes all chunks of a specific type from a PNG file.
 *
 * Returns: %TRUE on success
 */
GRL_AVAILABLE_IN_ALL
gboolean        grl_png_chunk_remove        (const gchar    *filename,
                                             const gchar    *chunk_type,
                                             GError        **error);

/**
 * grl_png_chunk_write_text:
 * @filename: (type filename): Path to PNG file
 * @keyword: Text keyword (max 79 characters)
 * @text: Text content
 * @error: (nullable): Return location for error
 *
 * Writes a tEXt (text) chunk to a PNG file.
 *
 * Returns: %TRUE on success
 */
GRL_AVAILABLE_IN_ALL
gboolean        grl_png_chunk_write_text    (const gchar    *filename,
                                             const gchar    *keyword,
                                             const gchar    *text,
                                             GError        **error);

/**
 * grl_png_chunk_write_gamma:
 * @filename: (type filename): Path to PNG file
 * @gamma: Gamma value (typically 2.2)
 * @error: (nullable): Return location for error
 *
 * Writes a gAMA (gamma) chunk to a PNG file.
 *
 * Returns: %TRUE on success
 */
GRL_AVAILABLE_IN_ALL
gboolean        grl_png_chunk_write_gamma   (const gchar    *filename,
                                             gfloat          gamma,
                                             GError        **error);

/**
 * grl_png_chunk_write_srgb:
 * @filename: (type filename): Path to PNG file
 * @rendering_intent: sRGB rendering intent (0-3)
 * @error: (nullable): Return location for error
 *
 * Writes an sRGB chunk to a PNG file.
 * Rendering intents: 0=Perceptual, 1=Relative, 2=Saturation, 3=Absolute
 *
 * Returns: %TRUE on success
 */
GRL_AVAILABLE_IN_ALL
gboolean        grl_png_chunk_write_srgb    (const gchar    *filename,
                                             gint            rendering_intent,
                                             GError        **error);

/**
 * grl_png_chunk_write_time:
 * @filename: (type filename): Path to PNG file
 * @year: Year (e.g., 2025)
 * @month: Month (1-12)
 * @day: Day (1-31)
 * @hour: Hour (0-23)
 * @min: Minute (0-59)
 * @sec: Second (0-60)
 * @error: (nullable): Return location for error
 *
 * Writes a tIME (timestamp) chunk to a PNG file.
 *
 * Returns: %TRUE on success
 */
GRL_AVAILABLE_IN_ALL
gboolean        grl_png_chunk_write_time    (const gchar    *filename,
                                             gint            year,
                                             gint            month,
                                             gint            day,
                                             gint            hour,
                                             gint            min,
                                             gint            sec,
                                             GError        **error);

/**
 * grl_png_chunk_check_valid:
 * @filename: (type filename): Path to PNG file
 * @error: (nullable): Return location for error
 *
 * Validates all chunk CRCs in a PNG file.
 *
 * Returns: %TRUE if all chunks are valid
 */
GRL_AVAILABLE_IN_ALL
gboolean        grl_png_chunk_check_valid   (const gchar    *filename,
                                             GError        **error);

G_END_DECLS
