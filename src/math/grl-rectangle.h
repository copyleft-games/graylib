/* grl-rectangle.h
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * 2D axis-aligned rectangle type.
 */

#pragma once

#if !defined(GRAYLIB_INSIDE) && !defined(GRAYLIB_COMPILATION)
#error "Only <graylib.h> can be included directly."
#endif

#include <glib-object.h>
#include "grl-version.h"
#include "grl-vector2.h"

G_BEGIN_DECLS

/* Forward declaration for this file */
typedef struct _GrlRectangle GrlRectangle;

#define GRL_TYPE_RECTANGLE (grl_rectangle_get_type ())

/**
 * GrlRectangle:
 * @x: X position of top-left corner
 * @y: Y position of top-left corner
 * @width: Width of the rectangle
 * @height: Height of the rectangle
 *
 * A 2D axis-aligned rectangle defined by position and size.
 *
 * This is a GBoxed type that can be used in GObject properties
 * and signals, and is fully supported by GObject Introspection.
 */
struct _GrlRectangle
{
    gfloat x;
    gfloat y;
    gfloat width;
    gfloat height;
};

GRL_AVAILABLE_IN_ALL
GType               grl_rectangle_get_type      (void) G_GNUC_CONST;

/*
 * Constructors
 */

GRL_AVAILABLE_IN_ALL
GrlRectangle *      grl_rectangle_new           (gfloat              x,
                                                 gfloat              y,
                                                 gfloat              width,
                                                 gfloat              height);

GRL_AVAILABLE_IN_ALL
GrlRectangle *      grl_rectangle_new_empty     (void);

GRL_AVAILABLE_IN_ALL
GrlRectangle *      grl_rectangle_new_from_vectors (const GrlVector2 *position,
                                                    const GrlVector2 *size);

GRL_AVAILABLE_IN_ALL
GrlRectangle *      grl_rectangle_new_from_corners (const GrlVector2 *top_left,
                                                    const GrlVector2 *bottom_right);

GRL_AVAILABLE_IN_ALL
GrlRectangle *      grl_rectangle_copy          (const GrlRectangle *self);

GRL_AVAILABLE_IN_ALL
void                grl_rectangle_free          (GrlRectangle       *self);

/*
 * Stack allocation helper
 */

/**
 * grl_rectangle_init:
 * @x: X position
 * @y: Y position
 * @width: Width
 * @height: Height
 *
 * Creates a stack-allocated rectangle.
 *
 * Returns: A stack-allocated #GrlRectangle
 */
static inline GrlRectangle
grl_rectangle_init (gfloat x,
                    gfloat y,
                    gfloat width,
                    gfloat height)
{
    GrlRectangle r;
    r.x = x;
    r.y = y;
    r.width = width;
    r.height = height;
    return r;
}

/*
 * Accessors
 */

GRL_AVAILABLE_IN_ALL
gfloat              grl_rectangle_get_x         (const GrlRectangle *self);

GRL_AVAILABLE_IN_ALL
gfloat              grl_rectangle_get_y         (const GrlRectangle *self);

GRL_AVAILABLE_IN_ALL
gfloat              grl_rectangle_get_width     (const GrlRectangle *self);

GRL_AVAILABLE_IN_ALL
gfloat              grl_rectangle_get_height    (const GrlRectangle *self);

GRL_AVAILABLE_IN_ALL
void                grl_rectangle_set           (GrlRectangle       *self,
                                                 gfloat              x,
                                                 gfloat              y,
                                                 gfloat              width,
                                                 gfloat              height);

/*
 * Derived properties
 */

GRL_AVAILABLE_IN_ALL
gfloat              grl_rectangle_get_left      (const GrlRectangle *self);

GRL_AVAILABLE_IN_ALL
gfloat              grl_rectangle_get_right     (const GrlRectangle *self);

GRL_AVAILABLE_IN_ALL
gfloat              grl_rectangle_get_top       (const GrlRectangle *self);

GRL_AVAILABLE_IN_ALL
gfloat              grl_rectangle_get_bottom    (const GrlRectangle *self);

GRL_AVAILABLE_IN_ALL
GrlVector2 *        grl_rectangle_get_position  (const GrlRectangle *self);

GRL_AVAILABLE_IN_ALL
GrlVector2 *        grl_rectangle_get_size      (const GrlRectangle *self);

GRL_AVAILABLE_IN_ALL
GrlVector2 *        grl_rectangle_get_center    (const GrlRectangle *self);

GRL_AVAILABLE_IN_ALL
gfloat              grl_rectangle_get_area      (const GrlRectangle *self);

GRL_AVAILABLE_IN_ALL
gfloat              grl_rectangle_get_perimeter (const GrlRectangle *self);

/*
 * Operations
 */

GRL_AVAILABLE_IN_ALL
GrlRectangle *      grl_rectangle_expand        (const GrlRectangle *self,
                                                 gfloat              amount);

GRL_AVAILABLE_IN_ALL
GrlRectangle *      grl_rectangle_expand_xy     (const GrlRectangle *self,
                                                 gfloat              h_amount,
                                                 gfloat              v_amount);

GRL_AVAILABLE_IN_ALL
GrlRectangle *      grl_rectangle_translate     (const GrlRectangle *self,
                                                 gfloat              dx,
                                                 gfloat              dy);

GRL_AVAILABLE_IN_ALL
GrlRectangle *      grl_rectangle_translate_v   (const GrlRectangle *self,
                                                 const GrlVector2   *offset);

GRL_AVAILABLE_IN_ALL
GrlRectangle *      grl_rectangle_scale         (const GrlRectangle *self,
                                                 gfloat              scale_x,
                                                 gfloat              scale_y);

/*
 * Collision / Containment
 */

GRL_AVAILABLE_IN_ALL
gboolean            grl_rectangle_contains_point (const GrlRectangle *self,
                                                  gfloat              x,
                                                  gfloat              y);

GRL_AVAILABLE_IN_ALL
gboolean            grl_rectangle_contains_point_v (const GrlRectangle *self,
                                                    const GrlVector2   *point);

GRL_AVAILABLE_IN_ALL
gboolean            grl_rectangle_contains_rect (const GrlRectangle *self,
                                                 const GrlRectangle *other);

GRL_AVAILABLE_IN_ALL
gboolean            grl_rectangle_intersects    (const GrlRectangle *a,
                                                 const GrlRectangle *b);

GRL_AVAILABLE_IN_ALL
GrlRectangle *      grl_rectangle_intersection  (const GrlRectangle *a,
                                                 const GrlRectangle *b);

GRL_AVAILABLE_IN_ALL
GrlRectangle *      grl_rectangle_union         (const GrlRectangle *a,
                                                 const GrlRectangle *b);

/*
 * Comparison
 */

GRL_AVAILABLE_IN_ALL
gboolean            grl_rectangle_equal         (const GrlRectangle *a,
                                                 const GrlRectangle *b);

GRL_AVAILABLE_IN_ALL
gboolean            grl_rectangle_is_empty      (const GrlRectangle *self);

/*
 * Utility
 */

GRL_AVAILABLE_IN_ALL
gchar *             grl_rectangle_to_string     (const GrlRectangle *self);

G_DEFINE_AUTOPTR_CLEANUP_FUNC (GrlRectangle, grl_rectangle_free)

G_END_DECLS
