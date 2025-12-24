/* grl-scene-manager.c
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Implementation of stack-based scene manager.
 */

#include "config.h"
#include "grl-scene-manager.h"

/**
 * SECTION:grl-scene-manager
 * @Title: GrlSceneManager
 * @Short_description: Stack-based scene management
 *
 * #GrlSceneManager provides a stack-based approach to managing game scenes.
 * Scenes can be pushed onto the stack (making them active), popped off
 * (returning to the previous scene), or replaced entirely.
 *
 * The scene manager handles calling the appropriate lifecycle methods
 * (enter, exit, pause, resume) on scenes as they transition.
 *
 * # Scene Stack Behavior
 *
 * - **Push**: The current scene is paused, the new scene enters and becomes active.
 * - **Pop**: The current scene exits, the previous scene resumes and becomes active.
 * - **Replace**: The current scene exits, the new scene enters (no pausing involved).
 * - **Clear**: All scenes exit, the stack becomes empty.
 *
 * # Example Usage
 *
 * |[<!-- language="C" -->
 * g_autoptr(GrlSceneManager) manager = grl_scene_manager_new ();
 * g_autoptr(GrlScene) menu = grl_scene_new_with_name ("menu");
 * g_autoptr(GrlScene) game = grl_scene_new_with_name ("game");
 *
 * // Start with menu scene
 * grl_scene_manager_push (manager, menu);
 *
 * // Later, push game scene (menu is paused)
 * grl_scene_manager_push (manager, game);
 *
 * // In game loop
 * grl_scene_manager_update (manager, delta);
 * grl_scene_manager_draw (manager);
 *
 * // Return to menu
 * grl_scene_manager_pop (manager);
 * ]|
 */

typedef struct
{
    GQueue *scene_stack;
} GrlSceneManagerPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (GrlSceneManager, grl_scene_manager, G_TYPE_OBJECT)

enum
{
    PROP_0,
    PROP_CURRENT_SCENE,
    PROP_SCENE_COUNT,
    PROP_IS_EMPTY,
    N_PROPS
};

static GParamSpec *properties[N_PROPS];

enum
{
    SIGNAL_SCENE_PUSHED,
    SIGNAL_SCENE_POPPED,
    SIGNAL_SCENE_CHANGED,
    N_SIGNALS
};

static guint signals[N_SIGNALS];

/*
 * Default virtual method implementations
 */

static void
grl_scene_manager_real_scene_pushed (GrlSceneManager *self,
                                     GrlScene        *scene)
{
    /* Default implementation does nothing */
}

static void
grl_scene_manager_real_scene_popped (GrlSceneManager *self,
                                     GrlScene        *scene)
{
    /* Default implementation does nothing */
}

static void
grl_scene_manager_real_scene_changed (GrlSceneManager *self,
                                      GrlScene        *old_scene,
                                      GrlScene        *new_scene)
{
    /* Default implementation does nothing */
}

/*
 * GObject implementation
 */

static void
grl_scene_manager_finalize (GObject *object)
{
    GrlSceneManager *self = GRL_SCENE_MANAGER (object);
    GrlSceneManagerPrivate *priv = grl_scene_manager_get_instance_private (self);

    /* Exit and unref all scenes */
    while (!g_queue_is_empty (priv->scene_stack))
    {
        GrlScene *scene = g_queue_pop_head (priv->scene_stack);
        if (grl_scene_get_active (scene))
            grl_scene_exit (scene);
        g_object_unref (scene);
    }

    g_queue_free (priv->scene_stack);

    G_OBJECT_CLASS (grl_scene_manager_parent_class)->finalize (object);
}

static void
grl_scene_manager_get_property (GObject    *object,
                                guint       prop_id,
                                GValue     *value,
                                GParamSpec *pspec)
{
    GrlSceneManager *self = GRL_SCENE_MANAGER (object);
    GrlSceneManagerPrivate *priv = grl_scene_manager_get_instance_private (self);

    switch (prop_id)
    {
    case PROP_CURRENT_SCENE:
        g_value_set_object (value, g_queue_peek_head (priv->scene_stack));
        break;

    case PROP_SCENE_COUNT:
        g_value_set_uint (value, g_queue_get_length (priv->scene_stack));
        break;

    case PROP_IS_EMPTY:
        g_value_set_boolean (value, g_queue_is_empty (priv->scene_stack));
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
grl_scene_manager_class_init (GrlSceneManagerClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->finalize = grl_scene_manager_finalize;
    object_class->get_property = grl_scene_manager_get_property;

    /* Default virtual implementations */
    klass->scene_pushed = grl_scene_manager_real_scene_pushed;
    klass->scene_popped = grl_scene_manager_real_scene_popped;
    klass->scene_changed = grl_scene_manager_real_scene_changed;

    /**
     * GrlSceneManager:current-scene:
     *
     * The currently active scene (top of the stack).
     */
    properties[PROP_CURRENT_SCENE] =
        g_param_spec_object ("current-scene",
                             "Current Scene",
                             "The current active scene",
                             GRL_TYPE_SCENE,
                             G_PARAM_READABLE |
                             G_PARAM_STATIC_STRINGS);

    /**
     * GrlSceneManager:scene-count:
     *
     * The number of scenes on the stack.
     */
    properties[PROP_SCENE_COUNT] =
        g_param_spec_uint ("scene-count",
                           "Scene Count",
                           "Number of scenes on the stack",
                           0, G_MAXUINT, 0,
                           G_PARAM_READABLE |
                           G_PARAM_STATIC_STRINGS);

    /**
     * GrlSceneManager:is-empty:
     *
     * Whether the scene stack is empty.
     */
    properties[PROP_IS_EMPTY] =
        g_param_spec_boolean ("is-empty",
                              "Is Empty",
                              "Whether the stack is empty",
                              TRUE,
                              G_PARAM_READABLE |
                              G_PARAM_STATIC_STRINGS);

    g_object_class_install_properties (object_class, N_PROPS, properties);

    /**
     * GrlSceneManager::scene-pushed:
     * @self: The #GrlSceneManager
     * @scene: The scene that was pushed
     *
     * Emitted after a scene is pushed onto the stack.
     */
    signals[SIGNAL_SCENE_PUSHED] =
        g_signal_new ("scene-pushed",
                      G_TYPE_FROM_CLASS (klass),
                      G_SIGNAL_RUN_LAST,
                      G_STRUCT_OFFSET (GrlSceneManagerClass, scene_pushed),
                      NULL, NULL,
                      NULL,
                      G_TYPE_NONE, 1,
                      GRL_TYPE_SCENE);

    /**
     * GrlSceneManager::scene-popped:
     * @self: The #GrlSceneManager
     * @scene: The scene that was popped
     *
     * Emitted after a scene is popped from the stack.
     */
    signals[SIGNAL_SCENE_POPPED] =
        g_signal_new ("scene-popped",
                      G_TYPE_FROM_CLASS (klass),
                      G_SIGNAL_RUN_LAST,
                      G_STRUCT_OFFSET (GrlSceneManagerClass, scene_popped),
                      NULL, NULL,
                      NULL,
                      G_TYPE_NONE, 1,
                      GRL_TYPE_SCENE);

    /**
     * GrlSceneManager::scene-changed:
     * @self: The #GrlSceneManager
     * @old_scene: (nullable): The previous active scene
     * @new_scene: (nullable): The new active scene
     *
     * Emitted when the active scene changes.
     */
    signals[SIGNAL_SCENE_CHANGED] =
        g_signal_new ("scene-changed",
                      G_TYPE_FROM_CLASS (klass),
                      G_SIGNAL_RUN_LAST,
                      G_STRUCT_OFFSET (GrlSceneManagerClass, scene_changed),
                      NULL, NULL,
                      NULL,
                      G_TYPE_NONE, 2,
                      GRL_TYPE_SCENE,
                      GRL_TYPE_SCENE);
}

static void
grl_scene_manager_init (GrlSceneManager *self)
{
    GrlSceneManagerPrivate *priv = grl_scene_manager_get_instance_private (self);

    priv->scene_stack = g_queue_new ();
}

/*
 * Public API - Constructor
 */

/**
 * grl_scene_manager_new:
 *
 * Creates a new scene manager.
 *
 * Returns: (transfer full): A new #GrlSceneManager
 */
GrlSceneManager *
grl_scene_manager_new (void)
{
    return g_object_new (GRL_TYPE_SCENE_MANAGER, NULL);
}

/*
 * Public API - Stack Operations
 */

/**
 * grl_scene_manager_push:
 * @self: A #GrlSceneManager
 * @scene: (transfer none): The scene to push
 *
 * Pushes a scene onto the stack. The current scene (if any) is paused,
 * and the new scene is entered and becomes active.
 *
 * The scene manager takes a reference to the scene.
 */
void
grl_scene_manager_push (GrlSceneManager *self,
                        GrlScene        *scene)
{
    GrlSceneManagerPrivate *priv;
    GrlScene *old_scene;

    g_return_if_fail (GRL_IS_SCENE_MANAGER (self));
    g_return_if_fail (GRL_IS_SCENE (scene));

    priv = grl_scene_manager_get_instance_private (self);

    /* Pause current scene if exists */
    old_scene = g_queue_peek_head (priv->scene_stack);
    if (old_scene != NULL)
    {
        grl_scene_pause (old_scene);
    }

    /* Push and enter new scene */
    g_queue_push_head (priv->scene_stack, g_object_ref (scene));
    grl_scene_enter (scene);

    /* Notify */
    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_CURRENT_SCENE]);
    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_SCENE_COUNT]);
    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_IS_EMPTY]);

    g_signal_emit (self, signals[SIGNAL_SCENE_PUSHED], 0, scene);
    g_signal_emit (self, signals[SIGNAL_SCENE_CHANGED], 0, old_scene, scene);
}

/**
 * grl_scene_manager_pop:
 * @self: A #GrlSceneManager
 *
 * Pops the current scene from the stack. The scene exits, and the
 * previous scene (if any) resumes and becomes active.
 *
 * Returns: (transfer full) (nullable): The popped scene, or %NULL if empty
 */
GrlScene *
grl_scene_manager_pop (GrlSceneManager *self)
{
    GrlSceneManagerPrivate *priv;
    GrlScene *popped;
    GrlScene *new_current;

    g_return_val_if_fail (GRL_IS_SCENE_MANAGER (self), NULL);

    priv = grl_scene_manager_get_instance_private (self);

    if (g_queue_is_empty (priv->scene_stack))
        return NULL;

    /* Pop and exit current scene */
    popped = g_queue_pop_head (priv->scene_stack);
    grl_scene_exit (popped);

    /* Resume previous scene if exists */
    new_current = g_queue_peek_head (priv->scene_stack);
    if (new_current != NULL)
    {
        grl_scene_resume (new_current);
    }

    /* Notify */
    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_CURRENT_SCENE]);
    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_SCENE_COUNT]);
    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_IS_EMPTY]);

    g_signal_emit (self, signals[SIGNAL_SCENE_POPPED], 0, popped);
    g_signal_emit (self, signals[SIGNAL_SCENE_CHANGED], 0, popped, new_current);

    return popped;
}

/**
 * grl_scene_manager_replace:
 * @self: A #GrlSceneManager
 * @scene: (transfer none): The scene to replace with
 *
 * Replaces the current scene with a new one. The current scene exits,
 * and the new scene enters. No pausing is involved.
 *
 * If the stack is empty, this is equivalent to push().
 */
void
grl_scene_manager_replace (GrlSceneManager *self,
                           GrlScene        *scene)
{
    GrlSceneManagerPrivate *priv;
    GrlScene *old_scene = NULL;

    g_return_if_fail (GRL_IS_SCENE_MANAGER (self));
    g_return_if_fail (GRL_IS_SCENE (scene));

    priv = grl_scene_manager_get_instance_private (self);

    /* Exit and remove current scene if exists */
    if (!g_queue_is_empty (priv->scene_stack))
    {
        old_scene = g_queue_pop_head (priv->scene_stack);
        grl_scene_exit (old_scene);
    }

    /* Push and enter new scene */
    g_queue_push_head (priv->scene_stack, g_object_ref (scene));
    grl_scene_enter (scene);

    /* Notify */
    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_CURRENT_SCENE]);

    g_signal_emit (self, signals[SIGNAL_SCENE_CHANGED], 0, old_scene, scene);

    /* Unref old scene after signals */
    if (old_scene != NULL)
    {
        g_signal_emit (self, signals[SIGNAL_SCENE_POPPED], 0, old_scene);
        g_object_unref (old_scene);
    }

    g_signal_emit (self, signals[SIGNAL_SCENE_PUSHED], 0, scene);
}

/**
 * grl_scene_manager_clear:
 * @self: A #GrlSceneManager
 *
 * Clears all scenes from the stack. Each scene's exit() method is called.
 */
void
grl_scene_manager_clear (GrlSceneManager *self)
{
    GrlSceneManagerPrivate *priv;
    GrlScene *old_current;

    g_return_if_fail (GRL_IS_SCENE_MANAGER (self));

    priv = grl_scene_manager_get_instance_private (self);

    if (g_queue_is_empty (priv->scene_stack))
        return;

    old_current = g_queue_peek_head (priv->scene_stack);

    /* Exit and unref all scenes */
    while (!g_queue_is_empty (priv->scene_stack))
    {
        GrlScene *scene = g_queue_pop_head (priv->scene_stack);
        if (grl_scene_get_active (scene))
            grl_scene_exit (scene);
        g_signal_emit (self, signals[SIGNAL_SCENE_POPPED], 0, scene);
        g_object_unref (scene);
    }

    /* Notify */
    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_CURRENT_SCENE]);
    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_SCENE_COUNT]);
    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_IS_EMPTY]);

    g_signal_emit (self, signals[SIGNAL_SCENE_CHANGED], 0, old_current, NULL);
}

/**
 * grl_scene_manager_pop_to:
 * @self: A #GrlSceneManager
 * @scene: The scene to pop to
 *
 * Pops scenes until the specified scene is on top. If the scene is not
 * in the stack, no action is taken.
 */
void
grl_scene_manager_pop_to (GrlSceneManager *self,
                          GrlScene        *scene)
{
    GrlSceneManagerPrivate *priv;
    GrlScene *current;

    g_return_if_fail (GRL_IS_SCENE_MANAGER (self));
    g_return_if_fail (GRL_IS_SCENE (scene));

    priv = grl_scene_manager_get_instance_private (self);

    /* Check if scene is in stack */
    if (g_queue_find (priv->scene_stack, scene) == NULL)
        return;

    /* Pop until we reach the target scene */
    current = g_queue_peek_head (priv->scene_stack);
    while (current != scene && !g_queue_is_empty (priv->scene_stack))
    {
        GrlScene *popped = grl_scene_manager_pop (self);
        g_object_unref (popped);
        current = g_queue_peek_head (priv->scene_stack);
    }
}

/**
 * grl_scene_manager_pop_to_root:
 * @self: A #GrlSceneManager
 *
 * Pops all scenes except the bottom (root) scene.
 */
void
grl_scene_manager_pop_to_root (GrlSceneManager *self)
{
    GrlSceneManagerPrivate *priv;

    g_return_if_fail (GRL_IS_SCENE_MANAGER (self));

    priv = grl_scene_manager_get_instance_private (self);

    while (g_queue_get_length (priv->scene_stack) > 1)
    {
        GrlScene *popped = grl_scene_manager_pop (self);
        g_object_unref (popped);
    }
}

/*
 * Public API - Accessors
 */

/**
 * grl_scene_manager_get_current:
 * @self: A #GrlSceneManager
 *
 * Gets the current (top) scene.
 *
 * Returns: (transfer none) (nullable): The current scene, or %NULL if empty
 */
GrlScene *
grl_scene_manager_get_current (GrlSceneManager *self)
{
    GrlSceneManagerPrivate *priv;

    g_return_val_if_fail (GRL_IS_SCENE_MANAGER (self), NULL);

    priv = grl_scene_manager_get_instance_private (self);

    return g_queue_peek_head (priv->scene_stack);
}

/**
 * grl_scene_manager_peek:
 * @self: A #GrlSceneManager
 * @index: The index (0 = top/current)
 *
 * Gets a scene at a specific index in the stack.
 *
 * Returns: (transfer none) (nullable): The scene at @index, or %NULL
 */
GrlScene *
grl_scene_manager_peek (GrlSceneManager *self,
                        guint            index)
{
    GrlSceneManagerPrivate *priv;

    g_return_val_if_fail (GRL_IS_SCENE_MANAGER (self), NULL);

    priv = grl_scene_manager_get_instance_private (self);

    return g_queue_peek_nth (priv->scene_stack, index);
}

/**
 * grl_scene_manager_get_count:
 * @self: A #GrlSceneManager
 *
 * Gets the number of scenes on the stack.
 *
 * Returns: The scene count
 */
guint
grl_scene_manager_get_count (GrlSceneManager *self)
{
    GrlSceneManagerPrivate *priv;

    g_return_val_if_fail (GRL_IS_SCENE_MANAGER (self), 0);

    priv = grl_scene_manager_get_instance_private (self);

    return g_queue_get_length (priv->scene_stack);
}

/**
 * grl_scene_manager_is_empty:
 * @self: A #GrlSceneManager
 *
 * Checks if the scene stack is empty.
 *
 * Returns: %TRUE if empty
 */
gboolean
grl_scene_manager_is_empty (GrlSceneManager *self)
{
    GrlSceneManagerPrivate *priv;

    g_return_val_if_fail (GRL_IS_SCENE_MANAGER (self), TRUE);

    priv = grl_scene_manager_get_instance_private (self);

    return g_queue_is_empty (priv->scene_stack);
}

/**
 * grl_scene_manager_contains:
 * @self: A #GrlSceneManager
 * @scene: The scene to check for
 *
 * Checks if a scene is in the stack.
 *
 * Returns: %TRUE if the scene is in the stack
 */
gboolean
grl_scene_manager_contains (GrlSceneManager *self,
                            GrlScene        *scene)
{
    GrlSceneManagerPrivate *priv;

    g_return_val_if_fail (GRL_IS_SCENE_MANAGER (self), FALSE);
    g_return_val_if_fail (GRL_IS_SCENE (scene), FALSE);

    priv = grl_scene_manager_get_instance_private (self);

    return g_queue_find (priv->scene_stack, scene) != NULL;
}

/**
 * grl_scene_manager_find_by_name:
 * @self: A #GrlSceneManager
 * @name: The scene name to find
 *
 * Finds a scene by name in the stack.
 *
 * Returns: (transfer none) (nullable): The scene, or %NULL if not found
 */
GrlScene *
grl_scene_manager_find_by_name (GrlSceneManager *self,
                                const gchar     *name)
{
    GrlSceneManagerPrivate *priv;
    GList *l;

    g_return_val_if_fail (GRL_IS_SCENE_MANAGER (self), NULL);
    g_return_val_if_fail (name != NULL, NULL);

    priv = grl_scene_manager_get_instance_private (self);

    for (l = priv->scene_stack->head; l != NULL; l = l->next)
    {
        GrlScene *scene = GRL_SCENE (l->data);
        const gchar *scene_name = grl_scene_get_name (scene);

        if (g_strcmp0 (scene_name, name) == 0)
            return scene;
    }

    return NULL;
}

/*
 * Public API - Game Loop Integration
 */

/**
 * grl_scene_manager_update:
 * @self: A #GrlSceneManager
 * @delta: Time since last frame in seconds
 *
 * Updates the current scene. Only the top scene is updated.
 */
void
grl_scene_manager_update (GrlSceneManager *self,
                          gfloat           delta)
{
    GrlSceneManagerPrivate *priv;
    GrlScene *current;

    g_return_if_fail (GRL_IS_SCENE_MANAGER (self));

    priv = grl_scene_manager_get_instance_private (self);

    current = g_queue_peek_head (priv->scene_stack);
    if (current != NULL)
    {
        grl_scene_update (current, delta);
    }
}

/**
 * grl_scene_manager_draw:
 * @self: A #GrlSceneManager
 *
 * Draws the current scene. Only the top scene is drawn.
 */
void
grl_scene_manager_draw (GrlSceneManager *self)
{
    GrlSceneManagerPrivate *priv;
    GrlScene *current;

    g_return_if_fail (GRL_IS_SCENE_MANAGER (self));

    priv = grl_scene_manager_get_instance_private (self);

    current = g_queue_peek_head (priv->scene_stack);
    if (current != NULL)
    {
        grl_scene_draw (current);
    }
}

/**
 * grl_scene_manager_update_all:
 * @self: A #GrlSceneManager
 * @delta: Time since last frame in seconds
 *
 * Updates all scenes in the stack (bottom to top). This is useful
 * when you want paused scenes to continue receiving updates
 * (e.g., for animations).
 */
void
grl_scene_manager_update_all (GrlSceneManager *self,
                              gfloat           delta)
{
    GrlSceneManagerPrivate *priv;
    GList *l;

    g_return_if_fail (GRL_IS_SCENE_MANAGER (self));

    priv = grl_scene_manager_get_instance_private (self);

    /* Update from bottom to top */
    for (l = priv->scene_stack->tail; l != NULL; l = l->prev)
    {
        GrlScene *scene = GRL_SCENE (l->data);
        grl_scene_update (scene, delta);
    }
}

/**
 * grl_scene_manager_draw_all:
 * @self: A #GrlSceneManager
 *
 * Draws all visible scenes in the stack (bottom to top). This creates
 * a layered effect where each scene is drawn on top of the previous.
 * This is useful for overlay scenes like pause menus.
 */
void
grl_scene_manager_draw_all (GrlSceneManager *self)
{
    GrlSceneManagerPrivate *priv;
    GList *l;

    g_return_if_fail (GRL_IS_SCENE_MANAGER (self));

    priv = grl_scene_manager_get_instance_private (self);

    /* Draw from bottom to top for proper layering */
    for (l = priv->scene_stack->tail; l != NULL; l = l->prev)
    {
        GrlScene *scene = GRL_SCENE (l->data);
        grl_scene_draw (scene);
    }
}
