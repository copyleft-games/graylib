/* grl-camera2d.h
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * 2D camera for transforming the view with offset, rotation, and zoom.
 */

#pragma once

#if !defined(GRAYLIB_INSIDE) && !defined(GRAYLIB_COMPILATION)
#error "Only <graylib.h> can be included directly."
#endif

#include <glib-object.h>
#include "../grl-version.h"
#include "../grl-types.h"
#include "../math/grl-vector2.h"

G_BEGIN_DECLS

#define GRL_TYPE_CAMERA2D (grl_camera2d_get_type())

GRL_AVAILABLE_IN_ALL
G_DECLARE_DERIVABLE_TYPE (GrlCamera2D, grl_camera2d, GRL, CAMERA2D, GObject)

/**
 * GrlCamera2DClass:
 * @parent_class: The parent class
 *
 * The class structure for #GrlCamera2D.
 */
struct _GrlCamera2DClass
{
    GObjectClass parent_class;

    /*< private >*/
    gpointer _reserved[8];
};

/**
 * grl_camera2d_new:
 *
 * Creates a new 2D camera with default settings.
 * Default offset is (0, 0), target is (0, 0), rotation is 0, zoom is 1.0.
 *
 * Returns: (transfer full): A new #GrlCamera2D
 */
GRL_AVAILABLE_IN_ALL
GrlCamera2D *       grl_camera2d_new                    (void);

/**
 * grl_camera2d_get_offset:
 * @self: A #GrlCamera2D
 *
 * Gets the camera offset (displacement from target).
 *
 * Returns: (transfer full): The offset as a #GrlVector2
 */
GRL_AVAILABLE_IN_ALL
GrlVector2 *        grl_camera2d_get_offset             (GrlCamera2D        *self);

/**
 * grl_camera2d_set_offset:
 * @self: A #GrlCamera2D
 * @offset: The new offset
 *
 * Sets the camera offset (displacement from target).
 * This is typically set to half the screen size to center the camera.
 */
GRL_AVAILABLE_IN_ALL
void                grl_camera2d_set_offset             (GrlCamera2D        *self,
                                                         GrlVector2         *offset);

/**
 * grl_camera2d_set_offset_xy:
 * @self: A #GrlCamera2D
 * @x: X offset
 * @y: Y offset
 *
 * Sets the camera offset from individual coordinates.
 */
GRL_AVAILABLE_IN_ALL
void                grl_camera2d_set_offset_xy          (GrlCamera2D        *self,
                                                         gfloat              x,
                                                         gfloat              y);

/**
 * grl_camera2d_get_target:
 * @self: A #GrlCamera2D
 *
 * Gets the camera target (rotation and zoom origin).
 *
 * Returns: (transfer full): The target as a #GrlVector2
 */
GRL_AVAILABLE_IN_ALL
GrlVector2 *        grl_camera2d_get_target             (GrlCamera2D        *self);

/**
 * grl_camera2d_set_target:
 * @self: A #GrlCamera2D
 * @target: The new target position
 *
 * Sets the camera target (rotation and zoom origin).
 * This is typically the position you want the camera to look at.
 */
GRL_AVAILABLE_IN_ALL
void                grl_camera2d_set_target             (GrlCamera2D        *self,
                                                         GrlVector2         *target);

/**
 * grl_camera2d_set_target_xy:
 * @self: A #GrlCamera2D
 * @x: X target
 * @y: Y target
 *
 * Sets the camera target from individual coordinates.
 */
GRL_AVAILABLE_IN_ALL
void                grl_camera2d_set_target_xy          (GrlCamera2D        *self,
                                                         gfloat              x,
                                                         gfloat              y);

/**
 * grl_camera2d_get_rotation:
 * @self: A #GrlCamera2D
 *
 * Gets the camera rotation in degrees.
 *
 * Returns: The rotation in degrees
 */
GRL_AVAILABLE_IN_ALL
gfloat              grl_camera2d_get_rotation           (GrlCamera2D        *self);

/**
 * grl_camera2d_set_rotation:
 * @self: A #GrlCamera2D
 * @rotation: The rotation in degrees
 *
 * Sets the camera rotation in degrees.
 */
GRL_AVAILABLE_IN_ALL
void                grl_camera2d_set_rotation           (GrlCamera2D        *self,
                                                         gfloat              rotation);

/**
 * grl_camera2d_get_zoom:
 * @self: A #GrlCamera2D
 *
 * Gets the camera zoom level.
 *
 * Returns: The zoom level (1.0 is normal)
 */
GRL_AVAILABLE_IN_ALL
gfloat              grl_camera2d_get_zoom               (GrlCamera2D        *self);

/**
 * grl_camera2d_set_zoom:
 * @self: A #GrlCamera2D
 * @zoom: The zoom level (must be > 0)
 *
 * Sets the camera zoom level. Values greater than 1.0 zoom in,
 * values less than 1.0 zoom out.
 */
GRL_AVAILABLE_IN_ALL
void                grl_camera2d_set_zoom               (GrlCamera2D        *self,
                                                         gfloat              zoom);

/**
 * grl_camera2d_begin:
 * @self: A #GrlCamera2D
 *
 * Begins 2D mode with this camera. All drawing operations until
 * grl_camera2d_end() is called will be transformed by this camera.
 */
GRL_AVAILABLE_IN_ALL
void                grl_camera2d_begin                  (GrlCamera2D        *self);

/**
 * grl_camera2d_end:
 * @self: A #GrlCamera2D
 *
 * Ends 2D camera mode and returns to default rendering.
 */
GRL_AVAILABLE_IN_ALL
void                grl_camera2d_end                    (GrlCamera2D        *self);

/**
 * grl_camera2d_get_world_to_screen:
 * @self: A #GrlCamera2D
 * @world_pos: A world position
 *
 * Converts a world position to screen coordinates using this camera.
 *
 * Returns: (transfer full): The screen position as a #GrlVector2
 */
GRL_AVAILABLE_IN_ALL
GrlVector2 *        grl_camera2d_get_world_to_screen    (GrlCamera2D        *self,
                                                         GrlVector2         *world_pos);

/**
 * grl_camera2d_get_screen_to_world:
 * @self: A #GrlCamera2D
 * @screen_pos: A screen position
 *
 * Converts a screen position to world coordinates using this camera.
 *
 * Returns: (transfer full): The world position as a #GrlVector2
 */
GRL_AVAILABLE_IN_ALL
GrlVector2 *        grl_camera2d_get_screen_to_world    (GrlCamera2D        *self,
                                                         GrlVector2         *screen_pos);

G_END_DECLS
