/* grl-image-font.c
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Headless TTF/OTF font rasteriser backed by stb_truetype.
 *
 * stb_truetype note
 * =================
 * STBTT_STATIC is defined so that every stb symbol is file-local, avoiding
 * duplicate-symbol conflicts with raylib (which also embeds stb_truetype).
 * STB_TRUETYPE_IMPLEMENTATION is defined here and only here; no other
 * translation unit in graylib includes stb_truetype.h.
 *
 * Compositing note
 * ================
 * Each grl_image_draw_text_ttf() call rasterises glyphs into a temporary
 * RGBA #GrlImage (the "glyph layer"), then calls grl_image_composite() with
 * GRL_PORTER_DUFF_SRC_OVER to merge it into the destination.  This lets the
 * destination's clip rectangle and colour-space setting take effect without
 * grl-image-font.c ever touching grl-image.c internals.  The destination's
 * per-image blend-mode property is not consulted; SRC_OVER is always used for
 * the glyph composite step.
 */

#include "config.h"

/*
 * Pull in stb_truetype as a static compilation unit.
 *
 * STBTT_STATIC makes all stb symbols file-local (static linkage), avoiding
 * duplicate-symbol conflicts with raylib which also embeds stb_truetype.
 *
 * Suppress -Wunused-function: stb_truetype defines many static helpers that
 * are not called in every configuration; we only use a subset.
 */
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#endif

#define STBTT_STATIC
#define STB_TRUETYPE_IMPLEMENTATION
/* Resolved via -isystem $(RAYLIB_SRC) (already in LIB_CFLAGS); adding the
 * external/ dir to the include path directly would expose raylib's
 * Windows-only dirent.h shim and break the Linux build. */
#include <external/stb_truetype.h>

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

#include "grl-image-font.h"
#include "grl-image.h"
#include "../math/grl-color.h"
#include "../math/grl-vector2.h"
#include "../grl-enums.h"

#include <glib.h>
#include <glib/gstdio.h>
#include <string.h>
#include <math.h>

/* -------------------------------------------------------------------------
 * Private structure
 * -------------------------------------------------------------------------
 */

struct _GrlImageFont
{
    GObject         parent_instance;

    guint8         *font_data;  /* owned copy of the font file bytes */
    gsize           font_len;
    stbtt_fontinfo  info;
    gboolean        valid;      /* TRUE once info has been successfully initialised */
};

G_DEFINE_TYPE (GrlImageFont, grl_image_font, G_TYPE_OBJECT)

G_DEFINE_QUARK (grl-image-font-error-quark, grl_image_font_error)

/* -------------------------------------------------------------------------
 * GObject boilerplate
 * -------------------------------------------------------------------------
 */

static void
grl_image_font_finalize (GObject *object)
{
    GrlImageFont *self = GRL_IMAGE_FONT (object);

    g_clear_pointer (&self->font_data, g_free);
    self->font_len = 0;
    self->valid = FALSE;

    G_OBJECT_CLASS (grl_image_font_parent_class)->finalize (object);
}

static void
grl_image_font_class_init (GrlImageFontClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->finalize = grl_image_font_finalize;
}

static void
grl_image_font_init (GrlImageFont *self)
{
    self->font_data = NULL;
    self->font_len  = 0;
    self->valid     = FALSE;
    memset (&self->info, 0, sizeof self->info);
}

/* -------------------------------------------------------------------------
 * Internal: initialise stbtt_fontinfo from the already-copied buffer
 * -------------------------------------------------------------------------
 */

static gboolean
font_init_from_buffer (GrlImageFont  *self,
                       GError       **error)
{
    int offset;

    offset = stbtt_GetFontOffsetForIndex (self->font_data, 0);
    if (offset < 0)
    {
        g_set_error (error, GRL_IMAGE_FONT_ERROR,
                     GRL_IMAGE_FONT_ERROR_PARSE,
                     "Not a valid TrueType/OpenType font");
        return FALSE;
    }

    if (!stbtt_InitFont (&self->info, self->font_data, offset))
    {
        g_set_error (error, GRL_IMAGE_FONT_ERROR,
                     GRL_IMAGE_FONT_ERROR_PARSE,
                     "Failed to initialise font tables");
        return FALSE;
    }

    self->valid = TRUE;
    return TRUE;
}

/* -------------------------------------------------------------------------
 * Public constructors
 * -------------------------------------------------------------------------
 */

/**
 * grl_image_font_new_from_file:
 * @filename: (type filename): Path to a TTF or OTF font file.
 * @error: (nullable): Return location for error, or %NULL.
 *
 * Loads a TrueType or OpenType font from @filename.
 *
 * Returns: (transfer full) (nullable): A new #GrlImageFont, or %NULL on error.
 */
GrlImageFont *
grl_image_font_new_from_file (const gchar  *filename,
                               GError      **error)
{
    GrlImageFont *self;
    gchar        *buf;
    gsize         len;

    g_return_val_if_fail (filename != NULL, NULL);

    if (!g_file_get_contents (filename, &buf, &len, NULL))
    {
        g_set_error (error, GRL_IMAGE_FONT_ERROR,
                     GRL_IMAGE_FONT_ERROR_OPEN,
                     "Failed to read font file '%s'", filename);
        return NULL;
    }

    self = g_object_new (GRL_TYPE_IMAGE_FONT, NULL);
    self->font_data = (guint8 *)buf;  /* g_file_get_contents allocates with g_malloc */
    self->font_len  = len;

    if (!font_init_from_buffer (self, error))
    {
        g_object_unref (self);
        return NULL;
    }

    return self;
}

/**
 * grl_image_font_new_from_memory:
 * @data: (array length=len) (element-type guint8): Font file bytes.
 * @len: Number of bytes in @data.
 * @error: (nullable): Return location for error, or %NULL.
 *
 * Loads a font from an in-memory buffer.  @data is copied internally.
 *
 * Returns: (transfer full) (nullable): A new #GrlImageFont, or %NULL on error.
 */
GrlImageFont *
grl_image_font_new_from_memory (const guint8 *data,
                                 gsize         len,
                                 GError      **error)
{
    GrlImageFont *self;

    g_return_val_if_fail (data != NULL, NULL);
    g_return_val_if_fail (len > 0, NULL);

    self = g_object_new (GRL_TYPE_IMAGE_FONT, NULL);
    self->font_data = (guint8 *)g_memdup2 (data, len);
    self->font_len  = len;

    if (!font_init_from_buffer (self, error))
    {
        g_object_unref (self);
        return NULL;
    }

    return self;
}

/* -------------------------------------------------------------------------
 * Metrics
 * -------------------------------------------------------------------------
 */

/**
 * grl_image_font_get_v_metrics:
 * @self: A #GrlImageFont.
 * @px_size: Desired pixel height (em height, > 0).
 * @ascent: (out) (optional): Return location for the ascent in pixels.
 * @descent: (out) (optional): Return location for the descent in pixels (≤ 0).
 * @line_gap: (out) (optional): Return location for the inter-line gap.
 *
 * Queries the vertical metrics of the font scaled to @px_size.
 */
void
grl_image_font_get_v_metrics (GrlImageFont *self,
                               gfloat        px_size,
                               gfloat       *ascent,
                               gfloat       *descent,
                               gfloat       *line_gap)
{
    float  scale;
    int    raw_ascent;
    int    raw_descent;
    int    raw_line_gap;

    g_return_if_fail (GRL_IS_IMAGE_FONT (self));
    g_return_if_fail (self->valid);

    scale = stbtt_ScaleForPixelHeight (&self->info, px_size);
    stbtt_GetFontVMetrics (&self->info, &raw_ascent, &raw_descent, &raw_line_gap);

    if (ascent)
        *ascent   = (gfloat)raw_ascent   * scale;
    if (descent)
        *descent  = (gfloat)raw_descent  * scale;
    if (line_gap)
        *line_gap = (gfloat)raw_line_gap * scale;
}

/**
 * grl_image_font_has_glyph:
 * @self: A #GrlImageFont.
 * @codepoint: A Unicode codepoint.
 *
 * Returns %TRUE if the font contains a glyph for @codepoint.
 *
 * Returns: %TRUE if the glyph is present.
 */
gboolean
grl_image_font_has_glyph (GrlImageFont *self,
                           gunichar      codepoint)
{
    unsigned char raw;

    g_return_val_if_fail (GRL_IS_IMAGE_FONT (self), FALSE);
    g_return_val_if_fail (self->valid, FALSE);

    raw = (stbtt_FindGlyphIndex (&self->info, (int)codepoint) != 0) ? 1 : 0;
    return raw != 0;
}

/* -------------------------------------------------------------------------
 * Internal: measure helpers
 * -------------------------------------------------------------------------
 */

/* Advance width of a single codepoint (unscaled). */
static int
font_get_advance (GrlImageFont *self,
                  gunichar      cp)
{
    int advance;

    stbtt_GetCodepointHMetrics (&self->info, (int)cp, &advance, NULL);
    return advance;
}

/* Kern advance between two adjacent codepoints (unscaled). */
static int
font_get_kern (GrlImageFont *self,
               gunichar      cp1,
               gunichar      cp2)
{
    return stbtt_GetCodepointKernAdvance (&self->info, (int)cp1, (int)cp2);
}

/*
 * Measure all lines of @text and return the maximum advance width (in scaled
 * pixels), the line height (scaled), and the number of lines.
 */
static void
font_measure_lines (GrlImageFont *self,
                    const gchar  *text,
                    float         scale,
                    float        *out_max_width,
                    gint         *out_line_count)
{
    const gchar *p;
    const gchar *next_p;
    float        cur_width;
    float        max_width;
    gint         lines;
    gunichar     cp;
    gunichar     prev_cp;

    cur_width = 0.0f;
    max_width = 0.0f;
    lines     = 1;
    prev_cp   = 0;

    for (p = text; *p != '\0'; p = next_p)
    {
        cp = g_utf8_get_char (p);
        next_p = g_utf8_next_char (p);

        if (cp == (gunichar)'\r')
        {
            prev_cp = 0;
            continue;
        }

        if (cp == (gunichar)'\n')
        {
            if (cur_width > max_width)
                max_width = cur_width;
            cur_width = 0.0f;
            lines++;
            prev_cp = 0;
            continue;
        }

        /* Apply kerning from the previous glyph. */
        if (prev_cp != 0)
            cur_width += (float)font_get_kern (self, prev_cp, cp) * scale;

        cur_width += (float)font_get_advance (self, cp) * scale;
        prev_cp = cp;
    }

    if (cur_width > max_width)
        max_width = cur_width;

    if (out_max_width)
        *out_max_width = max_width;
    if (out_line_count)
        *out_line_count = lines;
}

/* -------------------------------------------------------------------------
 * Public: measure
 * -------------------------------------------------------------------------
 */

/**
 * grl_image_measure_text_ttf:
 * @font: A #GrlImageFont.
 * @text: (nullable): UTF-8 text to measure.
 * @px_size: Desired em height in pixels.
 *
 * Measures @text without rasterising.
 *
 * Returns: (transfer full): Bounding dimensions as a #GrlVector2 (.x = width,
 *   .y = height).  Free with grl_vector2_free().
 */
GrlVector2 *
grl_image_measure_text_ttf (GrlImageFont *font,
                             const gchar  *text,
                             gfloat        px_size)
{
    float  scale;
    float  line_height;
    float  max_width;
    gint   line_count;
    int    ascent;
    int    descent;
    int    line_gap;

    g_return_val_if_fail (GRL_IS_IMAGE_FONT (font), grl_vector2_new (0.0f, 0.0f));
    g_return_val_if_fail (font->valid, grl_vector2_new (0.0f, 0.0f));

    if (text == NULL || *text == '\0' || px_size <= 0.0f)
        return grl_vector2_new (0.0f, 0.0f);

    scale = stbtt_ScaleForPixelHeight (&font->info, px_size);
    stbtt_GetFontVMetrics (&font->info, &ascent, &descent, &line_gap);
    line_height = (float)(ascent - descent + line_gap) * scale;

    font_measure_lines (font, text, scale, &max_width, &line_count);

    return grl_vector2_new (max_width, line_height * (float)line_count);
}

/* -------------------------------------------------------------------------
 * Public: draw
 * -------------------------------------------------------------------------
 */

/**
 * grl_image_draw_text_ttf:
 * @self: A #GrlImage to draw onto.
 * @font: The #GrlImageFont to use.
 * @text: (nullable): UTF-8 text to render.
 * @x: Left edge of the text block, in destination-image pixels.
 * @y: Top edge of the first line's em box, in destination-image pixels.
 * @px_size: Desired em height in pixels (≤ 0 is a no-op).
 * @color: Foreground colour.
 *
 * Renders @text onto @self at (@x, @y) using @font at @px_size pixels.
 *
 * Glyphs are rasterised into a temporary RGBA layer then composited onto @self
 * with %GRL_PORTER_DUFF_SRC_OVER.  @self's clip rectangle and colour space are
 * honoured by the composite step; the per-image blend-mode is not consulted.
 */
void
grl_image_draw_text_ttf (GrlImage       *self,
                          GrlImageFont   *font,
                          const gchar    *text,
                          gint            x,
                          gint            y,
                          gfloat          px_size,
                          const GrlColor *color)
{
    float        scale;
    float        line_height;
    float        max_width;
    gint         line_count;
    int          raw_ascent;
    int          raw_descent;
    int          raw_line_gap;
    float        ascent_px;
    GrlImage    *layer;
    GrlColor    *transparent;
    const gchar *p;
    const gchar *next_p;
    float        pen_x;
    float        pen_y;
    gunichar     cp;
    gunichar     prev_cp;
    gint         layer_w;
    gint         layer_h;

    g_return_if_fail (GRL_IS_IMAGE (self));
    g_return_if_fail (GRL_IS_IMAGE_FONT (font));
    g_return_if_fail (font->valid);
    g_return_if_fail (color != NULL);

    if (text == NULL || *text == '\0' || px_size <= 0.0f)
        return;

    scale = stbtt_ScaleForPixelHeight (&font->info, px_size);
    stbtt_GetFontVMetrics (&font->info, &raw_ascent, &raw_descent, &raw_line_gap);
    ascent_px   = (float)raw_ascent * scale;
    line_height = (float)(raw_ascent - raw_descent + raw_line_gap) * scale;

    /* Measure to determine the required layer size. */
    font_measure_lines (font, text, scale, &max_width, &line_count);

    layer_w = (gint)ceilf (max_width);
    layer_h = (gint)ceilf (line_height * (float)line_count);

    if (layer_w <= 0 || layer_h <= 0)
        return;

    /* Create a fully-transparent RGBA layer. */
    transparent = grl_color_new (0, 0, 0, 0);
    layer = grl_image_new_color (layer_w, layer_h, transparent);
    grl_color_free (transparent);

    if (layer == NULL)
        return;

    /* Rasterise each glyph into the layer. */
    pen_x   = 0.0f;
    pen_y   = ascent_px;
    prev_cp = 0;

    for (p = text; *p != '\0'; p = next_p)
    {
        int      bm_w;
        int      bm_h;
        int      bm_xoff;
        int      bm_yoff;
        int      glyph_index;
        guint8  *bitmap;
        gint     px;
        gint     py;
        gint     bx;
        gint     by;

        cp     = g_utf8_get_char (p);
        next_p = g_utf8_next_char (p);

        if (cp == (gunichar)'\r')
        {
            prev_cp = 0;
            continue;
        }

        if (cp == (gunichar)'\n')
        {
            pen_x   = 0.0f;
            pen_y  += line_height;
            prev_cp = 0;
            continue;
        }

        /* Apply kerning from the previous glyph. */
        if (prev_cp != 0)
            pen_x += (float)font_get_kern (font, prev_cp, cp) * scale;

        glyph_index = stbtt_FindGlyphIndex (&font->info, (int)cp);

        /* Missing glyph: still advance by a space width (advance of 0x20 or
         * ~0.5 em), but render nothing. */
        if (glyph_index == 0)
        {
            int space_adv;

            stbtt_GetCodepointHMetrics (&font->info, 0x20, &space_adv, NULL);
            pen_x  += (float)space_adv * scale;
            prev_cp = cp;
            continue;
        }

        /* Rasterise to an 8-bit coverage bitmap. */
        bitmap = stbtt_GetCodepointBitmap (&font->info,
                                            scale, scale,
                                            (int)cp,
                                            &bm_w, &bm_h,
                                            &bm_xoff, &bm_yoff);

        if (bitmap != NULL)
        {
            /* Pixel origin in the layer: pen + stb's bearing offset. */
            px = (gint)roundf (pen_x) + bm_xoff;
            py = (gint)roundf (pen_y) + bm_yoff;

            for (by = 0; by < bm_h; by++)
            {
                for (bx = 0; bx < bm_w; bx++)
                {
                    guint8     coverage;
                    guint8     alpha;
                    GrlColor  *glyph_color;

                    coverage = bitmap[by * bm_w + bx];
                    if (coverage == 0)
                        continue;

                    /* Scale color alpha by coverage (integer multiply ÷ 255). */
                    alpha = (guint8)(((guint)color->a * (guint)coverage + 127u) / 255u);

                    glyph_color = grl_color_new (color->r, color->g, color->b, alpha);
                    grl_image_draw_pixel (layer, px + bx, py + by, glyph_color);
                    grl_color_free (glyph_color);
                }
            }

            stbtt_FreeBitmap (bitmap, NULL);
        }

        /* Advance pen by the glyph's advance width. */
        {
            int adv;

            stbtt_GetCodepointHMetrics (&font->info, (int)cp, &adv, NULL);
            pen_x += (float)adv * scale;
        }

        prev_cp = cp;
    }

    /* Composite the glyph layer onto the destination image. */
    grl_image_composite (self, layer, GRL_PORTER_DUFF_SRC_OVER, x, y);

    g_object_unref (layer);
}
