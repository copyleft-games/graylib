/* grl-transform.h
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Decomposed 3D transform: translation, rotation, scale.
 */

#pragma once

#if !defined(GRAYLIB_INSIDE) && !defined(GRAYLIB_COMPILATION)
#error "Only <graylib.h> can be included directly."
#endif

#include <glib-object.h>
#include "../grl-version.h"
#include "../math/grl-vector3.h"
#include "../math/grl-quaternion.h"

G_BEGIN_DECLS

/* Forward declaration for this file */
typedef struct _GrlTransform GrlTransform;

#define GRL_TYPE_TRANSFORM (grl_transform_get_type ())

/**
 * GrlTransform:
 * @translation: Position component
 * @rotation: Orientation as a quaternion
 * @scale: Per-axis scale
 *
 * A decomposed 3D transform (translation, rotation, scale). This is a
 * GBoxed value type backing raylib 6.0's `Transform`, used for skeleton
 * bind poses and per-keyframe bone poses in skeletal animation. Its memory
 * layout matches raylib's `Transform`.
 */
struct _GrlTransform
{
    GrlVector3    translation;
    GrlQuaternion rotation;
    GrlVector3    scale;
};

GRL_AVAILABLE_IN_ALL
GType            grl_transform_get_type        (void) G_GNUC_CONST;

GRL_AVAILABLE_IN_ALL
GrlTransform *   grl_transform_new             (const GrlVector3    *translation,
                                                const GrlQuaternion *rotation,
                                                const GrlVector3    *scale);

GRL_AVAILABLE_IN_ALL
GrlTransform *   grl_transform_new_identity    (void);

GRL_AVAILABLE_IN_ALL
GrlTransform *   grl_transform_copy            (const GrlTransform  *self);

GRL_AVAILABLE_IN_ALL
void             grl_transform_free            (GrlTransform        *self);

/*
 * Accessors (each returns a freshly allocated copy)
 */

GRL_AVAILABLE_IN_ALL
GrlVector3 *     grl_transform_get_translation (const GrlTransform  *self);

GRL_AVAILABLE_IN_ALL
GrlQuaternion *  grl_transform_get_rotation    (const GrlTransform  *self);

GRL_AVAILABLE_IN_ALL
GrlVector3 *     grl_transform_get_scale       (const GrlTransform  *self);

G_DEFINE_AUTOPTR_CLEANUP_FUNC (GrlTransform, grl_transform_free)

G_END_DECLS
