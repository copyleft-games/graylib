/* grl-model-animation.c
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#include "grl-model-animation.h"
#include <gio/gio.h>
#include <raylib.h>

/**
 * SECTION:grl-model-animation
 * @Title: GrlModelAnimation
 * @Short_description: Skeletal animation for 3D models
 *
 * #GrlModelAnimation represents skeletal animation data that can be
 * applied to 3D models with compatible bone structures.
 *
 * # Loading Animations
 *
 * Animations are loaded from model files that contain animation data:
 * |[<!-- language="C" -->
 * g_autoptr(GError) error = NULL;
 * gint anim_count;
 * GrlModelAnimation **anims = grl_model_animation_load ("character.iqm",
 *                                                        &anim_count, &error);
 * if (anims == NULL)
 *     g_printerr ("Failed: %s\n", error->message);
 * ]|
 *
 * # Applying Animations
 *
 * Update the model bones each frame:
 * |[<!-- language="C" -->
 * grl_model_animation_update (animation, model, frame);
 * ]|
 */

typedef struct
{
    ModelAnimation   animation;
    gboolean         valid;
    gboolean         owns_data;
} GrlModelAnimationPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (GrlModelAnimation, grl_model_animation, G_TYPE_OBJECT)

enum
{
    PROP_0,
    PROP_NAME,
    PROP_FRAME_COUNT,
    PROP_BONE_COUNT,
    N_PROPS
};

static GParamSpec *properties[N_PROPS];

static void
grl_model_animation_finalize (GObject *object)
{
    GrlModelAnimation *self = GRL_MODEL_ANIMATION (object);
    GrlModelAnimationPrivate *priv = grl_model_animation_get_instance_private (self);

    if (priv->valid && priv->owns_data)
    {
        /* raylib 6.0 removed the public singular UnloadModelAnimation
         * (only the array form UnloadModelAnimations remains) and renamed the
         * fields: frameCount -> keyframeCount, framePoses -> keyframePoses.
         * Bones now live in Model.skeleton, so an animation only owns its
         * keyframe poses.  Free them directly, mirroring raylib's per-animation
         * unload (RL_FREE defaults to free()). */
        ModelAnimation *anim = &priv->animation;
        gint f;

        if (anim->keyframePoses != NULL)
        {
            for (f = 0; f < anim->keyframeCount; f++)
                RL_FREE (anim->keyframePoses[f]);
            RL_FREE (anim->keyframePoses);
        }
        priv->valid = FALSE;
    }

    G_OBJECT_CLASS (grl_model_animation_parent_class)->finalize (object);
}

static void
grl_model_animation_get_property (GObject    *object,
                                  guint       prop_id,
                                  GValue     *value,
                                  GParamSpec *pspec)
{
    GrlModelAnimation *self = GRL_MODEL_ANIMATION (object);
    GrlModelAnimationPrivate *priv = grl_model_animation_get_instance_private (self);

    switch (prop_id)
    {
    case PROP_NAME:
        g_value_set_string (value, priv->valid ? priv->animation.name : "");
        break;

    case PROP_FRAME_COUNT:
        g_value_set_int (value, priv->valid ? priv->animation.keyframeCount : 0);
        break;

    case PROP_BONE_COUNT:
        g_value_set_int (value, priv->valid ? priv->animation.boneCount : 0);
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
grl_model_animation_class_init (GrlModelAnimationClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->finalize = grl_model_animation_finalize;
    object_class->get_property = grl_model_animation_get_property;

    /**
     * GrlModelAnimation:name:
     *
     * The name of the animation.
     */
    properties[PROP_NAME] =
        g_param_spec_string ("name",
                             "Name",
                             "Animation name",
                             "",
                             G_PARAM_READABLE |
                             G_PARAM_STATIC_STRINGS);

    /**
     * GrlModelAnimation:frame-count:
     *
     * Number of frames in the animation.
     */
    properties[PROP_FRAME_COUNT] =
        g_param_spec_int ("frame-count",
                          "Frame Count",
                          "Number of animation frames",
                          0, G_MAXINT, 0,
                          G_PARAM_READABLE |
                          G_PARAM_STATIC_STRINGS);

    /**
     * GrlModelAnimation:bone-count:
     *
     * Number of bones in the animation.
     */
    properties[PROP_BONE_COUNT] =
        g_param_spec_int ("bone-count",
                          "Bone Count",
                          "Number of bones",
                          0, G_MAXINT, 0,
                          G_PARAM_READABLE |
                          G_PARAM_STATIC_STRINGS);

    g_object_class_install_properties (object_class, N_PROPS, properties);
}

static void
grl_model_animation_init (GrlModelAnimation *self)
{
    GrlModelAnimationPrivate *priv = grl_model_animation_get_instance_private (self);

    priv->valid = FALSE;
    priv->owns_data = FALSE;
    memset (&priv->animation, 0, sizeof (ModelAnimation));
}

/*
 * Internal helper to wrap raylib ModelAnimation
 */
static GrlModelAnimation *
grl_model_animation_from_raylib (ModelAnimation anim,
                                 gboolean       owns_data)
{
    GrlModelAnimation *self;
    GrlModelAnimationPrivate *priv;

    self = g_object_new (GRL_TYPE_MODEL_ANIMATION, NULL);
    priv = grl_model_animation_get_instance_private (self);

    priv->animation = anim;
    priv->valid = TRUE;
    priv->owns_data = owns_data;

    return self;
}

/**
 * grl_model_animation_load:
 * @filename: (type filename): Path to animation file
 * @count: (out): Return location for animation count
 * @error: (nullable): Return location for error
 *
 * Loads animations from a file.
 * Supported formats include IQM, GLTF, and other formats with animation.
 *
 * Returns: (transfer full) (array length=count) (nullable):
 *          Array of animations, or %NULL on error.
 *          Free each animation with g_object_unref() and the array with g_free().
 */
GrlModelAnimation **
grl_model_animation_load (const gchar  *filename,
                          gint         *count,
                          GError      **error)
{
    ModelAnimation *anims;
    GrlModelAnimation **result;
    gint i;

    g_return_val_if_fail (filename != NULL, NULL);
    g_return_val_if_fail (count != NULL, NULL);

    *count = 0;

    anims = LoadModelAnimations (filename, count);

    if (anims == NULL || *count == 0)
    {
        g_set_error (error,
                     G_IO_ERROR,
                     G_IO_ERROR_FAILED,
                     "Failed to load animations from: %s",
                     filename);
        return NULL;
    }

    result = g_new (GrlModelAnimation *, *count);

    for (i = 0; i < *count; i++)
    {
        /* First animation owns the data array, others just reference it */
        result[i] = grl_model_animation_from_raylib (anims[i], (i == 0));
    }

    /* Free the raylib array structure (but not the animation data) */
    RL_FREE (anims);

    return result;
}

/**
 * grl_model_animation_get_name:
 * @self: A #GrlModelAnimation
 *
 * Gets the name of the animation (usually the filename or clip name).
 *
 * Returns: (transfer none): The animation name
 */
const gchar *
grl_model_animation_get_name (GrlModelAnimation *self)
{
    GrlModelAnimationPrivate *priv;

    g_return_val_if_fail (GRL_IS_MODEL_ANIMATION (self), "");

    priv = grl_model_animation_get_instance_private (self);

    return priv->valid ? priv->animation.name : "";
}

/**
 * grl_model_animation_get_frame_count:
 * @self: A #GrlModelAnimation
 *
 * Gets the total number of frames in the animation.
 *
 * Returns: The frame count
 */
gint
grl_model_animation_get_frame_count (GrlModelAnimation *self)
{
    GrlModelAnimationPrivate *priv;

    g_return_val_if_fail (GRL_IS_MODEL_ANIMATION (self), 0);

    priv = grl_model_animation_get_instance_private (self);

    return priv->valid ? priv->animation.keyframeCount : 0;
}

/**
 * grl_model_animation_get_bone_count:
 * @self: A #GrlModelAnimation
 *
 * Gets the number of bones in the animation skeleton.
 *
 * Returns: The bone count
 */
gint
grl_model_animation_get_bone_count (GrlModelAnimation *self)
{
    GrlModelAnimationPrivate *priv;

    g_return_val_if_fail (GRL_IS_MODEL_ANIMATION (self), 0);

    priv = grl_model_animation_get_instance_private (self);

    return priv->valid ? priv->animation.boneCount : 0;
}

/**
 * grl_model_animation_get_frame_pose:
 * @self: A #GrlModelAnimation
 * @frame: Keyframe index in range [0, frame_count)
 * @bone: Bone index in range [0, bone_count)
 *
 * Gets the local pose transform of a bone at a given animation keyframe.
 *
 * Returns: (transfer full) (nullable): A new #GrlTransform, or %NULL if
 *          @frame or @bone is out of range
 */
GrlTransform *
grl_model_animation_get_frame_pose (GrlModelAnimation *self,
                                    gint               frame,
                                    gint               bone)
{
    GrlModelAnimationPrivate *priv;
    Transform                 t;
    g_autoptr (GrlVector3)    translation = NULL;
    g_autoptr (GrlQuaternion) rotation = NULL;
    g_autoptr (GrlVector3)    scale = NULL;

    g_return_val_if_fail (GRL_IS_MODEL_ANIMATION (self), NULL);

    priv = grl_model_animation_get_instance_private (self);

    if (!priv->valid || priv->animation.keyframePoses == NULL)
        return NULL;

    if (frame < 0 || frame >= priv->animation.keyframeCount)
        return NULL;

    if (bone < 0 || bone >= priv->animation.boneCount)
        return NULL;

    /* keyframePoses is a ModelAnimPose* (Transform**): [frame][bone] */
    t = priv->animation.keyframePoses[frame][bone];
    translation = grl_vector3_new (t.translation.x, t.translation.y, t.translation.z);
    rotation = grl_quaternion_new (t.rotation.x, t.rotation.y, t.rotation.z, t.rotation.w);
    scale = grl_vector3_new (t.scale.x, t.scale.y, t.scale.z);

    return grl_transform_new (translation, rotation, scale);
}

/**
 * grl_model_animation_update:
 * @self: A #GrlModelAnimation
 * @model: The model to animate
 * @frame: The animation frame (will wrap if > frame_count)
 *
 * Updates the model's bone transforms to match the specified animation frame.
 * The frame number will automatically wrap if it exceeds the animation length.
 */
void
grl_model_animation_update (GrlModelAnimation *self,
                            GrlModel          *model,
                            gint               frame)
{
    GrlModelAnimationPrivate *priv;
    Model *rl_model;

    g_return_if_fail (GRL_IS_MODEL_ANIMATION (self));
    g_return_if_fail (GRL_IS_MODEL (model));

    priv = grl_model_animation_get_instance_private (self);

    if (!priv->valid)
        return;

    rl_model = (Model *)grl_model_get_handle (model);
    UpdateModelAnimation (*rl_model, priv->animation, frame);
}

/**
 * grl_model_animation_blend:
 * @model: The model to animate
 * @anim_a: First animation
 * @frame_a: Frame within @anim_a
 * @anim_b: Second animation to blend toward
 * @frame_b: Frame within @anim_b
 * @blend: Blend factor in [0.0, 1.0] (0.0 = @anim_a, 1.0 = @anim_b)
 *
 * Updates @model's pose by blending between two animation frames using
 * raylib 6.0's UpdateModelAnimationEx().
 */
void
grl_model_animation_blend (GrlModel          *model,
                           GrlModelAnimation *anim_a,
                           gfloat             frame_a,
                           GrlModelAnimation *anim_b,
                           gfloat             frame_b,
                           gfloat             blend)
{
    GrlModelAnimationPrivate *priv_a;
    GrlModelAnimationPrivate *priv_b;
    Model                    *rl_model;

    g_return_if_fail (GRL_IS_MODEL (model));
    g_return_if_fail (GRL_IS_MODEL_ANIMATION (anim_a));
    g_return_if_fail (GRL_IS_MODEL_ANIMATION (anim_b));

    priv_a = grl_model_animation_get_instance_private (anim_a);
    priv_b = grl_model_animation_get_instance_private (anim_b);

    if (!priv_a->valid || !priv_b->valid)
        return;

    rl_model = (Model *)grl_model_get_handle (model);
    UpdateModelAnimationEx (*rl_model, priv_a->animation, frame_a,
                            priv_b->animation, frame_b, blend);
}

/**
 * grl_model_animation_is_valid:
 * @self: A #GrlModelAnimation
 * @model: The model to check against
 *
 * Checks if the animation is compatible with the given model.
 * The model must have the same bone structure as the animation.
 *
 * Returns: %TRUE if compatible
 */
gboolean
grl_model_animation_is_valid (GrlModelAnimation *self,
                              GrlModel          *model)
{
    GrlModelAnimationPrivate *priv;
    Model *rl_model;

    g_return_val_if_fail (GRL_IS_MODEL_ANIMATION (self), FALSE);
    g_return_val_if_fail (GRL_IS_MODEL (model), FALSE);

    priv = grl_model_animation_get_instance_private (self);

    if (!priv->valid)
        return FALSE;

    rl_model = (Model *)grl_model_get_handle (model);

    return IsModelAnimationValid (*rl_model, priv->animation);
}

/**
 * grl_model_animation_get_handle:
 * @self: A #GrlModelAnimation
 *
 * Gets the internal raylib ModelAnimation pointer.
 * This is for internal use and advanced users only.
 *
 * Returns: (transfer none): Internal animation pointer
 */
gpointer
grl_model_animation_get_handle (GrlModelAnimation *self)
{
    GrlModelAnimationPrivate *priv;

    g_return_val_if_fail (GRL_IS_MODEL_ANIMATION (self), NULL);

    priv = grl_model_animation_get_instance_private (self);

    return &priv->animation;
}
