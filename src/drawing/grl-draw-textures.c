/* grl-draw-textures.c
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Texture drawing functions.
 */

#include "config.h"
#include "grl-draw.h"
#include "grl-fast2d-private.h"
#include <math.h>
#include <raylib.h>

/**
 * SECTION:grl-draw-textures
 * @title: Texture Drawing
 * @short_description: Functions for drawing textures
 *
 * Functions to draw #GrlTexture objects to the screen with various
 * transformations including rotation, scaling, and source rectangle
 * selection.
 *
 * All texture drawing must occur between grl_window_begin_drawing()
 * and grl_window_end_drawing() calls.
 *
 * Example:
 * |[<!-- language="C" -->
 * g_autoptr(GrlTexture) tex = grl_texture_new_from_file ("sprite.png");
 * g_autoptr(GrlColor) white = grl_color_new_white ();
 *
 * grl_window_begin_drawing (window);
 * grl_draw_clear_background (bg);
 * grl_draw_texture (tex, 100, 100, white);
 * grl_window_end_drawing (window);
 * ]|
 */

/*
 * Helper macros for type conversion
 */

#define GRL_TO_RAYLIB_COLOR(c) \
    ((Color){ .r = (c)->r, .g = (c)->g, .b = (c)->b, .a = (c)->a })

#define GRL_TO_RAYLIB_VECTOR2(v) \
    ((Vector2){ .x = (v)->x, .y = (v)->y })

#define GRL_TO_RAYLIB_RECTANGLE(r) \
    ((Rectangle){ .x = (r)->x, .y = (r)->y, .width = (r)->width, .height = (r)->height })

/*
 * =============================================================================
 * Texture Drawing Functions
 *
 * Every form ends in DrawTexturePro(); graylib computes the same source
 * and destination rectangles raylib's DrawTexture*() wrappers compute and
 * draws through the fast 2D path (grl-fast2d-private.h).
 * =============================================================================
 */

/* draw_texture_ex: DrawTextureEx()'s rectangles, then the fast path */
static void
draw_texture_ex (const Texture2D *tex,
                 Vector2          position,
                 gfloat           rotation,
                 gfloat           scale,
                 Color            tint)
{
    Rectangle source;
    Rectangle dest;
    Vector2   origin;

    source.x = 0.0f;
    source.y = 0.0f;
    source.width = (float) tex->width;
    source.height = (float) tex->height;
    dest.x = position.x;
    dest.y = position.y;
    dest.width = (float) tex->width * scale;
    dest.height = (float) tex->height * scale;
    origin.x = 0.0f;
    origin.y = 0.0f;
    grl_fast2d_draw_texture_pro (*tex, source, dest, origin, rotation, tint);
}

/*
 * grl_draw_texture:
 * @texture: Texture to draw
 * @x: X position
 * @y: Y position
 * @tint: Color tint (use white for no tint)
 *
 * Draws a texture at the specified position.
 */
void
grl_draw_texture (GrlTexture     *texture,
                  gint            x,
                  gint            y,
                  const GrlColor *tint)
{
    Texture2D *tex;
    Vector2    position;

    g_return_if_fail (GRL_IS_TEXTURE (texture));
    g_return_if_fail (tint != NULL);

    tex = (Texture2D *)grl_texture_get_handle (texture);
    position.x = (gfloat) x;
    position.y = (gfloat) y;
    draw_texture_ex (tex, position, 0.0f, 1.0f, GRL_TO_RAYLIB_COLOR (tint));
}

/*
 * grl_draw_texture_v:
 * @texture: Texture to draw
 * @position: Position vector
 * @tint: Color tint
 *
 * Draws a texture at a position defined by a vector.
 */
void
grl_draw_texture_v (GrlTexture       *texture,
                    const GrlVector2 *position,
                    const GrlColor   *tint)
{
    Texture2D *tex;

    g_return_if_fail (GRL_IS_TEXTURE (texture));
    g_return_if_fail (position != NULL);
    g_return_if_fail (tint != NULL);

    tex = (Texture2D *)grl_texture_get_handle (texture);
    draw_texture_ex (tex, GRL_TO_RAYLIB_VECTOR2 (position), 0.0f, 1.0f,
                     GRL_TO_RAYLIB_COLOR (tint));
}

/*
 * grl_draw_texture_ex:
 * @texture: Texture to draw
 * @position: Position vector
 * @rotation: Rotation in degrees
 * @scale: Scale factor
 * @tint: Color tint
 *
 * Draws a texture with rotation and scaling.
 */
void
grl_draw_texture_ex (GrlTexture       *texture,
                     const GrlVector2 *position,
                     gfloat            rotation,
                     gfloat            scale,
                     const GrlColor   *tint)
{
    Texture2D *tex;

    g_return_if_fail (GRL_IS_TEXTURE (texture));
    g_return_if_fail (position != NULL);
    g_return_if_fail (tint != NULL);

    tex = (Texture2D *)grl_texture_get_handle (texture);
    draw_texture_ex (tex, GRL_TO_RAYLIB_VECTOR2 (position), rotation, scale,
                     GRL_TO_RAYLIB_COLOR (tint));
}

/*
 * grl_draw_texture_rec:
 * @texture: Texture to draw
 * @source: Source rectangle (region of texture to draw)
 * @position: Position to draw at
 * @tint: Color tint
 *
 * Draws a portion of a texture defined by a source rectangle.
 * Useful for sprite sheets and texture atlases.
 */
void
grl_draw_texture_rec (GrlTexture         *texture,
                      const GrlRectangle *source,
                      const GrlVector2   *position,
                      const GrlColor     *tint)
{
    Texture2D *tex;
    Rectangle  dest;
    Vector2    origin;

    g_return_if_fail (GRL_IS_TEXTURE (texture));
    g_return_if_fail (source != NULL);
    g_return_if_fail (position != NULL);
    g_return_if_fail (tint != NULL);

    tex = (Texture2D *)grl_texture_get_handle (texture);

    /* DrawTextureRec(): the destination is the source's size */
    dest.x = position->x;
    dest.y = position->y;
    dest.width = fabsf (source->width);
    dest.height = fabsf (source->height);
    origin.x = 0.0f;
    origin.y = 0.0f;
    grl_fast2d_draw_texture_pro (*tex, GRL_TO_RAYLIB_RECTANGLE (source), dest,
                                 origin, 0.0f, GRL_TO_RAYLIB_COLOR (tint));
}

/*
 * grl_draw_texture_pro:
 * @texture: Texture to draw
 * @source: Source rectangle (region of texture)
 * @dest: Destination rectangle (where to draw)
 * @origin: Origin point for rotation
 * @rotation: Rotation in degrees
 * @tint: Color tint
 *
 * Draws a texture with full control over source, destination,
 * rotation origin, and rotation angle.
 *
 * This is the most flexible texture drawing function. The source
 * rectangle defines which part of the texture to draw, the destination
 * rectangle defines where and at what size to draw it, and the origin
 * defines the pivot point for rotation.
 */
void
grl_draw_texture_pro (GrlTexture         *texture,
                      const GrlRectangle *source,
                      const GrlRectangle *dest,
                      const GrlVector2   *origin,
                      gfloat              rotation,
                      const GrlColor     *tint)
{
    Texture2D *tex;

    g_return_if_fail (GRL_IS_TEXTURE (texture));
    g_return_if_fail (source != NULL);
    g_return_if_fail (dest != NULL);
    g_return_if_fail (origin != NULL);
    g_return_if_fail (tint != NULL);

    tex = (Texture2D *)grl_texture_get_handle (texture);
    grl_fast2d_draw_texture_pro (*tex,
                                 GRL_TO_RAYLIB_RECTANGLE (source),
                                 GRL_TO_RAYLIB_RECTANGLE (dest),
                                 GRL_TO_RAYLIB_VECTOR2 (origin),
                                 rotation,
                                 GRL_TO_RAYLIB_COLOR (tint));
}
