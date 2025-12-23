/* grl-draw-text.c
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Text drawing functions.
 */

#include "config.h"
#include "grl-draw.h"
#include <raylib.h>

/**
 * SECTION:grl-draw-text
 * @title: Text Drawing
 * @short_description: Functions for drawing text
 *
 * Functions to draw text using either the default font or custom
 * #GrlFont objects. Supports various text rendering options including
 * rotation, custom spacing, and Unicode codepoints.
 *
 * All text drawing must occur between grl_window_begin_drawing()
 * and grl_window_end_drawing() calls.
 *
 * Example - Simple text:
 * |[<!-- language="C" -->
 * g_autoptr(GrlColor) white = grl_color_new_white ();
 *
 * grl_draw_text ("Hello World!", 100, 100, 20, white);
 * ]|
 *
 * Example - Custom font:
 * |[<!-- language="C" -->
 * g_autoptr(GrlFont) font = grl_font_new_from_file ("fonts/arial.ttf");
 * g_autoptr(GrlVector2) pos = grl_vector2_new (100.0f, 100.0f);
 * g_autoptr(GrlColor) white = grl_color_new_white ();
 *
 * grl_draw_text_ex (font, "Hello World!", pos, 24.0f, 2.0f, white);
 * ]|
 */

/*
 * Helper macros for type conversion
 */

#define GRL_TO_RAYLIB_COLOR(c) \
    ((Color){ .r = (c)->r, .g = (c)->g, .b = (c)->b, .a = (c)->a })

#define GRL_TO_RAYLIB_VECTOR2(v) \
    ((Vector2){ .x = (v)->x, .y = (v)->y })

/*
 * =============================================================================
 * Text Drawing Functions
 * =============================================================================
 */

/**
 * grl_draw_fps:
 * @x: X position
 * @y: Y position
 *
 * Draws the current FPS value at the specified position.
 * Uses the default font with a green color.
 */
void
grl_draw_fps (gint x,
              gint y)
{
    DrawFPS (x, y);
}

/**
 * grl_draw_text:
 * @text: Text to draw
 * @x: X position
 * @y: Y position
 * @font_size: Font size in pixels
 * @color: Text color
 *
 * Draws text using the default raylib font.
 * For more control, use grl_draw_text_ex() with a custom font.
 */
void
grl_draw_text (const gchar    *text,
               gint            x,
               gint            y,
               gint            font_size,
               const GrlColor *color)
{
    g_return_if_fail (text != NULL);
    g_return_if_fail (color != NULL);

    DrawText (text, x, y, font_size, GRL_TO_RAYLIB_COLOR (color));
}

/**
 * grl_draw_text_ex:
 * @font: Font to use
 * @text: Text to draw
 * @position: Position vector
 * @font_size: Font size in pixels
 * @spacing: Character spacing (additional space between characters)
 * @tint: Text color
 *
 * Draws text using a custom font with control over size and spacing.
 */
void
grl_draw_text_ex (GrlFont          *font,
                  const gchar      *text,
                  const GrlVector2 *position,
                  gfloat            font_size,
                  gfloat            spacing,
                  const GrlColor   *tint)
{
    Font *fnt;

    g_return_if_fail (GRL_IS_FONT (font));
    g_return_if_fail (text != NULL);
    g_return_if_fail (position != NULL);
    g_return_if_fail (tint != NULL);

    fnt = (Font *)grl_font_get_handle (font);
    DrawTextEx (*fnt,
                text,
                GRL_TO_RAYLIB_VECTOR2 (position),
                font_size,
                spacing,
                GRL_TO_RAYLIB_COLOR (tint));
}

/**
 * grl_draw_text_pro:
 * @font: Font to use
 * @text: Text to draw
 * @position: Position vector
 * @origin: Origin point for rotation (relative to position)
 * @rotation: Rotation in degrees
 * @font_size: Font size in pixels
 * @spacing: Character spacing
 * @tint: Text color
 *
 * Draws text with rotation support. The origin point defines
 * the pivot point for rotation relative to the text position.
 */
void
grl_draw_text_pro (GrlFont          *font,
                   const gchar      *text,
                   const GrlVector2 *position,
                   const GrlVector2 *origin,
                   gfloat            rotation,
                   gfloat            font_size,
                   gfloat            spacing,
                   const GrlColor   *tint)
{
    Font *fnt;

    g_return_if_fail (GRL_IS_FONT (font));
    g_return_if_fail (text != NULL);
    g_return_if_fail (position != NULL);
    g_return_if_fail (origin != NULL);
    g_return_if_fail (tint != NULL);

    fnt = (Font *)grl_font_get_handle (font);
    DrawTextPro (*fnt,
                 text,
                 GRL_TO_RAYLIB_VECTOR2 (position),
                 GRL_TO_RAYLIB_VECTOR2 (origin),
                 rotation,
                 font_size,
                 spacing,
                 GRL_TO_RAYLIB_COLOR (tint));
}

/**
 * grl_draw_text_codepoint:
 * @font: Font to use
 * @codepoint: Unicode codepoint to draw
 * @position: Position vector
 * @font_size: Font size in pixels
 * @tint: Text color
 *
 * Draws a single character identified by its Unicode codepoint.
 * Useful for drawing special characters or when iterating through
 * text manually.
 */
void
grl_draw_text_codepoint (GrlFont          *font,
                         gint              codepoint,
                         const GrlVector2 *position,
                         gfloat            font_size,
                         const GrlColor   *tint)
{
    Font *fnt;

    g_return_if_fail (GRL_IS_FONT (font));
    g_return_if_fail (position != NULL);
    g_return_if_fail (tint != NULL);

    fnt = (Font *)grl_font_get_handle (font);
    DrawTextCodepoint (*fnt,
                       codepoint,
                       GRL_TO_RAYLIB_VECTOR2 (position),
                       font_size,
                       GRL_TO_RAYLIB_COLOR (tint));
}

/**
 * grl_measure_text:
 * @text: Text to measure
 * @font_size: Font size in pixels
 *
 * Measures the width of text when rendered with the default font.
 * For measuring with a custom font, use grl_font_measure_text().
 *
 * Returns: Text width in pixels
 */
gint
grl_measure_text (const gchar *text,
                  gint         font_size)
{
    g_return_val_if_fail (text != NULL, 0);

    return MeasureText (text, font_size);
}
