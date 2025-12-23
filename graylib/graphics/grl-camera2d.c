/* grl-camera2d.c
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#include "grl-camera2d.h"
#include <raylib.h>

/**
 * SECTION:grl-camera2d
 * @Title: GrlCamera2D
 * @Short_description: 2D camera for transforming the view
 *
 * #GrlCamera2D provides 2D camera functionality with offset, target,
 * rotation, and zoom. Use it to create scrolling worlds, zoom effects,
 * and camera following behavior.
 *
 * # Example Usage
 *
 * |[<!-- language="C" -->
 * g_autoptr(GrlCamera2D) camera = grl_camera2d_new ();
 *
 * // Center camera on screen and follow player
 * grl_camera2d_set_offset_xy (camera, 400.0f, 300.0f);
 * grl_camera2d_set_target (camera, player_position);
 * grl_camera2d_set_zoom (camera, 2.0f);
 *
 * // In draw loop
 * grl_camera2d_begin (camera);
 * // Draw world objects here
 * grl_camera2d_end (camera);
 * ]|
 */

typedef struct
{
    gfloat offset_x;
    gfloat offset_y;
    gfloat target_x;
    gfloat target_y;
    gfloat rotation;
    gfloat zoom;
} GrlCamera2DPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (GrlCamera2D, grl_camera2d, G_TYPE_OBJECT)

enum
{
    PROP_0,
    PROP_OFFSET_X,
    PROP_OFFSET_Y,
    PROP_TARGET_X,
    PROP_TARGET_Y,
    PROP_ROTATION,
    PROP_ZOOM,
    N_PROPS
};

static GParamSpec *properties[N_PROPS];

/*
 * Helper to convert our private data to a raylib Camera2D struct
 */
static Camera2D
grl_camera2d_to_raylib (GrlCamera2D *self)
{
    GrlCamera2DPrivate *priv = grl_camera2d_get_instance_private (self);
    Camera2D camera;

    camera.offset.x = priv->offset_x;
    camera.offset.y = priv->offset_y;
    camera.target.x = priv->target_x;
    camera.target.y = priv->target_y;
    camera.rotation = priv->rotation;
    camera.zoom = priv->zoom;

    return camera;
}

static void
grl_camera2d_get_property (GObject    *object,
                           guint       prop_id,
                           GValue     *value,
                           GParamSpec *pspec)
{
    GrlCamera2D *self = GRL_CAMERA2D (object);
    GrlCamera2DPrivate *priv = grl_camera2d_get_instance_private (self);

    switch (prop_id)
    {
    case PROP_OFFSET_X:
        g_value_set_float (value, priv->offset_x);
        break;

    case PROP_OFFSET_Y:
        g_value_set_float (value, priv->offset_y);
        break;

    case PROP_TARGET_X:
        g_value_set_float (value, priv->target_x);
        break;

    case PROP_TARGET_Y:
        g_value_set_float (value, priv->target_y);
        break;

    case PROP_ROTATION:
        g_value_set_float (value, priv->rotation);
        break;

    case PROP_ZOOM:
        g_value_set_float (value, priv->zoom);
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
grl_camera2d_set_property (GObject      *object,
                           guint         prop_id,
                           const GValue *value,
                           GParamSpec   *pspec)
{
    GrlCamera2D *self = GRL_CAMERA2D (object);
    GrlCamera2DPrivate *priv = grl_camera2d_get_instance_private (self);

    switch (prop_id)
    {
    case PROP_OFFSET_X:
        priv->offset_x = g_value_get_float (value);
        break;

    case PROP_OFFSET_Y:
        priv->offset_y = g_value_get_float (value);
        break;

    case PROP_TARGET_X:
        priv->target_x = g_value_get_float (value);
        break;

    case PROP_TARGET_Y:
        priv->target_y = g_value_get_float (value);
        break;

    case PROP_ROTATION:
        priv->rotation = g_value_get_float (value);
        break;

    case PROP_ZOOM:
        {
            gfloat zoom = g_value_get_float (value);
            if (zoom > 0.0f)
                priv->zoom = zoom;
        }
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
grl_camera2d_class_init (GrlCamera2DClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->get_property = grl_camera2d_get_property;
    object_class->set_property = grl_camera2d_set_property;

    /**
     * GrlCamera2D:offset-x:
     *
     * The X component of the camera offset.
     */
    properties[PROP_OFFSET_X] =
        g_param_spec_float ("offset-x",
                            "Offset X",
                            "Camera X offset",
                            -G_MAXFLOAT, G_MAXFLOAT, 0.0f,
                            G_PARAM_READWRITE |
                            G_PARAM_STATIC_STRINGS);

    /**
     * GrlCamera2D:offset-y:
     *
     * The Y component of the camera offset.
     */
    properties[PROP_OFFSET_Y] =
        g_param_spec_float ("offset-y",
                            "Offset Y",
                            "Camera Y offset",
                            -G_MAXFLOAT, G_MAXFLOAT, 0.0f,
                            G_PARAM_READWRITE |
                            G_PARAM_STATIC_STRINGS);

    /**
     * GrlCamera2D:target-x:
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
     * GrlCamera2D:target-y:
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
     * GrlCamera2D:rotation:
     *
     * The camera rotation in degrees.
     */
    properties[PROP_ROTATION] =
        g_param_spec_float ("rotation",
                            "Rotation",
                            "Camera rotation in degrees",
                            -G_MAXFLOAT, G_MAXFLOAT, 0.0f,
                            G_PARAM_READWRITE |
                            G_PARAM_STATIC_STRINGS);

    /**
     * GrlCamera2D:zoom:
     *
     * The camera zoom level. Must be greater than 0.
     */
    properties[PROP_ZOOM] =
        g_param_spec_float ("zoom",
                            "Zoom",
                            "Camera zoom level",
                            0.001f, G_MAXFLOAT, 1.0f,
                            G_PARAM_READWRITE |
                            G_PARAM_STATIC_STRINGS);

    g_object_class_install_properties (object_class, N_PROPS, properties);
}

static void
grl_camera2d_init (GrlCamera2D *self)
{
    GrlCamera2DPrivate *priv = grl_camera2d_get_instance_private (self);

    priv->offset_x = 0.0f;
    priv->offset_y = 0.0f;
    priv->target_x = 0.0f;
    priv->target_y = 0.0f;
    priv->rotation = 0.0f;
    priv->zoom = 1.0f;
}

/**
 * grl_camera2d_new:
 *
 * Creates a new 2D camera with default settings.
 *
 * Returns: (transfer full): A new #GrlCamera2D
 */
GrlCamera2D *
grl_camera2d_new (void)
{
    return g_object_new (GRL_TYPE_CAMERA2D, NULL);
}

/**
 * grl_camera2d_get_offset:
 * @self: A #GrlCamera2D
 *
 * Gets the camera offset.
 *
 * Returns: (transfer full): The offset as a #GrlVector2
 */
GrlVector2 *
grl_camera2d_get_offset (GrlCamera2D *self)
{
    GrlCamera2DPrivate *priv;

    g_return_val_if_fail (GRL_IS_CAMERA2D (self), NULL);

    priv = grl_camera2d_get_instance_private (self);
    return grl_vector2_new (priv->offset_x, priv->offset_y);
}

/**
 * grl_camera2d_set_offset:
 * @self: A #GrlCamera2D
 * @offset: The new offset
 *
 * Sets the camera offset.
 */
void
grl_camera2d_set_offset (GrlCamera2D *self,
                         GrlVector2  *offset)
{
    GrlCamera2DPrivate *priv;

    g_return_if_fail (GRL_IS_CAMERA2D (self));
    g_return_if_fail (offset != NULL);

    priv = grl_camera2d_get_instance_private (self);
    priv->offset_x = offset->x;
    priv->offset_y = offset->y;

    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_OFFSET_X]);
    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_OFFSET_Y]);
}

/**
 * grl_camera2d_set_offset_xy:
 * @self: A #GrlCamera2D
 * @x: X offset
 * @y: Y offset
 *
 * Sets the camera offset from individual coordinates.
 */
void
grl_camera2d_set_offset_xy (GrlCamera2D *self,
                            gfloat       x,
                            gfloat       y)
{
    GrlCamera2DPrivate *priv;

    g_return_if_fail (GRL_IS_CAMERA2D (self));

    priv = grl_camera2d_get_instance_private (self);
    priv->offset_x = x;
    priv->offset_y = y;

    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_OFFSET_X]);
    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_OFFSET_Y]);
}

/**
 * grl_camera2d_get_target:
 * @self: A #GrlCamera2D
 *
 * Gets the camera target.
 *
 * Returns: (transfer full): The target as a #GrlVector2
 */
GrlVector2 *
grl_camera2d_get_target (GrlCamera2D *self)
{
    GrlCamera2DPrivate *priv;

    g_return_val_if_fail (GRL_IS_CAMERA2D (self), NULL);

    priv = grl_camera2d_get_instance_private (self);
    return grl_vector2_new (priv->target_x, priv->target_y);
}

/**
 * grl_camera2d_set_target:
 * @self: A #GrlCamera2D
 * @target: The new target position
 *
 * Sets the camera target.
 */
void
grl_camera2d_set_target (GrlCamera2D *self,
                         GrlVector2  *target)
{
    GrlCamera2DPrivate *priv;

    g_return_if_fail (GRL_IS_CAMERA2D (self));
    g_return_if_fail (target != NULL);

    priv = grl_camera2d_get_instance_private (self);
    priv->target_x = target->x;
    priv->target_y = target->y;

    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_TARGET_X]);
    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_TARGET_Y]);
}

/**
 * grl_camera2d_set_target_xy:
 * @self: A #GrlCamera2D
 * @x: X target
 * @y: Y target
 *
 * Sets the camera target from individual coordinates.
 */
void
grl_camera2d_set_target_xy (GrlCamera2D *self,
                            gfloat       x,
                            gfloat       y)
{
    GrlCamera2DPrivate *priv;

    g_return_if_fail (GRL_IS_CAMERA2D (self));

    priv = grl_camera2d_get_instance_private (self);
    priv->target_x = x;
    priv->target_y = y;

    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_TARGET_X]);
    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_TARGET_Y]);
}

/**
 * grl_camera2d_get_rotation:
 * @self: A #GrlCamera2D
 *
 * Gets the camera rotation in degrees.
 *
 * Returns: The rotation in degrees
 */
gfloat
grl_camera2d_get_rotation (GrlCamera2D *self)
{
    GrlCamera2DPrivate *priv;

    g_return_val_if_fail (GRL_IS_CAMERA2D (self), 0.0f);

    priv = grl_camera2d_get_instance_private (self);
    return priv->rotation;
}

/**
 * grl_camera2d_set_rotation:
 * @self: A #GrlCamera2D
 * @rotation: The rotation in degrees
 *
 * Sets the camera rotation in degrees.
 */
void
grl_camera2d_set_rotation (GrlCamera2D *self,
                           gfloat       rotation)
{
    GrlCamera2DPrivate *priv;

    g_return_if_fail (GRL_IS_CAMERA2D (self));

    priv = grl_camera2d_get_instance_private (self);
    priv->rotation = rotation;

    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_ROTATION]);
}

/**
 * grl_camera2d_get_zoom:
 * @self: A #GrlCamera2D
 *
 * Gets the camera zoom level.
 *
 * Returns: The zoom level
 */
gfloat
grl_camera2d_get_zoom (GrlCamera2D *self)
{
    GrlCamera2DPrivate *priv;

    g_return_val_if_fail (GRL_IS_CAMERA2D (self), 1.0f);

    priv = grl_camera2d_get_instance_private (self);
    return priv->zoom;
}

/**
 * grl_camera2d_set_zoom:
 * @self: A #GrlCamera2D
 * @zoom: The zoom level (must be > 0)
 *
 * Sets the camera zoom level.
 */
void
grl_camera2d_set_zoom (GrlCamera2D *self,
                       gfloat       zoom)
{
    GrlCamera2DPrivate *priv;

    g_return_if_fail (GRL_IS_CAMERA2D (self));
    g_return_if_fail (zoom > 0.0f);

    priv = grl_camera2d_get_instance_private (self);
    priv->zoom = zoom;

    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_ZOOM]);
}

/**
 * grl_camera2d_begin:
 * @self: A #GrlCamera2D
 *
 * Begins 2D mode with this camera.
 */
void
grl_camera2d_begin (GrlCamera2D *self)
{
    Camera2D camera;

    g_return_if_fail (GRL_IS_CAMERA2D (self));

    camera = grl_camera2d_to_raylib (self);
    BeginMode2D (camera);
}

/**
 * grl_camera2d_end:
 * @self: A #GrlCamera2D
 *
 * Ends 2D camera mode.
 */
void
grl_camera2d_end (GrlCamera2D *self)
{
    g_return_if_fail (GRL_IS_CAMERA2D (self));

    EndMode2D ();
}

/**
 * grl_camera2d_get_world_to_screen:
 * @self: A #GrlCamera2D
 * @world_pos: A world position
 *
 * Converts a world position to screen coordinates.
 *
 * Returns: (transfer full): The screen position
 */
GrlVector2 *
grl_camera2d_get_world_to_screen (GrlCamera2D *self,
                                  GrlVector2  *world_pos)
{
    Camera2D camera;
    Vector2 world;
    Vector2 screen;

    g_return_val_if_fail (GRL_IS_CAMERA2D (self), NULL);
    g_return_val_if_fail (world_pos != NULL, NULL);

    camera = grl_camera2d_to_raylib (self);
    world.x = world_pos->x;
    world.y = world_pos->y;
    screen = GetWorldToScreen2D (world, camera);

    return grl_vector2_new (screen.x, screen.y);
}

/**
 * grl_camera2d_get_screen_to_world:
 * @self: A #GrlCamera2D
 * @screen_pos: A screen position
 *
 * Converts a screen position to world coordinates.
 *
 * Returns: (transfer full): The world position
 */
GrlVector2 *
grl_camera2d_get_screen_to_world (GrlCamera2D *self,
                                  GrlVector2  *screen_pos)
{
    Camera2D camera;
    Vector2 screen;
    Vector2 world;

    g_return_val_if_fail (GRL_IS_CAMERA2D (self), NULL);
    g_return_val_if_fail (screen_pos != NULL, NULL);

    camera = grl_camera2d_to_raylib (self);
    screen.x = screen_pos->x;
    screen.y = screen_pos->y;
    world = GetScreenToWorld2D (screen, camera);

    return grl_vector2_new (world.x, world.y);
}
