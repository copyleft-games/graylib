/* grl-transform.c
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Implementation of the decomposed 3D transform value type.
 */

#include "config.h"
#include "grl-transform.h"
#include <string.h>

/**
 * SECTION:grl-transform
 * @Title: GrlTransform
 * @Short_description: Decomposed 3D transform (translation, rotation, scale)
 *
 * #GrlTransform stores a 3D transform as a translation vector, a rotation
 * quaternion and a per-axis scale. It is a GBoxed value type wrapping
 * raylib's `Transform`, used by skeletal animation for skeleton bind poses
 * (grl_model_get_bind_pose_transform()) and per-keyframe bone poses
 * (grl_model_animation_get_frame_pose()).
 */

G_DEFINE_BOXED_TYPE (GrlTransform, grl_transform,
                     grl_transform_copy, grl_transform_free)

/**
 * grl_transform_new:
 * @translation: (nullable): Translation component (NULL = origin)
 * @rotation: (nullable): Rotation quaternion (NULL = identity)
 * @scale: (nullable): Scale component (NULL = unit scale)
 *
 * Creates a new #GrlTransform from its components.
 *
 * Returns: (transfer full): A newly allocated #GrlTransform
 */
GrlTransform *
grl_transform_new (const GrlVector3    *translation,
                   const GrlQuaternion *rotation,
                   const GrlVector3    *scale)
{
    GrlTransform *self;

    self = g_new0 (GrlTransform, 1);

    if (translation != NULL)
        self->translation = *translation;

    if (rotation != NULL)
        self->rotation = *rotation;
    else
        self->rotation.w = 1.0f;

    if (scale != NULL)
        self->scale = *scale;
    else
    {
        self->scale.x = 1.0f;
        self->scale.y = 1.0f;
        self->scale.z = 1.0f;
    }

    return self;
}

/**
 * grl_transform_new_identity:
 *
 * Creates an identity #GrlTransform (zero translation, identity rotation,
 * unit scale).
 *
 * Returns: (transfer full): A newly allocated identity #GrlTransform
 */
GrlTransform *
grl_transform_new_identity (void)
{
    return grl_transform_new (NULL, NULL, NULL);
}

/**
 * grl_transform_copy:
 * @self: A #GrlTransform
 *
 * Creates a deep copy of @self.
 *
 * Returns: (transfer full): A newly allocated #GrlTransform
 */
GrlTransform *
grl_transform_copy (const GrlTransform *self)
{
    GrlTransform *copy;

    g_return_val_if_fail (self != NULL, NULL);

    copy = g_new0 (GrlTransform, 1);
    memcpy (copy, self, sizeof (GrlTransform));

    return copy;
}

/**
 * grl_transform_free:
 * @self: A #GrlTransform
 *
 * Frees a transform allocated with grl_transform_new() or
 * grl_transform_copy().
 */
void
grl_transform_free (GrlTransform *self)
{
    g_return_if_fail (self != NULL);

    g_free (self);
}

/**
 * grl_transform_get_translation:
 * @self: A #GrlTransform
 *
 * Gets a copy of the translation component.
 *
 * Returns: (transfer full): A newly allocated #GrlVector3
 */
GrlVector3 *
grl_transform_get_translation (const GrlTransform *self)
{
    g_return_val_if_fail (self != NULL, NULL);

    return grl_vector3_copy (&self->translation);
}

/**
 * grl_transform_get_rotation:
 * @self: A #GrlTransform
 *
 * Gets a copy of the rotation component.
 *
 * Returns: (transfer full): A newly allocated #GrlQuaternion
 */
GrlQuaternion *
grl_transform_get_rotation (const GrlTransform *self)
{
    g_return_val_if_fail (self != NULL, NULL);

    return grl_quaternion_copy (&self->rotation);
}

/**
 * grl_transform_get_scale:
 * @self: A #GrlTransform
 *
 * Gets a copy of the scale component.
 *
 * Returns: (transfer full): A newly allocated #GrlVector3
 */
GrlVector3 *
grl_transform_get_scale (const GrlTransform *self)
{
    g_return_val_if_fail (self != NULL, NULL);

    return grl_vector3_copy (&self->scale);
}
