/* grl-scene.c
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Implementation of scene container.
 */

#include "config.h"
#include "grl-scene.h"
#include "grl-entity.h"
#include "grl-drawable.h"
#include "grl-updatable.h"

/**
 * SECTION:grl-scene
 * @Title: GrlScene
 * @Short_description: A container for game entities
 *
 * #GrlScene is a container that manages a collection of entities.
 * It provides lifecycle methods (enter, exit, pause, resume) that
 * can be overridden in subclasses to implement custom scene behavior.
 *
 * Scenes automatically update and draw their entities, calling
 * the appropriate interface methods on each entity.
 *
 * # Creating a Custom Scene
 *
 * To create a custom scene, subclass #GrlScene and override the
 * virtual methods:
 *
 * |[<!-- language="C" -->
 * #define MY_TYPE_GAME_SCENE (my_game_scene_get_type ())
 * G_DECLARE_FINAL_TYPE (MyGameScene, my_game_scene, MY, GAME_SCENE, GrlScene)
 *
 * static void
 * my_game_scene_enter (GrlScene *scene)
 * {
 *     MyGameScene *self = MY_GAME_SCENE (scene);
 *
 *     // Load resources, create entities
 *     g_autoptr(GrlSprite) player = grl_sprite_new ();
 *     grl_scene_add_entity (scene, GRL_ENTITY (player));
 *
 *     // Chain up
 *     GRL_SCENE_CLASS (my_game_scene_parent_class)->enter (scene);
 * }
 *
 * static void
 * my_game_scene_class_init (MyGameSceneClass *klass)
 * {
 *     GrlSceneClass *scene_class = GRL_SCENE_CLASS (klass);
 *     scene_class->enter = my_game_scene_enter;
 * }
 * ]|
 */

typedef struct
{
    gchar    *name;
    GList    *entities;
    gboolean  active;
    gboolean  paused;
    gboolean  visible;
    gboolean  update_when_paused;
    gboolean  draw_when_paused;
    gboolean  entities_dirty;  /* Need z-sort */
} GrlScenePrivate;

G_DEFINE_TYPE_WITH_PRIVATE (GrlScene, grl_scene, G_TYPE_OBJECT)

enum
{
    PROP_0,
    PROP_NAME,
    PROP_ACTIVE,
    PROP_PAUSED,
    PROP_VISIBLE,
    PROP_UPDATE_WHEN_PAUSED,
    PROP_DRAW_WHEN_PAUSED,
    PROP_ENTITY_COUNT,
    N_PROPS
};

static GParamSpec *properties[N_PROPS];

enum
{
    SIGNAL_ENTERED,
    SIGNAL_EXITED,
    SIGNAL_PAUSED,
    SIGNAL_RESUMED,
    SIGNAL_ENTITY_ADDED,
    SIGNAL_ENTITY_REMOVED,
    N_SIGNALS
};

static guint signals[N_SIGNALS];

/*
 * Helper function to compare entities by z-index for sorting
 */
static gint
compare_entities_by_z (gconstpointer a,
                       gconstpointer b)
{
    GrlEntity *entity_a = GRL_ENTITY ((gpointer)a);
    GrlEntity *entity_b = GRL_ENTITY ((gpointer)b);
    gint z_a;
    gint z_b;

    if (GRL_IS_DRAWABLE (entity_a))
        z_a = grl_drawable_get_z_index (GRL_DRAWABLE (entity_a));
    else
        z_a = 0;

    if (GRL_IS_DRAWABLE (entity_b))
        z_b = grl_drawable_get_z_index (GRL_DRAWABLE (entity_b));
    else
        z_b = 0;

    return z_a - z_b;
}

/*
 * Default virtual method implementations
 */

static void
grl_scene_real_enter (GrlScene *self)
{
    GrlScenePrivate *priv = grl_scene_get_instance_private (self);

    priv->active = TRUE;
    priv->paused = FALSE;
    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_ACTIVE]);
    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_PAUSED]);
}

static void
grl_scene_real_exit (GrlScene *self)
{
    GrlScenePrivate *priv = grl_scene_get_instance_private (self);

    priv->active = FALSE;
    priv->paused = FALSE;
    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_ACTIVE]);
    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_PAUSED]);
}

static void
grl_scene_real_pause (GrlScene *self)
{
    GrlScenePrivate *priv = grl_scene_get_instance_private (self);

    priv->paused = TRUE;
    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_PAUSED]);
}

static void
grl_scene_real_resume (GrlScene *self)
{
    GrlScenePrivate *priv = grl_scene_get_instance_private (self);

    priv->paused = FALSE;
    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_PAUSED]);
}

static void
grl_scene_real_update (GrlScene *self,
                       gfloat    delta)
{
    GrlScenePrivate *priv = grl_scene_get_instance_private (self);
    GList *l;

    /* Skip update if paused and not set to update when paused */
    if (priv->paused && !priv->update_when_paused)
        return;

    /* Update all updatable entities */
    for (l = priv->entities; l != NULL; l = l->next)
    {
        GrlEntity *entity = GRL_ENTITY (l->data);

        if (GRL_IS_UPDATABLE (entity))
        {
            grl_updatable_update (GRL_UPDATABLE (entity), delta);
        }
    }
}

static void
grl_scene_real_draw (GrlScene *self)
{
    GrlScenePrivate *priv = grl_scene_get_instance_private (self);
    GList *l;

    /* Skip draw if not visible */
    if (!priv->visible)
        return;

    /* Skip draw if paused and not set to draw when paused */
    if (priv->paused && !priv->draw_when_paused)
        return;

    /* Sort entities by z-index if dirty */
    if (priv->entities_dirty)
    {
        priv->entities = g_list_sort (priv->entities, compare_entities_by_z);
        priv->entities_dirty = FALSE;
    }

    /* Draw all drawable entities */
    for (l = priv->entities; l != NULL; l = l->next)
    {
        GrlEntity *entity = GRL_ENTITY (l->data);

        if (GRL_IS_DRAWABLE (entity))
        {
            grl_drawable_draw (GRL_DRAWABLE (entity));
        }
    }
}

/*
 * GObject implementation
 */

static void
grl_scene_finalize (GObject *object)
{
    GrlScene *self = GRL_SCENE (object);
    GrlScenePrivate *priv = grl_scene_get_instance_private (self);

    g_clear_pointer (&priv->name, g_free);
    g_list_free_full (priv->entities, g_object_unref);
    priv->entities = NULL;

    G_OBJECT_CLASS (grl_scene_parent_class)->finalize (object);
}

static void
grl_scene_get_property (GObject    *object,
                        guint       prop_id,
                        GValue     *value,
                        GParamSpec *pspec)
{
    GrlScene *self = GRL_SCENE (object);
    GrlScenePrivate *priv = grl_scene_get_instance_private (self);

    switch (prop_id)
    {
    case PROP_NAME:
        g_value_set_string (value, priv->name);
        break;

    case PROP_ACTIVE:
        g_value_set_boolean (value, priv->active);
        break;

    case PROP_PAUSED:
        g_value_set_boolean (value, priv->paused);
        break;

    case PROP_VISIBLE:
        g_value_set_boolean (value, priv->visible);
        break;

    case PROP_UPDATE_WHEN_PAUSED:
        g_value_set_boolean (value, priv->update_when_paused);
        break;

    case PROP_DRAW_WHEN_PAUSED:
        g_value_set_boolean (value, priv->draw_when_paused);
        break;

    case PROP_ENTITY_COUNT:
        g_value_set_uint (value, g_list_length (priv->entities));
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
grl_scene_set_property (GObject      *object,
                        guint         prop_id,
                        const GValue *value,
                        GParamSpec   *pspec)
{
    GrlScene *self = GRL_SCENE (object);
    GrlScenePrivate *priv = grl_scene_get_instance_private (self);

    switch (prop_id)
    {
    case PROP_NAME:
        g_free (priv->name);
        priv->name = g_value_dup_string (value);
        break;

    case PROP_VISIBLE:
        priv->visible = g_value_get_boolean (value);
        break;

    case PROP_UPDATE_WHEN_PAUSED:
        priv->update_when_paused = g_value_get_boolean (value);
        break;

    case PROP_DRAW_WHEN_PAUSED:
        priv->draw_when_paused = g_value_get_boolean (value);
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
grl_scene_class_init (GrlSceneClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->finalize = grl_scene_finalize;
    object_class->get_property = grl_scene_get_property;
    object_class->set_property = grl_scene_set_property;

    /* Default virtual implementations */
    klass->enter = grl_scene_real_enter;
    klass->exit = grl_scene_real_exit;
    klass->pause = grl_scene_real_pause;
    klass->resume = grl_scene_real_resume;
    klass->update = grl_scene_real_update;
    klass->draw = grl_scene_real_draw;

    /**
     * GrlScene:name:
     *
     * The name of the scene, for identification and debugging.
     */
    properties[PROP_NAME] =
        g_param_spec_string ("name",
                             "Name",
                             "The scene name",
                             NULL,
                             G_PARAM_READWRITE |
                             G_PARAM_STATIC_STRINGS);

    /**
     * GrlScene:active:
     *
     * Whether the scene is currently active (entered but not exited).
     */
    properties[PROP_ACTIVE] =
        g_param_spec_boolean ("active",
                              "Active",
                              "Whether the scene is active",
                              FALSE,
                              G_PARAM_READABLE |
                              G_PARAM_STATIC_STRINGS);

    /**
     * GrlScene:paused:
     *
     * Whether the scene is currently paused.
     */
    properties[PROP_PAUSED] =
        g_param_spec_boolean ("paused",
                              "Paused",
                              "Whether the scene is paused",
                              FALSE,
                              G_PARAM_READABLE |
                              G_PARAM_STATIC_STRINGS);

    /**
     * GrlScene:visible:
     *
     * Whether the scene should be drawn.
     */
    properties[PROP_VISIBLE] =
        g_param_spec_boolean ("visible",
                              "Visible",
                              "Whether the scene is visible",
                              TRUE,
                              G_PARAM_READWRITE |
                              G_PARAM_STATIC_STRINGS |
                              G_PARAM_EXPLICIT_NOTIFY);

    /**
     * GrlScene:update-when-paused:
     *
     * Whether to continue updating entities when the scene is paused.
     */
    properties[PROP_UPDATE_WHEN_PAUSED] =
        g_param_spec_boolean ("update-when-paused",
                              "Update When Paused",
                              "Whether to update when paused",
                              FALSE,
                              G_PARAM_READWRITE |
                              G_PARAM_STATIC_STRINGS);

    /**
     * GrlScene:draw-when-paused:
     *
     * Whether to continue drawing when the scene is paused.
     * This is useful for showing a paused game state with an overlay.
     */
    properties[PROP_DRAW_WHEN_PAUSED] =
        g_param_spec_boolean ("draw-when-paused",
                              "Draw When Paused",
                              "Whether to draw when paused",
                              TRUE,
                              G_PARAM_READWRITE |
                              G_PARAM_STATIC_STRINGS);

    /**
     * GrlScene:entity-count:
     *
     * The number of entities in the scene.
     */
    properties[PROP_ENTITY_COUNT] =
        g_param_spec_uint ("entity-count",
                           "Entity Count",
                           "Number of entities in the scene",
                           0, G_MAXUINT, 0,
                           G_PARAM_READABLE |
                           G_PARAM_STATIC_STRINGS);

    g_object_class_install_properties (object_class, N_PROPS, properties);

    /**
     * GrlScene::entered:
     * @self: The #GrlScene
     *
     * Emitted when the scene is entered.
     */
    signals[SIGNAL_ENTERED] =
        g_signal_new ("entered",
                      G_TYPE_FROM_CLASS (klass),
                      G_SIGNAL_RUN_LAST,
                      0,
                      NULL, NULL,
                      NULL,
                      G_TYPE_NONE, 0);

    /**
     * GrlScene::exited:
     * @self: The #GrlScene
     *
     * Emitted when the scene is exited.
     */
    signals[SIGNAL_EXITED] =
        g_signal_new ("exited",
                      G_TYPE_FROM_CLASS (klass),
                      G_SIGNAL_RUN_LAST,
                      0,
                      NULL, NULL,
                      NULL,
                      G_TYPE_NONE, 0);

    /**
     * GrlScene::paused:
     * @self: The #GrlScene
     *
     * Emitted when the scene is paused.
     */
    signals[SIGNAL_PAUSED] =
        g_signal_new ("paused",
                      G_TYPE_FROM_CLASS (klass),
                      G_SIGNAL_RUN_LAST,
                      0,
                      NULL, NULL,
                      NULL,
                      G_TYPE_NONE, 0);

    /**
     * GrlScene::resumed:
     * @self: The #GrlScene
     *
     * Emitted when the scene is resumed.
     */
    signals[SIGNAL_RESUMED] =
        g_signal_new ("resumed",
                      G_TYPE_FROM_CLASS (klass),
                      G_SIGNAL_RUN_LAST,
                      0,
                      NULL, NULL,
                      NULL,
                      G_TYPE_NONE, 0);

    /**
     * GrlScene::entity-added:
     * @self: The #GrlScene
     * @entity: The entity that was added
     *
     * Emitted when an entity is added to the scene.
     */
    signals[SIGNAL_ENTITY_ADDED] =
        g_signal_new ("entity-added",
                      G_TYPE_FROM_CLASS (klass),
                      G_SIGNAL_RUN_LAST,
                      0,
                      NULL, NULL,
                      NULL,
                      G_TYPE_NONE, 1,
                      GRL_TYPE_ENTITY);

    /**
     * GrlScene::entity-removed:
     * @self: The #GrlScene
     * @entity: The entity that was removed
     *
     * Emitted when an entity is removed from the scene.
     */
    signals[SIGNAL_ENTITY_REMOVED] =
        g_signal_new ("entity-removed",
                      G_TYPE_FROM_CLASS (klass),
                      G_SIGNAL_RUN_LAST,
                      0,
                      NULL, NULL,
                      NULL,
                      G_TYPE_NONE, 1,
                      GRL_TYPE_ENTITY);
}

static void
grl_scene_init (GrlScene *self)
{
    GrlScenePrivate *priv = grl_scene_get_instance_private (self);

    priv->name = NULL;
    priv->entities = NULL;
    priv->active = FALSE;
    priv->paused = FALSE;
    priv->visible = TRUE;
    priv->update_when_paused = FALSE;
    priv->draw_when_paused = TRUE;
    priv->entities_dirty = FALSE;
}

/*
 * Public API - Constructors
 */

/**
 * grl_scene_new:
 *
 * Creates a new empty scene.
 *
 * Returns: (transfer full): A new #GrlScene
 */
GrlScene *
grl_scene_new (void)
{
    return g_object_new (GRL_TYPE_SCENE, NULL);
}

/**
 * grl_scene_new_with_name:
 * @name: The scene name
 *
 * Creates a new scene with the given name.
 *
 * Returns: (transfer full): A new #GrlScene
 */
GrlScene *
grl_scene_new_with_name (const gchar *name)
{
    return g_object_new (GRL_TYPE_SCENE,
                         "name", name,
                         NULL);
}

/*
 * Public API - Properties
 */

/**
 * grl_scene_get_name:
 * @self: A #GrlScene
 *
 * Gets the scene's name.
 *
 * Returns: (transfer none) (nullable): The scene name
 */
const gchar *
grl_scene_get_name (GrlScene *self)
{
    GrlScenePrivate *priv;

    g_return_val_if_fail (GRL_IS_SCENE (self), NULL);

    priv = grl_scene_get_instance_private (self);

    return priv->name;
}

/**
 * grl_scene_set_name:
 * @self: A #GrlScene
 * @name: (nullable): The scene name
 *
 * Sets the scene's name.
 */
void
grl_scene_set_name (GrlScene    *self,
                    const gchar *name)
{
    GrlScenePrivate *priv;

    g_return_if_fail (GRL_IS_SCENE (self));

    priv = grl_scene_get_instance_private (self);

    if (g_strcmp0 (priv->name, name) == 0)
        return;

    g_free (priv->name);
    priv->name = g_strdup (name);
    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_NAME]);
}

/**
 * grl_scene_get_active:
 * @self: A #GrlScene
 *
 * Gets whether the scene is active.
 *
 * Returns: %TRUE if active
 */
gboolean
grl_scene_get_active (GrlScene *self)
{
    GrlScenePrivate *priv;

    g_return_val_if_fail (GRL_IS_SCENE (self), FALSE);

    priv = grl_scene_get_instance_private (self);

    return priv->active;
}

/**
 * grl_scene_get_paused:
 * @self: A #GrlScene
 *
 * Gets whether the scene is paused.
 *
 * Returns: %TRUE if paused
 */
gboolean
grl_scene_get_paused (GrlScene *self)
{
    GrlScenePrivate *priv;

    g_return_val_if_fail (GRL_IS_SCENE (self), FALSE);

    priv = grl_scene_get_instance_private (self);

    return priv->paused;
}

/**
 * grl_scene_get_visible:
 * @self: A #GrlScene
 *
 * Gets whether the scene is visible.
 *
 * Returns: %TRUE if visible
 */
gboolean
grl_scene_get_visible (GrlScene *self)
{
    GrlScenePrivate *priv;

    g_return_val_if_fail (GRL_IS_SCENE (self), TRUE);

    priv = grl_scene_get_instance_private (self);

    return priv->visible;
}

/**
 * grl_scene_set_visible:
 * @self: A #GrlScene
 * @visible: Whether the scene should be visible
 *
 * Sets whether the scene should be drawn.
 */
void
grl_scene_set_visible (GrlScene *self,
                       gboolean  visible)
{
    GrlScenePrivate *priv;

    g_return_if_fail (GRL_IS_SCENE (self));

    priv = grl_scene_get_instance_private (self);

    if (priv->visible == visible)
        return;

    priv->visible = visible;
    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_VISIBLE]);
}

/**
 * grl_scene_get_update_when_paused:
 * @self: A #GrlScene
 *
 * Gets whether the scene updates when paused.
 *
 * Returns: %TRUE if updates when paused
 */
gboolean
grl_scene_get_update_when_paused (GrlScene *self)
{
    GrlScenePrivate *priv;

    g_return_val_if_fail (GRL_IS_SCENE (self), FALSE);

    priv = grl_scene_get_instance_private (self);

    return priv->update_when_paused;
}

/**
 * grl_scene_set_update_when_paused:
 * @self: A #GrlScene
 * @update: Whether to update when paused
 *
 * Sets whether the scene should update when paused.
 */
void
grl_scene_set_update_when_paused (GrlScene *self,
                                  gboolean  update)
{
    GrlScenePrivate *priv;

    g_return_if_fail (GRL_IS_SCENE (self));

    priv = grl_scene_get_instance_private (self);

    priv->update_when_paused = update;
}

/**
 * grl_scene_get_draw_when_paused:
 * @self: A #GrlScene
 *
 * Gets whether the scene draws when paused.
 *
 * Returns: %TRUE if draws when paused
 */
gboolean
grl_scene_get_draw_when_paused (GrlScene *self)
{
    GrlScenePrivate *priv;

    g_return_val_if_fail (GRL_IS_SCENE (self), TRUE);

    priv = grl_scene_get_instance_private (self);

    return priv->draw_when_paused;
}

/**
 * grl_scene_set_draw_when_paused:
 * @self: A #GrlScene
 * @draw: Whether to draw when paused
 *
 * Sets whether the scene should draw when paused.
 */
void
grl_scene_set_draw_when_paused (GrlScene *self,
                                gboolean  draw)
{
    GrlScenePrivate *priv;

    g_return_if_fail (GRL_IS_SCENE (self));

    priv = grl_scene_get_instance_private (self);

    priv->draw_when_paused = draw;
}

/*
 * Public API - Entity Management
 */

/**
 * grl_scene_add_entity:
 * @self: A #GrlScene
 * @entity: (transfer none): The entity to add
 *
 * Adds an entity to the scene. The scene takes a reference
 * to the entity.
 */
void
grl_scene_add_entity (GrlScene  *self,
                      GrlEntity *entity)
{
    GrlScenePrivate *priv;

    g_return_if_fail (GRL_IS_SCENE (self));
    g_return_if_fail (GRL_IS_ENTITY (entity));

    priv = grl_scene_get_instance_private (self);

    /* Don't add duplicates */
    if (g_list_find (priv->entities, entity) != NULL)
        return;

    priv->entities = g_list_append (priv->entities, g_object_ref (entity));
    priv->entities_dirty = TRUE;

    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_ENTITY_COUNT]);
    g_signal_emit (self, signals[SIGNAL_ENTITY_ADDED], 0, entity);
}

/**
 * grl_scene_remove_entity:
 * @self: A #GrlScene
 * @entity: The entity to remove
 *
 * Removes an entity from the scene. The reference held by
 * the scene is released.
 */
void
grl_scene_remove_entity (GrlScene  *self,
                         GrlEntity *entity)
{
    GrlScenePrivate *priv;
    GList *link;

    g_return_if_fail (GRL_IS_SCENE (self));
    g_return_if_fail (GRL_IS_ENTITY (entity));

    priv = grl_scene_get_instance_private (self);

    link = g_list_find (priv->entities, entity);
    if (link == NULL)
        return;

    priv->entities = g_list_delete_link (priv->entities, link);

    g_signal_emit (self, signals[SIGNAL_ENTITY_REMOVED], 0, entity);
    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_ENTITY_COUNT]);

    g_object_unref (entity);
}

/**
 * grl_scene_clear_entities:
 * @self: A #GrlScene
 *
 * Removes all entities from the scene.
 */
void
grl_scene_clear_entities (GrlScene *self)
{
    GrlScenePrivate *priv;

    g_return_if_fail (GRL_IS_SCENE (self));

    priv = grl_scene_get_instance_private (self);

    g_list_free_full (priv->entities, g_object_unref);
    priv->entities = NULL;
    priv->entities_dirty = FALSE;

    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_ENTITY_COUNT]);
}

/**
 * grl_scene_get_entities:
 * @self: A #GrlScene
 *
 * Gets the list of entities in the scene.
 *
 * Returns: (transfer none) (element-type GrlEntity): The entity list
 */
GList *
grl_scene_get_entities (GrlScene *self)
{
    GrlScenePrivate *priv;

    g_return_val_if_fail (GRL_IS_SCENE (self), NULL);

    priv = grl_scene_get_instance_private (self);

    return priv->entities;
}

/**
 * grl_scene_get_entity_count:
 * @self: A #GrlScene
 *
 * Gets the number of entities in the scene.
 *
 * Returns: The entity count
 */
guint
grl_scene_get_entity_count (GrlScene *self)
{
    GrlScenePrivate *priv;

    g_return_val_if_fail (GRL_IS_SCENE (self), 0);

    priv = grl_scene_get_instance_private (self);

    return g_list_length (priv->entities);
}

/**
 * grl_scene_find_entity_by_tag:
 * @self: A #GrlScene
 * @tag: The tag to search for
 *
 * Finds the first entity with the given tag.
 *
 * Returns: (transfer none) (nullable): The entity, or %NULL if not found
 */
GrlEntity *
grl_scene_find_entity_by_tag (GrlScene    *self,
                              const gchar *tag)
{
    GrlScenePrivate *priv;
    GList *l;

    g_return_val_if_fail (GRL_IS_SCENE (self), NULL);
    g_return_val_if_fail (tag != NULL, NULL);

    priv = grl_scene_get_instance_private (self);

    for (l = priv->entities; l != NULL; l = l->next)
    {
        GrlEntity *entity = GRL_ENTITY (l->data);
        const gchar *entity_tag = grl_entity_get_tag (entity);

        if (g_strcmp0 (entity_tag, tag) == 0)
            return entity;
    }

    return NULL;
}

/**
 * grl_scene_find_entities_by_tag:
 * @self: A #GrlScene
 * @tag: The tag to search for
 *
 * Finds all entities with the given tag.
 *
 * Returns: (transfer container) (element-type GrlEntity): A list of entities
 */
GList *
grl_scene_find_entities_by_tag (GrlScene    *self,
                                const gchar *tag)
{
    GrlScenePrivate *priv;
    GList *result = NULL;
    GList *l;

    g_return_val_if_fail (GRL_IS_SCENE (self), NULL);
    g_return_val_if_fail (tag != NULL, NULL);

    priv = grl_scene_get_instance_private (self);

    for (l = priv->entities; l != NULL; l = l->next)
    {
        GrlEntity *entity = GRL_ENTITY (l->data);
        const gchar *entity_tag = grl_entity_get_tag (entity);

        if (g_strcmp0 (entity_tag, tag) == 0)
            result = g_list_prepend (result, entity);
    }

    return g_list_reverse (result);
}

/**
 * grl_scene_foreach_entity:
 * @self: A #GrlScene
 * @func: (scope call): The callback function
 * @user_data: (closure func): User data for @func
 *
 * Iterates over all entities in the scene, calling @func for each one.
 * Iteration stops if @func returns %FALSE.
 */
void
grl_scene_foreach_entity (GrlScene             *self,
                          GrlSceneForeachFunc   func,
                          gpointer              user_data)
{
    GrlScenePrivate *priv;
    GList *l;

    g_return_if_fail (GRL_IS_SCENE (self));
    g_return_if_fail (func != NULL);

    priv = grl_scene_get_instance_private (self);

    for (l = priv->entities; l != NULL; l = l->next)
    {
        if (!func (GRL_ENTITY (l->data), user_data))
            break;
    }
}

/*
 * Public API - Lifecycle Methods
 */

/**
 * grl_scene_enter:
 * @self: A #GrlScene
 *
 * Called when the scene becomes active. This calls the virtual
 * enter() method and emits the #GrlScene::entered signal.
 */
void
grl_scene_enter (GrlScene *self)
{
    g_return_if_fail (GRL_IS_SCENE (self));

    GRL_SCENE_GET_CLASS (self)->enter (self);
    g_signal_emit (self, signals[SIGNAL_ENTERED], 0);
}

/**
 * grl_scene_exit:
 * @self: A #GrlScene
 *
 * Called when the scene becomes inactive. This calls the virtual
 * exit() method and emits the #GrlScene::exited signal.
 */
void
grl_scene_exit (GrlScene *self)
{
    g_return_if_fail (GRL_IS_SCENE (self));

    GRL_SCENE_GET_CLASS (self)->exit (self);
    g_signal_emit (self, signals[SIGNAL_EXITED], 0);
}

/**
 * grl_scene_pause:
 * @self: A #GrlScene
 *
 * Called when another scene is pushed on top. This calls the
 * virtual pause() method and emits the #GrlScene::paused signal.
 */
void
grl_scene_pause (GrlScene *self)
{
    g_return_if_fail (GRL_IS_SCENE (self));

    GRL_SCENE_GET_CLASS (self)->pause (self);
    g_signal_emit (self, signals[SIGNAL_PAUSED], 0);
}

/**
 * grl_scene_resume:
 * @self: A #GrlScene
 *
 * Called when this scene becomes the top scene again. This calls
 * the virtual resume() method and emits the #GrlScene::resumed signal.
 */
void
grl_scene_resume (GrlScene *self)
{
    g_return_if_fail (GRL_IS_SCENE (self));

    GRL_SCENE_GET_CLASS (self)->resume (self);
    g_signal_emit (self, signals[SIGNAL_RESUMED], 0);
}

/**
 * grl_scene_update:
 * @self: A #GrlScene
 * @delta: Time since last frame in seconds
 *
 * Updates the scene and all its entities. Calls the virtual
 * update() method which by default updates all GrlUpdatable entities.
 */
void
grl_scene_update (GrlScene *self,
                  gfloat    delta)
{
    g_return_if_fail (GRL_IS_SCENE (self));

    GRL_SCENE_GET_CLASS (self)->update (self, delta);
}

/**
 * grl_scene_draw:
 * @self: A #GrlScene
 *
 * Draws the scene and all its entities. Calls the virtual
 * draw() method which by default draws all GrlDrawable entities
 * sorted by z-index.
 */
void
grl_scene_draw (GrlScene *self)
{
    g_return_if_fail (GRL_IS_SCENE (self));

    GRL_SCENE_GET_CLASS (self)->draw (self);
}

/*
 * Public API - Utility
 */

/**
 * grl_scene_sort_entities_by_z:
 * @self: A #GrlScene
 *
 * Forces a re-sort of entities by z-index. Normally this happens
 * automatically before drawing, but you can call this manually
 * if you need the sorted order immediately.
 */
void
grl_scene_sort_entities_by_z (GrlScene *self)
{
    GrlScenePrivate *priv;

    g_return_if_fail (GRL_IS_SCENE (self));

    priv = grl_scene_get_instance_private (self);

    priv->entities = g_list_sort (priv->entities, compare_entities_by_z);
    priv->entities_dirty = FALSE;
}
