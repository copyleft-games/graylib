/* grl-camera3d.c
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#include "grl-camera3d.h"
#include <raylib.h>

/**
 * SECTION:grl-camera3d
 * @Title: GrlCamera3D
 * @Short_description: 3D camera for 3D scene rendering
 *
 * #GrlCamera3D provides 3D camera functionality with position, target,
 * up vector, field of view, and projection mode settings.
 *
 * # Example Usage
 *
 * |[<!-- language="C" -->
 * g_autoptr(GrlCamera3D) camera = grl_camera3d_new ();
 *
 * // Position camera and look at origin
 * grl_camera3d_set_position_xyz (camera, 10.0f, 10.0f, 10.0f);
 * grl_camera3d_set_target_xyz (camera, 0.0f, 0.0f, 0.0f);
 * grl_camera3d_set_fovy (camera, 45.0f);
 *
 * // In draw loop
 * grl_camera3d_begin (camera);
 * // Draw 3D objects here
 * grl_camera3d_end (camera);
 * ]|
 */

typedef struct
{
    gfloat position_x;
    gfloat position_y;
    gfloat position_z;
    gfloat target_x;
    gfloat target_y;
    gfloat target_z;
    gfloat up_x;
    gfloat up_y;
    gfloat up_z;
    gfloat fovy;
    GrlCameraProjection projection;
} GrlCamera3DPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (GrlCamera3D, grl_camera3d, G_TYPE_OBJECT)

enum
{
    PROP_0,
    PROP_POSITION_X,
    PROP_POSITION_Y,
    PROP_POSITION_Z,
    PROP_TARGET_X,
    PROP_TARGET_Y,
    PROP_TARGET_Z,
    PROP_UP_X,
    PROP_UP_Y,
    PROP_UP_Z,
    PROP_FOVY,
    PROP_PROJECTION,
    N_PROPS
};

static GParamSpec *properties[N_PROPS];

/*
 * Helper to convert our private data to a raylib Camera3D struct
 */
static Camera3D
grl_camera3d_to_raylib (GrlCamera3D *self)
{
    GrlCamera3DPrivate *priv = grl_camera3d_get_instance_private (self);
    Camera3D camera;

    camera.position.x = priv->position_x;
    camera.position.y = priv->position_y;
    camera.position.z = priv->position_z;
    camera.target.x = priv->target_x;
    camera.target.y = priv->target_y;
    camera.target.z = priv->target_z;
    camera.up.x = priv->up_x;
    camera.up.y = priv->up_y;
    camera.up.z = priv->up_z;
    camera.fovy = priv->fovy;
    camera.projection = (int)priv->projection;

    return camera;
}

static void
grl_camera3d_get_property (GObject    *object,
                           guint       prop_id,
                           GValue     *value,
                           GParamSpec *pspec)
{
    GrlCamera3D *self = GRL_CAMERA3D (object);
    GrlCamera3DPrivate *priv = grl_camera3d_get_instance_private (self);

    switch (prop_id)
    {
    case PROP_POSITION_X:
        g_value_set_float (value, priv->position_x);
        break;

    case PROP_POSITION_Y:
        g_value_set_float (value, priv->position_y);
        break;

    case PROP_POSITION_Z:
        g_value_set_float (value, priv->position_z);
        break;

    case PROP_TARGET_X:
        g_value_set_float (value, priv->target_x);
        break;

    case PROP_TARGET_Y:
        g_value_set_float (value, priv->target_y);
        break;

    case PROP_TARGET_Z:
        g_value_set_float (value, priv->target_z);
        break;

    case PROP_UP_X:
        g_value_set_float (value, priv->up_x);
        break;

    case PROP_UP_Y:
        g_value_set_float (value, priv->up_y);
        break;

    case PROP_UP_Z:
        g_value_set_float (value, priv->up_z);
        break;

    case PROP_FOVY:
        g_value_set_float (value, priv->fovy);
        break;

    case PROP_PROJECTION:
        g_value_set_enum (value, priv->projection);
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
grl_camera3d_set_property (GObject      *object,
                           guint         prop_id,
                           const GValue *value,
                           GParamSpec   *pspec)
{
    GrlCamera3D *self = GRL_CAMERA3D (object);
    GrlCamera3DPrivate *priv = grl_camera3d_get_instance_private (self);

    switch (prop_id)
    {
    case PROP_POSITION_X:
        priv->position_x = g_value_get_float (value);
        break;

    case PROP_POSITION_Y:
        priv->position_y = g_value_get_float (value);
        break;

    case PROP_POSITION_Z:
        priv->position_z = g_value_get_float (value);
        break;

    case PROP_TARGET_X:
        priv->target_x = g_value_get_float (value);
        break;

    case PROP_TARGET_Y:
        priv->target_y = g_value_get_float (value);
        break;

    case PROP_TARGET_Z:
        priv->target_z = g_value_get_float (value);
        break;

    case PROP_UP_X:
        priv->up_x = g_value_get_float (value);
        break;

    case PROP_UP_Y:
        priv->up_y = g_value_get_float (value);
        break;

    case PROP_UP_Z:
        priv->up_z = g_value_get_float (value);
        break;

    case PROP_FOVY:
        priv->fovy = g_value_get_float (value);
        break;

    case PROP_PROJECTION:
        priv->projection = g_value_get_enum (value);
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
grl_camera3d_class_init (GrlCamera3DClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->get_property = grl_camera3d_get_property;
    object_class->set_property = grl_camera3d_set_property;

    /**
     * GrlCamera3D:position-x:
     *
     * The X component of the camera position.
     */
    properties[PROP_POSITION_X] =
        g_param_spec_float ("position-x",
                            "Position X",
                            "Camera X position",
                            -G_MAXFLOAT, G_MAXFLOAT, 0.0f,
                            G_PARAM_READWRITE |
                            G_PARAM_STATIC_STRINGS);

    /**
     * GrlCamera3D:position-y:
     *
     * The Y component of the camera position.
     */
    properties[PROP_POSITION_Y] =
        g_param_spec_float ("position-y",
                            "Position Y",
                            "Camera Y position",
                            -G_MAXFLOAT, G_MAXFLOAT, 10.0f,
                            G_PARAM_READWRITE |
                            G_PARAM_STATIC_STRINGS);

    /**
     * GrlCamera3D:position-z:
     *
     * The Z component of the camera position.
     */
    properties[PROP_POSITION_Z] =
        g_param_spec_float ("position-z",
                            "Position Z",
                            "Camera Z position",
                            -G_MAXFLOAT, G_MAXFLOAT, 10.0f,
                            G_PARAM_READWRITE |
                            G_PARAM_STATIC_STRINGS);

    /**
     * GrlCamera3D:target-x:
     *
     * The X component of the camera target.
     */
    properties[PROP_TARGET_X] =
        g_param_spec_float ("target-x",
                            "Target X",
                            "Camera X target",
                            -G_MAXFLOAT, G_MAXFLOAT, 0.0f,
                            G_PARAM_READWRITE |
                            G_PARAM_STATIC_STRINGS);

    /**
     * GrlCamera3D:target-y:
     *
     * The Y component of the camera target.
     */
    properties[PROP_TARGET_Y] =
        g_param_spec_float ("target-y",
                            "Target Y",
                            "Camera Y target",
                            -G_MAXFLOAT, G_MAXFLOAT, 0.0f,
                            G_PARAM_READWRITE |
                            G_PARAM_STATIC_STRINGS);

    /**
     * GrlCamera3D:target-z:
     *
     * The Z component of the camera target.
     */
    properties[PROP_TARGET_Z] =
        g_param_spec_float ("target-z",
                            "Target Z",
                            "Camera Z target",
                            -G_MAXFLOAT, G_MAXFLOAT, 0.0f,
                            G_PARAM_READWRITE |
                            G_PARAM_STATIC_STRINGS);

    /**
     * GrlCamera3D:up-x:
     *
     * The X component of the camera up vector.
     */
    properties[PROP_UP_X] =
        g_param_spec_float ("up-x",
                            "Up X",
                            "Camera up X",
                            -G_MAXFLOAT, G_MAXFLOAT, 0.0f,
                            G_PARAM_READWRITE |
                            G_PARAM_STATIC_STRINGS);

    /**
     * GrlCamera3D:up-y:
     *
     * The Y component of the camera up vector.
     */
    properties[PROP_UP_Y] =
        g_param_spec_float ("up-y",
                            "Up Y",
                            "Camera up Y",
                            -G_MAXFLOAT, G_MAXFLOAT, 1.0f,
                            G_PARAM_READWRITE |
                            G_PARAM_STATIC_STRINGS);

    /**
     * GrlCamera3D:up-z:
     *
     * The Z component of the camera up vector.
     */
    properties[PROP_UP_Z] =
        g_param_spec_float ("up-z",
                            "Up Z",
                            "Camera up Z",
                            -G_MAXFLOAT, G_MAXFLOAT, 0.0f,
                            G_PARAM_READWRITE |
                            G_PARAM_STATIC_STRINGS);

    /**
     * GrlCamera3D:fovy:
     *
     * The camera field of view in degrees (Y-axis).
     */
    properties[PROP_FOVY] =
        g_param_spec_float ("fovy",
                            "FOV Y",
                            "Camera field of view in degrees",
                            1.0f, 180.0f, 45.0f,
                            G_PARAM_READWRITE |
                            G_PARAM_STATIC_STRINGS);

    /**
     * GrlCamera3D:projection:
     *
     * The camera projection mode.
     */
    properties[PROP_PROJECTION] =
        g_param_spec_enum ("projection",
                           "Projection",
                           "Camera projection mode",
                           GRL_TYPE_CAMERA_PROJECTION,
                           GRL_CAMERA_PERSPECTIVE,
                           G_PARAM_READWRITE |
                           G_PARAM_STATIC_STRINGS);

    g_object_class_install_properties (object_class, N_PROPS, properties);
}

static void
grl_camera3d_init (GrlCamera3D *self)
{
    GrlCamera3DPrivate *priv = grl_camera3d_get_instance_private (self);

    /* Default camera position looking at origin */
    priv->position_x = 0.0f;
    priv->position_y = 10.0f;
    priv->position_z = 10.0f;
    priv->target_x = 0.0f;
    priv->target_y = 0.0f;
    priv->target_z = 0.0f;
    priv->up_x = 0.0f;
    priv->up_y = 1.0f;
    priv->up_z = 0.0f;
    priv->fovy = 45.0f;
    priv->projection = GRL_CAMERA_PERSPECTIVE;
}

/**
 * grl_camera3d_new:
 *
 * Creates a new 3D camera with default settings.
 *
 * Returns: (transfer full): A new #GrlCamera3D
 */
GrlCamera3D *
grl_camera3d_new (void)
{
    return g_object_new (GRL_TYPE_CAMERA3D, NULL);
}

/**
 * grl_camera3d_get_position:
 * @self: A #GrlCamera3D
 *
 * Gets the camera position.
 *
 * Returns: (transfer full): The position as a #GrlVector3
 */
GrlVector3 *
grl_camera3d_get_position (GrlCamera3D *self)
{
    GrlCamera3DPrivate *priv;

    g_return_val_if_fail (GRL_IS_CAMERA3D (self), NULL);

    priv = grl_camera3d_get_instance_private (self);
    return grl_vector3_new (priv->position_x, priv->position_y, priv->position_z);
}

/**
 * grl_camera3d_set_position:
 * @self: A #GrlCamera3D
 * @position: The new position
 *
 * Sets the camera position.
 */
void
grl_camera3d_set_position (GrlCamera3D *self,
                           GrlVector3  *position)
{
    GrlCamera3DPrivate *priv;

    g_return_if_fail (GRL_IS_CAMERA3D (self));
    g_return_if_fail (position != NULL);

    priv = grl_camera3d_get_instance_private (self);
    priv->position_x = position->x;
    priv->position_y = position->y;
    priv->position_z = position->z;

    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_POSITION_X]);
    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_POSITION_Y]);
    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_POSITION_Z]);
}

/**
 * grl_camera3d_set_position_xyz:
 * @self: A #GrlCamera3D
 * @x: X position
 * @y: Y position
 * @z: Z position
 *
 * Sets the camera position from individual coordinates.
 */
void
grl_camera3d_set_position_xyz (GrlCamera3D *self,
                               gfloat       x,
                               gfloat       y,
                               gfloat       z)
{
    GrlCamera3DPrivate *priv;

    g_return_if_fail (GRL_IS_CAMERA3D (self));

    priv = grl_camera3d_get_instance_private (self);
    priv->position_x = x;
    priv->position_y = y;
    priv->position_z = z;

    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_POSITION_X]);
    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_POSITION_Y]);
    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_POSITION_Z]);
}

/**
 * grl_camera3d_get_target:
 * @self: A #GrlCamera3D
 *
 * Gets the camera target.
 *
 * Returns: (transfer full): The target as a #GrlVector3
 */
GrlVector3 *
grl_camera3d_get_target (GrlCamera3D *self)
{
    GrlCamera3DPrivate *priv;

    g_return_val_if_fail (GRL_IS_CAMERA3D (self), NULL);

    priv = grl_camera3d_get_instance_private (self);
    return grl_vector3_new (priv->target_x, priv->target_y, priv->target_z);
}

/**
 * grl_camera3d_set_target:
 * @self: A #GrlCamera3D
 * @target: The new target
 *
 * Sets the camera target.
 */
void
grl_camera3d_set_target (GrlCamera3D *self,
                         GrlVector3  *target)
{
    GrlCamera3DPrivate *priv;

    g_return_if_fail (GRL_IS_CAMERA3D (self));
    g_return_if_fail (target != NULL);

    priv = grl_camera3d_get_instance_private (self);
    priv->target_x = target->x;
    priv->target_y = target->y;
    priv->target_z = target->z;

    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_TARGET_X]);
    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_TARGET_Y]);
    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_TARGET_Z]);
}

/**
 * grl_camera3d_set_target_xyz:
 * @self: A #GrlCamera3D
 * @x: X target
 * @y: Y target
 * @z: Z target
 *
 * Sets the camera target from individual coordinates.
 */
void
grl_camera3d_set_target_xyz (GrlCamera3D *self,
                             gfloat       x,
                             gfloat       y,
                             gfloat       z)
{
    GrlCamera3DPrivate *priv;

    g_return_if_fail (GRL_IS_CAMERA3D (self));

    priv = grl_camera3d_get_instance_private (self);
    priv->target_x = x;
    priv->target_y = y;
    priv->target_z = z;

    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_TARGET_X]);
    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_TARGET_Y]);
    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_TARGET_Z]);
}

/**
 * grl_camera3d_get_up:
 * @self: A #GrlCamera3D
 *
 * Gets the camera up vector.
 *
 * Returns: (transfer full): The up vector as a #GrlVector3
 */
GrlVector3 *
grl_camera3d_get_up (GrlCamera3D *self)
{
    GrlCamera3DPrivate *priv;

    g_return_val_if_fail (GRL_IS_CAMERA3D (self), NULL);

    priv = grl_camera3d_get_instance_private (self);
    return grl_vector3_new (priv->up_x, priv->up_y, priv->up_z);
}

/**
 * grl_camera3d_set_up:
 * @self: A #GrlCamera3D
 * @up: The new up vector
 *
 * Sets the camera up vector.
 */
void
grl_camera3d_set_up (GrlCamera3D *self,
                     GrlVector3  *up)
{
    GrlCamera3DPrivate *priv;

    g_return_if_fail (GRL_IS_CAMERA3D (self));
    g_return_if_fail (up != NULL);

    priv = grl_camera3d_get_instance_private (self);
    priv->up_x = up->x;
    priv->up_y = up->y;
    priv->up_z = up->z;

    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_UP_X]);
    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_UP_Y]);
    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_UP_Z]);
}

/**
 * grl_camera3d_get_fovy:
 * @self: A #GrlCamera3D
 *
 * Gets the camera field of view.
 *
 * Returns: The field of view in degrees
 */
gfloat
grl_camera3d_get_fovy (GrlCamera3D *self)
{
    GrlCamera3DPrivate *priv;

    g_return_val_if_fail (GRL_IS_CAMERA3D (self), 45.0f);

    priv = grl_camera3d_get_instance_private (self);
    return priv->fovy;
}

/**
 * grl_camera3d_set_fovy:
 * @self: A #GrlCamera3D
 * @fovy: The field of view in degrees
 *
 * Sets the camera field of view.
 */
void
grl_camera3d_set_fovy (GrlCamera3D *self,
                       gfloat       fovy)
{
    GrlCamera3DPrivate *priv;

    g_return_if_fail (GRL_IS_CAMERA3D (self));
    g_return_if_fail (fovy > 0.0f && fovy < 180.0f);

    priv = grl_camera3d_get_instance_private (self);
    priv->fovy = fovy;

    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_FOVY]);
}

/**
 * grl_camera3d_get_projection:
 * @self: A #GrlCamera3D
 *
 * Gets the camera projection mode.
 *
 * Returns: The projection mode
 */
GrlCameraProjection
grl_camera3d_get_projection (GrlCamera3D *self)
{
    GrlCamera3DPrivate *priv;

    g_return_val_if_fail (GRL_IS_CAMERA3D (self), GRL_CAMERA_PERSPECTIVE);

    priv = grl_camera3d_get_instance_private (self);
    return priv->projection;
}

/**
 * grl_camera3d_set_projection:
 * @self: A #GrlCamera3D
 * @projection: The projection mode
 *
 * Sets the camera projection mode.
 */
void
grl_camera3d_set_projection (GrlCamera3D        *self,
                             GrlCameraProjection projection)
{
    GrlCamera3DPrivate *priv;

    g_return_if_fail (GRL_IS_CAMERA3D (self));

    priv = grl_camera3d_get_instance_private (self);
    priv->projection = projection;

    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_PROJECTION]);
}

/**
 * grl_camera3d_begin:
 * @self: A #GrlCamera3D
 *
 * Begins 3D mode with this camera.
 */
void
grl_camera3d_begin (GrlCamera3D *self)
{
    Camera3D camera;

    g_return_if_fail (GRL_IS_CAMERA3D (self));

    camera = grl_camera3d_to_raylib (self);
    BeginMode3D (camera);
}

/**
 * grl_camera3d_end:
 * @self: A #GrlCamera3D
 *
 * Ends 3D camera mode.
 */
void
grl_camera3d_end (GrlCamera3D *self)
{
    g_return_if_fail (GRL_IS_CAMERA3D (self));

    EndMode3D ();
}
