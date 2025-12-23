/* grl-vector4.h
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * 4D vector type for colors, quaternions, and homogeneous coordinates.
 */

#pragma once

#if !defined(GRAYLIB_INSIDE) && !defined(GRAYLIB_COMPILATION)
#error "Only <graylib.h> can be included directly."
#endif

#include <glib-object.h>
#include "../grl-version.h"

G_BEGIN_DECLS

typedef struct _GrlVector4 GrlVector4;

/**
 * GrlVector4:
 * @x: X component
 * @y: Y component
 * @z: Z component
 * @w: W component
 *
 * A 4D vector for colors, quaternions, homogeneous coordinates, etc.
 */
struct _GrlVector4
{
    gfloat x;
    gfloat y;
    gfloat z;
    gfloat w;
};

#define GRL_TYPE_VECTOR4 (grl_vector4_get_type())

GRL_AVAILABLE_IN_ALL
GType           grl_vector4_get_type        (void) G_GNUC_CONST;

/**
 * grl_vector4_new:
 * @x: X component
 * @y: Y component
 * @z: Z component
 * @w: W component
 *
 * Creates a new 4D vector.
 *
 * Returns: (transfer full): A newly allocated #GrlVector4
 */
GRL_AVAILABLE_IN_ALL
GrlVector4 *    grl_vector4_new             (gfloat              x,
                                             gfloat              y,
                                             gfloat              z,
                                             gfloat              w);

/**
 * grl_vector4_copy:
 * @self: (nullable): A #GrlVector4
 *
 * Creates a copy of the vector.
 *
 * Returns: (transfer full) (nullable): A copy, or %NULL
 */
GRL_AVAILABLE_IN_ALL
GrlVector4 *    grl_vector4_copy            (const GrlVector4   *self);

/**
 * grl_vector4_free:
 * @self: (nullable): A #GrlVector4
 *
 * Frees a vector allocated with grl_vector4_new() or grl_vector4_copy().
 */
GRL_AVAILABLE_IN_ALL
void            grl_vector4_free            (GrlVector4         *self);

G_DEFINE_AUTOPTR_CLEANUP_FUNC (GrlVector4, grl_vector4_free)

/**
 * grl_vector4_new_zero:
 *
 * Creates a new zero vector (0, 0, 0, 0).
 *
 * Returns: (transfer full): A new zero vector
 */
GRL_AVAILABLE_IN_ALL
GrlVector4 *    grl_vector4_new_zero        (void);

/**
 * grl_vector4_new_one:
 *
 * Creates a new vector with all components set to 1.
 *
 * Returns: (transfer full): A new (1, 1, 1, 1) vector
 */
GRL_AVAILABLE_IN_ALL
GrlVector4 *    grl_vector4_new_one         (void);

/**
 * grl_vector4_new_identity:
 *
 * Creates a new identity quaternion (0, 0, 0, 1).
 *
 * Returns: (transfer full): A new identity quaternion
 */
GRL_AVAILABLE_IN_ALL
GrlVector4 *    grl_vector4_new_identity    (void);

/**
 * grl_vector4_add:
 * @a: First vector
 * @b: Second vector
 *
 * Adds two vectors.
 *
 * Returns: (transfer full): A new vector with the sum
 */
GRL_AVAILABLE_IN_ALL
GrlVector4 *    grl_vector4_add             (const GrlVector4   *a,
                                             const GrlVector4   *b);

/**
 * grl_vector4_subtract:
 * @a: First vector
 * @b: Second vector
 *
 * Subtracts @b from @a.
 *
 * Returns: (transfer full): A new vector with the difference
 */
GRL_AVAILABLE_IN_ALL
GrlVector4 *    grl_vector4_subtract        (const GrlVector4   *a,
                                             const GrlVector4   *b);

/**
 * grl_vector4_scale:
 * @self: A vector
 * @scale: Scale factor
 *
 * Scales a vector by a scalar value.
 *
 * Returns: (transfer full): A new scaled vector
 */
GRL_AVAILABLE_IN_ALL
GrlVector4 *    grl_vector4_scale           (const GrlVector4   *self,
                                             gfloat              scale);

/**
 * grl_vector4_length:
 * @self: A vector
 *
 * Calculates the length (magnitude) of the vector.
 *
 * Returns: The vector length
 */
GRL_AVAILABLE_IN_ALL
gfloat          grl_vector4_length          (const GrlVector4   *self);

/**
 * grl_vector4_length_sqr:
 * @self: A vector
 *
 * Calculates the squared length of the vector.
 * More efficient than grl_vector4_length() when you only need comparison.
 *
 * Returns: The squared vector length
 */
GRL_AVAILABLE_IN_ALL
gfloat          grl_vector4_length_sqr      (const GrlVector4   *self);

/**
 * grl_vector4_normalize:
 * @self: A vector
 *
 * Returns a normalized (unit length) version of the vector.
 *
 * Returns: (transfer full): A new normalized vector
 */
GRL_AVAILABLE_IN_ALL
GrlVector4 *    grl_vector4_normalize       (const GrlVector4   *self);

/**
 * grl_vector4_dot:
 * @a: First vector
 * @b: Second vector
 *
 * Calculates the dot product of two vectors.
 *
 * Returns: The dot product
 */
GRL_AVAILABLE_IN_ALL
gfloat          grl_vector4_dot             (const GrlVector4   *a,
                                             const GrlVector4   *b);

/**
 * grl_vector4_lerp:
 * @a: Start vector
 * @b: End vector
 * @amount: Interpolation amount (0.0 to 1.0)
 *
 * Linearly interpolates between two vectors.
 *
 * Returns: (transfer full): A new interpolated vector
 */
GRL_AVAILABLE_IN_ALL
GrlVector4 *    grl_vector4_lerp            (const GrlVector4   *a,
                                             const GrlVector4   *b,
                                             gfloat              amount);

/**
 * grl_vector4_negate:
 * @self: A vector
 *
 * Returns the negation of the vector.
 *
 * Returns: (transfer full): A new negated vector
 */
GRL_AVAILABLE_IN_ALL
GrlVector4 *    grl_vector4_negate          (const GrlVector4   *self);

/**
 * grl_vector4_equal:
 * @a: First vector
 * @b: Second vector
 *
 * Checks if two vectors are exactly equal.
 *
 * Returns: %TRUE if equal
 */
GRL_AVAILABLE_IN_ALL
gboolean        grl_vector4_equal           (const GrlVector4   *a,
                                             const GrlVector4   *b);

G_END_DECLS
