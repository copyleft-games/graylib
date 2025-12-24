/* grl-vector3.h
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * 3D vector type for positions, directions, and 3D math.
 */

#pragma once

#if !defined(GRAYLIB_INSIDE) && !defined(GRAYLIB_COMPILATION)
#error "Only <graylib.h> can be included directly."
#endif

#include <glib-object.h>
#include "../grl-version.h"

G_BEGIN_DECLS

typedef struct _GrlVector3 GrlVector3;

/**
 * GrlVector3:
 * @x: X component
 * @y: Y component
 * @z: Z component
 *
 * A 3D vector for positions, directions, velocities, etc.
 */
struct _GrlVector3
{
    gfloat x;
    gfloat y;
    gfloat z;
};

#define GRL_TYPE_VECTOR3 (grl_vector3_get_type())

GRL_AVAILABLE_IN_ALL
GType           grl_vector3_get_type        (void) G_GNUC_CONST;

/**
 * grl_vector3_new:
 * @x: X component
 * @y: Y component
 * @z: Z component
 *
 * Creates a new 3D vector.
 *
 * Returns: (transfer full): A newly allocated #GrlVector3
 */
GRL_AVAILABLE_IN_ALL
GrlVector3 *    grl_vector3_new             (gfloat              x,
                                             gfloat              y,
                                             gfloat              z);

/**
 * grl_vector3_copy:
 * @self: (nullable): A #GrlVector3
 *
 * Creates a copy of the vector.
 *
 * Returns: (transfer full) (nullable): A copy, or %NULL
 */
GRL_AVAILABLE_IN_ALL
GrlVector3 *    grl_vector3_copy            (const GrlVector3   *self);

/**
 * grl_vector3_free:
 * @self: (nullable): A #GrlVector3
 *
 * Frees a vector allocated with grl_vector3_new() or grl_vector3_copy().
 */
GRL_AVAILABLE_IN_ALL
void            grl_vector3_free            (GrlVector3         *self);

G_DEFINE_AUTOPTR_CLEANUP_FUNC (GrlVector3, grl_vector3_free)

/**
 * grl_vector3_new_zero:
 *
 * Creates a new zero vector (0, 0, 0).
 *
 * Returns: (transfer full): A new zero vector
 */
GRL_AVAILABLE_IN_ALL
GrlVector3 *    grl_vector3_new_zero        (void);

/**
 * grl_vector3_new_one:
 *
 * Creates a new vector with all components set to 1.
 *
 * Returns: (transfer full): A new (1, 1, 1) vector
 */
GRL_AVAILABLE_IN_ALL
GrlVector3 *    grl_vector3_new_one         (void);

/**
 * grl_vector3_new_up:
 *
 * Creates a new up vector (0, 1, 0).
 *
 * Returns: (transfer full): A new up vector
 */
GRL_AVAILABLE_IN_ALL
GrlVector3 *    grl_vector3_new_up          (void);

/**
 * grl_vector3_new_forward:
 *
 * Creates a new forward vector (0, 0, -1).
 *
 * Returns: (transfer full): A new forward vector
 */
GRL_AVAILABLE_IN_ALL
GrlVector3 *    grl_vector3_new_forward     (void);

/**
 * grl_vector3_new_right:
 *
 * Creates a new right vector (1, 0, 0).
 *
 * Returns: (transfer full): A new right vector
 */
GRL_AVAILABLE_IN_ALL
GrlVector3 *    grl_vector3_new_right       (void);

/**
 * grl_vector3_add:
 * @a: First vector
 * @b: Second vector
 *
 * Adds two vectors.
 *
 * Returns: (transfer full): A new vector with the sum
 */
GRL_AVAILABLE_IN_ALL
GrlVector3 *    grl_vector3_add             (const GrlVector3   *a,
                                             const GrlVector3   *b);

/**
 * grl_vector3_subtract:
 * @a: First vector
 * @b: Second vector
 *
 * Subtracts @b from @a.
 *
 * Returns: (transfer full): A new vector with the difference
 */
GRL_AVAILABLE_IN_ALL
GrlVector3 *    grl_vector3_subtract        (const GrlVector3   *a,
                                             const GrlVector3   *b);

/**
 * grl_vector3_scale:
 * @self: A vector
 * @scale: Scale factor
 *
 * Scales a vector by a scalar value.
 *
 * Returns: (transfer full): A new scaled vector
 */
GRL_AVAILABLE_IN_ALL
GrlVector3 *    grl_vector3_scale           (const GrlVector3   *self,
                                             gfloat              scale);

/**
 * grl_vector3_length:
 * @self: A vector
 *
 * Calculates the length (magnitude) of the vector.
 *
 * Returns: The vector length
 */
GRL_AVAILABLE_IN_ALL
gfloat          grl_vector3_length          (const GrlVector3   *self);

/**
 * grl_vector3_length_sqr:
 * @self: A vector
 *
 * Calculates the squared length of the vector.
 * More efficient than grl_vector3_length() when you only need comparison.
 *
 * Returns: The squared vector length
 */
GRL_AVAILABLE_IN_ALL
gfloat          grl_vector3_length_sqr      (const GrlVector3   *self);

/**
 * grl_vector3_normalize:
 * @self: A vector
 *
 * Returns a normalized (unit length) version of the vector.
 *
 * Returns: (transfer full): A new normalized vector
 */
GRL_AVAILABLE_IN_ALL
GrlVector3 *    grl_vector3_normalize       (const GrlVector3   *self);

/**
 * grl_vector3_dot:
 * @a: First vector
 * @b: Second vector
 *
 * Calculates the dot product of two vectors.
 *
 * Returns: The dot product
 */
GRL_AVAILABLE_IN_ALL
gfloat          grl_vector3_dot             (const GrlVector3   *a,
                                             const GrlVector3   *b);

/**
 * grl_vector3_cross:
 * @a: First vector
 * @b: Second vector
 *
 * Calculates the cross product of two vectors.
 *
 * Returns: (transfer full): A new vector with the cross product
 */
GRL_AVAILABLE_IN_ALL
GrlVector3 *    grl_vector3_cross           (const GrlVector3   *a,
                                             const GrlVector3   *b);

/**
 * grl_vector3_distance:
 * @a: First vector
 * @b: Second vector
 *
 * Calculates the distance between two points.
 *
 * Returns: The distance
 */
GRL_AVAILABLE_IN_ALL
gfloat          grl_vector3_distance        (const GrlVector3   *a,
                                             const GrlVector3   *b);

/**
 * grl_vector3_lerp:
 * @a: Start vector
 * @b: End vector
 * @amount: Interpolation amount (0.0 to 1.0)
 *
 * Linearly interpolates between two vectors.
 *
 * Returns: (transfer full): A new interpolated vector
 */
GRL_AVAILABLE_IN_ALL
GrlVector3 *    grl_vector3_lerp            (const GrlVector3   *a,
                                             const GrlVector3   *b,
                                             gfloat              amount);

/**
 * grl_vector3_negate:
 * @self: A vector
 *
 * Returns the negation of the vector.
 *
 * Returns: (transfer full): A new negated vector
 */
GRL_AVAILABLE_IN_ALL
GrlVector3 *    grl_vector3_negate          (const GrlVector3   *self);

/**
 * grl_vector3_equal:
 * @a: First vector
 * @b: Second vector
 *
 * Checks if two vectors are exactly equal.
 *
 * Returns: %TRUE if equal
 */
GRL_AVAILABLE_IN_ALL
gboolean        grl_vector3_equal           (const GrlVector3   *a,
                                             const GrlVector3   *b);

G_END_DECLS
