/* grl-entity.c
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Base entity class for game objects.
 */

#include "config.h"
#include "grl-entity.h"
#include "grl-drawable.h"
#include "grl-updatable.h"
#include "grl-collidable.h"
#include "grl-interpolation-private.h"
#include "../grl-type-check-private.h"

/* Every accessor validates its instance, and game objects are subclasses,
 * which miss G_TYPE_CHECK_INSTANCE_TYPE's exact-type fast path. Same check,
 * cached per instance type (see grl-type-check-private.h); the stringified
 * g_return_*_if_fail() expressions are unchanged. */
static gsize entity_check_cache;
#undef GRL_IS_ENTITY
#define GRL_IS_ENTITY(obj) \
    (grl_type_check_instance_cached ((obj), GRL_TYPE_ENTITY, &entity_check_cache))

/**
 * SECTION:grl-entity
 * @title: GrlEntity
 * @short_description: Base entity class for game objects
 *
 * #GrlEntity is the base class for game objects. It provides:
 *
 * - Position, size, rotation, and scale properties
 * - Origin (pivot point) for transformations
 * - Visibility and active state management
 * - Z-index for draw ordering
 * - Tag system for identification
 *
 * Entities implement the #GrlDrawable, #GrlUpdatable, and #GrlCollidable
 * interfaces, making them suitable for use in scenes.
 *
 * # Render interpolation
 *
 * Position, rotation and scale are interpolated for drawing when a
 * fixed-timestep game loop drives the #GrlInterpolationTrack clock (see
 * grl_interpolation_set_enabled()). The getters always return the true
 * simulation state; draw code reads grl_entity_get_render_pose(), which
 * blends the pose at the start of the latest step with the current one.
 * Use grl_entity_teleport() (or grl_entity_reset_interpolation() after
 * the setters) for warps and respawns, and #GrlEntity:interpolate to opt
 * an entity out.
 *
 * # Creating Custom Entities
 *
 * |[<!-- language="C" -->
 * #define MY_TYPE_PLAYER (my_player_get_type ())
 * G_DECLARE_FINAL_TYPE (MyPlayer, my_player, MY, PLAYER, GrlEntity)
 *
 * struct _MyPlayer
 * {
 *     GrlEntity parent_instance;
 *     gfloat speed;
 *     gint health;
 * };
 *
 * static void
 * my_player_update (GrlEntity *entity, gfloat delta)
 * {
 *     MyPlayer *self = MY_PLAYER (entity);
 *
 *     if (grl_input_is_key_down (GRL_KEY_RIGHT))
 *         grl_entity_move_xy (entity, self->speed, 0, delta);
 *     if (grl_input_is_key_down (GRL_KEY_LEFT))
 *         grl_entity_move_xy (entity, -self->speed, 0, delta);
 * }
 *
 * static void
 * my_player_class_init (MyPlayerClass *klass)
 * {
 *     GrlEntityClass *entity_class = GRL_ENTITY_CLASS (klass);
 *     entity_class->update = my_player_update;
 * }
 * ]|
 */

typedef struct
{
    gfloat      x;
    gfloat      y;
    gfloat      width;
    gfloat      height;
    gfloat      rotation;
    gfloat      scale;
    gfloat      origin_x;
    gfloat      origin_y;
    gint        z_index;
    gboolean    visible;
    gboolean    active;
    gchar      *tag;

    /* Render interpolation: the pose at the start of the latest step
     * this entity moved in (see grl-interpolation.h) */
    gfloat                prev_x;
    gfloat                prev_y;
    gfloat                prev_rotation;
    gfloat                prev_scale;
    GrlInterpolationTrack interp;
} GrlEntityPrivate;

static void grl_entity_drawable_init   (GrlDrawableInterface   *iface);
static void grl_entity_updatable_init  (GrlUpdatableInterface  *iface);
static void grl_entity_collidable_init (GrlCollidableInterface *iface);

G_DEFINE_TYPE_WITH_CODE (GrlEntity, grl_entity, G_TYPE_OBJECT,
    G_ADD_PRIVATE (GrlEntity)
    G_IMPLEMENT_INTERFACE (GRL_TYPE_DRAWABLE, grl_entity_drawable_init)
    G_IMPLEMENT_INTERFACE (GRL_TYPE_UPDATABLE, grl_entity_updatable_init)
    G_IMPLEMENT_INTERFACE (GRL_TYPE_COLLIDABLE, grl_entity_collidable_init))

enum
{
    PROP_0,
    PROP_X,
    PROP_Y,
    PROP_WIDTH,
    PROP_HEIGHT,
    PROP_ROTATION,
    PROP_SCALE,
    PROP_Z_INDEX,
    PROP_VISIBLE,
    PROP_ACTIVE,
    PROP_TAG,
    PROP_INTERPOLATE,
    N_PROPS
};

static GParamSpec *properties[N_PROPS];

/*
 * Render interpolation helpers
 */

/* The first write in a simulation step keeps the pose the step began
 * with, before any field changes */
static void
entity_capture_previous (GrlEntityPrivate *priv)
{
    priv->prev_x = priv->x;
    priv->prev_y = priv->y;
    priv->prev_rotation = priv->rotation;
    priv->prev_scale = priv->scale;
}

/* Stores one interpolated field (x, y, rotation or scale) */
static void
entity_write (GrlEntityPrivate *priv,
              gfloat           *current,
              gfloat           *previous,
              gfloat            value)
{
    if (grl_interpolation_track_begin_write_inline (&priv->interp))
        entity_capture_previous (priv);

    grl_interpolation_track_write_float_inline (&priv->interp, current,
                                                previous, value);
}

/*
 * GrlDrawable interface implementation
 */

static void
grl_entity_drawable_draw (GrlDrawable *drawable)
{
    GrlEntity *self = GRL_ENTITY (drawable);
    GrlEntityClass *klass = GRL_ENTITY_GET_CLASS (self);

    if (klass->draw != NULL)
        klass->draw (self);
}

static gboolean
grl_entity_drawable_get_visible (GrlDrawable *drawable)
{
    GrlEntity *self = GRL_ENTITY (drawable);
    GrlEntityPrivate *priv = grl_entity_get_instance_private (self);

    return priv->visible;
}

static void
grl_entity_drawable_set_visible (GrlDrawable *drawable,
                                 gboolean     visible)
{
    GrlEntity *self = GRL_ENTITY (drawable);
    GrlEntityPrivate *priv = grl_entity_get_instance_private (self);

    if (priv->visible != visible)
    {
        priv->visible = visible;
        g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_VISIBLE]);
    }
}

static gint
grl_entity_drawable_get_z_index (GrlDrawable *drawable)
{
    GrlEntity *self = GRL_ENTITY (drawable);
    GrlEntityPrivate *priv = grl_entity_get_instance_private (self);

    return priv->z_index;
}

static void
grl_entity_drawable_set_z_index (GrlDrawable *drawable,
                                 gint         z_index)
{
    GrlEntity *self = GRL_ENTITY (drawable);
    GrlEntityPrivate *priv = grl_entity_get_instance_private (self);

    if (priv->z_index != z_index)
    {
        priv->z_index = z_index;
        g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_Z_INDEX]);
    }
}

static void
grl_entity_drawable_init (GrlDrawableInterface *iface)
{
    iface->draw = grl_entity_drawable_draw;
    iface->get_visible = grl_entity_drawable_get_visible;
    iface->set_visible = grl_entity_drawable_set_visible;
    iface->get_z_index = grl_entity_drawable_get_z_index;
    iface->set_z_index = grl_entity_drawable_set_z_index;
}

/*
 * GrlUpdatable interface implementation
 */

static void
grl_entity_updatable_update (GrlUpdatable *updatable,
                             gfloat        delta)
{
    GrlEntity *self = GRL_ENTITY (updatable);
    GrlEntityClass *klass = GRL_ENTITY_GET_CLASS (self);

    if (klass->update != NULL)
        klass->update (self, delta);
}

static gboolean
grl_entity_updatable_get_active (GrlUpdatable *updatable)
{
    GrlEntity *self = GRL_ENTITY (updatable);
    GrlEntityPrivate *priv = grl_entity_get_instance_private (self);

    return priv->active;
}

static void
grl_entity_updatable_set_active (GrlUpdatable *updatable,
                                 gboolean      active)
{
    GrlEntity *self = GRL_ENTITY (updatable);
    GrlEntityPrivate *priv = grl_entity_get_instance_private (self);

    if (priv->active != active)
    {
        priv->active = active;
        g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_ACTIVE]);
    }
}

static void
grl_entity_updatable_init (GrlUpdatableInterface *iface)
{
    iface->update = grl_entity_updatable_update;
    iface->get_active = grl_entity_updatable_get_active;
    iface->set_active = grl_entity_updatable_set_active;
}

/*
 * GrlCollidable interface implementation
 */

static GrlRectangle *
grl_entity_collidable_get_bounds (GrlCollidable *collidable)
{
    GrlEntity *self = GRL_ENTITY (collidable);
    GrlEntityClass *klass = GRL_ENTITY_GET_CLASS (self);

    if (klass->get_bounds != NULL)
        return klass->get_bounds (self);

    /* Default: use entity dimensions */
    {
        GrlEntityPrivate *priv = grl_entity_get_instance_private (self);

        return grl_rectangle_new (priv->x, priv->y, priv->width, priv->height);
    }
}

static void
grl_entity_collidable_on_collision (GrlCollidable *collidable,
                                    GrlCollidable *other)
{
    GrlEntity *self = GRL_ENTITY (collidable);
    GrlEntityClass *klass = GRL_ENTITY_GET_CLASS (self);

    if (klass->on_collision != NULL && GRL_IS_ENTITY (other))
        klass->on_collision (self, GRL_ENTITY (other));
}

static void
grl_entity_collidable_init (GrlCollidableInterface *iface)
{
    iface->get_bounds = grl_entity_collidable_get_bounds;
    iface->on_collision = grl_entity_collidable_on_collision;
}

/*
 * GObject implementation
 */

static void
grl_entity_finalize (GObject *object)
{
    GrlEntity *self = GRL_ENTITY (object);
    GrlEntityPrivate *priv = grl_entity_get_instance_private (self);

    g_clear_pointer (&priv->tag, g_free);

    G_OBJECT_CLASS (grl_entity_parent_class)->finalize (object);
}

static void
grl_entity_get_property (GObject    *object,
                         guint       prop_id,
                         GValue     *value,
                         GParamSpec *pspec)
{
    GrlEntity *self = GRL_ENTITY (object);
    GrlEntityPrivate *priv = grl_entity_get_instance_private (self);

    switch (prop_id)
    {
    case PROP_X:
        g_value_set_float (value, priv->x);
        break;

    case PROP_Y:
        g_value_set_float (value, priv->y);
        break;

    case PROP_WIDTH:
        g_value_set_float (value, priv->width);
        break;

    case PROP_HEIGHT:
        g_value_set_float (value, priv->height);
        break;

    case PROP_ROTATION:
        g_value_set_float (value, priv->rotation);
        break;

    case PROP_SCALE:
        g_value_set_float (value, priv->scale);
        break;

    case PROP_Z_INDEX:
        g_value_set_int (value, priv->z_index);
        break;

    case PROP_VISIBLE:
        g_value_set_boolean (value, priv->visible);
        break;

    case PROP_ACTIVE:
        g_value_set_boolean (value, priv->active);
        break;

    case PROP_TAG:
        g_value_set_string (value, priv->tag);
        break;

    case PROP_INTERPOLATE:
        g_value_set_boolean (value, priv->interp.enabled);
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
grl_entity_set_property (GObject      *object,
                         guint         prop_id,
                         const GValue *value,
                         GParamSpec   *pspec)
{
    GrlEntity *self = GRL_ENTITY (object);
    GrlEntityPrivate *priv = grl_entity_get_instance_private (self);

    switch (prop_id)
    {
    case PROP_X:
        entity_write (priv, &priv->x, &priv->prev_x, g_value_get_float (value));
        break;

    case PROP_Y:
        entity_write (priv, &priv->y, &priv->prev_y, g_value_get_float (value));
        break;

    case PROP_WIDTH:
        priv->width = g_value_get_float (value);
        break;

    case PROP_HEIGHT:
        priv->height = g_value_get_float (value);
        break;

    case PROP_ROTATION:
        entity_write (priv, &priv->rotation, &priv->prev_rotation,
                      g_value_get_float (value));
        break;

    case PROP_SCALE:
        entity_write (priv, &priv->scale, &priv->prev_scale,
                      g_value_get_float (value));
        break;

    case PROP_Z_INDEX:
        priv->z_index = g_value_get_int (value);
        break;

    case PROP_VISIBLE:
        priv->visible = g_value_get_boolean (value);
        break;

    case PROP_ACTIVE:
        priv->active = g_value_get_boolean (value);
        break;

    case PROP_TAG:
        g_free (priv->tag);
        priv->tag = g_value_dup_string (value);
        break;

    case PROP_INTERPOLATE:
        grl_entity_set_interpolate (self, g_value_get_boolean (value));
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
grl_entity_class_init (GrlEntityClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->finalize = grl_entity_finalize;
    object_class->get_property = grl_entity_get_property;
    object_class->set_property = grl_entity_set_property;

    /**
     * GrlEntity:x:
     *
     * The X position of the entity.
     */
    properties[PROP_X] =
        g_param_spec_float ("x", "X", "X position",
                            -G_MAXFLOAT, G_MAXFLOAT, 0.0f,
                            G_PARAM_READWRITE |
                            G_PARAM_STATIC_STRINGS |
                            G_PARAM_EXPLICIT_NOTIFY);

    /**
     * GrlEntity:y:
     *
     * The Y position of the entity.
     */
    properties[PROP_Y] =
        g_param_spec_float ("y", "Y", "Y position",
                            -G_MAXFLOAT, G_MAXFLOAT, 0.0f,
                            G_PARAM_READWRITE |
                            G_PARAM_STATIC_STRINGS |
                            G_PARAM_EXPLICIT_NOTIFY);

    /**
     * GrlEntity:width:
     *
     * The width of the entity.
     */
    properties[PROP_WIDTH] =
        g_param_spec_float ("width", "Width", "Entity width",
                            0.0f, G_MAXFLOAT, 0.0f,
                            G_PARAM_READWRITE |
                            G_PARAM_STATIC_STRINGS |
                            G_PARAM_EXPLICIT_NOTIFY);

    /**
     * GrlEntity:height:
     *
     * The height of the entity.
     */
    properties[PROP_HEIGHT] =
        g_param_spec_float ("height", "Height", "Entity height",
                            0.0f, G_MAXFLOAT, 0.0f,
                            G_PARAM_READWRITE |
                            G_PARAM_STATIC_STRINGS |
                            G_PARAM_EXPLICIT_NOTIFY);

    /**
     * GrlEntity:rotation:
     *
     * The rotation of the entity in degrees.
     */
    properties[PROP_ROTATION] =
        g_param_spec_float ("rotation", "Rotation", "Rotation in degrees",
                            -G_MAXFLOAT, G_MAXFLOAT, 0.0f,
                            G_PARAM_READWRITE |
                            G_PARAM_STATIC_STRINGS |
                            G_PARAM_EXPLICIT_NOTIFY);

    /**
     * GrlEntity:scale:
     *
     * The scale factor of the entity.
     */
    properties[PROP_SCALE] =
        g_param_spec_float ("scale", "Scale", "Scale factor",
                            0.0f, G_MAXFLOAT, 1.0f,
                            G_PARAM_READWRITE |
                            G_PARAM_STATIC_STRINGS |
                            G_PARAM_EXPLICIT_NOTIFY);

    /**
     * GrlEntity:z-index:
     *
     * The z-index (draw order) of the entity.
     */
    properties[PROP_Z_INDEX] =
        g_param_spec_int ("z-index", "Z-Index", "Draw order",
                          G_MININT, G_MAXINT, 0,
                          G_PARAM_READWRITE |
                          G_PARAM_STATIC_STRINGS |
                          G_PARAM_EXPLICIT_NOTIFY);

    /**
     * GrlEntity:visible:
     *
     * Whether the entity is visible.
     */
    properties[PROP_VISIBLE] =
        g_param_spec_boolean ("visible", "Visible", "Whether visible",
                              TRUE,
                              G_PARAM_READWRITE |
                              G_PARAM_STATIC_STRINGS |
                              G_PARAM_EXPLICIT_NOTIFY);

    /**
     * GrlEntity:active:
     *
     * Whether the entity is active (receives updates).
     */
    properties[PROP_ACTIVE] =
        g_param_spec_boolean ("active", "Active", "Whether active",
                              TRUE,
                              G_PARAM_READWRITE |
                              G_PARAM_STATIC_STRINGS |
                              G_PARAM_EXPLICIT_NOTIFY);

    /**
     * GrlEntity:tag:
     *
     * A tag string for identifying the entity type.
     */
    properties[PROP_TAG] =
        g_param_spec_string ("tag", "Tag", "Entity tag",
                             NULL,
                             G_PARAM_READWRITE |
                             G_PARAM_STATIC_STRINGS);

    /**
     * GrlEntity:interpolate:
     *
     * Whether the entity's position, rotation and scale are interpolated
     * for drawing while a fixed-timestep loop drives render
     * interpolation. Turn it off for entities that are positioned per
     * frame (cursors, UI anchors) or that must be drawn exactly at their
     * simulation pose.
     */
    properties[PROP_INTERPOLATE] =
        g_param_spec_boolean ("interpolate", "Interpolate",
                              "Whether the pose is interpolated for drawing",
                              TRUE,
                              G_PARAM_READWRITE |
                              G_PARAM_STATIC_STRINGS |
                              G_PARAM_EXPLICIT_NOTIFY);

    g_object_class_install_properties (object_class, N_PROPS, properties);
}

static void
grl_entity_init (GrlEntity *self)
{
    GrlEntityPrivate *priv = grl_entity_get_instance_private (self);

    priv->x = 0.0f;
    priv->y = 0.0f;
    priv->width = 0.0f;
    priv->height = 0.0f;
    priv->rotation = 0.0f;
    priv->scale = 1.0f;
    priv->origin_x = 0.0f;
    priv->origin_y = 0.0f;
    priv->z_index = 0;
    priv->visible = TRUE;
    priv->active = TRUE;
    priv->tag = NULL;

    /* New entities start with previous == current */
    entity_capture_previous (priv);
    grl_interpolation_track_init (&priv->interp);
}

/*
 * Public API
 */

GrlEntity *
grl_entity_new (void)
{
    return g_object_new (GRL_TYPE_ENTITY, NULL);
}

GrlEntity *
grl_entity_new_at (gfloat x,
                   gfloat y)
{
    return g_object_new (GRL_TYPE_ENTITY,
                         "x", x,
                         "y", y,
                         NULL);
}

GrlVector2 *
grl_entity_get_position (GrlEntity *self)
{
    GrlEntityPrivate *priv;

    g_return_val_if_fail (GRL_IS_ENTITY (self), NULL);

    priv = grl_entity_get_instance_private (self);

    return grl_vector2_new (priv->x, priv->y);
}

void
grl_entity_set_position (GrlEntity  *self,
                         GrlVector2 *position)
{
    g_return_if_fail (GRL_IS_ENTITY (self));
    g_return_if_fail (position != NULL);

    grl_entity_set_position_xy (self, position->x, position->y);
}

void
grl_entity_set_position_xy (GrlEntity *self,
                            gfloat     x,
                            gfloat     y)
{
    GrlEntityPrivate *priv;

    g_return_if_fail (GRL_IS_ENTITY (self));

    priv = grl_entity_get_instance_private (self);

    if (priv->x != x)
    {
        entity_write (priv, &priv->x, &priv->prev_x, x);
        g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_X]);
    }

    if (priv->y != y)
    {
        entity_write (priv, &priv->y, &priv->prev_y, y);
        g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_Y]);
    }
}

gfloat
grl_entity_get_x (GrlEntity *self)
{
    GrlEntityPrivate *priv;

    g_return_val_if_fail (GRL_IS_ENTITY (self), 0.0f);

    priv = grl_entity_get_instance_private (self);

    return priv->x;
}

void
grl_entity_set_x (GrlEntity *self,
                  gfloat     x)
{
    GrlEntityPrivate *priv;

    g_return_if_fail (GRL_IS_ENTITY (self));

    priv = grl_entity_get_instance_private (self);

    if (priv->x != x)
    {
        entity_write (priv, &priv->x, &priv->prev_x, x);
        g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_X]);
    }
}

gfloat
grl_entity_get_y (GrlEntity *self)
{
    GrlEntityPrivate *priv;

    g_return_val_if_fail (GRL_IS_ENTITY (self), 0.0f);

    priv = grl_entity_get_instance_private (self);

    return priv->y;
}

void
grl_entity_set_y (GrlEntity *self,
                  gfloat     y)
{
    GrlEntityPrivate *priv;

    g_return_if_fail (GRL_IS_ENTITY (self));

    priv = grl_entity_get_instance_private (self);

    if (priv->y != y)
    {
        entity_write (priv, &priv->y, &priv->prev_y, y);
        g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_Y]);
    }
}

gfloat
grl_entity_get_width (GrlEntity *self)
{
    GrlEntityPrivate *priv;

    g_return_val_if_fail (GRL_IS_ENTITY (self), 0.0f);

    priv = grl_entity_get_instance_private (self);

    return priv->width;
}

void
grl_entity_set_width (GrlEntity *self,
                      gfloat     width)
{
    GrlEntityPrivate *priv;

    g_return_if_fail (GRL_IS_ENTITY (self));

    priv = grl_entity_get_instance_private (self);

    if (priv->width != width)
    {
        priv->width = width;
        g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_WIDTH]);
    }
}

gfloat
grl_entity_get_height (GrlEntity *self)
{
    GrlEntityPrivate *priv;

    g_return_val_if_fail (GRL_IS_ENTITY (self), 0.0f);

    priv = grl_entity_get_instance_private (self);

    return priv->height;
}

void
grl_entity_set_height (GrlEntity *self,
                       gfloat     height)
{
    GrlEntityPrivate *priv;

    g_return_if_fail (GRL_IS_ENTITY (self));

    priv = grl_entity_get_instance_private (self);

    if (priv->height != height)
    {
        priv->height = height;
        g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_HEIGHT]);
    }
}

gfloat
grl_entity_get_rotation (GrlEntity *self)
{
    GrlEntityPrivate *priv;

    g_return_val_if_fail (GRL_IS_ENTITY (self), 0.0f);

    priv = grl_entity_get_instance_private (self);

    return priv->rotation;
}

void
grl_entity_set_rotation (GrlEntity *self,
                         gfloat     rotation)
{
    GrlEntityPrivate *priv;

    g_return_if_fail (GRL_IS_ENTITY (self));

    priv = grl_entity_get_instance_private (self);

    if (priv->rotation != rotation)
    {
        entity_write (priv, &priv->rotation, &priv->prev_rotation, rotation);
        g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_ROTATION]);
    }
}

gfloat
grl_entity_get_scale (GrlEntity *self)
{
    GrlEntityPrivate *priv;

    g_return_val_if_fail (GRL_IS_ENTITY (self), 1.0f);

    priv = grl_entity_get_instance_private (self);

    return priv->scale;
}

void
grl_entity_set_scale (GrlEntity *self,
                      gfloat     scale)
{
    GrlEntityPrivate *priv;

    g_return_if_fail (GRL_IS_ENTITY (self));

    priv = grl_entity_get_instance_private (self);

    if (priv->scale != scale)
    {
        entity_write (priv, &priv->scale, &priv->prev_scale, scale);
        g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_SCALE]);
    }
}

GrlVector2 *
grl_entity_get_origin (GrlEntity *self)
{
    GrlEntityPrivate *priv;

    g_return_val_if_fail (GRL_IS_ENTITY (self), NULL);

    priv = grl_entity_get_instance_private (self);

    return grl_vector2_new (priv->origin_x, priv->origin_y);
}

void
grl_entity_get_origin_xy (GrlEntity *self,
                          gfloat    *x,
                          gfloat    *y)
{
    GrlEntityPrivate *priv;

    g_return_if_fail (GRL_IS_ENTITY (self));

    priv = grl_entity_get_instance_private (self);

    if (x != NULL)
        *x = priv->origin_x;
    if (y != NULL)
        *y = priv->origin_y;
}

void
grl_entity_set_origin (GrlEntity  *self,
                       GrlVector2 *origin)
{
    GrlEntityPrivate *priv;

    g_return_if_fail (GRL_IS_ENTITY (self));
    g_return_if_fail (origin != NULL);

    priv = grl_entity_get_instance_private (self);

    priv->origin_x = origin->x;
    priv->origin_y = origin->y;
}

void
grl_entity_center_origin (GrlEntity *self)
{
    GrlEntityPrivate *priv;

    g_return_if_fail (GRL_IS_ENTITY (self));

    priv = grl_entity_get_instance_private (self);

    priv->origin_x = priv->width / 2.0f;
    priv->origin_y = priv->height / 2.0f;
}

void
grl_entity_move (GrlEntity  *self,
                 GrlVector2 *velocity,
                 gfloat      delta)
{
    g_return_if_fail (GRL_IS_ENTITY (self));
    g_return_if_fail (velocity != NULL);

    grl_entity_move_xy (self, velocity->x, velocity->y, delta);
}

void
grl_entity_move_xy (GrlEntity *self,
                    gfloat     vx,
                    gfloat     vy,
                    gfloat     delta)
{
    GrlEntityPrivate *priv;

    g_return_if_fail (GRL_IS_ENTITY (self));

    priv = grl_entity_get_instance_private (self);

    entity_write (priv, &priv->x, &priv->prev_x, priv->x + vx * delta);
    entity_write (priv, &priv->y, &priv->prev_y, priv->y + vy * delta);

    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_X]);
    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_Y]);
}

void
grl_entity_translate (GrlEntity  *self,
                      GrlVector2 *offset)
{
    GrlEntityPrivate *priv;

    g_return_if_fail (GRL_IS_ENTITY (self));
    g_return_if_fail (offset != NULL);

    priv = grl_entity_get_instance_private (self);

    entity_write (priv, &priv->x, &priv->prev_x, priv->x + offset->x);
    entity_write (priv, &priv->y, &priv->prev_y, priv->y + offset->y);

    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_X]);
    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_Y]);
}

gboolean
grl_entity_get_visible (GrlEntity *self)
{
    GrlEntityPrivate *priv;

    g_return_val_if_fail (GRL_IS_ENTITY (self), FALSE);

    priv = grl_entity_get_instance_private (self);

    return priv->visible;
}

void
grl_entity_set_visible (GrlEntity *self,
                        gboolean   visible)
{
    grl_drawable_set_visible (GRL_DRAWABLE (self), visible);
}

gboolean
grl_entity_get_active (GrlEntity *self)
{
    GrlEntityPrivate *priv;

    g_return_val_if_fail (GRL_IS_ENTITY (self), FALSE);

    priv = grl_entity_get_instance_private (self);

    return priv->active;
}

void
grl_entity_set_active (GrlEntity *self,
                       gboolean   active)
{
    grl_updatable_set_active (GRL_UPDATABLE (self), active);
}

gint
grl_entity_get_z_index (GrlEntity *self)
{
    GrlEntityPrivate *priv;

    g_return_val_if_fail (GRL_IS_ENTITY (self), 0);

    priv = grl_entity_get_instance_private (self);

    return priv->z_index;
}

void
grl_entity_set_z_index (GrlEntity *self,
                        gint       z_index)
{
    grl_drawable_set_z_index (GRL_DRAWABLE (self), z_index);
}

const gchar *
grl_entity_get_tag (GrlEntity *self)
{
    GrlEntityPrivate *priv;

    g_return_val_if_fail (GRL_IS_ENTITY (self), NULL);

    priv = grl_entity_get_instance_private (self);

    return priv->tag;
}

void
grl_entity_set_tag (GrlEntity   *self,
                    const gchar *tag)
{
    GrlEntityPrivate *priv;

    g_return_if_fail (GRL_IS_ENTITY (self));

    priv = grl_entity_get_instance_private (self);

    g_free (priv->tag);
    priv->tag = g_strdup (tag);

    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_TAG]);
}

gboolean
grl_entity_has_tag (GrlEntity   *self,
                    const gchar *tag)
{
    GrlEntityPrivate *priv;

    g_return_val_if_fail (GRL_IS_ENTITY (self), FALSE);

    if (tag == NULL)
        return FALSE;

    priv = grl_entity_get_instance_private (self);

    if (priv->tag == NULL)
        return FALSE;

    return g_strcmp0 (priv->tag, tag) == 0;
}

/*
 * Render interpolation
 */

void
grl_entity_get_render_pose (GrlEntity *self,
                            gfloat    *x,
                            gfloat    *y,
                            gfloat    *rotation,
                            gfloat    *scale)
{
    GrlEntityPrivate *priv;
    gfloat            alpha;

    g_return_if_fail (GRL_IS_ENTITY (self));

    priv = grl_entity_get_instance_private (self);

    /* Not moved in the latest step (or interpolation off): true pose */
    if (!grl_interpolation_track_get_blend_inline (&priv->interp, &alpha))
    {
        if (x != NULL)
            *x = priv->x;
        if (y != NULL)
            *y = priv->y;
        if (rotation != NULL)
            *rotation = priv->rotation;
        if (scale != NULL)
            *scale = priv->scale;
        return;
    }

    /* Blend from the pose the step began with to the true pose */
    if (x != NULL)
        *x = grl_interpolation_lerp_inline (priv->prev_x, priv->x, alpha);
    if (y != NULL)
        *y = grl_interpolation_lerp_inline (priv->prev_y, priv->y, alpha);
    if (rotation != NULL)
        *rotation = grl_interpolation_lerp_angle_inline (priv->prev_rotation,
                                                         priv->rotation, alpha,
                                                         360.0f);
    if (scale != NULL)
        *scale = grl_interpolation_lerp_inline (priv->prev_scale, priv->scale,
                                                alpha);
}

void
grl_entity_get_render_position_xy (GrlEntity *self,
                                   gfloat    *x,
                                   gfloat    *y)
{
    grl_entity_get_render_pose (self, x, y, NULL, NULL);
}

gfloat
grl_entity_get_render_rotation (GrlEntity *self)
{
    gfloat rotation;

    g_return_val_if_fail (GRL_IS_ENTITY (self), 0.0f);

    rotation = 0.0f;
    grl_entity_get_render_pose (self, NULL, NULL, &rotation, NULL);
    return rotation;
}

gfloat
grl_entity_get_render_scale (GrlEntity *self)
{
    gfloat scale;

    g_return_val_if_fail (GRL_IS_ENTITY (self), 1.0f);

    scale = 1.0f;
    grl_entity_get_render_pose (self, NULL, NULL, NULL, &scale);
    return scale;
}

void
grl_entity_teleport (GrlEntity *self,
                     gfloat     x,
                     gfloat     y)
{
    g_return_if_fail (GRL_IS_ENTITY (self));

    grl_entity_set_position_xy (self, x, y);
    grl_entity_reset_interpolation (self);
}

void
grl_entity_reset_interpolation (GrlEntity *self)
{
    GrlEntityPrivate *priv;

    g_return_if_fail (GRL_IS_ENTITY (self));

    priv = grl_entity_get_instance_private (self);

    /* Previous == current, and later writes in this step follow along */
    entity_capture_previous (priv);
    grl_interpolation_track_reset (&priv->interp);
}

gboolean
grl_entity_get_interpolate (GrlEntity *self)
{
    GrlEntityPrivate *priv;

    g_return_val_if_fail (GRL_IS_ENTITY (self), FALSE);

    priv = grl_entity_get_instance_private (self);

    return priv->interp.enabled;
}

void
grl_entity_set_interpolate (GrlEntity *self,
                            gboolean   interpolate)
{
    GrlEntityPrivate *priv;

    g_return_if_fail (GRL_IS_ENTITY (self));

    priv = grl_entity_get_instance_private (self);

    interpolate = (interpolate != FALSE);
    if (priv->interp.enabled == interpolate)
        return;

    /* Either way the entity starts over from its true pose */
    priv->interp.enabled = interpolate;
    grl_entity_reset_interpolation (self);
    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_INTERPOLATE]);
}
