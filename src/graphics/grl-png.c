/* grl-png.c
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#include "config.h"
#include "grl-png.h"

/* Note: RPNG_DEFLATE_IMPLEMENTATION is not defined because raylib
 * already includes sdefl/sinfl (the deflate implementation used by rpng).
 * We only define RPNG_IMPLEMENTATION to get the rpng functions.
 */
#define RPNG_IMPLEMENTATION
#include <rpng.h>

/*
 * Error domain for PNG operations
 */
#define GRL_PNG_ERROR (grl_png_error_quark ())

static GQuark
grl_png_error_quark (void)
{
    return g_quark_from_static_string ("grl-png-error-quark");
}

/*
 * =============================================================================
 * GrlPngChunk Implementation
 * =============================================================================
 */

G_DEFINE_BOXED_TYPE (GrlPngChunk, grl_png_chunk,
                     grl_png_chunk_copy, grl_png_chunk_free)

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
GrlPngChunk *
grl_png_chunk_new (const gchar  *type,
                   const guint8 *data,
                   gint          length)
{
    GrlPngChunk *chunk;

    g_return_val_if_fail (type != NULL, NULL);
    g_return_val_if_fail (length >= 0, NULL);

    chunk = g_new0 (GrlPngChunk, 1);
    chunk->length = length;

    /* Copy type (4 bytes) */
    memcpy (chunk->type, type, 4);

    /* Copy data if present */
    if (length > 0 && data != NULL)
    {
        chunk->data = g_memdup2 (data, length);
    }
    else
    {
        chunk->data = NULL;
    }

    /* CRC will be computed when writing */
    chunk->crc = 0;

    return chunk;
}

/**
 * grl_png_chunk_copy:
 * @self: A #GrlPngChunk
 *
 * Creates a copy of a PNG chunk.
 *
 * Returns: (transfer full): A copy of @self
 */
GrlPngChunk *
grl_png_chunk_copy (const GrlPngChunk *self)
{
    GrlPngChunk *copy;

    g_return_val_if_fail (self != NULL, NULL);

    copy = g_new0 (GrlPngChunk, 1);
    copy->length = self->length;
    memcpy (copy->type, self->type, 4);
    copy->crc = self->crc;

    if (self->length > 0 && self->data != NULL)
        copy->data = g_memdup2 (self->data, self->length);
    else
        copy->data = NULL;

    return copy;
}

/**
 * grl_png_chunk_free:
 * @self: A #GrlPngChunk
 *
 * Frees a PNG chunk and its data.
 */
void
grl_png_chunk_free (GrlPngChunk *self)
{
    if (self == NULL)
        return;

    g_free (self->data);
    g_free (self);
}

/**
 * grl_png_chunk_get_type_string:
 * @self: A #GrlPngChunk
 *
 * Gets the chunk type as a null-terminated string.
 *
 * Returns: (transfer full): The chunk type string
 */
gchar *
grl_png_chunk_get_type_string (GrlPngChunk *self)
{
    g_return_val_if_fail (self != NULL, NULL);

    return g_strndup (self->type, 4);
}

/*
 * =============================================================================
 * GrlPngPalette Implementation
 * =============================================================================
 */

G_DEFINE_BOXED_TYPE (GrlPngPalette, grl_png_palette,
                     grl_png_palette_copy, grl_png_palette_free)

/**
 * grl_png_palette_new:
 * @colors: (array length=count): Array of colors
 * @count: Number of colors
 *
 * Creates a new PNG palette.
 *
 * Returns: (transfer full): A new #GrlPngPalette
 */
GrlPngPalette *
grl_png_palette_new (const GrlColor *colors,
                     gint            count)
{
    GrlPngPalette *palette;
    gint           i;

    g_return_val_if_fail (count >= 0, NULL);

    palette = g_new0 (GrlPngPalette, 1);
    palette->color_count = count;

    if (count > 0)
    {
        palette->colors = g_new (GrlColor, count);
        if (colors != NULL)
        {
            for (i = 0; i < count; i++)
            {
                palette->colors[i] = colors[i];
            }
        }
        else
        {
            memset (palette->colors, 0, sizeof (GrlColor) * count);
        }
    }
    else
    {
        palette->colors = NULL;
    }

    return palette;
}

/**
 * grl_png_palette_new_empty:
 * @count: Number of palette entries to allocate
 *
 * Creates a new empty PNG palette with the specified number of entries.
 *
 * Returns: (transfer full): A new #GrlPngPalette
 */
GrlPngPalette *
grl_png_palette_new_empty (gint count)
{
    return grl_png_palette_new (NULL, count);
}

/**
 * grl_png_palette_copy:
 * @self: A #GrlPngPalette
 *
 * Creates a copy of a PNG palette.
 *
 * Returns: (transfer full): A copy of @self
 */
GrlPngPalette *
grl_png_palette_copy (const GrlPngPalette *self)
{
    g_return_val_if_fail (self != NULL, NULL);

    return grl_png_palette_new (self->colors, self->color_count);
}

/**
 * grl_png_palette_free:
 * @self: A #GrlPngPalette
 *
 * Frees a PNG palette.
 */
void
grl_png_palette_free (GrlPngPalette *self)
{
    if (self == NULL)
        return;

    g_free (self->colors);
    g_free (self);
}

/**
 * grl_png_palette_get_color:
 * @self: A #GrlPngPalette
 * @index: Color index (0-255)
 *
 * Gets a color from the palette.
 *
 * Returns: (transfer full) (nullable): The color at @index, or %NULL if out of range
 */
GrlColor *
grl_png_palette_get_color (GrlPngPalette *self,
                           gint           index)
{
    g_return_val_if_fail (self != NULL, NULL);
    g_return_val_if_fail (index >= 0, NULL);
    g_return_val_if_fail (index < self->color_count, NULL);

    return grl_color_new (self->colors[index].r,
                          self->colors[index].g,
                          self->colors[index].b,
                          self->colors[index].a);
}

/**
 * grl_png_palette_set_color:
 * @self: A #GrlPngPalette
 * @index: Color index (0-255)
 * @color: The color to set
 *
 * Sets a color in the palette.
 */
void
grl_png_palette_set_color (GrlPngPalette *self,
                           gint           index,
                           GrlColor      *color)
{
    g_return_if_fail (self != NULL);
    g_return_if_fail (index >= 0);
    g_return_if_fail (index < self->color_count);
    g_return_if_fail (color != NULL);

    self->colors[index].r = color->r;
    self->colors[index].g = color->g;
    self->colors[index].b = color->b;
    self->colors[index].a = color->a;
}

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
gint
grl_png_chunk_count (const gchar  *filename,
                     GError      **error)
{
    int count;

    g_return_val_if_fail (filename != NULL, -1);

    count = rpng_chunk_count (filename);

    if (count < 0)
    {
        g_set_error (error,
                     GRL_PNG_ERROR,
                     1,
                     "Failed to read PNG file: %s",
                     filename);
        return -1;
    }

    return count;
}

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
GrlPngChunk *
grl_png_chunk_read (const gchar  *filename,
                    const gchar  *chunk_type,
                    GError      **error)
{
    rpng_chunk   rpng_result;
    GrlPngChunk *chunk;

    g_return_val_if_fail (filename != NULL, NULL);
    g_return_val_if_fail (chunk_type != NULL, NULL);

    rpng_result = rpng_chunk_read (filename, chunk_type);

    if (rpng_result.data == NULL)
    {
        /* Chunk not found or error */
        return NULL;
    }

    chunk = g_new0 (GrlPngChunk, 1);
    chunk->length = rpng_result.length;
    memcpy (chunk->type, rpng_result.type, 4);
    chunk->crc = rpng_result.crc;

    if (rpng_result.length > 0)
    {
        chunk->data = g_memdup2 (rpng_result.data, rpng_result.length);
        RPNG_FREE (rpng_result.data);
    }
    else
    {
        chunk->data = NULL;
    }

    return chunk;
}

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
GrlPngChunk **
grl_png_chunk_read_all (const gchar  *filename,
                        gint         *count,
                        GError      **error)
{
    rpng_chunk   *rpng_chunks;
    GrlPngChunk **chunks;
    int           chunk_count;
    int           i;

    g_return_val_if_fail (filename != NULL, NULL);
    g_return_val_if_fail (count != NULL, NULL);

    *count = 0;

    rpng_chunks = rpng_chunk_read_all (filename, &chunk_count);

    if (rpng_chunks == NULL || chunk_count <= 0)
    {
        g_set_error (error,
                     GRL_PNG_ERROR,
                     1,
                     "Failed to read PNG chunks from: %s",
                     filename);
        return NULL;
    }

    chunks = g_new0 (GrlPngChunk *, chunk_count);

    for (i = 0; i < chunk_count; i++)
    {
        chunks[i] = g_new0 (GrlPngChunk, 1);
        chunks[i]->length = rpng_chunks[i].length;
        memcpy (chunks[i]->type, rpng_chunks[i].type, 4);
        chunks[i]->crc = rpng_chunks[i].crc;

        if (rpng_chunks[i].length > 0 && rpng_chunks[i].data != NULL)
        {
            chunks[i]->data = g_memdup2 (rpng_chunks[i].data, rpng_chunks[i].length);
            RPNG_FREE (rpng_chunks[i].data);
        }
        else
        {
            chunks[i]->data = NULL;
        }
    }

    RPNG_FREE (rpng_chunks);

    *count = chunk_count;
    return chunks;
}

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
gboolean
grl_png_chunk_write (const gchar *filename,
                     GrlPngChunk *chunk,
                     GError     **error)
{
    rpng_chunk rpng_data;

    g_return_val_if_fail (filename != NULL, FALSE);
    g_return_val_if_fail (chunk != NULL, FALSE);

    rpng_data.length = chunk->length;
    memcpy (rpng_data.type, chunk->type, 4);
    rpng_data.data = (char *)chunk->data;
    rpng_data.crc = chunk->crc;

    rpng_chunk_write (filename, rpng_data);

    return TRUE;
}

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
gboolean
grl_png_chunk_remove (const gchar *filename,
                      const gchar *chunk_type,
                      GError     **error)
{
    g_return_val_if_fail (filename != NULL, FALSE);
    g_return_val_if_fail (chunk_type != NULL, FALSE);

    rpng_chunk_remove (filename, chunk_type);

    return TRUE;
}

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
gboolean
grl_png_chunk_write_text (const gchar *filename,
                          const gchar *keyword,
                          const gchar *text,
                          GError     **error)
{
    g_return_val_if_fail (filename != NULL, FALSE);
    g_return_val_if_fail (keyword != NULL, FALSE);
    g_return_val_if_fail (text != NULL, FALSE);

    rpng_chunk_write_text (filename, (char *)keyword, (char *)text);

    return TRUE;
}

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
gboolean
grl_png_chunk_write_gamma (const gchar *filename,
                           gfloat       gamma,
                           GError     **error)
{
    g_return_val_if_fail (filename != NULL, FALSE);

    rpng_chunk_write_gamma (filename, gamma);

    return TRUE;
}

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
gboolean
grl_png_chunk_write_srgb (const gchar *filename,
                          gint         rendering_intent,
                          GError     **error)
{
    g_return_val_if_fail (filename != NULL, FALSE);
    g_return_val_if_fail (rendering_intent >= 0 && rendering_intent <= 3, FALSE);

    rpng_chunk_write_srgb (filename, (char)rendering_intent);

    return TRUE;
}

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
gboolean
grl_png_chunk_write_time (const gchar *filename,
                          gint         year,
                          gint         month,
                          gint         day,
                          gint         hour,
                          gint         min,
                          gint         sec,
                          GError     **error)
{
    g_return_val_if_fail (filename != NULL, FALSE);

    rpng_chunk_write_time (filename, (short)year, (char)month, (char)day,
                           (char)hour, (char)min, (char)sec);

    return TRUE;
}

/**
 * grl_png_chunk_check_valid:
 * @filename: (type filename): Path to PNG file
 * @error: (nullable): Return location for error
 *
 * Validates all chunk CRCs in a PNG file.
 *
 * Returns: %TRUE if all chunks are valid
 */
gboolean
grl_png_chunk_check_valid (const gchar  *filename,
                           GError      **error)
{
    unsigned char result;

    g_return_val_if_fail (filename != NULL, FALSE);

    result = rpng_chunk_check_all_valid (filename);

    if (!result)
    {
        g_set_error (error,
                     GRL_PNG_ERROR,
                     2,
                     "PNG file has invalid chunk CRCs: %s",
                     filename);
        return FALSE;
    }

    return TRUE;
}
