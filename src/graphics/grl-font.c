/* grl-font.c
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Font implementation.
 */

#include "config.h"
#include "grl-font.h"
#include "../resources/grl-resource-pack.h"
#include <raylib.h>

/**
 * SECTION:grl-font
 * @title: GrlFont
 * @short_description: Font for text rendering
 *
 * #GrlFont represents a font loaded into GPU memory for text rendering.
 * Fonts can be loaded from various file formats (TTF, OTF, FNT, etc.)
 * or created from images.
 *
 * Example - Loading and using a font:
 * |[<!-- language="C" -->
 * g_autoptr(GrlFont) font = grl_font_new_from_file ("fonts/arial.ttf");
 * g_autoptr(GrlVector2) pos = grl_vector2_new (100.0f, 100.0f);
 * g_autoptr(GrlColor) white = grl_color_new_white ();
 *
 * grl_draw_text_ex (font, "Hello World!", pos, 32.0f, 2.0f, white);
 * ]|
 *
 * Example - Measuring text:
 * |[<!-- language="C" -->
 * g_autoptr(GrlFont) font = grl_font_new_from_file ("fonts/arial.ttf");
 * g_autoptr(GrlVector2) size = grl_font_measure_text (font, "Hello", 24.0f, 1.0f);
 *
 * g_print ("Text size: %.0f x %.0f\n", size->x, size->y);
 * ]|
 */

struct _GrlFont
{
    GObject parent_instance;

    Font handle;
    gboolean valid;
    gboolean is_default;  /* Track if this is the default font (don't unload) */
};

G_DEFINE_TYPE (GrlFont, grl_font, G_TYPE_OBJECT)

enum
{
    PROP_0,
    PROP_BASE_SIZE,
    PROP_GLYPH_COUNT,
    PROP_GLYPH_PADDING,
    PROP_VALID,
    N_PROPS
};

static GParamSpec *properties[N_PROPS];

/*
 * Helper macros
 */

#define GRL_TO_RAYLIB_COLOR(c) \
    ((Color){ .r = (c)->r, .g = (c)->g, .b = (c)->b, .a = (c)->a })

/*
 * Private helper to create GrlFont from raylib Font
 */

static GrlFont *
grl_font_new_from_handle (Font       handle,
                          gboolean   is_default)
{
    GrlFont *self;

    self = g_object_new (GRL_TYPE_FONT, NULL);
    self->handle = handle;
    self->valid = IsFontValid (handle);
    self->is_default = is_default;

    return self;
}

/*
 * GObject virtual methods
 */

static void
grl_font_get_property (GObject    *object,
                       guint       prop_id,
                       GValue     *value,
                       GParamSpec *pspec)
{
    GrlFont *self = GRL_FONT (object);

    switch (prop_id)
    {
    case PROP_BASE_SIZE:
        g_value_set_int (value, self->handle.baseSize);
        break;
    case PROP_GLYPH_COUNT:
        g_value_set_int (value, self->handle.glyphCount);
        break;
    case PROP_GLYPH_PADDING:
        g_value_set_int (value, self->handle.glyphPadding);
        break;
    case PROP_VALID:
        g_value_set_boolean (value, self->valid);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
grl_font_finalize (GObject *object)
{
    GrlFont *self = GRL_FONT (object);

    /*
     * Unload font from GPU memory if valid and not the default font.
     * The default font is managed by raylib and should not be unloaded.
     */
    if (self->valid && !self->is_default)
    {
        UnloadFont (self->handle);
        self->valid = FALSE;
    }

    G_OBJECT_CLASS (grl_font_parent_class)->finalize (object);
}

static void
grl_font_class_init (GrlFontClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->get_property = grl_font_get_property;
    object_class->finalize = grl_font_finalize;

    /**
     * GrlFont:base-size:
     *
     * The base size of the font (default character height in pixels).
     */
    properties[PROP_BASE_SIZE] =
        g_param_spec_int ("base-size", NULL, NULL,
                          0, G_MAXINT, 0,
                          G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

    /**
     * GrlFont:glyph-count:
     *
     * The number of glyphs (characters) in the font.
     */
    properties[PROP_GLYPH_COUNT] =
        g_param_spec_int ("glyph-count", NULL, NULL,
                          0, G_MAXINT, 0,
                          G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

    /**
     * GrlFont:glyph-padding:
     *
     * The padding around font glyphs.
     */
    properties[PROP_GLYPH_PADDING] =
        g_param_spec_int ("glyph-padding", NULL, NULL,
                          0, G_MAXINT, 0,
                          G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

    /**
     * GrlFont:valid:
     *
     * Whether the font is valid and ready for use.
     */
    properties[PROP_VALID] =
        g_param_spec_boolean ("valid", NULL, NULL,
                              FALSE,
                              G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

    g_object_class_install_properties (object_class, N_PROPS, properties);
}

static void
grl_font_init (GrlFont *self)
{
    self->handle = (Font){ 0 };
    self->valid = FALSE;
    self->is_default = FALSE;
}

/*
 * =============================================================================
 * Constructors
 * =============================================================================
 */

/**
 * grl_font_new_default:
 *
 * Gets the default raylib font.
 *
 * Returns: (transfer full): The default #GrlFont
 */
GrlFont *
grl_font_new_default (void)
{
    Font font;

    font = GetFontDefault ();

    return grl_font_new_from_handle (font, TRUE);
}

/**
 * grl_font_new_from_file:
 * @filename: Path to font file
 *
 * Loads a font from a file.
 *
 * Returns: (transfer full): A new #GrlFont
 */
GrlFont *
grl_font_new_from_file (const gchar *filename)
{
    Font font;

    g_return_val_if_fail (filename != NULL, NULL);

    font = LoadFont (filename);

    return grl_font_new_from_handle (font, FALSE);
}

/**
 * grl_font_new_from_file_ex:
 * @filename: Path to font file
 * @font_size: Desired font size in pixels
 * @codepoints: (array length=codepoint_count) (nullable): Unicode codepoints
 * @codepoint_count: Number of codepoints
 *
 * Loads a font with extended parameters.
 *
 * Returns: (transfer full): A new #GrlFont
 */
GrlFont *
grl_font_new_from_file_ex (const gchar *filename,
                           gint         font_size,
                           gint        *codepoints,
                           gint         codepoint_count)
{
    Font font;

    g_return_val_if_fail (filename != NULL, NULL);

    font = LoadFontEx (filename, font_size, codepoints, codepoint_count);

    return grl_font_new_from_handle (font, FALSE);
}

/**
 * grl_font_new_from_image:
 * @image: Image containing font glyphs
 * @key: Color key for transparency
 * @first_char: First character code in the font
 *
 * Creates a font from an image (XNA-style sprite font).
 *
 * Returns: (transfer full): A new #GrlFont
 */
GrlFont *
grl_font_new_from_image (GrlImage       *image,
                         const GrlColor *key,
                         gint            first_char)
{
    Image *img_handle;
    Font font;

    g_return_val_if_fail (GRL_IS_IMAGE (image), NULL);
    g_return_val_if_fail (key != NULL, NULL);

    img_handle = (Image *)grl_image_get_handle (image);
    font = LoadFontFromImage (*img_handle, GRL_TO_RAYLIB_COLOR (key), first_char);

    return grl_font_new_from_handle (font, FALSE);
}

/**
 * grl_font_new_from_memory:
 * @file_type: Font format hint (e.g. ".ttf")
 * @data: (array length=data_size): Raw font file data
 * @data_size: Size of @data in bytes
 * @font_size: Desired font size in pixels
 * @codepoints: (array length=codepoint_count) (nullable): Unicode codepoints
 * @codepoint_count: Number of codepoints
 *
 * Loads a font from memory buffer.
 *
 * Returns: (transfer full): A new #GrlFont
 */
GrlFont *
grl_font_new_from_memory (const gchar  *file_type,
                          const guint8 *data,
                          gsize         data_size,
                          gint          font_size,
                          gint         *codepoints,
                          gint          codepoint_count)
{
    Font font;

    g_return_val_if_fail (file_type != NULL, NULL);
    g_return_val_if_fail (data != NULL, NULL);
    g_return_val_if_fail (data_size > 0, NULL);

    font = LoadFontFromMemory (file_type,
                               (const unsigned char *)data,
                               (int)data_size,
                               font_size,
                               codepoints,
                               codepoint_count);

    return grl_font_new_from_handle (font, FALSE);
}

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
GrlFont *
grl_font_new_from_resource (GrlResourcePack *pack,
                            guint32          resource_id,
                            gint             font_size,
                            const gchar     *file_type,
                            GError         **error)
{
    GrlFont *font;
    guint8 *data;
    gsize size;
    const gchar *type_hint;

    g_return_val_if_fail (GRL_IS_RESOURCE_PACK (pack), NULL);
    g_return_val_if_fail (font_size > 0, NULL);
    g_return_val_if_fail (error == NULL || *error == NULL, NULL);

    /* Load raw data from resource pack */
    data = grl_resource_pack_load_raw (pack, resource_id, &size, error);
    if (data == NULL)
        return NULL;

    /* Use provided file type or default to TTF */
    type_hint = (file_type != NULL) ? file_type : ".ttf";

    /* Load font from memory with default codepoints (NULL = ASCII 32-126) */
    font = grl_font_new_from_memory (type_hint, data, size, font_size, NULL, 0);
    g_free (data);

    if (font == NULL || !grl_font_is_valid (font))
    {
        g_clear_object (&font);
        g_set_error (error,
                     GRL_RESOURCE_PACK_ERROR,
                     GRL_RESOURCE_PACK_ERROR_CORRUPTED_DATA,
                     "Failed to load font from resource %u",
                     resource_id);
        return NULL;
    }

    return font;
}

/*
 * =============================================================================
 * Properties
 * =============================================================================
 */

/**
 * grl_font_get_base_size:
 * @self: A #GrlFont
 *
 * Gets the base size of the font.
 *
 * Returns: Base font size in pixels
 */
gint
grl_font_get_base_size (GrlFont *self)
{
    g_return_val_if_fail (GRL_IS_FONT (self), 0);

    return self->handle.baseSize;
}

/**
 * grl_font_get_glyph_count:
 * @self: A #GrlFont
 *
 * Gets the number of glyphs in the font.
 *
 * Returns: Number of glyphs
 */
gint
grl_font_get_glyph_count (GrlFont *self)
{
    g_return_val_if_fail (GRL_IS_FONT (self), 0);

    return self->handle.glyphCount;
}

/**
 * grl_font_get_glyph_padding:
 * @self: A #GrlFont
 *
 * Gets the padding around font glyphs.
 *
 * Returns: Glyph padding in pixels
 */
gint
grl_font_get_glyph_padding (GrlFont *self)
{
    g_return_val_if_fail (GRL_IS_FONT (self), 0);

    return self->handle.glyphPadding;
}

/**
 * grl_font_is_valid:
 * @self: A #GrlFont
 *
 * Checks if the font is valid.
 *
 * Returns: %TRUE if valid
 */
gboolean
grl_font_is_valid (GrlFont *self)
{
    g_return_val_if_fail (GRL_IS_FONT (self), FALSE);

    return self->valid;
}

/*
 * =============================================================================
 * Text Measurement
 * =============================================================================
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
GrlVector2 *
grl_font_measure_text (GrlFont     *self,
                       const gchar *text,
                       gfloat       font_size,
                       gfloat       spacing)
{
    Vector2 size;

    g_return_val_if_fail (GRL_IS_FONT (self), NULL);
    g_return_val_if_fail (text != NULL, NULL);

    size = MeasureTextEx (self->handle, text, font_size, spacing);

    return grl_vector2_new (size.x, size.y);
}

/*
 * =============================================================================
 * Glyph Information
 * =============================================================================
 */

/**
 * grl_font_get_glyph_index:
 * @self: A #GrlFont
 * @codepoint: Unicode codepoint
 *
 * Gets the glyph index for a Unicode codepoint.
 *
 * Returns: Glyph index in the font
 */
gint
grl_font_get_glyph_index (GrlFont *self,
                          gint     codepoint)
{
    g_return_val_if_fail (GRL_IS_FONT (self), 0);

    return GetGlyphIndex (self->handle, codepoint);
}

/**
 * grl_font_get_glyph_atlas_rec:
 * @self: A #GrlFont
 * @codepoint: Unicode codepoint
 *
 * Gets the rectangle in the font atlas for a specific glyph.
 *
 * Returns: (transfer full): A #GrlRectangle for the glyph
 */
GrlRectangle *
grl_font_get_glyph_atlas_rec (GrlFont *self,
                              gint     codepoint)
{
    Rectangle rec;

    g_return_val_if_fail (GRL_IS_FONT (self), NULL);

    rec = GetGlyphAtlasRec (self->handle, codepoint);

    return grl_rectangle_new (rec.x, rec.y, rec.width, rec.height);
}

/*
 * =============================================================================
 * Internal
 * =============================================================================
 */

/**
 * grl_font_get_handle:
 * @self: A #GrlFont
 *
 * Gets the internal raylib Font handle.
 * This is for internal use by drawing functions.
 *
 * Returns: (transfer none): Pointer to the Font handle
 */
gpointer
grl_font_get_handle (GrlFont *self)
{
    g_return_val_if_fail (GRL_IS_FONT (self), NULL);

    return &self->handle;
}
