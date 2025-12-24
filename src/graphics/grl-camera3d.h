/* grl-camera3d.h
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * 3D camera for 3D scene rendering.
 */

#pragma once

#if !defined(GRAYLIB_INSIDE) && !defined(GRAYLIB_COMPILATION)
#error "Only <graylib.h> can be included directly."
#endif

#include <glib-object.h>
#include "../grl-version.h"
#include "../grl-types.h"
#include "../grl-enums.h"
#include "../math/grl-vector3.h"

G_BEGIN_DECLS

#define GRL_TYPE_CAMERA3D (grl_camera3d_get_type())

GRL_AVAILABLE_IN_ALL
G_DECLARE_DERIVABLE_TYPE (GrlCamera3D, grl_camera3d, GRL, CAMERA3D, GObject)

/**
 * GrlCamera3DClass:
 * @parent_class: The parent class
 *
 * The class structure for #GrlCamera3D.
 */
struct _GrlCamera3DClass
{
    GObjectClass parent_class;

    /*< private >*/
    gpointer _reserved[8];
};

/**
 * grl_camera3d_new:
 *
 * Creates a new 3D camera with default settings.
 * Default position is (0, 10, 10), target is (0, 0, 0),
 * up is (0, 1, 0), fovy is 45, and projection is perspective.
 *
 * Returns: (transfer full): A new #GrlCamera3D
 */
GRL_AVAILABLE_IN_ALL
GrlCamera3D *       grl_camera3d_new                    (void);

/**
 * grl_camera3d_get_position:
 * @self: A #GrlCamera3D
 *
 * Gets the camera position.
 *
 * Returns: (transfer full): The position as a #GrlVector3
 */
GRL_AVAILABLE_IN_ALL
GrlVector3 *        grl_camera3d_get_position           (GrlCamera3D        *self);

/**
 * grl_camera3d_set_position:
 * @self: A #GrlCamera3D
 * @position: The new position
 *
 * Sets the camera position.
 */
GRL_AVAILABLE_IN_ALL
void                grl_camera3d_set_position           (GrlCamera3D        *self,
                                                         GrlVector3         *position);

/**
 * grl_camera3d_set_position_xyz:
 * @self: A #GrlCamera3D
 * @x: X position
 * @y: Y position
 * @z: Z position
 *
 * Sets the camera position from individual coordinates.
 */
GRL_AVAILABLE_IN_ALL
void                grl_camera3d_set_position_xyz       (GrlCamera3D        *self,
                                                         gfloat              x,
                                                         gfloat              y,
                                                         gfloat              z);

/**
 * grl_camera3d_get_target:
 * @self: A #GrlCamera3D
 *
 * Gets the camera target (the point the camera looks at).
 *
 * Returns: (transfer full): The target as a #GrlVector3
 */
GRL_AVAILABLE_IN_ALL
GrlVector3 *        grl_camera3d_get_target             (GrlCamera3D        *self);

/**
 * grl_camera3d_set_target:
 * @self: A #GrlCamera3D
 * @target: The new target
 *
 * Sets the camera target (the point the camera looks at).
 */
GRL_AVAILABLE_IN_ALL
void                grl_camera3d_set_target             (GrlCamera3D        *self,
                                                         GrlVector3         *target);

/**
 * grl_camera3d_set_target_xyz:
 * @self: A #GrlCamera3D
 * @x: X target
 * @y: Y target
 * @z: Z target
 *
 * Sets the camera target from individual coordinates.
 */
GRL_AVAILABLE_IN_ALL
void                grl_camera3d_set_target_xyz         (GrlCamera3D        *self,
                                                         gfloat              x,
                                                         gfloat              y,
                                                         gfloat              z);

/**
 * grl_camera3d_get_up:
 * @self: A #GrlCamera3D
 *
 * Gets the camera up vector.
 *
 * Returns: (transfer full): The up vector as a #GrlVector3
 */
GRL_AVAILABLE_IN_ALL
GrlVector3 *        grl_camera3d_get_up                 (GrlCamera3D        *self);

/**
 * grl_camera3d_set_up:
 * @self: A #GrlCamera3D
 * @up: The new up vector
 *
 * Sets the camera up vector.
 */
GRL_AVAILABLE_IN_ALL
void                grl_camera3d_set_up                 (GrlCamera3D        *self,
                                                         GrlVector3         *up);

/**
 * grl_camera3d_get_fovy:
 * @self: A #GrlCamera3D
 *
 * Gets the camera field of view (Y-axis, in degrees).
 * In orthographic mode, this is used as the near plane width.
 *
 * Returns: The field of view in degrees
 */
GRL_AVAILABLE_IN_ALL
gfloat              grl_camera3d_get_fovy               (GrlCamera3D        *self);

/**
 * grl_camera3d_set_fovy:
 * @self: A #GrlCamera3D
 * @fovy: The field of view in degrees
 *
 * Sets the camera field of view.
 */
GRL_AVAILABLE_IN_ALL
void                grl_camera3d_set_fovy               (GrlCamera3D        *self,
                                                         gfloat              fovy);

/**
 * grl_camera3d_get_projection:
 * @self: A #GrlCamera3D
 *
 * Gets the camera projection mode.
 *
 * Returns: The #GrlCameraProjection mode
 */
GRL_AVAILABLE_IN_ALL
GrlCameraProjection grl_camera3d_get_projection         (GrlCamera3D        *self);

/**
 * grl_camera3d_set_projection:
 * @self: A #GrlCamera3D
 * @projection: The projection mode
 *
 * Sets the camera projection mode (perspective or orthographic).
 */
GRL_AVAILABLE_IN_ALL
void                grl_camera3d_set_projection         (GrlCamera3D        *self,
                                                         GrlCameraProjection projection);

/**
 * grl_camera3d_begin:
 * @self: A #GrlCamera3D
 *
 * Begins 3D mode with this camera. All drawing operations until
 * grl_camera3d_end() is called will be rendered in 3D.
 */
GRL_AVAILABLE_IN_ALL
void                grl_camera3d_begin                  (GrlCamera3D        *self);

/**
 * grl_camera3d_end:
 * @self: A #GrlCamera3D
 *
 * Ends 3D camera mode and returns to 2D rendering.
 */
GRL_AVAILABLE_IN_ALL
void                grl_camera3d_end                    (GrlCamera3D        *self);

G_END_DECLS
