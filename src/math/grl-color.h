/* grl-color.h
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * RGBA color type with 8-bit components.
 */

#pragma once

#if !defined(GRAYLIB_INSIDE) && !defined(GRAYLIB_COMPILATION)
#error "Only <graylib.h> can be included directly."
#endif

#include <glib-object.h>
#include "grl-version.h"

G_BEGIN_DECLS

/* Forward declaration for this file */
typedef struct _GrlColor GrlColor;

#define GRL_TYPE_COLOR (grl_color_get_type ())

/**
 * GrlColor:
 * @r: Red component (0-255)
 * @g: Green component (0-255)
 * @b: Blue component (0-255)
 * @a: Alpha component (0-255, 255 = fully opaque)
 *
 * An RGBA color with 8-bit components.
 *
 * This is a GBoxed type that can be used in GObject properties
 * and signals, and is fully supported by GObject Introspection.
 */
struct _GrlColor
{
    guint8 r;
    guint8 g;
    guint8 b;
    guint8 a;
};

GRL_AVAILABLE_IN_ALL
GType           grl_color_get_type          (void) G_GNUC_CONST;

/*
 * Constructors
 */

GRL_AVAILABLE_IN_ALL
GrlColor *      grl_color_new               (guint8              r,
                                             guint8              g,
                                             guint8              b,
                                             guint8              a);

GRL_AVAILABLE_IN_ALL
GrlColor *      grl_color_new_rgb           (guint8              r,
                                             guint8              g,
                                             guint8              b);

GRL_AVAILABLE_IN_ALL
GrlColor *      grl_color_new_from_hsv      (gfloat              h,
                                             gfloat              s,
                                             gfloat              v);

GRL_AVAILABLE_IN_ALL
GrlColor *      grl_color_new_from_int      (guint32             hex_value);

GRL_AVAILABLE_IN_ALL
GrlColor *      grl_color_copy              (const GrlColor     *self);

GRL_AVAILABLE_IN_ALL
void            grl_color_free              (GrlColor           *self);

/*
 * Stack allocation helper
 */

/**
 * grl_color_init:
 * @r: Red component (0-255)
 * @g: Green component (0-255)
 * @b: Blue component (0-255)
 * @a: Alpha component (0-255)
 *
 * Creates a stack-allocated color.
 *
 * Returns: A stack-allocated #GrlColor
 */
static inline GrlColor
grl_color_init (guint8 r,
                guint8 g,
                guint8 b,
                guint8 a)
{
    GrlColor c;
    c.r = r;
    c.g = g;
    c.b = b;
    c.a = a;
    return c;
}

/*
 * Predefined Colors
 */

GRL_AVAILABLE_IN_ALL
GrlColor *      grl_color_new_white         (void);

GRL_AVAILABLE_IN_ALL
GrlColor *      grl_color_new_black         (void);

GRL_AVAILABLE_IN_ALL
GrlColor *      grl_color_new_blank         (void);

GRL_AVAILABLE_IN_ALL
GrlColor *      grl_color_new_red           (void);

GRL_AVAILABLE_IN_ALL
GrlColor *      grl_color_new_green         (void);

GRL_AVAILABLE_IN_ALL
GrlColor *      grl_color_new_blue          (void);

GRL_AVAILABLE_IN_ALL
GrlColor *      grl_color_new_yellow        (void);

GRL_AVAILABLE_IN_ALL
GrlColor *      grl_color_new_magenta       (void);

GRL_AVAILABLE_IN_ALL
GrlColor *      grl_color_new_cyan          (void);

GRL_AVAILABLE_IN_ALL
GrlColor *      grl_color_new_gray          (void);

GRL_AVAILABLE_IN_ALL
GrlColor *      grl_color_new_darkgray      (void);

GRL_AVAILABLE_IN_ALL
GrlColor *      grl_color_new_lightgray     (void);

GRL_AVAILABLE_IN_ALL
GrlColor *      grl_color_new_raywhite      (void);

/*
 * Accessors
 */

GRL_AVAILABLE_IN_ALL
guint8          grl_color_get_r             (const GrlColor     *self);

GRL_AVAILABLE_IN_ALL
guint8          grl_color_get_g             (const GrlColor     *self);

GRL_AVAILABLE_IN_ALL
guint8          grl_color_get_b             (const GrlColor     *self);

GRL_AVAILABLE_IN_ALL
guint8          grl_color_get_a             (const GrlColor     *self);

GRL_AVAILABLE_IN_ALL
void            grl_color_set               (GrlColor           *self,
                                             guint8              r,
                                             guint8              g,
                                             guint8              b,
                                             guint8              a);

/*
 * Operations
 */

GRL_AVAILABLE_IN_ALL
GrlColor *      grl_color_fade              (const GrlColor     *self,
                                             gfloat              alpha);

GRL_AVAILABLE_IN_ALL
GrlColor *      grl_color_tint              (const GrlColor     *self,
                                             const GrlColor     *tint);

GRL_AVAILABLE_IN_ALL
GrlColor *      grl_color_brightness        (const GrlColor     *self,
                                             gfloat              factor);

GRL_AVAILABLE_IN_ALL
GrlColor *      grl_color_contrast          (const GrlColor     *self,
                                             gfloat              contrast);

GRL_AVAILABLE_IN_ALL
GrlColor *      grl_color_alpha             (const GrlColor     *self,
                                             gfloat              alpha);

GRL_AVAILABLE_IN_ALL
GrlColor *      grl_color_alpha_blend       (const GrlColor     *dst,
                                             const GrlColor     *src);

GRL_AVAILABLE_IN_ALL
GrlColor *      grl_color_lerp              (const GrlColor     *a,
                                             const GrlColor     *b,
                                             gfloat              t);

GRL_AVAILABLE_IN_ALL
GrlColor *      grl_color_lerp_oklab        (const GrlColor     *a,
                                             const GrlColor     *b,
                                             gfloat              t);

/*
 * Conversions
 */

GRL_AVAILABLE_IN_ALL
guint32         grl_color_to_int            (const GrlColor     *self);

GRL_AVAILABLE_IN_ALL
void            grl_color_to_hsv            (const GrlColor     *self,
                                             gfloat             *h,
                                             gfloat             *s,
                                             gfloat             *v);

GRL_AVAILABLE_IN_ALL
void            grl_color_normalize         (const GrlColor     *self,
                                             gfloat             *r,
                                             gfloat             *g,
                                             gfloat             *b,
                                             gfloat             *a);

/*
 * Comparison
 */

GRL_AVAILABLE_IN_ALL
gboolean        grl_color_equal             (const GrlColor     *a,
                                             const GrlColor     *b);

/*
 * Utility
 */

GRL_AVAILABLE_IN_ALL
gchar *         grl_color_to_string         (const GrlColor     *self);

GRL_AVAILABLE_IN_ALL
gchar *         grl_color_to_hex            (const GrlColor     *self);

G_DEFINE_AUTOPTR_CLEANUP_FUNC (GrlColor, grl_color_free)

G_END_DECLS
