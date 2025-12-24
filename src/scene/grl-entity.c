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
    N_PROPS
};

static GParamSpec *properties[N_PROPS];

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
        priv->x = g_value_get_float (value);
        break;

    case PROP_Y:
        priv->y = g_value_get_float (value);
        break;

    case PROP_WIDTH:
        priv->width = g_value_get_float (value);
        break;

    case PROP_HEIGHT:
        priv->height = g_value_get_float (value);
        break;

    case PROP_ROTATION:
        priv->rotation = g_value_get_float (value);
        break;

    case PROP_SCALE:
        priv->scale = g_value_get_float (value);
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
        priv->x = x;
        g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_X]);
    }

    if (priv->y != y)
    {
        priv->y = y;
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
        priv->x = x;
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
        priv->y = y;
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
        priv->rotation = rotation;
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
        priv->scale = scale;
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

    priv->x += vx * delta;
    priv->y += vy * delta;

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

    priv->x += offset->x;
    priv->y += offset->y;

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
