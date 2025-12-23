/* grl-color.c
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Implementation of RGBA color type.
 */

#include "grl-color.h"
#include <math.h>

/**
 * SECTION:grl-color
 * @Title: GrlColor
 * @Short_description: An RGBA color type
 *
 * #GrlColor represents an RGBA color with 8-bit components (0-255).
 * It is used throughout Graylib for specifying colors for drawing,
 * tinting, and other visual effects.
 *
 * # Example
 *
 * |[<!-- language="C" -->
 * g_autoptr(GrlColor) bg = grl_color_new (40, 40, 60, 255);
 * g_autoptr(GrlColor) red = grl_color_new_red ();
 * g_autoptr(GrlColor) faded = grl_color_fade (red, 0.5f);
 * ]|
 */

G_DEFINE_BOXED_TYPE (GrlColor, grl_color,
                     grl_color_copy, grl_color_free)

/*
 * Constructors
 */

/**
 * grl_color_new:
 * @r: Red component (0-255)
 * @g: Green component (0-255)
 * @b: Blue component (0-255)
 * @a: Alpha component (0-255)
 *
 * Creates a new color with the given RGBA components.
 *
 * Returns: (transfer full): A newly allocated #GrlColor
 */
GrlColor *
grl_color_new (guint8 r,
               guint8 g,
               guint8 b,
               guint8 a)
{
    GrlColor *self;

    self = g_new (GrlColor, 1);
    self->r = r;
    self->g = g;
    self->b = b;
    self->a = a;

    return self;
}

/**
 * grl_color_new_rgb:
 * @r: Red component (0-255)
 * @g: Green component (0-255)
 * @b: Blue component (0-255)
 *
 * Creates a new fully opaque color with the given RGB components.
 *
 * Returns: (transfer full): A newly allocated #GrlColor
 */
GrlColor *
grl_color_new_rgb (guint8 r,
                   guint8 g,
                   guint8 b)
{
    return grl_color_new (r, g, b, 255);
}

/**
 * grl_color_new_from_hsv:
 * @h: Hue (0.0 to 360.0)
 * @s: Saturation (0.0 to 1.0)
 * @v: Value/Brightness (0.0 to 1.0)
 *
 * Creates a new color from HSV values.
 *
 * Returns: (transfer full): A newly allocated #GrlColor
 */
GrlColor *
grl_color_new_from_hsv (gfloat h,
                        gfloat s,
                        gfloat v)
{
    gfloat c;
    gfloat x;
    gfloat m;
    gfloat r;
    gfloat g;
    gfloat b;
    gint   hi;

    /* Normalize hue to 0-360 range */
    while (h < 0.0f)
        h += 360.0f;
    while (h >= 360.0f)
        h -= 360.0f;

    c = v * s;
    hi = (gint)(h / 60.0f) % 6;
    x = c * (1.0f - fabsf (fmodf (h / 60.0f, 2.0f) - 1.0f));
    m = v - c;

    switch (hi)
    {
    case 0:
        r = c; g = x; b = 0;
        break;
    case 1:
        r = x; g = c; b = 0;
        break;
    case 2:
        r = 0; g = c; b = x;
        break;
    case 3:
        r = 0; g = x; b = c;
        break;
    case 4:
        r = x; g = 0; b = c;
        break;
    case 5:
    default:
        r = c; g = 0; b = x;
        break;
    }

    return grl_color_new (
        (guint8)((r + m) * 255.0f),
        (guint8)((g + m) * 255.0f),
        (guint8)((b + m) * 255.0f),
        255
    );
}

/**
 * grl_color_new_from_int:
 * @hex_value: Color as 0xRRGGBBAA
 *
 * Creates a new color from a 32-bit integer.
 *
 * Returns: (transfer full): A newly allocated #GrlColor
 */
GrlColor *
grl_color_new_from_int (guint32 hex_value)
{
    return grl_color_new (
        (guint8)((hex_value >> 24) & 0xFF),
        (guint8)((hex_value >> 16) & 0xFF),
        (guint8)((hex_value >> 8) & 0xFF),
        (guint8)(hex_value & 0xFF)
    );
}

/**
 * grl_color_copy:
 * @self: (nullable): A #GrlColor
 *
 * Creates a copy of the color.
 *
 * Returns: (transfer full) (nullable): A copy of @self, or %NULL
 */
GrlColor *
grl_color_copy (const GrlColor *self)
{
    if (self == NULL)
        return NULL;

    return grl_color_new (self->r, self->g, self->b, self->a);
}

/**
 * grl_color_free:
 * @self: (nullable): A #GrlColor
 *
 * Frees a color allocated with grl_color_new() or grl_color_copy().
 */
void
grl_color_free (GrlColor *self)
{
    g_free (self);
}

/*
 * Predefined Colors
 */

/**
 * grl_color_new_white:
 *
 * Creates a new white color (255, 255, 255, 255).
 *
 * Returns: (transfer full): A newly allocated #GrlColor
 */
GrlColor *
grl_color_new_white (void)
{
    return grl_color_new (255, 255, 255, 255);
}

/**
 * grl_color_new_black:
 *
 * Creates a new black color (0, 0, 0, 255).
 *
 * Returns: (transfer full): A newly allocated #GrlColor
 */
GrlColor *
grl_color_new_black (void)
{
    return grl_color_new (0, 0, 0, 255);
}

/**
 * grl_color_new_blank:
 *
 * Creates a new transparent color (0, 0, 0, 0).
 *
 * Returns: (transfer full): A newly allocated #GrlColor
 */
GrlColor *
grl_color_new_blank (void)
{
    return grl_color_new (0, 0, 0, 0);
}

/**
 * grl_color_new_red:
 *
 * Creates a new red color (230, 41, 55, 255).
 *
 * Returns: (transfer full): A newly allocated #GrlColor
 */
GrlColor *
grl_color_new_red (void)
{
    return grl_color_new (230, 41, 55, 255);
}

/**
 * grl_color_new_green:
 *
 * Creates a new green color (0, 228, 48, 255).
 *
 * Returns: (transfer full): A newly allocated #GrlColor
 */
GrlColor *
grl_color_new_green (void)
{
    return grl_color_new (0, 228, 48, 255);
}

/**
 * grl_color_new_blue:
 *
 * Creates a new blue color (0, 121, 241, 255).
 *
 * Returns: (transfer full): A newly allocated #GrlColor
 */
GrlColor *
grl_color_new_blue (void)
{
    return grl_color_new (0, 121, 241, 255);
}

/**
 * grl_color_new_yellow:
 *
 * Creates a new yellow color (253, 249, 0, 255).
 *
 * Returns: (transfer full): A newly allocated #GrlColor
 */
GrlColor *
grl_color_new_yellow (void)
{
    return grl_color_new (253, 249, 0, 255);
}

/**
 * grl_color_new_magenta:
 *
 * Creates a new magenta color (255, 0, 255, 255).
 *
 * Returns: (transfer full): A newly allocated #GrlColor
 */
GrlColor *
grl_color_new_magenta (void)
{
    return grl_color_new (255, 0, 255, 255);
}

/**
 * grl_color_new_cyan:
 *
 * Creates a new cyan color (0, 255, 255, 255).
 *
 * Returns: (transfer full): A newly allocated #GrlColor
 */
GrlColor *
grl_color_new_cyan (void)
{
    return grl_color_new (0, 255, 255, 255);
}

/**
 * grl_color_new_gray:
 *
 * Creates a new gray color (130, 130, 130, 255).
 *
 * Returns: (transfer full): A newly allocated #GrlColor
 */
GrlColor *
grl_color_new_gray (void)
{
    return grl_color_new (130, 130, 130, 255);
}

/**
 * grl_color_new_darkgray:
 *
 * Creates a new dark gray color (80, 80, 80, 255).
 *
 * Returns: (transfer full): A newly allocated #GrlColor
 */
GrlColor *
grl_color_new_darkgray (void)
{
    return grl_color_new (80, 80, 80, 255);
}

/**
 * grl_color_new_lightgray:
 *
 * Creates a new light gray color (200, 200, 200, 255).
 *
 * Returns: (transfer full): A newly allocated #GrlColor
 */
GrlColor *
grl_color_new_lightgray (void)
{
    return grl_color_new (200, 200, 200, 255);
}

/**
 * grl_color_new_raywhite:
 *
 * Creates a new raylib-style white color (245, 245, 245, 255).
 *
 * Returns: (transfer full): A newly allocated #GrlColor
 */
GrlColor *
grl_color_new_raywhite (void)
{
    return grl_color_new (245, 245, 245, 255);
}

/*
 * Accessors
 */

/**
 * grl_color_get_r:
 * @self: A #GrlColor
 *
 * Gets the red component.
 *
 * Returns: The red component (0-255)
 */
guint8
grl_color_get_r (const GrlColor *self)
{
    g_return_val_if_fail (self != NULL, 0);
    return self->r;
}

/**
 * grl_color_get_g:
 * @self: A #GrlColor
 *
 * Gets the green component.
 *
 * Returns: The green component (0-255)
 */
guint8
grl_color_get_g (const GrlColor *self)
{
    g_return_val_if_fail (self != NULL, 0);
    return self->g;
}

/**
 * grl_color_get_b:
 * @self: A #GrlColor
 *
 * Gets the blue component.
 *
 * Returns: The blue component (0-255)
 */
guint8
grl_color_get_b (const GrlColor *self)
{
    g_return_val_if_fail (self != NULL, 0);
    return self->b;
}

/**
 * grl_color_get_a:
 * @self: A #GrlColor
 *
 * Gets the alpha component.
 *
 * Returns: The alpha component (0-255)
 */
guint8
grl_color_get_a (const GrlColor *self)
{
    g_return_val_if_fail (self != NULL, 0);
    return self->a;
}

/**
 * grl_color_set:
 * @self: A #GrlColor
 * @r: Red component (0-255)
 * @g: Green component (0-255)
 * @b: Blue component (0-255)
 * @a: Alpha component (0-255)
 *
 * Sets all components of the color.
 */
void
grl_color_set (GrlColor *self,
               guint8    r,
               guint8    g,
               guint8    b,
               guint8    a)
{
    g_return_if_fail (self != NULL);

    self->r = r;
    self->g = g;
    self->b = b;
    self->a = a;
}

/*
 * Operations
 */

/**
 * grl_color_fade:
 * @self: A #GrlColor
 * @alpha: Alpha multiplier (0.0 to 1.0)
 *
 * Creates a faded version of the color by multiplying its alpha.
 *
 * Returns: (transfer full): A new faded color
 */
GrlColor *
grl_color_fade (const GrlColor *self,
                gfloat          alpha)
{
    g_return_val_if_fail (self != NULL, NULL);

    alpha = CLAMP (alpha, 0.0f, 1.0f);

    return grl_color_new (
        self->r,
        self->g,
        self->b,
        (guint8)(self->a * alpha)
    );
}

/**
 * grl_color_tint:
 * @self: A #GrlColor
 * @tint: Tint color
 *
 * Tints the color by multiplying with another color.
 *
 * Returns: (transfer full): A new tinted color
 */
GrlColor *
grl_color_tint (const GrlColor *self,
                const GrlColor *tint)
{
    g_return_val_if_fail (self != NULL, NULL);
    g_return_val_if_fail (tint != NULL, NULL);

    return grl_color_new (
        (guint8)((self->r * tint->r) / 255),
        (guint8)((self->g * tint->g) / 255),
        (guint8)((self->b * tint->b) / 255),
        (guint8)((self->a * tint->a) / 255)
    );
}

/**
 * grl_color_brightness:
 * @self: A #GrlColor
 * @factor: Brightness factor (negative = darker, positive = brighter)
 *
 * Adjusts the brightness of the color.
 *
 * Returns: (transfer full): A new adjusted color
 */
GrlColor *
grl_color_brightness (const GrlColor *self,
                      gfloat          factor)
{
    gint r;
    gint g;
    gint b;

    g_return_val_if_fail (self != NULL, NULL);

    factor = CLAMP (factor, -1.0f, 1.0f);

    if (factor < 0.0f)
    {
        factor = 1.0f + factor;
        r = (gint)(self->r * factor);
        g = (gint)(self->g * factor);
        b = (gint)(self->b * factor);
    }
    else
    {
        r = (gint)((255 - self->r) * factor + self->r);
        g = (gint)((255 - self->g) * factor + self->g);
        b = (gint)((255 - self->b) * factor + self->b);
    }

    return grl_color_new (
        (guint8)CLAMP (r, 0, 255),
        (guint8)CLAMP (g, 0, 255),
        (guint8)CLAMP (b, 0, 255),
        self->a
    );
}

/**
 * grl_color_contrast:
 * @self: A #GrlColor
 * @contrast: Contrast factor (-1.0 to 1.0)
 *
 * Adjusts the contrast of the color.
 *
 * Returns: (transfer full): A new adjusted color
 */
GrlColor *
grl_color_contrast (const GrlColor *self,
                    gfloat          contrast)
{
    gfloat factor;
    gint   r;
    gint   g;
    gint   b;

    g_return_val_if_fail (self != NULL, NULL);

    contrast = CLAMP (contrast, -1.0f, 1.0f);
    contrast = (1.0f + contrast);
    contrast *= contrast;

    r = (gint)(((self->r / 255.0f - 0.5f) * contrast + 0.5f) * 255.0f);
    g = (gint)(((self->g / 255.0f - 0.5f) * contrast + 0.5f) * 255.0f);
    b = (gint)(((self->b / 255.0f - 0.5f) * contrast + 0.5f) * 255.0f);

    return grl_color_new (
        (guint8)CLAMP (r, 0, 255),
        (guint8)CLAMP (g, 0, 255),
        (guint8)CLAMP (b, 0, 255),
        self->a
    );
}

/**
 * grl_color_alpha:
 * @self: A #GrlColor
 * @alpha: New alpha value (0.0 to 1.0)
 *
 * Creates a copy with a new alpha value.
 *
 * Returns: (transfer full): A new color with the specified alpha
 */
GrlColor *
grl_color_alpha (const GrlColor *self,
                 gfloat          alpha)
{
    g_return_val_if_fail (self != NULL, NULL);

    alpha = CLAMP (alpha, 0.0f, 1.0f);

    return grl_color_new (
        self->r,
        self->g,
        self->b,
        (guint8)(alpha * 255.0f)
    );
}

/**
 * grl_color_alpha_blend:
 * @dst: Destination color
 * @src: Source color (to blend on top)
 *
 * Blends the source color on top of the destination using alpha blending.
 *
 * Returns: (transfer full): A new blended color
 */
GrlColor *
grl_color_alpha_blend (const GrlColor *dst,
                       const GrlColor *src)
{
    gfloat src_alpha;
    gfloat dst_alpha;
    gfloat out_alpha;
    guint8 r;
    guint8 g;
    guint8 b;
    guint8 a;

    g_return_val_if_fail (dst != NULL, NULL);
    g_return_val_if_fail (src != NULL, NULL);

    src_alpha = src->a / 255.0f;
    dst_alpha = dst->a / 255.0f;
    out_alpha = src_alpha + dst_alpha * (1.0f - src_alpha);

    if (out_alpha > 0.0f)
    {
        r = (guint8)((src->r * src_alpha + dst->r * dst_alpha * (1.0f - src_alpha)) / out_alpha);
        g = (guint8)((src->g * src_alpha + dst->g * dst_alpha * (1.0f - src_alpha)) / out_alpha);
        b = (guint8)((src->b * src_alpha + dst->b * dst_alpha * (1.0f - src_alpha)) / out_alpha);
        a = (guint8)(out_alpha * 255.0f);
    }
    else
    {
        r = g = b = a = 0;
    }

    return grl_color_new (r, g, b, a);
}

/**
 * grl_color_lerp:
 * @a: Start color
 * @b: End color
 * @t: Interpolation factor (0.0 to 1.0)
 *
 * Linearly interpolates between two colors.
 *
 * Returns: (transfer full): A new interpolated color
 */
GrlColor *
grl_color_lerp (const GrlColor *a,
                const GrlColor *b,
                gfloat          t)
{
    g_return_val_if_fail (a != NULL, NULL);
    g_return_val_if_fail (b != NULL, NULL);

    t = CLAMP (t, 0.0f, 1.0f);

    return grl_color_new (
        (guint8)(a->r + t * (b->r - a->r)),
        (guint8)(a->g + t * (b->g - a->g)),
        (guint8)(a->b + t * (b->b - a->b)),
        (guint8)(a->a + t * (b->a - a->a))
    );
}

/*
 * Conversions
 */

/**
 * grl_color_to_int:
 * @self: A #GrlColor
 *
 * Converts the color to a 32-bit integer (0xRRGGBBAA).
 *
 * Returns: The color as an integer
 */
guint32
grl_color_to_int (const GrlColor *self)
{
    g_return_val_if_fail (self != NULL, 0);

    return ((guint32)self->r << 24) |
           ((guint32)self->g << 16) |
           ((guint32)self->b << 8) |
           (guint32)self->a;
}

/**
 * grl_color_to_hsv:
 * @self: A #GrlColor
 * @h: (out) (optional): Return location for hue (0-360)
 * @s: (out) (optional): Return location for saturation (0-1)
 * @v: (out) (optional): Return location for value (0-1)
 *
 * Converts the color to HSV values.
 */
void
grl_color_to_hsv (const GrlColor *self,
                  gfloat         *h,
                  gfloat         *s,
                  gfloat         *v)
{
    gfloat r_norm;
    gfloat g_norm;
    gfloat b_norm;
    gfloat max_val;
    gfloat min_val;
    gfloat delta;

    g_return_if_fail (self != NULL);

    r_norm = self->r / 255.0f;
    g_norm = self->g / 255.0f;
    b_norm = self->b / 255.0f;

    max_val = MAX (MAX (r_norm, g_norm), b_norm);
    min_val = MIN (MIN (r_norm, g_norm), b_norm);
    delta = max_val - min_val;

    if (v != NULL)
        *v = max_val;

    if (s != NULL)
        *s = (max_val > 0.0f) ? (delta / max_val) : 0.0f;

    if (h != NULL)
    {
        if (delta == 0.0f)
        {
            *h = 0.0f;
        }
        else if (max_val == r_norm)
        {
            *h = 60.0f * fmodf ((g_norm - b_norm) / delta, 6.0f);
        }
        else if (max_val == g_norm)
        {
            *h = 60.0f * ((b_norm - r_norm) / delta + 2.0f);
        }
        else
        {
            *h = 60.0f * ((r_norm - g_norm) / delta + 4.0f);
        }

        if (*h < 0.0f)
            *h += 360.0f;
    }
}

/**
 * grl_color_normalize:
 * @self: A #GrlColor
 * @r: (out) (optional): Return location for red (0-1)
 * @g: (out) (optional): Return location for green (0-1)
 * @b: (out) (optional): Return location for blue (0-1)
 * @a: (out) (optional): Return location for alpha (0-1)
 *
 * Converts the color to normalized floating-point values.
 */
void
grl_color_normalize (const GrlColor *self,
                     gfloat         *r,
                     gfloat         *g,
                     gfloat         *b,
                     gfloat         *a)
{
    g_return_if_fail (self != NULL);

    if (r != NULL)
        *r = self->r / 255.0f;
    if (g != NULL)
        *g = self->g / 255.0f;
    if (b != NULL)
        *b = self->b / 255.0f;
    if (a != NULL)
        *a = self->a / 255.0f;
}

/*
 * Comparison
 */

/**
 * grl_color_equal:
 * @a: First color
 * @b: Second color
 *
 * Checks if two colors are equal.
 *
 * Returns: %TRUE if the colors are equal
 */
gboolean
grl_color_equal (const GrlColor *a,
                 const GrlColor *b)
{
    if (a == b)
        return TRUE;
    if (a == NULL || b == NULL)
        return FALSE;

    return a->r == b->r &&
           a->g == b->g &&
           a->b == b->b &&
           a->a == b->a;
}

/*
 * Utility
 */

/**
 * grl_color_to_string:
 * @self: A #GrlColor
 *
 * Creates a string representation of the color.
 *
 * Returns: (transfer full): A newly allocated string
 */
gchar *
grl_color_to_string (const GrlColor *self)
{
    g_return_val_if_fail (self != NULL, NULL);

    return g_strdup_printf ("GrlColor(%u, %u, %u, %u)",
                            self->r, self->g, self->b, self->a);
}

/**
 * grl_color_to_hex:
 * @self: A #GrlColor
 *
 * Creates a hex string representation of the color (#RRGGBBAA).
 *
 * Returns: (transfer full): A newly allocated string
 */
gchar *
grl_color_to_hex (const GrlColor *self)
{
    g_return_val_if_fail (self != NULL, NULL);

    return g_strdup_printf ("#%02X%02X%02X%02X",
                            self->r, self->g, self->b, self->a);
}
