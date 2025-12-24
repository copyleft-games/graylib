/* grl-scene-manager.h
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Stack-based scene manager for handling scene transitions.
 */

#pragma once

#if !defined(GRAYLIB_INSIDE) && !defined(GRAYLIB_COMPILATION)
#error "Only <graylib.h> can be included directly."
#endif

#include <glib-object.h>
#include "../grl-version.h"
#include "../grl-types.h"
#include "grl-scene.h"

G_BEGIN_DECLS

#define GRL_TYPE_SCENE_MANAGER (grl_scene_manager_get_type ())

GRL_AVAILABLE_IN_ALL
G_DECLARE_DERIVABLE_TYPE (GrlSceneManager, grl_scene_manager, GRL, SCENE_MANAGER, GObject)

/**
 * GrlSceneManagerClass:
 * @parent_class: The parent class
 * @scene_pushed: Virtual method called after a scene is pushed
 * @scene_popped: Virtual method called after a scene is popped
 * @scene_changed: Virtual method called when the active scene changes
 *
 * The class structure for #GrlSceneManager.
 */
struct _GrlSceneManagerClass
{
    GObjectClass parent_class;

    /* Virtual methods */
    void (*scene_pushed)  (GrlSceneManager *self,
                           GrlScene        *scene);
    void (*scene_popped)  (GrlSceneManager *self,
                           GrlScene        *scene);
    void (*scene_changed) (GrlSceneManager *self,
                           GrlScene        *old_scene,
                           GrlScene        *new_scene);

    /*< private >*/
    gpointer _reserved[8];
};

/*
 * Constructors
 */

GRL_AVAILABLE_IN_ALL
GrlSceneManager *   grl_scene_manager_new           (void);

/*
 * Stack Operations
 */

GRL_AVAILABLE_IN_ALL
void                grl_scene_manager_push          (GrlSceneManager *self,
                                                     GrlScene        *scene);

GRL_AVAILABLE_IN_ALL
GrlScene *          grl_scene_manager_pop           (GrlSceneManager *self);

GRL_AVAILABLE_IN_ALL
void                grl_scene_manager_replace       (GrlSceneManager *self,
                                                     GrlScene        *scene);

GRL_AVAILABLE_IN_ALL
void                grl_scene_manager_clear         (GrlSceneManager *self);

GRL_AVAILABLE_IN_ALL
void                grl_scene_manager_pop_to        (GrlSceneManager *self,
                                                     GrlScene        *scene);

GRL_AVAILABLE_IN_ALL
void                grl_scene_manager_pop_to_root   (GrlSceneManager *self);

/*
 * Accessors
 */

GRL_AVAILABLE_IN_ALL
GrlScene *          grl_scene_manager_get_current   (GrlSceneManager *self);

GRL_AVAILABLE_IN_ALL
GrlScene *          grl_scene_manager_peek          (GrlSceneManager *self,
                                                     guint            index);

GRL_AVAILABLE_IN_ALL
guint               grl_scene_manager_get_count     (GrlSceneManager *self);

GRL_AVAILABLE_IN_ALL
gboolean            grl_scene_manager_is_empty      (GrlSceneManager *self);

GRL_AVAILABLE_IN_ALL
gboolean            grl_scene_manager_contains      (GrlSceneManager *self,
                                                     GrlScene        *scene);

GRL_AVAILABLE_IN_ALL
GrlScene *          grl_scene_manager_find_by_name  (GrlSceneManager *self,
                                                     const gchar     *name);

/*
 * Game Loop Integration
 */

GRL_AVAILABLE_IN_ALL
void                grl_scene_manager_update        (GrlSceneManager *self,
                                                     gfloat           delta);

GRL_AVAILABLE_IN_ALL
void                grl_scene_manager_draw          (GrlSceneManager *self);

GRL_AVAILABLE_IN_ALL
void                grl_scene_manager_update_all    (GrlSceneManager *self,
                                                     gfloat           delta);

GRL_AVAILABLE_IN_ALL
void                grl_scene_manager_draw_all      (GrlSceneManager *self);

G_END_DECLS
