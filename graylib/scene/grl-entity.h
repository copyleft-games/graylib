/* grl-entity.h
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Base entity class for game objects.
 */

#pragma once

#if !defined(GRAYLIB_INSIDE) && !defined(GRAYLIB_COMPILATION)
#error "Only <graylib.h> can be included directly."
#endif

#include <glib-object.h>
#include "../grl-version.h"
#include "../math/grl-vector2.h"
#include "../math/grl-rectangle.h"

G_BEGIN_DECLS

#define GRL_TYPE_ENTITY (grl_entity_get_type ())

GRL_AVAILABLE_IN_ALL
G_DECLARE_DERIVABLE_TYPE (GrlEntity, grl_entity, GRL, ENTITY, GObject)

/**
 * GrlEntityClass:
 * @parent_class: The parent class
 * @update: Virtual method to update the entity each frame
 * @draw: Virtual method to draw the entity
 * @get_bounds: Virtual method to get the collision bounds
 * @on_collision: Virtual method called when a collision occurs
 *
 * The class structure for #GrlEntity.
 *
 * Subclasses can override these virtual methods to customize behavior.
 */
struct _GrlEntityClass
{
    GObjectClass parent_class;

    /* Virtual methods */
    void            (*update)           (GrlEntity      *self,
                                         gfloat          delta);
    void            (*draw)             (GrlEntity      *self);
    GrlRectangle *  (*get_bounds)       (GrlEntity      *self);
    void            (*on_collision)     (GrlEntity      *self,
                                         GrlEntity      *other);

    /*< private >*/
    gpointer _reserved[8];
};

/*
 * Constructors
 */

/**
 * grl_entity_new:
 *
 * Creates a new entity at position (0, 0).
 *
 * Returns: (transfer full): A new #GrlEntity
 */
GRL_AVAILABLE_IN_ALL
GrlEntity *     grl_entity_new              (void);

/**
 * grl_entity_new_at:
 * @x: Initial X position
 * @y: Initial Y position
 *
 * Creates a new entity at the specified position.
 *
 * Returns: (transfer full): A new #GrlEntity
 */
GRL_AVAILABLE_IN_ALL
GrlEntity *     grl_entity_new_at           (gfloat          x,
                                             gfloat          y);

/*
 * Position
 */

/**
 * grl_entity_get_position:
 * @self: A #GrlEntity
 *
 * Gets the entity's position.
 *
 * Returns: (transfer full): A new #GrlVector2 with the position
 */
GRL_AVAILABLE_IN_ALL
GrlVector2 *    grl_entity_get_position     (GrlEntity      *self);

/**
 * grl_entity_set_position:
 * @self: A #GrlEntity
 * @position: The new position
 *
 * Sets the entity's position.
 */
GRL_AVAILABLE_IN_ALL
void            grl_entity_set_position     (GrlEntity      *self,
                                             GrlVector2     *position);

/**
 * grl_entity_set_position_xy:
 * @self: A #GrlEntity
 * @x: X coordinate
 * @y: Y coordinate
 *
 * Sets the entity's position using X and Y coordinates.
 */
GRL_AVAILABLE_IN_ALL
void            grl_entity_set_position_xy  (GrlEntity      *self,
                                             gfloat          x,
                                             gfloat          y);

/**
 * grl_entity_get_x:
 * @self: A #GrlEntity
 *
 * Gets the entity's X position.
 *
 * Returns: The X coordinate
 */
GRL_AVAILABLE_IN_ALL
gfloat          grl_entity_get_x            (GrlEntity      *self);

/**
 * grl_entity_set_x:
 * @self: A #GrlEntity
 * @x: The X coordinate
 *
 * Sets the entity's X position.
 */
GRL_AVAILABLE_IN_ALL
void            grl_entity_set_x            (GrlEntity      *self,
                                             gfloat          x);

/**
 * grl_entity_get_y:
 * @self: A #GrlEntity
 *
 * Gets the entity's Y position.
 *
 * Returns: The Y coordinate
 */
GRL_AVAILABLE_IN_ALL
gfloat          grl_entity_get_y            (GrlEntity      *self);

/**
 * grl_entity_set_y:
 * @self: A #GrlEntity
 * @y: The Y coordinate
 *
 * Sets the entity's Y position.
 */
GRL_AVAILABLE_IN_ALL
void            grl_entity_set_y            (GrlEntity      *self,
                                             gfloat          y);

/*
 * Size
 */

/**
 * grl_entity_get_width:
 * @self: A #GrlEntity
 *
 * Gets the entity's width.
 *
 * Returns: The width
 */
GRL_AVAILABLE_IN_ALL
gfloat          grl_entity_get_width        (GrlEntity      *self);

/**
 * grl_entity_set_width:
 * @self: A #GrlEntity
 * @width: The width
 *
 * Sets the entity's width.
 */
GRL_AVAILABLE_IN_ALL
void            grl_entity_set_width        (GrlEntity      *self,
                                             gfloat          width);

/**
 * grl_entity_get_height:
 * @self: A #GrlEntity
 *
 * Gets the entity's height.
 *
 * Returns: The height
 */
GRL_AVAILABLE_IN_ALL
gfloat          grl_entity_get_height       (GrlEntity      *self);

/**
 * grl_entity_set_height:
 * @self: A #GrlEntity
 * @height: The height
 *
 * Sets the entity's height.
 */
GRL_AVAILABLE_IN_ALL
void            grl_entity_set_height       (GrlEntity      *self,
                                             gfloat          height);

/*
 * Rotation and Scale
 */

/**
 * grl_entity_get_rotation:
 * @self: A #GrlEntity
 *
 * Gets the entity's rotation in degrees.
 *
 * Returns: The rotation in degrees
 */
GRL_AVAILABLE_IN_ALL
gfloat          grl_entity_get_rotation     (GrlEntity      *self);

/**
 * grl_entity_set_rotation:
 * @self: A #GrlEntity
 * @rotation: Rotation in degrees
 *
 * Sets the entity's rotation in degrees.
 */
GRL_AVAILABLE_IN_ALL
void            grl_entity_set_rotation     (GrlEntity      *self,
                                             gfloat          rotation);

/**
 * grl_entity_get_scale:
 * @self: A #GrlEntity
 *
 * Gets the entity's scale factor.
 *
 * Returns: The scale factor
 */
GRL_AVAILABLE_IN_ALL
gfloat          grl_entity_get_scale        (GrlEntity      *self);

/**
 * grl_entity_set_scale:
 * @self: A #GrlEntity
 * @scale: Scale factor (1.0 = normal size)
 *
 * Sets the entity's scale factor.
 */
GRL_AVAILABLE_IN_ALL
void            grl_entity_set_scale        (GrlEntity      *self,
                                             gfloat          scale);

/*
 * Origin (pivot point)
 */

/**
 * grl_entity_get_origin:
 * @self: A #GrlEntity
 *
 * Gets the entity's origin (pivot point) for rotation and scaling.
 *
 * Returns: (transfer full): A new #GrlVector2 with the origin
 */
GRL_AVAILABLE_IN_ALL
GrlVector2 *    grl_entity_get_origin       (GrlEntity      *self);

/**
 * grl_entity_set_origin:
 * @self: A #GrlEntity
 * @origin: The origin point
 *
 * Sets the entity's origin (pivot point) for rotation and scaling.
 * Origin is relative to the entity's position.
 */
GRL_AVAILABLE_IN_ALL
void            grl_entity_set_origin       (GrlEntity      *self,
                                             GrlVector2     *origin);

/**
 * grl_entity_center_origin:
 * @self: A #GrlEntity
 *
 * Centers the origin at the entity's center (width/2, height/2).
 */
GRL_AVAILABLE_IN_ALL
void            grl_entity_center_origin    (GrlEntity      *self);

/*
 * Movement
 */

/**
 * grl_entity_move:
 * @self: A #GrlEntity
 * @velocity: Movement vector
 * @delta: Time delta for frame-rate independence
 *
 * Moves the entity by velocity * delta.
 */
GRL_AVAILABLE_IN_ALL
void            grl_entity_move             (GrlEntity      *self,
                                             GrlVector2     *velocity,
                                             gfloat          delta);

/**
 * grl_entity_move_xy:
 * @self: A #GrlEntity
 * @vx: X velocity
 * @vy: Y velocity
 * @delta: Time delta for frame-rate independence
 *
 * Moves the entity by (vx, vy) * delta.
 */
GRL_AVAILABLE_IN_ALL
void            grl_entity_move_xy          (GrlEntity      *self,
                                             gfloat          vx,
                                             gfloat          vy,
                                             gfloat          delta);

/**
 * grl_entity_translate:
 * @self: A #GrlEntity
 * @offset: Translation vector
 *
 * Translates the entity by the given offset (immediate, no delta).
 */
GRL_AVAILABLE_IN_ALL
void            grl_entity_translate        (GrlEntity      *self,
                                             GrlVector2     *offset);

/*
 * State
 */

/**
 * grl_entity_get_visible:
 * @self: A #GrlEntity
 *
 * Gets whether the entity is visible.
 *
 * Returns: %TRUE if visible
 */
GRL_AVAILABLE_IN_ALL
gboolean        grl_entity_get_visible      (GrlEntity      *self);

/**
 * grl_entity_set_visible:
 * @self: A #GrlEntity
 * @visible: Whether the entity should be visible
 *
 * Sets whether the entity is visible.
 */
GRL_AVAILABLE_IN_ALL
void            grl_entity_set_visible      (GrlEntity      *self,
                                             gboolean        visible);

/**
 * grl_entity_get_active:
 * @self: A #GrlEntity
 *
 * Gets whether the entity is active (receives updates).
 *
 * Returns: %TRUE if active
 */
GRL_AVAILABLE_IN_ALL
gboolean        grl_entity_get_active       (GrlEntity      *self);

/**
 * grl_entity_set_active:
 * @self: A #GrlEntity
 * @active: Whether the entity should be active
 *
 * Sets whether the entity is active.
 */
GRL_AVAILABLE_IN_ALL
void            grl_entity_set_active       (GrlEntity      *self,
                                             gboolean        active);

/**
 * grl_entity_get_z_index:
 * @self: A #GrlEntity
 *
 * Gets the entity's z-index (draw order).
 *
 * Returns: The z-index
 */
GRL_AVAILABLE_IN_ALL
gint            grl_entity_get_z_index      (GrlEntity      *self);

/**
 * grl_entity_set_z_index:
 * @self: A #GrlEntity
 * @z_index: The z-index value
 *
 * Sets the entity's z-index (draw order).
 */
GRL_AVAILABLE_IN_ALL
void            grl_entity_set_z_index      (GrlEntity      *self,
                                             gint            z_index);

/*
 * Tag system
 */

/**
 * grl_entity_get_tag:
 * @self: A #GrlEntity
 *
 * Gets the entity's tag string.
 *
 * Returns: (nullable): The tag, or %NULL if not set
 */
GRL_AVAILABLE_IN_ALL
const gchar *   grl_entity_get_tag          (GrlEntity      *self);

/**
 * grl_entity_set_tag:
 * @self: A #GrlEntity
 * @tag: (nullable): The tag string
 *
 * Sets the entity's tag. Tags are useful for identifying
 * entity types during collision handling.
 */
GRL_AVAILABLE_IN_ALL
void            grl_entity_set_tag          (GrlEntity      *self,
                                             const gchar    *tag);

/**
 * grl_entity_has_tag:
 * @self: A #GrlEntity
 * @tag: The tag to check
 *
 * Checks if the entity has the specified tag.
 *
 * Returns: %TRUE if the entity has the tag
 */
GRL_AVAILABLE_IN_ALL
gboolean        grl_entity_has_tag          (GrlEntity      *self,
                                             const gchar    *tag);

G_END_DECLS
