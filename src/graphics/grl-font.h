/* grl-font.h
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Font type for text rendering.
 */

#pragma once

#if !defined(GRAYLIB_INSIDE) && !defined(GRAYLIB_COMPILATION)
#error "Only <graylib.h> can be included directly."
#endif

#include <glib-object.h>
#include "../grl-version.h"
#include "../grl-enums.h"
#include "../math/grl-vector2.h"
#include "../math/grl-color.h"
#include "../math/grl-rectangle.h"
#include "grl-image.h"
#include "grl-texture.h"

G_BEGIN_DECLS

#define GRL_TYPE_FONT (grl_font_get_type ())

GRL_AVAILABLE_IN_ALL
G_DECLARE_FINAL_TYPE (GrlFont, grl_font, GRL, FONT, GObject)

/*
 * Constructors
 */

/**
 * grl_font_new_default:
 *
 * Gets the default raylib font. This font is always available and
 * doesn't require loading any external files.
 *
 * Returns: (transfer full): The default #GrlFont
 */
GRL_AVAILABLE_IN_ALL
GrlFont *           grl_font_new_default        (void);

/**
 * grl_font_new_from_file:
 * @filename: Path to font file (TTF, OTF, FNT, etc.)
 *
 * Loads a font from a file. Supports TrueType (.ttf), OpenType (.otf),
 * BMFont (.fnt), and other common formats.
 *
 * Returns: (transfer full): A new #GrlFont, or %NULL on error
 */
GRL_AVAILABLE_IN_ALL
GrlFont *           grl_font_new_from_file      (const gchar        *filename);

/**
 * grl_font_new_from_file_ex:
 * @filename: Path to font file
 * @font_size: Desired font size in pixels
 * @codepoints: (array length=codepoint_count) (nullable): Unicode codepoints to load, or %NULL for default
 * @codepoint_count: Number of codepoints, or 0 for default character set
 *
 * Loads a font with extended parameters. Allows specifying the font size
 * and which characters to include in the font atlas.
 *
 * Returns: (transfer full): A new #GrlFont
 */
GRL_AVAILABLE_IN_ALL
GrlFont *           grl_font_new_from_file_ex   (const gchar        *filename,
                                                 gint                font_size,
                                                 gint               *codepoints,
                                                 gint                codepoint_count);

/**
 * grl_font_new_from_image:
 * @image: Image containing font glyphs
 * @key: Color key for transparency
 * @first_char: First character code in the font
 *
 * Creates a font from an image (XNA-style sprite font).
 * The image should contain character glyphs arranged in a row.
 *
 * Returns: (transfer full): A new #GrlFont
 */
GRL_AVAILABLE_IN_ALL
GrlFont *           grl_font_new_from_image     (GrlImage           *image,
                                                 const GrlColor     *key,
                                                 gint                first_char);

/**
 * grl_font_new_from_memory:
 * @file_type: Font format hint (e.g. ".ttf")
 * @data: (array length=data_size): Raw font file data
 * @data_size: Size of @data in bytes
 * @font_size: Desired font size in pixels
 * @codepoints: (array length=codepoint_count) (nullable): Unicode codepoints to load
 * @codepoint_count: Number of codepoints
 *
 * Loads a font from memory buffer.
 *
 * Returns: (transfer full): A new #GrlFont
 */
GRL_AVAILABLE_IN_ALL
GrlFont *           grl_font_new_from_memory    (const gchar        *file_type,
                                                 const guint8       *data,
                                                 gsize               data_size,
                                                 gint                font_size,
                                                 gint               *codepoints,
                                                 gint                codepoint_count);

/* Forward declaration */
typedef struct _GrlResourcePack GrlResourcePack;

/**
 * grl_font_new_from_resource:
 * @pack: A #GrlResourcePack
 * @resource_id: The resource ID to load
 * @font_size: Desired font size in pixels
 * @file_type: (nullable): File type hint (e.g., ".ttf"), or %NULL to default to TTF
 * @error: (nullable): Return location for error, or %NULL
 *
 * Loads a font from a resource pack.
 *
 * If @file_type is %NULL, the function assumes TTF format.
 *
 * Returns: (transfer full) (nullable): A new #GrlFont, or %NULL on error
 */
GRL_AVAILABLE_IN_ALL
GrlFont *           grl_font_new_from_resource  (GrlResourcePack    *pack,
                                                 guint32             resource_id,
                                                 gint                font_size,
                                                 const gchar        *file_type,
                                                 GError            **error);

/*
 * Properties
 */

/**
 * grl_font_get_base_size:
 * @self: A #GrlFont
 *
 * Gets the base size of the font (default character height).
 *
 * Returns: Base font size in pixels
 */
GRL_AVAILABLE_IN_ALL
gint                grl_font_get_base_size      (GrlFont            *self);

/**
 * grl_font_get_glyph_count:
 * @self: A #GrlFont
 *
 * Gets the number of glyphs (characters) in the font.
 *
 * Returns: Number of glyphs
 */
GRL_AVAILABLE_IN_ALL
gint                grl_font_get_glyph_count    (GrlFont            *self);

/**
 * grl_font_get_glyph_padding:
 * @self: A #GrlFont
 *
 * Gets the padding around font glyphs.
 *
 * Returns: Glyph padding in pixels
 */
GRL_AVAILABLE_IN_ALL
gint                grl_font_get_glyph_padding  (GrlFont            *self);

/**
 * grl_font_is_valid:
 * @self: A #GrlFont
 *
 * Checks if the font was loaded successfully.
 *
 * Returns: %TRUE if valid
 */
GRL_AVAILABLE_IN_ALL
gboolean            grl_font_is_valid           (GrlFont            *self);

/*
 * Text measurement
 */

/**
 * grl_font_measure_text:
 * @self: A #GrlFont
 * @text: Text to measure
 * @font_size: Font size to use
 * @spacing: Character spacing
 *
 * Measures the size that text would occupy when rendered.
 *
 * Returns: (transfer full): A #GrlVector2 with width and height
 */
GRL_AVAILABLE_IN_ALL
GrlVector2 *        grl_font_measure_text       (GrlFont            *self,
                                                 const gchar        *text,
                                                 gfloat              font_size,
                                                 gfloat              spacing);

/*
 * Glyph information
 */

/**
 * grl_font_get_glyph_index:
 * @self: A #GrlFont
 * @codepoint: Unicode codepoint
 *
 * Gets the glyph index for a Unicode codepoint.
 * Falls back to '?' if the codepoint is not found.
 *
 * Returns: Glyph index in the font
 */
GRL_AVAILABLE_IN_ALL
gint                grl_font_get_glyph_index    (GrlFont            *self,
                                                 gint                codepoint);

/**
 * grl_font_get_glyph_atlas_rec:
 * @self: A #GrlFont
 * @codepoint: Unicode codepoint
 *
 * Gets the rectangle in the font atlas for a specific glyph.
 *
 * Returns: (transfer full): A #GrlRectangle for the glyph
 */
GRL_AVAILABLE_IN_ALL
GrlRectangle *      grl_font_get_glyph_atlas_rec (GrlFont           *self,
                                                  gint               codepoint);

/*
 * Internal - get raylib Font handle
 */

gpointer            grl_font_get_handle         (GrlFont            *self);

G_END_DECLS
