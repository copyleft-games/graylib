/* grl-scene.h
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Scene container for entities and game objects.
 */

#pragma once

#if !defined(GRAYLIB_INSIDE) && !defined(GRAYLIB_COMPILATION)
#error "Only <graylib.h> can be included directly."
#endif

#include <glib-object.h>
#include "../grl-version.h"
#include "../grl-types.h"

G_BEGIN_DECLS

#define GRL_TYPE_SCENE (grl_scene_get_type ())

GRL_AVAILABLE_IN_ALL
G_DECLARE_DERIVABLE_TYPE (GrlScene, grl_scene, GRL, SCENE, GObject)

/**
 * GrlSceneClass:
 * @parent_class: The parent class
 * @enter: Virtual method called when scene becomes active
 * @exit: Virtual method called when scene becomes inactive
 * @pause: Virtual method called when scene is paused (pushed under)
 * @resume: Virtual method called when scene is resumed (top again)
 * @update: Virtual method for scene update logic
 * @draw: Virtual method for scene drawing
 *
 * The class structure for #GrlScene.
 */
struct _GrlSceneClass
{
    GObjectClass parent_class;

    /* Virtual methods */
    void (*enter)   (GrlScene *self);
    void (*exit)    (GrlScene *self);
    void (*pause)   (GrlScene *self);
    void (*resume)  (GrlScene *self);
    void (*update)  (GrlScene *self,
                     gfloat    delta);
    void (*draw)    (GrlScene *self);

    /*< private >*/
    gpointer _reserved[8];
};

/*
 * Constructors
 */

GRL_AVAILABLE_IN_ALL
GrlScene *      grl_scene_new               (void);

GRL_AVAILABLE_IN_ALL
GrlScene *      grl_scene_new_with_name     (const gchar    *name);

/*
 * Properties
 */

GRL_AVAILABLE_IN_ALL
const gchar *   grl_scene_get_name          (GrlScene       *self);

GRL_AVAILABLE_IN_ALL
void            grl_scene_set_name          (GrlScene       *self,
                                             const gchar    *name);

GRL_AVAILABLE_IN_ALL
gboolean        grl_scene_get_active        (GrlScene       *self);

GRL_AVAILABLE_IN_ALL
gboolean        grl_scene_get_paused        (GrlScene       *self);

GRL_AVAILABLE_IN_ALL
gboolean        grl_scene_get_visible       (GrlScene       *self);

GRL_AVAILABLE_IN_ALL
void            grl_scene_set_visible       (GrlScene       *self,
                                             gboolean        visible);

GRL_AVAILABLE_IN_ALL
gboolean        grl_scene_get_update_when_paused (GrlScene  *self);

GRL_AVAILABLE_IN_ALL
void            grl_scene_set_update_when_paused (GrlScene  *self,
                                                  gboolean   update);

GRL_AVAILABLE_IN_ALL
gboolean        grl_scene_get_draw_when_paused   (GrlScene  *self);

GRL_AVAILABLE_IN_ALL
void            grl_scene_set_draw_when_paused   (GrlScene  *self,
                                                  gboolean   draw);

/*
 * Entity Management
 */

GRL_AVAILABLE_IN_ALL
void            grl_scene_add_entity        (GrlScene       *self,
                                             GrlEntity      *entity);

GRL_AVAILABLE_IN_ALL
void            grl_scene_remove_entity     (GrlScene       *self,
                                             GrlEntity      *entity);

GRL_AVAILABLE_IN_ALL
void            grl_scene_clear_entities    (GrlScene       *self);

GRL_AVAILABLE_IN_ALL
GList *         grl_scene_get_entities      (GrlScene       *self);

GRL_AVAILABLE_IN_ALL
guint           grl_scene_get_entity_count  (GrlScene       *self);

GRL_AVAILABLE_IN_ALL
GrlEntity *     grl_scene_find_entity_by_tag (GrlScene      *self,
                                              const gchar   *tag);

GRL_AVAILABLE_IN_ALL
GList *         grl_scene_find_entities_by_tag (GrlScene    *self,
                                                const gchar *tag);

/**
 * GrlSceneForeachFunc:
 * @entity: The current entity
 * @user_data: (closure): User data
 *
 * Callback function for iterating entities in a scene.
 *
 * Returns: %TRUE to continue iteration, %FALSE to stop
 */
typedef gboolean (*GrlSceneForeachFunc) (GrlEntity *entity,
                                         gpointer   user_data);

GRL_AVAILABLE_IN_ALL
void            grl_scene_foreach_entity    (GrlScene              *self,
                                             GrlSceneForeachFunc    func,
                                             gpointer               user_data);

/*
 * Lifecycle Methods (call virtual implementations)
 */

GRL_AVAILABLE_IN_ALL
void            grl_scene_enter             (GrlScene       *self);

GRL_AVAILABLE_IN_ALL
void            grl_scene_exit              (GrlScene       *self);

GRL_AVAILABLE_IN_ALL
void            grl_scene_pause             (GrlScene       *self);

GRL_AVAILABLE_IN_ALL
void            grl_scene_resume            (GrlScene       *self);

GRL_AVAILABLE_IN_ALL
void            grl_scene_update            (GrlScene       *self,
                                             gfloat          delta);

GRL_AVAILABLE_IN_ALL
void            grl_scene_draw              (GrlScene       *self);

/*
 * Utility
 */

GRL_AVAILABLE_IN_ALL
void            grl_scene_sort_entities_by_z (GrlScene      *self);

G_END_DECLS
