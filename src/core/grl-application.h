/* grl-application.h
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Application singleton for managing the Graylib/raylib lifecycle.
 */

#pragma once

#if !defined(GRAYLIB_INSIDE) && !defined(GRAYLIB_COMPILATION)
#error "Only <graylib.h> can be included directly."
#endif

#include <glib-object.h>
#include "grl-version.h"
#include "grl-enums.h"

G_BEGIN_DECLS

#define GRL_TYPE_APPLICATION (grl_application_get_type ())

GRL_AVAILABLE_IN_ALL
G_DECLARE_DERIVABLE_TYPE (GrlApplication, grl_application, GRL, APPLICATION, GObject)

/**
 * GrlApplicationClass:
 * @parent_class: The parent class
 * @startup: Called during application startup, before any windows are created.
 *           Subclasses can override to perform initialization.
 * @shutdown: Called during application shutdown, after all windows are closed.
 *            Subclasses can override to perform cleanup.
 * @activate: Called when the application is activated.
 *            Subclasses can override to create the main window.
 *
 * Class structure for #GrlApplication.
 */
struct _GrlApplicationClass
{
    GObjectClass parent_class;

    /*< public >*/

    /* Virtual methods */
    void (*startup)  (GrlApplication *self);
    void (*shutdown) (GrlApplication *self);
    void (*activate) (GrlApplication *self);

    /*< private >*/
    gpointer _reserved[8];
};

/*
 * Singleton access
 */

GRL_AVAILABLE_IN_ALL
GrlApplication *    grl_application_get_default     (void);

GRL_AVAILABLE_IN_ALL
gboolean            grl_application_is_initialized  (void);

/*
 * Constructors
 */

GRL_AVAILABLE_IN_ALL
GrlApplication *    grl_application_new             (const gchar        *application_id);

/*
 * Configuration (call before run)
 */

GRL_AVAILABLE_IN_ALL
void                grl_application_set_config_flags (GrlApplication     *self,
                                                      GrlConfigFlags      flags);

GRL_AVAILABLE_IN_ALL
GrlConfigFlags      grl_application_get_config_flags (GrlApplication     *self);

GRL_AVAILABLE_IN_ALL
void                grl_application_set_log_level   (GrlApplication     *self,
                                                     GrlLogLevel         level);

GRL_AVAILABLE_IN_ALL
GrlLogLevel         grl_application_get_log_level   (GrlApplication     *self);

GRL_AVAILABLE_IN_ALL
void                grl_application_set_target_fps  (GrlApplication     *self,
                                                     gint                fps);

GRL_AVAILABLE_IN_ALL
gint                grl_application_get_target_fps  (GrlApplication     *self);

/*
 * Event handling
 */

/**
 * grl_application_enable_event_waiting:
 * @self: A #GrlApplication
 *
 * Enables waiting for events on EndDrawing(). When enabled, the application
 * will block until an input event is received, reducing CPU usage for
 * applications that don't need continuous rendering (e.g., GUI tools).
 *
 * Call grl_application_disable_event_waiting() to return to normal polling.
 */
GRL_AVAILABLE_IN_ALL
void                grl_application_enable_event_waiting  (GrlApplication  *self);

/**
 * grl_application_disable_event_waiting:
 * @self: A #GrlApplication
 *
 * Disables waiting for events, returning to automatic event polling.
 * This is the default behavior.
 */
GRL_AVAILABLE_IN_ALL
void                grl_application_disable_event_waiting (GrlApplication  *self);

/*
 * Lifecycle
 */

GRL_AVAILABLE_IN_ALL
gint                grl_application_run             (GrlApplication     *self);

GRL_AVAILABLE_IN_ALL
void                grl_application_quit            (GrlApplication     *self);

GRL_AVAILABLE_IN_ALL
gboolean            grl_application_is_running      (GrlApplication     *self);

/*
 * Properties
 */

GRL_AVAILABLE_IN_ALL
const gchar *       grl_application_get_application_id (GrlApplication  *self);

/*
 * Timing information
 */

GRL_AVAILABLE_IN_ALL
gfloat              grl_application_get_frame_time  (GrlApplication     *self);

GRL_AVAILABLE_IN_ALL
gint                grl_application_get_fps         (GrlApplication     *self);

GRL_AVAILABLE_IN_ALL
gdouble             grl_application_get_time        (GrlApplication     *self);

GRL_AVAILABLE_IN_ALL
guint64             grl_application_get_frame_count (GrlApplication     *self);

G_END_DECLS
