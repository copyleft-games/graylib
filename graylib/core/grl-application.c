/* grl-application.c
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Application singleton implementation.
 */

#include "config.h"
#include "grl-application.h"
#include <raylib.h>

/**
 * SECTION:grl-application
 * @title: GrlApplication
 * @short_description: Application singleton for Graylib
 *
 * #GrlApplication is a singleton that manages the overall lifecycle of a
 * Graylib application. It handles raylib initialization and configuration,
 * and provides a main loop for running the application.
 *
 * Typical usage:
 *
 * |[<!-- language="C" -->
 * int main (int argc, char *argv[])
 * {
 *     g_autoptr(GrlApplication) app = NULL;
 *
 *     app = grl_application_new ("com.example.MyGame");
 *     grl_application_set_config_flags (app, GRL_CONFIG_FLAG_VSYNC_HINT);
 *     grl_application_set_target_fps (app, 60);
 *
 *     return grl_application_run (app);
 * }
 * ]|
 *
 * For more control, subclass #GrlApplication and override the virtual
 * methods #GrlApplicationClass.startup, #GrlApplicationClass.activate,
 * and #GrlApplicationClass.shutdown.
 */

typedef struct _GrlApplicationPrivate
{
    gchar          *application_id;
    GrlConfigFlags  config_flags;
    GrlLogLevel     log_level;
    gint            target_fps;
    gboolean        is_running;
    gboolean        quit_requested;
    guint64         frame_count;

    /* Registered windows - we track them for the main loop */
    GList          *windows;
} GrlApplicationPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (GrlApplication, grl_application, G_TYPE_OBJECT)

enum
{
    PROP_0,
    PROP_APPLICATION_ID,
    PROP_CONFIG_FLAGS,
    PROP_LOG_LEVEL,
    PROP_TARGET_FPS,
    PROP_IS_RUNNING,
    N_PROPS
};

static GParamSpec *properties[N_PROPS];

enum
{
    SIGNAL_STARTUP,
    SIGNAL_ACTIVATE,
    SIGNAL_SHUTDOWN,
    N_SIGNALS
};

static guint signals[N_SIGNALS];

/* Singleton instance */
static GrlApplication *default_application = NULL;

/*
 * Private helper functions
 */

static void
grl_application_real_startup (GrlApplication *self)
{
    GrlApplicationPrivate *priv = grl_application_get_instance_private (self);

    /* Apply configuration before any window is created */
    SetConfigFlags ((unsigned int)priv->config_flags);
    SetTraceLogLevel ((int)priv->log_level);

    if (priv->target_fps > 0)
    {
        SetTargetFPS (priv->target_fps);
    }
}

static void
grl_application_real_shutdown (GrlApplication *self)
{
    /* Default shutdown does nothing special - cleanup is handled in dispose */
}

static void
grl_application_real_activate (GrlApplication *self)
{
    /* Default activate does nothing - subclasses create windows here */
}

/*
 * GObject virtual methods
 */

static void
grl_application_get_property (GObject    *object,
                              guint       prop_id,
                              GValue     *value,
                              GParamSpec *pspec)
{
    GrlApplication *self = GRL_APPLICATION (object);
    GrlApplicationPrivate *priv = grl_application_get_instance_private (self);

    switch (prop_id)
    {
    case PROP_APPLICATION_ID:
        g_value_set_string (value, priv->application_id);
        break;

    case PROP_CONFIG_FLAGS:
        g_value_set_flags (value, priv->config_flags);
        break;

    case PROP_LOG_LEVEL:
        g_value_set_enum (value, priv->log_level);
        break;

    case PROP_TARGET_FPS:
        g_value_set_int (value, priv->target_fps);
        break;

    case PROP_IS_RUNNING:
        g_value_set_boolean (value, priv->is_running);
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static void
grl_application_set_property (GObject      *object,
                              guint         prop_id,
                              const GValue *value,
                              GParamSpec   *pspec)
{
    GrlApplication *self = GRL_APPLICATION (object);
    GrlApplicationPrivate *priv = grl_application_get_instance_private (self);

    switch (prop_id)
    {
    case PROP_APPLICATION_ID:
        g_free (priv->application_id);
        priv->application_id = g_value_dup_string (value);
        break;

    case PROP_CONFIG_FLAGS:
        priv->config_flags = g_value_get_flags (value);
        break;

    case PROP_LOG_LEVEL:
        priv->log_level = g_value_get_enum (value);
        break;

    case PROP_TARGET_FPS:
        priv->target_fps = g_value_get_int (value);
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static void
grl_application_dispose (GObject *object)
{
    GrlApplication *self = GRL_APPLICATION (object);
    GrlApplicationPrivate *priv = grl_application_get_instance_private (self);

    /* Clear the window list */
    g_list_free (priv->windows);
    priv->windows = NULL;

    G_OBJECT_CLASS (grl_application_parent_class)->dispose (object);
}

static void
grl_application_finalize (GObject *object)
{
    GrlApplication *self = GRL_APPLICATION (object);
    GrlApplicationPrivate *priv = grl_application_get_instance_private (self);

    g_free (priv->application_id);
    priv->application_id = NULL;

    /* Clear singleton if this was the default instance */
    if (default_application == self)
    {
        default_application = NULL;
    }

    G_OBJECT_CLASS (grl_application_parent_class)->finalize (object);
}

static void
grl_application_class_init (GrlApplicationClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->get_property = grl_application_get_property;
    object_class->set_property = grl_application_set_property;
    object_class->dispose = grl_application_dispose;
    object_class->finalize = grl_application_finalize;

    /* Virtual method defaults */
    klass->startup = grl_application_real_startup;
    klass->shutdown = grl_application_real_shutdown;
    klass->activate = grl_application_real_activate;

    /**
     * GrlApplication:application-id:
     *
     * The unique identifier for the application (e.g., "com.example.MyGame").
     * This is used for identification purposes and should follow reverse-DNS
     * naming conventions.
     */
    properties[PROP_APPLICATION_ID] =
        g_param_spec_string ("application-id",
                             "Application ID",
                             "The unique identifier for the application",
                             NULL,
                             G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

    /**
     * GrlApplication:config-flags:
     *
     * Configuration flags to set before window creation.
     * See #GrlConfigFlags for available options.
     */
    properties[PROP_CONFIG_FLAGS] =
        g_param_spec_flags ("config-flags",
                            "Config Flags",
                            "Configuration flags for raylib initialization",
                            GRL_TYPE_CONFIG_FLAGS,
                            0,
                            G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

    /**
     * GrlApplication:log-level:
     *
     * The minimum log level for trace output.
     * Messages below this level will be suppressed.
     */
    properties[PROP_LOG_LEVEL] =
        g_param_spec_enum ("log-level",
                           "Log Level",
                           "Minimum log level for trace output",
                           GRL_TYPE_LOG_LEVEL,
                           GRL_LOG_INFO,
                           G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

    /**
     * GrlApplication:target-fps:
     *
     * The target frames per second for the main loop.
     * Set to 0 to disable frame limiting.
     */
    properties[PROP_TARGET_FPS] =
        g_param_spec_int ("target-fps",
                          "Target FPS",
                          "Target frames per second (0 = unlimited)",
                          0,
                          G_MAXINT,
                          60,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

    /**
     * GrlApplication:is-running:
     *
     * Whether the application is currently running its main loop.
     */
    properties[PROP_IS_RUNNING] =
        g_param_spec_boolean ("is-running",
                              "Is Running",
                              "Whether the application is running",
                              FALSE,
                              G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

    g_object_class_install_properties (object_class, N_PROPS, properties);

    /**
     * GrlApplication::startup:
     * @application: The application that received the signal.
     *
     * Emitted when the application is starting up, before any windows
     * are created. This is the place to perform one-time initialization.
     */
    signals[SIGNAL_STARTUP] =
        g_signal_new ("startup",
                      G_TYPE_FROM_CLASS (klass),
                      G_SIGNAL_RUN_FIRST,
                      G_STRUCT_OFFSET (GrlApplicationClass, startup),
                      NULL, NULL,
                      NULL,
                      G_TYPE_NONE, 0);

    /**
     * GrlApplication::activate:
     * @application: The application that received the signal.
     *
     * Emitted when the application should present its main window.
     * This is typically where you create and show your game window.
     */
    signals[SIGNAL_ACTIVATE] =
        g_signal_new ("activate",
                      G_TYPE_FROM_CLASS (klass),
                      G_SIGNAL_RUN_FIRST,
                      G_STRUCT_OFFSET (GrlApplicationClass, activate),
                      NULL, NULL,
                      NULL,
                      G_TYPE_NONE, 0);

    /**
     * GrlApplication::shutdown:
     * @application: The application that received the signal.
     *
     * Emitted when the application is shutting down, after the main
     * loop has exited. This is the place to perform cleanup.
     */
    signals[SIGNAL_SHUTDOWN] =
        g_signal_new ("shutdown",
                      G_TYPE_FROM_CLASS (klass),
                      G_SIGNAL_RUN_FIRST,
                      G_STRUCT_OFFSET (GrlApplicationClass, shutdown),
                      NULL, NULL,
                      NULL,
                      G_TYPE_NONE, 0);
}

static void
grl_application_init (GrlApplication *self)
{
    GrlApplicationPrivate *priv = grl_application_get_instance_private (self);

    priv->application_id = NULL;
    priv->config_flags = 0;
    priv->log_level = GRL_LOG_INFO;
    priv->target_fps = 60;
    priv->is_running = FALSE;
    priv->quit_requested = FALSE;
    priv->frame_count = 0;
    priv->windows = NULL;

    /* Set as default if none exists */
    if (default_application == NULL)
    {
        default_application = self;
    }
}

/*
 * Public API
 */

/**
 * grl_application_get_default:
 *
 * Gets the default #GrlApplication instance. This is the first
 * application that was created, or %NULL if no application exists.
 *
 * Returns: (transfer none) (nullable): The default application, or %NULL.
 */
GrlApplication *
grl_application_get_default (void)
{
    return default_application;
}

/**
 * grl_application_is_initialized:
 *
 * Checks if a default #GrlApplication has been created.
 *
 * Returns: %TRUE if an application exists, %FALSE otherwise.
 */
gboolean
grl_application_is_initialized (void)
{
    return (default_application != NULL);
}

/**
 * grl_application_new:
 * @application_id: (nullable): A unique identifier for the application,
 *                  or %NULL.
 *
 * Creates a new #GrlApplication instance.
 *
 * Only one application should be created per process. If an application
 * already exists, it will still be set as the default and can be retrieved
 * with grl_application_get_default().
 *
 * Returns: (transfer full): A new #GrlApplication.
 */
GrlApplication *
grl_application_new (const gchar *application_id)
{
    return g_object_new (GRL_TYPE_APPLICATION,
                         "application-id", application_id,
                         NULL);
}

/**
 * grl_application_set_config_flags:
 * @self: A #GrlApplication.
 * @flags: Configuration flags to set.
 *
 * Sets the configuration flags for raylib initialization.
 * This must be called before grl_application_run() to take effect.
 *
 * See #GrlConfigFlags for available options.
 */
void
grl_application_set_config_flags (GrlApplication *self,
                                  GrlConfigFlags  flags)
{
    GrlApplicationPrivate *priv;

    g_return_if_fail (GRL_IS_APPLICATION (self));

    priv = grl_application_get_instance_private (self);

    if (priv->config_flags != flags)
    {
        priv->config_flags = flags;
        g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_CONFIG_FLAGS]);
    }
}

/**
 * grl_application_get_config_flags:
 * @self: A #GrlApplication.
 *
 * Gets the current configuration flags.
 *
 * Returns: The current #GrlConfigFlags.
 */
GrlConfigFlags
grl_application_get_config_flags (GrlApplication *self)
{
    GrlApplicationPrivate *priv;

    g_return_val_if_fail (GRL_IS_APPLICATION (self), 0);

    priv = grl_application_get_instance_private (self);
    return priv->config_flags;
}

/**
 * grl_application_set_log_level:
 * @self: A #GrlApplication.
 * @level: The minimum log level.
 *
 * Sets the minimum log level for raylib trace output.
 * Messages below this level will be suppressed.
 */
void
grl_application_set_log_level (GrlApplication *self,
                               GrlLogLevel     level)
{
    GrlApplicationPrivate *priv;

    g_return_if_fail (GRL_IS_APPLICATION (self));

    priv = grl_application_get_instance_private (self);

    if (priv->log_level != level)
    {
        priv->log_level = level;

        /* Apply immediately if running */
        if (priv->is_running)
        {
            SetTraceLogLevel ((int)level);
        }

        g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_LOG_LEVEL]);
    }
}

/**
 * grl_application_get_log_level:
 * @self: A #GrlApplication.
 *
 * Gets the current minimum log level.
 *
 * Returns: The current #GrlLogLevel.
 */
GrlLogLevel
grl_application_get_log_level (GrlApplication *self)
{
    GrlApplicationPrivate *priv;

    g_return_val_if_fail (GRL_IS_APPLICATION (self), GRL_LOG_INFO);

    priv = grl_application_get_instance_private (self);
    return priv->log_level;
}

/**
 * grl_application_set_target_fps:
 * @self: A #GrlApplication.
 * @fps: Target frames per second (0 = unlimited).
 *
 * Sets the target frame rate for the main loop.
 * Set to 0 to disable frame limiting.
 */
void
grl_application_set_target_fps (GrlApplication *self,
                                gint            fps)
{
    GrlApplicationPrivate *priv;

    g_return_if_fail (GRL_IS_APPLICATION (self));
    g_return_if_fail (fps >= 0);

    priv = grl_application_get_instance_private (self);

    if (priv->target_fps != fps)
    {
        priv->target_fps = fps;

        /* Apply immediately if running */
        if (priv->is_running && fps > 0)
        {
            SetTargetFPS (fps);
        }

        g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_TARGET_FPS]);
    }
}

/**
 * grl_application_get_target_fps:
 * @self: A #GrlApplication.
 *
 * Gets the target frame rate.
 *
 * Returns: The target FPS, or 0 if unlimited.
 */
gint
grl_application_get_target_fps (GrlApplication *self)
{
    GrlApplicationPrivate *priv;

    g_return_val_if_fail (GRL_IS_APPLICATION (self), 0);

    priv = grl_application_get_instance_private (self);
    return priv->target_fps;
}

/**
 * grl_application_run:
 * @self: A #GrlApplication.
 *
 * Runs the application. This starts the main loop and does not return
 * until grl_application_quit() is called or all windows are closed.
 *
 * This method emits the #GrlApplication::startup signal, then
 * #GrlApplication::activate, runs the main loop, and finally emits
 * #GrlApplication::shutdown before returning.
 *
 * Note: The actual main loop is driven by windows - this method sets
 * up the application context. Windows should be created in response
 * to the activate signal.
 *
 * Returns: An exit status code (0 for success).
 */
gint
grl_application_run (GrlApplication *self)
{
    GrlApplicationPrivate *priv;

    g_return_val_if_fail (GRL_IS_APPLICATION (self), 1);

    priv = grl_application_get_instance_private (self);

    if (priv->is_running)
    {
        g_warning ("GrlApplication is already running");
        return 1;
    }

    priv->is_running = TRUE;
    priv->quit_requested = FALSE;
    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_IS_RUNNING]);

    /* Startup phase */
    g_signal_emit (self, signals[SIGNAL_STARTUP], 0);

    /* Activate phase - subclasses create windows here */
    g_signal_emit (self, signals[SIGNAL_ACTIVATE], 0);

    /*
     * The actual main loop is managed by GrlWindow. When windows are
     * created and shown, they run until closed. We just wait for all
     * windows to close or for quit to be requested.
     *
     * For now, we return immediately - the window will run its own loop.
     * In a more sophisticated implementation, we might integrate with
     * GLib's main loop or manage multiple windows.
     */

    /* Shutdown phase */
    g_signal_emit (self, signals[SIGNAL_SHUTDOWN], 0);

    priv->is_running = FALSE;
    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_IS_RUNNING]);

    return 0;
}

/**
 * grl_application_quit:
 * @self: A #GrlApplication.
 *
 * Requests the application to quit. This will cause grl_application_run()
 * to return after the current frame completes.
 */
void
grl_application_quit (GrlApplication *self)
{
    GrlApplicationPrivate *priv;

    g_return_if_fail (GRL_IS_APPLICATION (self));

    priv = grl_application_get_instance_private (self);
    priv->quit_requested = TRUE;
}

/**
 * grl_application_is_running:
 * @self: A #GrlApplication.
 *
 * Checks if the application is currently running its main loop.
 *
 * Returns: %TRUE if running, %FALSE otherwise.
 */
gboolean
grl_application_is_running (GrlApplication *self)
{
    GrlApplicationPrivate *priv;

    g_return_val_if_fail (GRL_IS_APPLICATION (self), FALSE);

    priv = grl_application_get_instance_private (self);
    return priv->is_running;
}

/**
 * grl_application_get_application_id:
 * @self: A #GrlApplication.
 *
 * Gets the application identifier.
 *
 * Returns: (transfer none) (nullable): The application ID, or %NULL.
 */
const gchar *
grl_application_get_application_id (GrlApplication *self)
{
    GrlApplicationPrivate *priv;

    g_return_val_if_fail (GRL_IS_APPLICATION (self), NULL);

    priv = grl_application_get_instance_private (self);
    return priv->application_id;
}

/**
 * grl_application_get_frame_time:
 * @self: A #GrlApplication.
 *
 * Gets the time in seconds for the last frame.
 * This is the delta time to use for frame-rate independent movement.
 *
 * Returns: Time in seconds for the last frame.
 */
gfloat
grl_application_get_frame_time (GrlApplication *self)
{
    g_return_val_if_fail (GRL_IS_APPLICATION (self), 0.0f);

    return GetFrameTime ();
}

/**
 * grl_application_get_fps:
 * @self: A #GrlApplication.
 *
 * Gets the current frames per second.
 *
 * Returns: Current FPS.
 */
gint
grl_application_get_fps (GrlApplication *self)
{
    g_return_val_if_fail (GRL_IS_APPLICATION (self), 0);

    return GetFPS ();
}

/**
 * grl_application_get_time:
 * @self: A #GrlApplication.
 *
 * Gets the elapsed time in seconds since the window was initialized.
 *
 * Returns: Elapsed time in seconds.
 */
gdouble
grl_application_get_time (GrlApplication *self)
{
    g_return_val_if_fail (GRL_IS_APPLICATION (self), 0.0);

    return GetTime ();
}

/**
 * grl_application_get_frame_count:
 * @self: A #GrlApplication.
 *
 * Gets the total number of frames rendered since the application started.
 *
 * Returns: Total frame count.
 */
guint64
grl_application_get_frame_count (GrlApplication *self)
{
    GrlApplicationPrivate *priv;

    g_return_val_if_fail (GRL_IS_APPLICATION (self), 0);

    priv = grl_application_get_instance_private (self);
    return priv->frame_count;
}
