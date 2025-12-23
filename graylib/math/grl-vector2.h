/* grl-vector2.h
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * 2D vector type for positions, velocities, and other 2-component values.
 */

#pragma once

#if !defined(GRAYLIB_INSIDE) && !defined(GRAYLIB_COMPILATION)
#error "Only <graylib.h> can be included directly."
#endif

#include <glib-object.h>
#include "grl-version.h"

G_BEGIN_DECLS

/* Forward declaration for this file */
typedef struct _GrlVector2 GrlVector2;

#define GRL_TYPE_VECTOR2 (grl_vector2_get_type ())

/**
 * GrlVector2:
 * @x: X component
 * @y: Y component
 *
 * A 2D vector for positions, velocities, directions, and other
 * 2-component values.
 *
 * This is a GBoxed type that can be used in GObject properties
 * and signals, and is fully supported by GObject Introspection.
 *
 * # Stack Allocation
 *
 * For performance-critical code, you can use grl_vector2_init() to
 * create stack-allocated vectors:
 *
 * |[<!-- language="C" -->
 * GrlVector2 pos = grl_vector2_init (100.0f, 200.0f);
 * gfloat len = grl_vector2_length (&pos);
 * ]|
 *
 * # Heap Allocation
 *
 * For GObject properties and when ownership transfer is needed:
 *
 * |[<!-- language="C" -->
 * g_autoptr(GrlVector2) pos = grl_vector2_new (100.0f, 200.0f);
 * ]|
 */
struct _GrlVector2
{
    gfloat x;
    gfloat y;
};

GRL_AVAILABLE_IN_ALL
GType           grl_vector2_get_type        (void) G_GNUC_CONST;

GRL_AVAILABLE_IN_ALL
GrlVector2 *    grl_vector2_new             (gfloat              x,
                                             gfloat              y);

GRL_AVAILABLE_IN_ALL
GrlVector2 *    grl_vector2_new_zero        (void);

GRL_AVAILABLE_IN_ALL
GrlVector2 *    grl_vector2_new_one         (void);

GRL_AVAILABLE_IN_ALL
GrlVector2 *    grl_vector2_copy            (const GrlVector2   *self);

GRL_AVAILABLE_IN_ALL
void            grl_vector2_free            (GrlVector2         *self);

/*
 * Stack allocation helper
 */

/**
 * grl_vector2_init:
 * @x: X component
 * @y: Y component
 *
 * Creates a stack-allocated vector. This is more efficient than
 * grl_vector2_new() when you don't need heap allocation.
 *
 * Returns: A stack-allocated #GrlVector2
 */
static inline GrlVector2
grl_vector2_init (gfloat x,
                  gfloat y)
{
    GrlVector2 v;
    v.x = x;
    v.y = y;
    return v;
}

/*
 * Accessors
 */

GRL_AVAILABLE_IN_ALL
gfloat          grl_vector2_get_x           (const GrlVector2   *self);

GRL_AVAILABLE_IN_ALL
gfloat          grl_vector2_get_y           (const GrlVector2   *self);

GRL_AVAILABLE_IN_ALL
void            grl_vector2_set_x           (GrlVector2         *self,
                                             gfloat              x);

GRL_AVAILABLE_IN_ALL
void            grl_vector2_set_y           (GrlVector2         *self,
                                             gfloat              y);

GRL_AVAILABLE_IN_ALL
void            grl_vector2_set             (GrlVector2         *self,
                                             gfloat              x,
                                             gfloat              y);

/*
 * Operations (return new allocated vectors)
 */

GRL_AVAILABLE_IN_ALL
GrlVector2 *    grl_vector2_add             (const GrlVector2   *a,
                                             const GrlVector2   *b);

GRL_AVAILABLE_IN_ALL
GrlVector2 *    grl_vector2_subtract        (const GrlVector2   *a,
                                             const GrlVector2   *b);

GRL_AVAILABLE_IN_ALL
GrlVector2 *    grl_vector2_scale           (const GrlVector2   *self,
                                             gfloat              scalar);

GRL_AVAILABLE_IN_ALL
GrlVector2 *    grl_vector2_multiply        (const GrlVector2   *a,
                                             const GrlVector2   *b);

GRL_AVAILABLE_IN_ALL
GrlVector2 *    grl_vector2_divide          (const GrlVector2   *a,
                                             const GrlVector2   *b);

GRL_AVAILABLE_IN_ALL
GrlVector2 *    grl_vector2_negate          (const GrlVector2   *self);

GRL_AVAILABLE_IN_ALL
GrlVector2 *    grl_vector2_normalize       (const GrlVector2   *self);

GRL_AVAILABLE_IN_ALL
GrlVector2 *    grl_vector2_lerp            (const GrlVector2   *a,
                                             const GrlVector2   *b,
                                             gfloat              t);

GRL_AVAILABLE_IN_ALL
GrlVector2 *    grl_vector2_rotate          (const GrlVector2   *self,
                                             gfloat              angle);

GRL_AVAILABLE_IN_ALL
GrlVector2 *    grl_vector2_clamp           (const GrlVector2   *self,
                                             const GrlVector2   *min,
                                             const GrlVector2   *max);

GRL_AVAILABLE_IN_ALL
GrlVector2 *    grl_vector2_reflect         (const GrlVector2   *self,
                                             const GrlVector2   *normal);

/*
 * Scalar operations
 */

GRL_AVAILABLE_IN_ALL
gfloat          grl_vector2_length          (const GrlVector2   *self);

GRL_AVAILABLE_IN_ALL
gfloat          grl_vector2_length_sqr      (const GrlVector2   *self);

GRL_AVAILABLE_IN_ALL
gfloat          grl_vector2_dot             (const GrlVector2   *a,
                                             const GrlVector2   *b);

GRL_AVAILABLE_IN_ALL
gfloat          grl_vector2_distance        (const GrlVector2   *a,
                                             const GrlVector2   *b);

GRL_AVAILABLE_IN_ALL
gfloat          grl_vector2_distance_sqr    (const GrlVector2   *a,
                                             const GrlVector2   *b);

GRL_AVAILABLE_IN_ALL
gfloat          grl_vector2_angle           (const GrlVector2   *a,
                                             const GrlVector2   *b);

/*
 * Comparison
 */

GRL_AVAILABLE_IN_ALL
gboolean        grl_vector2_equal           (const GrlVector2   *a,
                                             const GrlVector2   *b);

GRL_AVAILABLE_IN_ALL
gboolean        grl_vector2_equal_epsilon   (const GrlVector2   *a,
                                             const GrlVector2   *b,
                                             gfloat              epsilon);

/*
 * Utility
 */

GRL_AVAILABLE_IN_ALL
gchar *         grl_vector2_to_string       (const GrlVector2   *self);

G_DEFINE_AUTOPTR_CLEANUP_FUNC (GrlVector2, grl_vector2_free)

G_END_DECLS
