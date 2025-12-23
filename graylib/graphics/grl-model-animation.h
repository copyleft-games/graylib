/* grl-model-animation.h
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Skeletal animation for 3D models.
 */

#pragma once

#if !defined(GRAYLIB_INSIDE) && !defined(GRAYLIB_COMPILATION)
#error "Only <graylib.h> can be included directly."
#endif

#include <glib-object.h>
#include "../grl-version.h"
#include "grl-model.h"

G_BEGIN_DECLS

#define GRL_TYPE_MODEL_ANIMATION (grl_model_animation_get_type())

GRL_AVAILABLE_IN_ALL
G_DECLARE_DERIVABLE_TYPE (GrlModelAnimation, grl_model_animation, GRL, MODEL_ANIMATION, GObject)

struct _GrlModelAnimationClass
{
    GObjectClass parent_class;

    gpointer _reserved[8];
};

/**
 * grl_model_animation_load:
 * @filename: (type filename): Path to animation file
 * @count: (out): Return location for animation count
 * @error: (nullable): Return location for error
 *
 * Loads animations from a file.
 * Returns an array of #GrlModelAnimation objects.
 *
 * Returns: (transfer full) (array length=count) (nullable):
 *          Array of animations, or %NULL on error. Free with g_free().
 */
GRL_AVAILABLE_IN_ALL
GrlModelAnimation **grl_model_animation_load        (const gchar        *filename,
                                                     gint               *count,
                                                     GError            **error);

/**
 * grl_model_animation_get_name:
 * @self: A #GrlModelAnimation
 *
 * Gets the name of the animation.
 *
 * Returns: (transfer none): The animation name
 */
GRL_AVAILABLE_IN_ALL
const gchar *       grl_model_animation_get_name    (GrlModelAnimation  *self);

/**
 * grl_model_animation_get_frame_count:
 * @self: A #GrlModelAnimation
 *
 * Gets the number of frames in the animation.
 *
 * Returns: The frame count
 */
GRL_AVAILABLE_IN_ALL
gint                grl_model_animation_get_frame_count (GrlModelAnimation *self);

/**
 * grl_model_animation_get_bone_count:
 * @self: A #GrlModelAnimation
 *
 * Gets the number of bones in the animation skeleton.
 *
 * Returns: The bone count
 */
GRL_AVAILABLE_IN_ALL
gint                grl_model_animation_get_bone_count (GrlModelAnimation *self);

/**
 * grl_model_animation_update:
 * @self: A #GrlModelAnimation
 * @model: The model to animate
 * @frame: The animation frame (will wrap if > frame_count)
 *
 * Updates the model bones to the specified animation frame.
 */
GRL_AVAILABLE_IN_ALL
void                grl_model_animation_update      (GrlModelAnimation  *self,
                                                     GrlModel           *model,
                                                     gint                frame);

/**
 * grl_model_animation_is_valid:
 * @self: A #GrlModelAnimation
 * @model: The model to check against
 *
 * Checks if the animation is compatible with the model.
 *
 * Returns: %TRUE if compatible
 */
GRL_AVAILABLE_IN_ALL
gboolean            grl_model_animation_is_valid    (GrlModelAnimation  *self,
                                                     GrlModel           *model);

/**
 * grl_model_animation_get_handle:
 * @self: A #GrlModelAnimation
 *
 * Gets the internal raylib ModelAnimation pointer.
 * This is for internal use and advanced users.
 *
 * Returns: (transfer none): Internal animation pointer
 */
GRL_AVAILABLE_IN_ALL
gpointer            grl_model_animation_get_handle  (GrlModelAnimation  *self);

G_END_DECLS
