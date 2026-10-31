/* grl-text-layout-private.h
 *
 * Copyright 2026 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Fast text drawing and measuring shared by grl-draw-text.c and
 * grl-font.c. Not installed, not introspected.
 *
 * raylib's DrawTextEx() and MeasureTextEx() look every glyph up with
 * GetGlyphIndex(), a linear scan of the font's glyph table (DrawTextEx()
 * does it twice per glyph, once more inside DrawTextCodepoint()). For
 * the ASCII range that is dozens of comparisons per character, and it
 * dominates the CPU cost of text-heavy UIs. These helpers produce the
 * same quads and the same measurements with an O(1) lookup:
 *
 *  - Fonts raylib loads hold their glyphs in codepoint order from a base
 *    codepoint (32 for the default font and for LoadFontEx() without a
 *    codepoint list), so glyph (codepoint - glyphs[0].value) is checked
 *    first; on any mismatch the exact GetGlyphIndex() search runs, so
 *    unordered fonts still resolve exactly as raylib would.
 *  - Text containing '\n' goes to raylib unchanged: its line spacing is
 *    a raylib-private global with no getter.
 *
 * raylib's sources are not modified; this re-states the layout logic of
 * DrawTextEx()/DrawTextCodepoint()/MeasureTextEx() on top of its public
 * API.
 */

#pragma once

#include <string.h>
#include <glib.h>
#include <raylib.h>

G_BEGIN_DECLS

/*
 * grl_text_glyph_index:
 *
 * Same result as GetGlyphIndex() (the first glyph with this codepoint,
 * else the '?' fallback, else 0), using a direct index when the font's
 * glyph table is in codepoint order. @font must be valid.
 */
static inline gint
grl_text_glyph_index (const Font *font,
                      gint        codepoint)
{
    gint guess;

    guess = codepoint - font->glyphs[0].value;
    if (guess >= 0 && guess < font->glyphCount &&
        font->glyphs[guess].value == codepoint)
        return guess;

    return GetGlyphIndex (*font, codepoint);
}

/*
 * grl_text_font_usable:
 *
 * IsFontValid() through an 8-bit read (raylib returns a one-byte bool).
 */
static inline gboolean
grl_text_font_usable (const Font *font)
{
    unsigned char raw;

    raw = IsFontValid (*font);
    return raw != 0;
}

/*
 * grl_text_draw_ex:
 *
 * DrawTextEx() with one O(1) glyph lookup per character.
 */
static inline void
grl_text_draw_ex (Font         font,
                  const gchar *text,
                  Vector2      position,
                  gfloat       font_size,
                  gfloat       spacing,
                  Color        tint)
{
    gfloat offset_x;
    gfloat scale;
    gfloat pad;
    gint   size;
    gint   i;

    /* Security check in case of not valid font, as raylib does */
    if (font.texture.id == 0)
        font = GetFontDefault ();

    /* Multi-line text, or a font raylib would treat as invalid (every
     * glyph lookup then answers 0): let raylib handle it. */
    if (strchr (text, '\n') != NULL || !grl_text_font_usable (&font))
    {
        DrawTextEx (font, text, position, font_size, spacing, tint);
        return;
    }

    size = (gint) strlen (text);
    offset_x = 0.0f;
    scale = font_size / (gfloat) font.baseSize;
    pad = (gfloat) font.glyphPadding;

    for (i = 0; i < size;)
    {
        gint codepoint_bytes;
        gint codepoint;
        gint index;

        codepoint_bytes = 0;
        codepoint = GetCodepointNext (&text[i], &codepoint_bytes);
        index = grl_text_glyph_index (&font, codepoint);

        if (codepoint != ' ' && codepoint != '\t')
        {
            /* DrawTextCodepoint(): the glyph quad, padding included */
            Rectangle dst;
            Rectangle src;
            Vector2   origin;

            dst.x = position.x + offset_x + font.glyphs[index].offsetX * scale - pad * scale;
            dst.y = position.y + font.glyphs[index].offsetY * scale - pad * scale;
            dst.width = (font.recs[index].width + 2.0f * pad) * scale;
            dst.height = (font.recs[index].height + 2.0f * pad) * scale;

            src.x = font.recs[index].x - pad;
            src.y = font.recs[index].y - pad;
            src.width = font.recs[index].width + 2.0f * pad;
            src.height = font.recs[index].height + 2.0f * pad;

            origin.x = 0.0f;
            origin.y = 0.0f;
            DrawTexturePro (font.texture, src, dst, origin, 0.0f, tint);
        }

        if (font.glyphs[index].advanceX == 0)
            offset_x += ((gfloat) font.recs[index].width * scale + spacing);
        else
            offset_x += ((gfloat) font.glyphs[index].advanceX * scale + spacing);

        i += codepoint_bytes;
    }
}

/*
 * grl_text_draw_default:
 *
 * DrawText(): the default font, at least 10 px, spacing size / 10.
 */
static inline void
grl_text_draw_default (const gchar *text,
                       gint         x,
                       gint         y,
                       gint         font_size,
                       Color        color)
{
    Font    font;
    Vector2 position;

    font = GetFontDefault ();
    if (font.texture.id == 0)
        return;

    if (font_size < 10)
        font_size = 10;

    position.x = (gfloat) x;
    position.y = (gfloat) y;
    grl_text_draw_ex (font, text, position, (gfloat) font_size,
                      (gfloat) (font_size / 10), color);
}

/*
 * grl_text_measure_ex:
 *
 * MeasureTextEx() with one O(1) glyph lookup per character.
 */
static inline Vector2
grl_text_measure_ex (Font         font,
                     const gchar *text,
                     gfloat       font_size,
                     gfloat       spacing)
{
    Vector2 result;
    gfloat  width;
    gint    size;
    gint    count;
    gint    i;

    result.x = 0.0f;
    result.y = 0.0f;

    if (font.texture.id == 0 || text == NULL || text[0] == '\0')
        return result;

    if (strchr (text, '\n') != NULL || !grl_text_font_usable (&font))
        return MeasureTextEx (font, text, font_size, spacing);

    size = (gint) strlen (text);
    width = 0.0f;
    count = 0;

    for (i = 0; i < size;)
    {
        gint codepoint_bytes;
        gint codepoint;
        gint index;

        codepoint_bytes = 0;
        codepoint = GetCodepointNext (&text[i], &codepoint_bytes);
        index = grl_text_glyph_index (&font, codepoint);
        i += codepoint_bytes;
        count++;

        if (font.glyphs[index].advanceX > 0)
            width += font.glyphs[index].advanceX;
        else
            width += (font.recs[index].width + font.glyphs[index].offsetX);
    }

    result.x = width * (font_size / (gfloat) font.baseSize) +
               (gfloat) ((count - 1) * spacing);
    result.y = font_size;

    return result;
}

/*
 * grl_text_measure_default:
 *
 * MeasureText(): width of @text in the default font.
 */
static inline gint
grl_text_measure_default (const gchar *text,
                          gint         font_size)
{
    Font    font;
    Vector2 size;

    font = GetFontDefault ();
    if (font.texture.id == 0)
        return 0;

    if (font_size < 10)
        font_size = 10;

    size = grl_text_measure_ex (font, text, (gfloat) font_size,
                                (gfloat) (font_size / 10));
    return (gint) size.x;
}

G_END_DECLS
