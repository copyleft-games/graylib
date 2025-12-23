/* grl-vector4.c
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#include "grl-vector4.h"
#include <math.h>

/**
 * SECTION:grl-vector4
 * @Title: GrlVector4
 * @Short_description: 4D vector type
 *
 * #GrlVector4 is a 4D vector type useful for colors, quaternions,
 * homogeneous coordinates, and other 4-component data.
 *
 * The type is a boxed type that can be used with GObject properties
 * and GValue.
 */

G_DEFINE_BOXED_TYPE (GrlVector4, grl_vector4, grl_vector4_copy, grl_vector4_free)

/**
 * grl_vector4_new:
 * @x: X component
 * @y: Y component
 * @z: Z component
 * @w: W component
 *
 * Creates a new 4D vector with the given components.
 *
 * Returns: (transfer full): A newly allocated #GrlVector4
 */
GrlVector4 *
grl_vector4_new (gfloat x,
                 gfloat y,
                 gfloat z,
                 gfloat w)
{
    GrlVector4 *self;

    self = g_slice_new (GrlVector4);
    self->x = x;
    self->y = y;
    self->z = z;
    self->w = w;

    return self;
}

/**
 * grl_vector4_copy:
 * @self: (nullable): A #GrlVector4
 *
 * Creates a copy of the vector.
 *
 * Returns: (transfer full) (nullable): A copy, or %NULL if @self is %NULL
 */
GrlVector4 *
grl_vector4_copy (const GrlVector4 *self)
{
    if (self == NULL)
        return NULL;

    return grl_vector4_new (self->x, self->y, self->z, self->w);
}

/**
 * grl_vector4_free:
 * @self: (nullable): A #GrlVector4
 *
 * Frees a vector allocated with grl_vector4_new() or grl_vector4_copy().
 */
void
grl_vector4_free (GrlVector4 *self)
{
    if (self != NULL)
        g_slice_free (GrlVector4, self);
}

/**
 * grl_vector4_new_zero:
 *
 * Creates a new zero vector (0, 0, 0, 0).
 *
 * Returns: (transfer full): A new zero vector
 */
GrlVector4 *
grl_vector4_new_zero (void)
{
    return grl_vector4_new (0.0f, 0.0f, 0.0f, 0.0f);
}

/**
 * grl_vector4_new_one:
 *
 * Creates a new vector with all components set to 1.
 *
 * Returns: (transfer full): A new (1, 1, 1, 1) vector
 */
GrlVector4 *
grl_vector4_new_one (void)
{
    return grl_vector4_new (1.0f, 1.0f, 1.0f, 1.0f);
}

/**
 * grl_vector4_new_identity:
 *
 * Creates a new identity quaternion (0, 0, 0, 1).
 * Useful when interpreting Vector4 as a quaternion rotation.
 *
 * Returns: (transfer full): A new identity quaternion
 */
GrlVector4 *
grl_vector4_new_identity (void)
{
    return grl_vector4_new (0.0f, 0.0f, 0.0f, 1.0f);
}

/**
 * grl_vector4_add:
 * @a: First vector
 * @b: Second vector
 *
 * Adds two vectors component-wise.
 *
 * Returns: (transfer full): A new vector with the sum
 */
GrlVector4 *
grl_vector4_add (const GrlVector4 *a,
                 const GrlVector4 *b)
{
    g_return_val_if_fail (a != NULL, NULL);
    g_return_val_if_fail (b != NULL, NULL);

    return grl_vector4_new (a->x + b->x, a->y + b->y, a->z + b->z, a->w + b->w);
}

/**
 * grl_vector4_subtract:
 * @a: First vector
 * @b: Second vector
 *
 * Subtracts @b from @a component-wise.
 *
 * Returns: (transfer full): A new vector with the difference
 */
GrlVector4 *
grl_vector4_subtract (const GrlVector4 *a,
                      const GrlVector4 *b)
{
    g_return_val_if_fail (a != NULL, NULL);
    g_return_val_if_fail (b != NULL, NULL);

    return grl_vector4_new (a->x - b->x, a->y - b->y, a->z - b->z, a->w - b->w);
}

/**
 * grl_vector4_scale:
 * @self: A vector
 * @scale: Scale factor
 *
 * Scales a vector by a scalar value.
 *
 * Returns: (transfer full): A new scaled vector
 */
GrlVector4 *
grl_vector4_scale (const GrlVector4 *self,
                   gfloat            scale)
{
    g_return_val_if_fail (self != NULL, NULL);

    return grl_vector4_new (self->x * scale, self->y * scale,
                            self->z * scale, self->w * scale);
}

/**
 * grl_vector4_length:
 * @self: A vector
 *
 * Calculates the Euclidean length (magnitude) of the vector.
 *
 * Returns: The vector length
 */
gfloat
grl_vector4_length (const GrlVector4 *self)
{
    g_return_val_if_fail (self != NULL, 0.0f);

    return sqrtf (self->x * self->x + self->y * self->y +
                  self->z * self->z + self->w * self->w);
}

/**
 * grl_vector4_length_sqr:
 * @self: A vector
 *
 * Calculates the squared length of the vector.
 * More efficient than grl_vector4_length() when you only need comparison.
 *
 * Returns: The squared vector length
 */
gfloat
grl_vector4_length_sqr (const GrlVector4 *self)
{
    g_return_val_if_fail (self != NULL, 0.0f);

    return self->x * self->x + self->y * self->y +
           self->z * self->z + self->w * self->w;
}

/**
 * grl_vector4_normalize:
 * @self: A vector
 *
 * Returns a normalized (unit length) version of the vector.
 * If the vector has zero length, returns a zero vector.
 *
 * Returns: (transfer full): A new normalized vector
 */
GrlVector4 *
grl_vector4_normalize (const GrlVector4 *self)
{
    gfloat length;
    gfloat inv_length;

    g_return_val_if_fail (self != NULL, NULL);

    length = grl_vector4_length (self);

    if (length == 0.0f)
        return grl_vector4_new_zero ();

    inv_length = 1.0f / length;

    return grl_vector4_new (self->x * inv_length, self->y * inv_length,
                            self->z * inv_length, self->w * inv_length);
}

/**
 * grl_vector4_dot:
 * @a: First vector
 * @b: Second vector
 *
 * Calculates the dot product of two vectors.
 *
 * Returns: The dot product
 */
gfloat
grl_vector4_dot (const GrlVector4 *a,
                 const GrlVector4 *b)
{
    g_return_val_if_fail (a != NULL, 0.0f);
    g_return_val_if_fail (b != NULL, 0.0f);

    return a->x * b->x + a->y * b->y + a->z * b->z + a->w * b->w;
}

/**
 * grl_vector4_lerp:
 * @a: Start vector
 * @b: End vector
 * @amount: Interpolation amount (0.0 to 1.0)
 *
 * Linearly interpolates between two vectors.
 * When @amount is 0, returns @a. When @amount is 1, returns @b.
 *
 * Returns: (transfer full): A new interpolated vector
 */
GrlVector4 *
grl_vector4_lerp (const GrlVector4 *a,
                  const GrlVector4 *b,
                  gfloat            amount)
{
    g_return_val_if_fail (a != NULL, NULL);
    g_return_val_if_fail (b != NULL, NULL);

    return grl_vector4_new (a->x + amount * (b->x - a->x),
                            a->y + amount * (b->y - a->y),
                            a->z + amount * (b->z - a->z),
                            a->w + amount * (b->w - a->w));
}

/**
 * grl_vector4_negate:
 * @self: A vector
 *
 * Returns the negation of the vector (all components negated).
 *
 * Returns: (transfer full): A new negated vector
 */
GrlVector4 *
grl_vector4_negate (const GrlVector4 *self)
{
    g_return_val_if_fail (self != NULL, NULL);

    return grl_vector4_new (-self->x, -self->y, -self->z, -self->w);
}

/**
 * grl_vector4_equal:
 * @a: First vector
 * @b: Second vector
 *
 * Checks if two vectors are exactly equal (all components match).
 *
 * Returns: %TRUE if equal
 */
gboolean
grl_vector4_equal (const GrlVector4 *a,
                   const GrlVector4 *b)
{
    if (a == b)
        return TRUE;
    if (a == NULL || b == NULL)
        return FALSE;

    return a->x == b->x && a->y == b->y && a->z == b->z && a->w == b->w;
}
