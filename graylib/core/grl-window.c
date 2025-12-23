/* grl-window.c
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Window management implementation.
 */

#include "config.h"
#include "grl-window.h"
#include <raylib.h>
#include <math.h>

/* Maximum number of gamepads to poll */
#define GRL_MAX_GAMEPADS 4

/* Axis dead zone threshold */
#define GRL_AXIS_DEADZONE 0.1f

/**
 * SECTION:grl-window
 * @title: GrlWindow
 * @short_description: Window management for Graylib
 *
 * #GrlWindow provides a GObject wrapper around raylib's window management
 * functionality. It handles window creation, configuration, and the
 * drawing/update loop.
 *
 * Typical usage:
 *
 * |[<!-- language="C" -->
 * g_autoptr(GrlWindow) window = NULL;
 * g_autoptr(GrlColor) bg = NULL;
 *
 * window = grl_window_new (800, 600, "My Game");
 * bg = grl_color_new_raywhite ();
 *
 * while (!grl_window_should_close (window))
 * {
 *     grl_window_begin_drawing (window);
 *     grl_window_clear_background (window, bg);
 *     // Draw game content here
 *     grl_window_end_drawing (window);
 * }
 * ]|
 *
 * For subclassing, override the virtual methods:
 * - #GrlWindowClass.update for game logic
 * - #GrlWindowClass.draw for rendering
 * - #GrlWindowClass.on_resize for handling resize events
 * - #GrlWindowClass.on_close to intercept close requests
 */

typedef struct _GrlWindowPrivate
{
    gchar          *title;
    gint            initial_width;
    gint            initial_height;
    gboolean        is_initialized;
    gint            target_fps;
    guint64         frame_count;

    /* Input tracking for delta calculation */
    gint            prev_mouse_x;
    gint            prev_mouse_y;
    gboolean        input_initialized;

    /* Gamepad axis tracking for change detection */
    gfloat          prev_axis[GRL_MAX_GAMEPADS][6];
} GrlWindowPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (GrlWindow, grl_window, G_TYPE_OBJECT)

enum
{
    PROP_0,
    PROP_TITLE,
    PROP_WIDTH,
    PROP_HEIGHT,
    PROP_TARGET_FPS,
    PROP_IS_INITIALIZED,
    N_PROPS
};

static GParamSpec *properties[N_PROPS];

enum
{
    SIGNAL_DRAW,
    SIGNAL_UPDATE,
    SIGNAL_RESIZE,
    SIGNAL_CLOSE_REQUEST,
    /* Input signals */
    SIGNAL_KEY_PRESSED,
    SIGNAL_KEY_RELEASED,
    SIGNAL_MOUSE_BUTTON_PRESSED,
    SIGNAL_MOUSE_BUTTON_RELEASED,
    SIGNAL_MOUSE_MOVED,
    SIGNAL_MOUSE_WHEEL,
    SIGNAL_GAMEPAD_BUTTON_PRESSED,
    SIGNAL_GAMEPAD_BUTTON_RELEASED,
    SIGNAL_GAMEPAD_AXIS_MOVED,
    N_SIGNALS
};

static guint signals[N_SIGNALS];

/*
 * Default virtual method implementations
 */

static void
grl_window_real_draw (GrlWindow *self)
{
    /* Default draw does nothing - subclasses override */
}

static void
grl_window_real_update (GrlWindow *self,
                        gfloat     delta_time)
{
    /* Default update does nothing - subclasses override */
}

static void
grl_window_real_on_resize (GrlWindow *self,
                           gint       width,
                           gint       height)
{
    /* Default resize handler does nothing */
}

static gboolean
grl_window_real_on_close (GrlWindow *self)
{
    /* Default allows close */
    return TRUE;
}

/* Input virtual method defaults */

static void
grl_window_real_on_key_pressed (GrlWindow *self,
                                 GrlKey     key)
{
    /* Default does nothing */
}

static void
grl_window_real_on_key_released (GrlWindow *self,
                                  GrlKey     key)
{
    /* Default does nothing */
}

static void
grl_window_real_on_mouse_button_pressed (GrlWindow      *self,
                                          GrlMouseButton  button,
                                          gint            x,
                                          gint            y)
{
    /* Default does nothing */
}

static void
grl_window_real_on_mouse_button_released (GrlWindow      *self,
                                           GrlMouseButton  button,
                                           gint            x,
                                           gint            y)
{
    /* Default does nothing */
}

static void
grl_window_real_on_mouse_moved (GrlWindow *self,
                                 gint       x,
                                 gint       y,
                                 gint       delta_x,
                                 gint       delta_y)
{
    /* Default does nothing */
}

static void
grl_window_real_on_mouse_wheel (GrlWindow *self,
                                 gfloat     x,
                                 gfloat     y)
{
    /* Default does nothing */
}

static void
grl_window_real_on_gamepad_button_pressed (GrlWindow        *self,
                                            gint              gamepad,
                                            GrlGamepadButton  button)
{
    /* Default does nothing */
}

static void
grl_window_real_on_gamepad_button_released (GrlWindow        *self,
                                             gint              gamepad,
                                             GrlGamepadButton  button)
{
    /* Default does nothing */
}

static void
grl_window_real_on_gamepad_axis_moved (GrlWindow      *self,
                                        gint            gamepad,
                                        GrlGamepadAxis  axis,
                                        gfloat          value)
{
    /* Default does nothing */
}

/*
 * GObject virtual methods
 */

static void
grl_window_get_property (GObject    *object,
                         guint       prop_id,
                         GValue     *value,
                         GParamSpec *pspec)
{
    GrlWindow *self = GRL_WINDOW (object);
    GrlWindowPrivate *priv = grl_window_get_instance_private (self);

    switch (prop_id)
    {
    case PROP_TITLE:
        g_value_set_string (value, priv->title);
        break;

    case PROP_WIDTH:
        if (priv->is_initialized)
        {
            g_value_set_int (value, GetScreenWidth ());
        }
        else
        {
            g_value_set_int (value, priv->initial_width);
        }
        break;

    case PROP_HEIGHT:
        if (priv->is_initialized)
        {
            g_value_set_int (value, GetScreenHeight ());
        }
        else
        {
            g_value_set_int (value, priv->initial_height);
        }
        break;

    case PROP_TARGET_FPS:
        g_value_set_int (value, priv->target_fps);
        break;

    case PROP_IS_INITIALIZED:
        g_value_set_boolean (value, priv->is_initialized);
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static void
grl_window_set_property (GObject      *object,
                         guint         prop_id,
                         const GValue *value,
                         GParamSpec   *pspec)
{
    GrlWindow *self = GRL_WINDOW (object);
    GrlWindowPrivate *priv = grl_window_get_instance_private (self);

    switch (prop_id)
    {
    case PROP_TITLE:
        g_free (priv->title);
        priv->title = g_value_dup_string (value);
        if (priv->is_initialized)
        {
            SetWindowTitle (priv->title);
        }
        break;

    case PROP_WIDTH:
        priv->initial_width = g_value_get_int (value);
        break;

    case PROP_HEIGHT:
        priv->initial_height = g_value_get_int (value);
        break;

    case PROP_TARGET_FPS:
        priv->target_fps = g_value_get_int (value);
        if (priv->is_initialized)
        {
            SetTargetFPS (priv->target_fps);
        }
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static void
grl_window_constructed (GObject *object)
{
    GrlWindow *self = GRL_WINDOW (object);
    GrlWindowPrivate *priv = grl_window_get_instance_private (self);

    G_OBJECT_CLASS (grl_window_parent_class)->constructed (object);

    /* Initialize the raylib window */
    InitWindow (priv->initial_width,
                priv->initial_height,
                priv->title ? priv->title : "Graylib Window");

    if (IsWindowReady ())
    {
        priv->is_initialized = TRUE;

        if (priv->target_fps > 0)
        {
            SetTargetFPS (priv->target_fps);
        }
    }
    else
    {
        g_warning ("Failed to initialize window");
    }
}

static void
grl_window_dispose (GObject *object)
{
    GrlWindow *self = GRL_WINDOW (object);
    GrlWindowPrivate *priv = grl_window_get_instance_private (self);

    /* Close the raylib window if it was initialized */
    if (priv->is_initialized && IsWindowReady ())
    {
        CloseWindow ();
        priv->is_initialized = FALSE;
    }

    G_OBJECT_CLASS (grl_window_parent_class)->dispose (object);
}

static void
grl_window_finalize (GObject *object)
{
    GrlWindow *self = GRL_WINDOW (object);
    GrlWindowPrivate *priv = grl_window_get_instance_private (self);

    g_free (priv->title);
    priv->title = NULL;

    G_OBJECT_CLASS (grl_window_parent_class)->finalize (object);
}

static void
grl_window_class_init (GrlWindowClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->get_property = grl_window_get_property;
    object_class->set_property = grl_window_set_property;
    object_class->constructed = grl_window_constructed;
    object_class->dispose = grl_window_dispose;
    object_class->finalize = grl_window_finalize;

    /* Virtual method defaults */
    klass->draw = grl_window_real_draw;
    klass->update = grl_window_real_update;
    klass->on_resize = grl_window_real_on_resize;
    klass->on_close = grl_window_real_on_close;

    /* Input virtual method defaults */
    klass->on_key_pressed = grl_window_real_on_key_pressed;
    klass->on_key_released = grl_window_real_on_key_released;
    klass->on_mouse_button_pressed = grl_window_real_on_mouse_button_pressed;
    klass->on_mouse_button_released = grl_window_real_on_mouse_button_released;
    klass->on_mouse_moved = grl_window_real_on_mouse_moved;
    klass->on_mouse_wheel = grl_window_real_on_mouse_wheel;
    klass->on_gamepad_button_pressed = grl_window_real_on_gamepad_button_pressed;
    klass->on_gamepad_button_released = grl_window_real_on_gamepad_button_released;
    klass->on_gamepad_axis_moved = grl_window_real_on_gamepad_axis_moved;

    /**
     * GrlWindow:title:
     *
     * The title displayed in the window title bar.
     */
    properties[PROP_TITLE] =
        g_param_spec_string ("title",
                             "Title",
                             "Window title",
                             "Graylib Window",
                             G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_STATIC_STRINGS);

    /**
     * GrlWindow:width:
     *
     * The width of the window in pixels.
     */
    properties[PROP_WIDTH] =
        g_param_spec_int ("width",
                          "Width",
                          "Window width in pixels",
                          1, G_MAXINT,
                          800,
                          G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

    /**
     * GrlWindow:height:
     *
     * The height of the window in pixels.
     */
    properties[PROP_HEIGHT] =
        g_param_spec_int ("height",
                          "Height",
                          "Window height in pixels",
                          1, G_MAXINT,
                          600,
                          G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

    /**
     * GrlWindow:target-fps:
     *
     * Target frames per second. Set to 0 for unlimited.
     */
    properties[PROP_TARGET_FPS] =
        g_param_spec_int ("target-fps",
                          "Target FPS",
                          "Target frames per second (0 = unlimited)",
                          0, G_MAXINT,
                          60,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

    /**
     * GrlWindow:is-initialized:
     *
     * Whether the window has been successfully initialized.
     */
    properties[PROP_IS_INITIALIZED] =
        g_param_spec_boolean ("is-initialized",
                              "Is Initialized",
                              "Whether the window is initialized",
                              FALSE,
                              G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

    g_object_class_install_properties (object_class, N_PROPS, properties);

    /**
     * GrlWindow::draw:
     * @window: The window that emitted the signal.
     *
     * Emitted each frame during the drawing phase.
     * Connect to this signal to perform custom drawing.
     */
    signals[SIGNAL_DRAW] =
        g_signal_new ("draw",
                      G_TYPE_FROM_CLASS (klass),
                      G_SIGNAL_RUN_LAST,
                      G_STRUCT_OFFSET (GrlWindowClass, draw),
                      NULL, NULL,
                      NULL,
                      G_TYPE_NONE, 0);

    /**
     * GrlWindow::update:
     * @window: The window that emitted the signal.
     * @delta_time: Time elapsed since the last frame in seconds.
     *
     * Emitted each frame during the update phase, before drawing.
     * Connect to this signal to update game state.
     */
    signals[SIGNAL_UPDATE] =
        g_signal_new ("update",
                      G_TYPE_FROM_CLASS (klass),
                      G_SIGNAL_RUN_LAST,
                      G_STRUCT_OFFSET (GrlWindowClass, update),
                      NULL, NULL,
                      NULL,
                      G_TYPE_NONE, 1, G_TYPE_FLOAT);

    /**
     * GrlWindow::resize:
     * @window: The window that emitted the signal.
     * @width: New width in pixels.
     * @height: New height in pixels.
     *
     * Emitted when the window is resized.
     */
    signals[SIGNAL_RESIZE] =
        g_signal_new ("resize",
                      G_TYPE_FROM_CLASS (klass),
                      G_SIGNAL_RUN_LAST,
                      G_STRUCT_OFFSET (GrlWindowClass, on_resize),
                      NULL, NULL,
                      NULL,
                      G_TYPE_NONE, 2, G_TYPE_INT, G_TYPE_INT);

    /**
     * GrlWindow::close-request:
     * @window: The window that emitted the signal.
     *
     * Emitted when the window close is requested (e.g., user clicks X).
     *
     * Returns: %TRUE to allow the close, %FALSE to cancel.
     */
    signals[SIGNAL_CLOSE_REQUEST] =
        g_signal_new ("close-request",
                      G_TYPE_FROM_CLASS (klass),
                      G_SIGNAL_RUN_LAST,
                      G_STRUCT_OFFSET (GrlWindowClass, on_close),
                      g_signal_accumulator_true_handled, NULL,
                      NULL,
                      G_TYPE_BOOLEAN, 0);

    /*
     * Input signals
     */

    /**
     * GrlWindow::key-pressed:
     * @window: The window that emitted the signal.
     * @key: The #GrlKey that was pressed.
     *
     * Emitted when a key is pressed. This signal is emitted once
     * when the key is first pressed, not continuously while held.
     * Use grl_input_is_key_down() for continuous key state checking.
     */
    signals[SIGNAL_KEY_PRESSED] =
        g_signal_new ("key-pressed",
                      G_TYPE_FROM_CLASS (klass),
                      G_SIGNAL_RUN_LAST,
                      G_STRUCT_OFFSET (GrlWindowClass, on_key_pressed),
                      NULL, NULL,
                      NULL,
                      G_TYPE_NONE, 1, GRL_TYPE_KEY);

    /**
     * GrlWindow::key-released:
     * @window: The window that emitted the signal.
     * @key: The #GrlKey that was released.
     *
     * Emitted when a key is released.
     */
    signals[SIGNAL_KEY_RELEASED] =
        g_signal_new ("key-released",
                      G_TYPE_FROM_CLASS (klass),
                      G_SIGNAL_RUN_LAST,
                      G_STRUCT_OFFSET (GrlWindowClass, on_key_released),
                      NULL, NULL,
                      NULL,
                      G_TYPE_NONE, 1, GRL_TYPE_KEY);

    /**
     * GrlWindow::mouse-button-pressed:
     * @window: The window that emitted the signal.
     * @button: The #GrlMouseButton that was pressed.
     * @x: Mouse X position when pressed.
     * @y: Mouse Y position when pressed.
     *
     * Emitted when a mouse button is pressed.
     */
    signals[SIGNAL_MOUSE_BUTTON_PRESSED] =
        g_signal_new ("mouse-button-pressed",
                      G_TYPE_FROM_CLASS (klass),
                      G_SIGNAL_RUN_LAST,
                      G_STRUCT_OFFSET (GrlWindowClass, on_mouse_button_pressed),
                      NULL, NULL,
                      NULL,
                      G_TYPE_NONE, 3, GRL_TYPE_MOUSE_BUTTON, G_TYPE_INT, G_TYPE_INT);

    /**
     * GrlWindow::mouse-button-released:
     * @window: The window that emitted the signal.
     * @button: The #GrlMouseButton that was released.
     * @x: Mouse X position when released.
     * @y: Mouse Y position when released.
     *
     * Emitted when a mouse button is released.
     */
    signals[SIGNAL_MOUSE_BUTTON_RELEASED] =
        g_signal_new ("mouse-button-released",
                      G_TYPE_FROM_CLASS (klass),
                      G_SIGNAL_RUN_LAST,
                      G_STRUCT_OFFSET (GrlWindowClass, on_mouse_button_released),
                      NULL, NULL,
                      NULL,
                      G_TYPE_NONE, 3, GRL_TYPE_MOUSE_BUTTON, G_TYPE_INT, G_TYPE_INT);

    /**
     * GrlWindow::mouse-moved:
     * @window: The window that emitted the signal.
     * @x: Current mouse X position.
     * @y: Current mouse Y position.
     * @delta_x: Change in X since last frame.
     * @delta_y: Change in Y since last frame.
     *
     * Emitted when the mouse moves.
     */
    signals[SIGNAL_MOUSE_MOVED] =
        g_signal_new ("mouse-moved",
                      G_TYPE_FROM_CLASS (klass),
                      G_SIGNAL_RUN_LAST,
                      G_STRUCT_OFFSET (GrlWindowClass, on_mouse_moved),
                      NULL, NULL,
                      NULL,
                      G_TYPE_NONE, 4, G_TYPE_INT, G_TYPE_INT, G_TYPE_INT, G_TYPE_INT);

    /**
     * GrlWindow::mouse-wheel:
     * @window: The window that emitted the signal.
     * @x: Horizontal scroll amount.
     * @y: Vertical scroll amount.
     *
     * Emitted when the mouse wheel is scrolled.
     */
    signals[SIGNAL_MOUSE_WHEEL] =
        g_signal_new ("mouse-wheel",
                      G_TYPE_FROM_CLASS (klass),
                      G_SIGNAL_RUN_LAST,
                      G_STRUCT_OFFSET (GrlWindowClass, on_mouse_wheel),
                      NULL, NULL,
                      NULL,
                      G_TYPE_NONE, 2, G_TYPE_FLOAT, G_TYPE_FLOAT);

    /**
     * GrlWindow::gamepad-button-pressed:
     * @window: The window that emitted the signal.
     * @gamepad: Gamepad index (0-3).
     * @button: The #GrlGamepadButton that was pressed.
     *
     * Emitted when a gamepad button is pressed.
     */
    signals[SIGNAL_GAMEPAD_BUTTON_PRESSED] =
        g_signal_new ("gamepad-button-pressed",
                      G_TYPE_FROM_CLASS (klass),
                      G_SIGNAL_RUN_LAST,
                      G_STRUCT_OFFSET (GrlWindowClass, on_gamepad_button_pressed),
                      NULL, NULL,
                      NULL,
                      G_TYPE_NONE, 2, G_TYPE_INT, GRL_TYPE_GAMEPAD_BUTTON);

    /**
     * GrlWindow::gamepad-button-released:
     * @window: The window that emitted the signal.
     * @gamepad: Gamepad index (0-3).
     * @button: The #GrlGamepadButton that was released.
     *
     * Emitted when a gamepad button is released.
     */
    signals[SIGNAL_GAMEPAD_BUTTON_RELEASED] =
        g_signal_new ("gamepad-button-released",
                      G_TYPE_FROM_CLASS (klass),
                      G_SIGNAL_RUN_LAST,
                      G_STRUCT_OFFSET (GrlWindowClass, on_gamepad_button_released),
                      NULL, NULL,
                      NULL,
                      G_TYPE_NONE, 2, G_TYPE_INT, GRL_TYPE_GAMEPAD_BUTTON);

    /**
     * GrlWindow::gamepad-axis-moved:
     * @window: The window that emitted the signal.
     * @gamepad: Gamepad index (0-3).
     * @axis: The #GrlGamepadAxis that moved.
     * @value: New axis value (-1.0 to 1.0).
     *
     * Emitted when a gamepad axis value changes.
     */
    signals[SIGNAL_GAMEPAD_AXIS_MOVED] =
        g_signal_new ("gamepad-axis-moved",
                      G_TYPE_FROM_CLASS (klass),
                      G_SIGNAL_RUN_LAST,
                      G_STRUCT_OFFSET (GrlWindowClass, on_gamepad_axis_moved),
                      NULL, NULL,
                      NULL,
                      G_TYPE_NONE, 3, G_TYPE_INT, GRL_TYPE_GAMEPAD_AXIS, G_TYPE_FLOAT);
}

static void
grl_window_init (GrlWindow *self)
{
    GrlWindowPrivate *priv = grl_window_get_instance_private (self);
    gint i, j;

    priv->title = NULL;
    priv->initial_width = 800;
    priv->initial_height = 600;
    priv->is_initialized = FALSE;
    priv->target_fps = 60;
    priv->frame_count = 0;

    /* Input tracking */
    priv->prev_mouse_x = 0;
    priv->prev_mouse_y = 0;
    priv->input_initialized = FALSE;

    /* Initialize previous axis values to zero */
    for (i = 0; i < GRL_MAX_GAMEPADS; i++)
    {
        for (j = 0; j < 6; j++)
        {
            priv->prev_axis[i][j] = 0.0f;
        }
    }
}

/*
 * Public API - Constructors
 */

/**
 * grl_window_new:
 * @width: Window width in pixels.
 * @height: Window height in pixels.
 * @title: Window title.
 *
 * Creates a new window and initializes the graphics context.
 *
 * Returns: (transfer full): A new #GrlWindow.
 */
GrlWindow *
grl_window_new (gint         width,
                gint         height,
                const gchar *title)
{
    return g_object_new (GRL_TYPE_WINDOW,
                         "width", width,
                         "height", height,
                         "title", title,
                         NULL);
}

/*
 * Public API - Window state
 */

/**
 * grl_window_should_close:
 * @self: A #GrlWindow.
 *
 * Checks if the window should close (user pressed ESC or clicked close).
 *
 * Returns: %TRUE if the window should close.
 */
gboolean
grl_window_should_close (GrlWindow *self)
{
    g_return_val_if_fail (GRL_IS_WINDOW (self), TRUE);

    return WindowShouldClose ();
}

/**
 * grl_window_set_should_close:
 * @self: A #GrlWindow.
 * @should_close: Whether the window should close.
 *
 * Sets the close flag for the window. Useful for implementing
 * custom close logic.
 */
void
grl_window_set_should_close (GrlWindow *self,
                             gboolean   should_close)
{
    g_return_if_fail (GRL_IS_WINDOW (self));

    /* raylib doesn't have a SetWindowShouldClose, but we can use
     * the exit key functionality or just close */
    if (should_close)
    {
        /* This will be handled by the main loop */
    }
}

/**
 * grl_window_is_ready:
 * @self: A #GrlWindow.
 *
 * Checks if the window has been successfully initialized.
 *
 * Returns: %TRUE if the window is ready.
 */
gboolean
grl_window_is_ready (GrlWindow *self)
{
    g_return_val_if_fail (GRL_IS_WINDOW (self), FALSE);

    return IsWindowReady ();
}

/**
 * grl_window_is_fullscreen:
 * @self: A #GrlWindow.
 *
 * Checks if the window is currently in fullscreen mode.
 *
 * Returns: %TRUE if fullscreen.
 */
gboolean
grl_window_is_fullscreen (GrlWindow *self)
{
    g_return_val_if_fail (GRL_IS_WINDOW (self), FALSE);

    return IsWindowFullscreen ();
}

/**
 * grl_window_is_hidden:
 * @self: A #GrlWindow.
 *
 * Checks if the window is currently hidden.
 *
 * Returns: %TRUE if hidden.
 */
gboolean
grl_window_is_hidden (GrlWindow *self)
{
    g_return_val_if_fail (GRL_IS_WINDOW (self), FALSE);

    return IsWindowHidden ();
}

/**
 * grl_window_is_minimized:
 * @self: A #GrlWindow.
 *
 * Checks if the window is currently minimized.
 *
 * Returns: %TRUE if minimized.
 */
gboolean
grl_window_is_minimized (GrlWindow *self)
{
    g_return_val_if_fail (GRL_IS_WINDOW (self), FALSE);

    return IsWindowMinimized ();
}

/**
 * grl_window_is_maximized:
 * @self: A #GrlWindow.
 *
 * Checks if the window is currently maximized.
 *
 * Returns: %TRUE if maximized.
 */
gboolean
grl_window_is_maximized (GrlWindow *self)
{
    g_return_val_if_fail (GRL_IS_WINDOW (self), FALSE);

    return IsWindowMaximized ();
}

/**
 * grl_window_is_focused:
 * @self: A #GrlWindow.
 *
 * Checks if the window currently has input focus.
 *
 * Returns: %TRUE if focused.
 */
gboolean
grl_window_is_focused (GrlWindow *self)
{
    g_return_val_if_fail (GRL_IS_WINDOW (self), FALSE);

    return IsWindowFocused ();
}

/**
 * grl_window_is_resized:
 * @self: A #GrlWindow.
 *
 * Checks if the window was resized since the last frame.
 *
 * Returns: %TRUE if resized.
 */
gboolean
grl_window_is_resized (GrlWindow *self)
{
    g_return_val_if_fail (GRL_IS_WINDOW (self), FALSE);

    return IsWindowResized ();
}

/**
 * grl_window_has_state:
 * @self: A #GrlWindow.
 * @flag: Configuration flag to check.
 *
 * Checks if a specific window state flag is enabled.
 *
 * Returns: %TRUE if the flag is set.
 */
gboolean
grl_window_has_state (GrlWindow      *self,
                      GrlConfigFlags  flag)
{
    g_return_val_if_fail (GRL_IS_WINDOW (self), FALSE);

    return IsWindowState ((unsigned int)flag);
}

/*
 * Public API - Window configuration
 */

/**
 * grl_window_set_state:
 * @self: A #GrlWindow.
 * @flags: Configuration flags to enable.
 *
 * Enables the specified window state flags.
 */
void
grl_window_set_state (GrlWindow      *self,
                      GrlConfigFlags  flags)
{
    g_return_if_fail (GRL_IS_WINDOW (self));

    SetWindowState ((unsigned int)flags);
}

/**
 * grl_window_clear_state:
 * @self: A #GrlWindow.
 * @flags: Configuration flags to disable.
 *
 * Disables the specified window state flags.
 */
void
grl_window_clear_state (GrlWindow      *self,
                        GrlConfigFlags  flags)
{
    g_return_if_fail (GRL_IS_WINDOW (self));

    ClearWindowState ((unsigned int)flags);
}

/**
 * grl_window_toggle_fullscreen:
 * @self: A #GrlWindow.
 *
 * Toggles between fullscreen and windowed mode.
 */
void
grl_window_toggle_fullscreen (GrlWindow *self)
{
    g_return_if_fail (GRL_IS_WINDOW (self));

    ToggleFullscreen ();
}

/**
 * grl_window_toggle_borderless:
 * @self: A #GrlWindow.
 *
 * Toggles borderless windowed mode.
 */
void
grl_window_toggle_borderless (GrlWindow *self)
{
    g_return_if_fail (GRL_IS_WINDOW (self));

    ToggleBorderlessWindowed ();
}

/**
 * grl_window_maximize:
 * @self: A #GrlWindow.
 *
 * Maximizes the window.
 */
void
grl_window_maximize (GrlWindow *self)
{
    g_return_if_fail (GRL_IS_WINDOW (self));

    MaximizeWindow ();
}

/**
 * grl_window_minimize:
 * @self: A #GrlWindow.
 *
 * Minimizes the window.
 */
void
grl_window_minimize (GrlWindow *self)
{
    g_return_if_fail (GRL_IS_WINDOW (self));

    MinimizeWindow ();
}

/**
 * grl_window_restore:
 * @self: A #GrlWindow.
 *
 * Restores the window from minimized/maximized state.
 */
void
grl_window_restore (GrlWindow *self)
{
    g_return_if_fail (GRL_IS_WINDOW (self));

    RestoreWindow ();
}

/**
 * grl_window_set_title:
 * @self: A #GrlWindow.
 * @title: New window title.
 *
 * Sets the window title.
 */
void
grl_window_set_title (GrlWindow   *self,
                      const gchar *title)
{
    GrlWindowPrivate *priv;

    g_return_if_fail (GRL_IS_WINDOW (self));
    g_return_if_fail (title != NULL);

    priv = grl_window_get_instance_private (self);

    g_free (priv->title);
    priv->title = g_strdup (title);

    if (priv->is_initialized)
    {
        SetWindowTitle (title);
    }

    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_TITLE]);
}

/**
 * grl_window_get_title:
 * @self: A #GrlWindow.
 *
 * Gets the window title.
 *
 * Returns: (transfer none): The window title.
 */
const gchar *
grl_window_get_title (GrlWindow *self)
{
    GrlWindowPrivate *priv;

    g_return_val_if_fail (GRL_IS_WINDOW (self), NULL);

    priv = grl_window_get_instance_private (self);
    return priv->title;
}

/**
 * grl_window_set_position:
 * @self: A #GrlWindow.
 * @x: X position on screen.
 * @y: Y position on screen.
 *
 * Sets the window position on screen.
 */
void
grl_window_set_position (GrlWindow *self,
                         gint       x,
                         gint       y)
{
    g_return_if_fail (GRL_IS_WINDOW (self));

    SetWindowPosition (x, y);
}

/**
 * grl_window_get_position:
 * @self: A #GrlWindow.
 *
 * Gets the window position on screen.
 *
 * Returns: (transfer full): A new #GrlVector2 with the position.
 */
GrlVector2 *
grl_window_get_position (GrlWindow *self)
{
    Vector2 pos;

    g_return_val_if_fail (GRL_IS_WINDOW (self), NULL);

    pos = GetWindowPosition ();
    return grl_vector2_new (pos.x, pos.y);
}

/**
 * grl_window_set_size:
 * @self: A #GrlWindow.
 * @width: New width.
 * @height: New height.
 *
 * Sets the window size.
 */
void
grl_window_set_size (GrlWindow *self,
                     gint       width,
                     gint       height)
{
    g_return_if_fail (GRL_IS_WINDOW (self));
    g_return_if_fail (width > 0);
    g_return_if_fail (height > 0);

    SetWindowSize (width, height);
}

/**
 * grl_window_get_width:
 * @self: A #GrlWindow.
 *
 * Gets the current window width.
 *
 * Returns: Window width in pixels.
 */
gint
grl_window_get_width (GrlWindow *self)
{
    g_return_val_if_fail (GRL_IS_WINDOW (self), 0);

    return GetScreenWidth ();
}

/**
 * grl_window_get_height:
 * @self: A #GrlWindow.
 *
 * Gets the current window height.
 *
 * Returns: Window height in pixels.
 */
gint
grl_window_get_height (GrlWindow *self)
{
    g_return_val_if_fail (GRL_IS_WINDOW (self), 0);

    return GetScreenHeight ();
}

/**
 * grl_window_set_min_size:
 * @self: A #GrlWindow.
 * @width: Minimum width.
 * @height: Minimum height.
 *
 * Sets the minimum window size (for resizable windows).
 */
void
grl_window_set_min_size (GrlWindow *self,
                         gint       width,
                         gint       height)
{
    g_return_if_fail (GRL_IS_WINDOW (self));

    SetWindowMinSize (width, height);
}

/**
 * grl_window_set_max_size:
 * @self: A #GrlWindow.
 * @width: Maximum width.
 * @height: Maximum height.
 *
 * Sets the maximum window size (for resizable windows).
 */
void
grl_window_set_max_size (GrlWindow *self,
                         gint       width,
                         gint       height)
{
    g_return_if_fail (GRL_IS_WINDOW (self));

    SetWindowMaxSize (width, height);
}

/**
 * grl_window_set_opacity:
 * @self: A #GrlWindow.
 * @opacity: Opacity value (0.0 to 1.0).
 *
 * Sets the window opacity.
 */
void
grl_window_set_opacity (GrlWindow *self,
                        gfloat     opacity)
{
    g_return_if_fail (GRL_IS_WINDOW (self));
    g_return_if_fail (opacity >= 0.0f && opacity <= 1.0f);

    SetWindowOpacity (opacity);
}

/**
 * grl_window_set_monitor:
 * @self: A #GrlWindow.
 * @monitor: Monitor index.
 *
 * Moves the window to the specified monitor.
 */
void
grl_window_set_monitor (GrlWindow *self,
                        gint       monitor)
{
    g_return_if_fail (GRL_IS_WINDOW (self));

    SetWindowMonitor (monitor);
}

/**
 * grl_window_focus:
 * @self: A #GrlWindow.
 *
 * Brings the window to focus.
 */
void
grl_window_focus (GrlWindow *self)
{
    g_return_if_fail (GRL_IS_WINDOW (self));

    SetWindowFocused ();
}

/*
 * Public API - Rendering dimensions
 */

/**
 * grl_window_get_render_width:
 * @self: A #GrlWindow.
 *
 * Gets the render width (considering HiDPI).
 *
 * Returns: Render width in pixels.
 */
gint
grl_window_get_render_width (GrlWindow *self)
{
    g_return_val_if_fail (GRL_IS_WINDOW (self), 0);

    return GetRenderWidth ();
}

/**
 * grl_window_get_render_height:
 * @self: A #GrlWindow.
 *
 * Gets the render height (considering HiDPI).
 *
 * Returns: Render height in pixels.
 */
gint
grl_window_get_render_height (GrlWindow *self)
{
    g_return_val_if_fail (GRL_IS_WINDOW (self), 0);

    return GetRenderHeight ();
}

/**
 * grl_window_get_scale_dpi:
 * @self: A #GrlWindow.
 *
 * Gets the DPI scale factor.
 *
 * Returns: (transfer full): A new #GrlVector2 with the scale factors.
 */
GrlVector2 *
grl_window_get_scale_dpi (GrlWindow *self)
{
    Vector2 scale;

    g_return_val_if_fail (GRL_IS_WINDOW (self), NULL);

    scale = GetWindowScaleDPI ();
    return grl_vector2_new (scale.x, scale.y);
}

/*
 * Public API - Monitor info
 */

/**
 * grl_window_get_monitor_count:
 * @self: A #GrlWindow.
 *
 * Gets the number of connected monitors.
 *
 * Returns: Number of monitors.
 */
gint
grl_window_get_monitor_count (GrlWindow *self)
{
    g_return_val_if_fail (GRL_IS_WINDOW (self), 0);

    return GetMonitorCount ();
}

/**
 * grl_window_get_current_monitor:
 * @self: A #GrlWindow.
 *
 * Gets the index of the monitor the window is currently on.
 *
 * Returns: Monitor index.
 */
gint
grl_window_get_current_monitor (GrlWindow *self)
{
    g_return_val_if_fail (GRL_IS_WINDOW (self), 0);

    return GetCurrentMonitor ();
}

/**
 * grl_window_get_monitor_width:
 * @self: A #GrlWindow.
 * @monitor: Monitor index.
 *
 * Gets the width of the specified monitor.
 *
 * Returns: Monitor width in pixels.
 */
gint
grl_window_get_monitor_width (GrlWindow *self,
                              gint       monitor)
{
    g_return_val_if_fail (GRL_IS_WINDOW (self), 0);

    return GetMonitorWidth (monitor);
}

/**
 * grl_window_get_monitor_height:
 * @self: A #GrlWindow.
 * @monitor: Monitor index.
 *
 * Gets the height of the specified monitor.
 *
 * Returns: Monitor height in pixels.
 */
gint
grl_window_get_monitor_height (GrlWindow *self,
                               gint       monitor)
{
    g_return_val_if_fail (GRL_IS_WINDOW (self), 0);

    return GetMonitorHeight (monitor);
}

/**
 * grl_window_get_monitor_refresh_rate:
 * @self: A #GrlWindow.
 * @monitor: Monitor index.
 *
 * Gets the refresh rate of the specified monitor.
 *
 * Returns: Refresh rate in Hz.
 */
gint
grl_window_get_monitor_refresh_rate (GrlWindow *self,
                                     gint       monitor)
{
    g_return_val_if_fail (GRL_IS_WINDOW (self), 0);

    return GetMonitorRefreshRate (monitor);
}

/**
 * grl_window_get_monitor_name:
 * @self: A #GrlWindow.
 * @monitor: Monitor index.
 *
 * Gets the name of the specified monitor.
 *
 * Returns: (transfer none): Monitor name.
 */
const gchar *
grl_window_get_monitor_name (GrlWindow *self,
                             gint       monitor)
{
    g_return_val_if_fail (GRL_IS_WINDOW (self), NULL);

    return GetMonitorName (monitor);
}

/*
 * Public API - Cursor
 */

/**
 * grl_window_show_cursor:
 * @self: A #GrlWindow.
 *
 * Shows the cursor.
 */
void
grl_window_show_cursor (GrlWindow *self)
{
    g_return_if_fail (GRL_IS_WINDOW (self));

    ShowCursor ();
}

/**
 * grl_window_hide_cursor:
 * @self: A #GrlWindow.
 *
 * Hides the cursor.
 */
void
grl_window_hide_cursor (GrlWindow *self)
{
    g_return_if_fail (GRL_IS_WINDOW (self));

    HideCursor ();
}

/**
 * grl_window_is_cursor_hidden:
 * @self: A #GrlWindow.
 *
 * Checks if the cursor is hidden.
 *
 * Returns: %TRUE if the cursor is hidden.
 */
gboolean
grl_window_is_cursor_hidden (GrlWindow *self)
{
    g_return_val_if_fail (GRL_IS_WINDOW (self), FALSE);

    return IsCursorHidden ();
}

/**
 * grl_window_enable_cursor:
 * @self: A #GrlWindow.
 *
 * Enables the cursor (unlocks it).
 */
void
grl_window_enable_cursor (GrlWindow *self)
{
    g_return_if_fail (GRL_IS_WINDOW (self));

    EnableCursor ();
}

/**
 * grl_window_disable_cursor:
 * @self: A #GrlWindow.
 *
 * Disables the cursor (locks it to the window).
 */
void
grl_window_disable_cursor (GrlWindow *self)
{
    g_return_if_fail (GRL_IS_WINDOW (self));

    DisableCursor ();
}

/**
 * grl_window_is_cursor_on_screen:
 * @self: A #GrlWindow.
 *
 * Checks if the cursor is currently on the screen.
 *
 * Returns: %TRUE if the cursor is on screen.
 */
gboolean
grl_window_is_cursor_on_screen (GrlWindow *self)
{
    g_return_val_if_fail (GRL_IS_WINDOW (self), FALSE);

    return IsCursorOnScreen ();
}

/**
 * grl_window_set_cursor:
 * @self: A #GrlWindow.
 * @cursor: The cursor type to set.
 *
 * Sets the mouse cursor shape.
 */
void
grl_window_set_cursor (GrlWindow      *self,
                       GrlMouseCursor  cursor)
{
    g_return_if_fail (GRL_IS_WINDOW (self));

    SetMouseCursor ((int)cursor);
}

/*
 * Public API - Drawing
 */

/**
 * grl_window_begin_drawing:
 * @self: A #GrlWindow.
 *
 * Begins the drawing phase for a frame. Call this before any
 * drawing operations.
 */
void
grl_window_begin_drawing (GrlWindow *self)
{
    GrlWindowPrivate *priv;

    g_return_if_fail (GRL_IS_WINDOW (self));

    priv = grl_window_get_instance_private (self);

    BeginDrawing ();
    priv->frame_count++;
}

/**
 * grl_window_end_drawing:
 * @self: A #GrlWindow.
 *
 * Ends the drawing phase and swaps buffers. Call this after
 * all drawing operations are complete.
 */
void
grl_window_end_drawing (GrlWindow *self)
{
    g_return_if_fail (GRL_IS_WINDOW (self));

    EndDrawing ();
}

/**
 * grl_window_clear_background:
 * @self: A #GrlWindow.
 * @color: Background color.
 *
 * Clears the screen with the specified color.
 */
void
grl_window_clear_background (GrlWindow      *self,
                             const GrlColor *color)
{
    Color raylib_color;

    g_return_if_fail (GRL_IS_WINDOW (self));
    g_return_if_fail (color != NULL);

    raylib_color.r = color->r;
    raylib_color.g = color->g;
    raylib_color.b = color->b;
    raylib_color.a = color->a;

    ClearBackground (raylib_color);
}

/*
 * Public API - Frame timing
 */

/**
 * grl_window_set_target_fps:
 * @self: A #GrlWindow.
 * @fps: Target frames per second.
 *
 * Sets the target frame rate.
 */
void
grl_window_set_target_fps (GrlWindow *self,
                           gint       fps)
{
    GrlWindowPrivate *priv;

    g_return_if_fail (GRL_IS_WINDOW (self));
    g_return_if_fail (fps >= 0);

    priv = grl_window_get_instance_private (self);
    priv->target_fps = fps;

    SetTargetFPS (fps);
    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_TARGET_FPS]);
}

/**
 * grl_window_get_fps:
 * @self: A #GrlWindow.
 *
 * Gets the current frames per second.
 *
 * Returns: Current FPS.
 */
gint
grl_window_get_fps (GrlWindow *self)
{
    g_return_val_if_fail (GRL_IS_WINDOW (self), 0);

    return GetFPS ();
}

/**
 * grl_window_get_frame_time:
 * @self: A #GrlWindow.
 *
 * Gets the time for the last frame in seconds.
 *
 * Returns: Delta time in seconds.
 */
gfloat
grl_window_get_frame_time (GrlWindow *self)
{
    g_return_val_if_fail (GRL_IS_WINDOW (self), 0.0f);

    return GetFrameTime ();
}

/**
 * grl_window_get_time:
 * @self: A #GrlWindow.
 *
 * Gets the elapsed time since window initialization.
 *
 * Returns: Elapsed time in seconds.
 */
gdouble
grl_window_get_time (GrlWindow *self)
{
    g_return_val_if_fail (GRL_IS_WINDOW (self), 0.0);

    return GetTime ();
}

/*
 * Public API - Input polling
 */

/* Array of all keys to poll for press/release events */
static const GrlKey grl_polled_keys[] =
{
    GRL_KEY_SPACE, GRL_KEY_ESCAPE, GRL_KEY_ENTER, GRL_KEY_TAB, GRL_KEY_BACKSPACE,
    GRL_KEY_INSERT, GRL_KEY_DELETE, GRL_KEY_RIGHT, GRL_KEY_LEFT, GRL_KEY_DOWN, GRL_KEY_UP,
    GRL_KEY_PAGE_UP, GRL_KEY_PAGE_DOWN, GRL_KEY_HOME, GRL_KEY_END,
    GRL_KEY_CAPS_LOCK, GRL_KEY_SCROLL_LOCK, GRL_KEY_NUM_LOCK, GRL_KEY_PRINT_SCREEN, GRL_KEY_PAUSE,
    GRL_KEY_F1, GRL_KEY_F2, GRL_KEY_F3, GRL_KEY_F4, GRL_KEY_F5, GRL_KEY_F6,
    GRL_KEY_F7, GRL_KEY_F8, GRL_KEY_F9, GRL_KEY_F10, GRL_KEY_F11, GRL_KEY_F12,
    GRL_KEY_LEFT_SHIFT, GRL_KEY_LEFT_CONTROL, GRL_KEY_LEFT_ALT, GRL_KEY_LEFT_SUPER,
    GRL_KEY_RIGHT_SHIFT, GRL_KEY_RIGHT_CONTROL, GRL_KEY_RIGHT_ALT, GRL_KEY_RIGHT_SUPER,
    GRL_KEY_KB_MENU,
    GRL_KEY_KP_0, GRL_KEY_KP_1, GRL_KEY_KP_2, GRL_KEY_KP_3, GRL_KEY_KP_4,
    GRL_KEY_KP_5, GRL_KEY_KP_6, GRL_KEY_KP_7, GRL_KEY_KP_8, GRL_KEY_KP_9,
    GRL_KEY_KP_DECIMAL, GRL_KEY_KP_DIVIDE, GRL_KEY_KP_MULTIPLY, GRL_KEY_KP_SUBTRACT,
    GRL_KEY_KP_ADD, GRL_KEY_KP_ENTER, GRL_KEY_KP_EQUAL,
    GRL_KEY_APOSTROPHE, GRL_KEY_COMMA, GRL_KEY_MINUS, GRL_KEY_PERIOD, GRL_KEY_SLASH,
    GRL_KEY_ZERO, GRL_KEY_ONE, GRL_KEY_TWO, GRL_KEY_THREE, GRL_KEY_FOUR,
    GRL_KEY_FIVE, GRL_KEY_SIX, GRL_KEY_SEVEN, GRL_KEY_EIGHT, GRL_KEY_NINE,
    GRL_KEY_SEMICOLON, GRL_KEY_EQUAL,
    GRL_KEY_A, GRL_KEY_B, GRL_KEY_C, GRL_KEY_D, GRL_KEY_E, GRL_KEY_F, GRL_KEY_G,
    GRL_KEY_H, GRL_KEY_I, GRL_KEY_J, GRL_KEY_K, GRL_KEY_L, GRL_KEY_M, GRL_KEY_N,
    GRL_KEY_O, GRL_KEY_P, GRL_KEY_Q, GRL_KEY_R, GRL_KEY_S, GRL_KEY_T, GRL_KEY_U,
    GRL_KEY_V, GRL_KEY_W, GRL_KEY_X, GRL_KEY_Y, GRL_KEY_Z,
    GRL_KEY_LEFT_BRACKET, GRL_KEY_BACKSLASH, GRL_KEY_RIGHT_BRACKET, GRL_KEY_GRAVE
};

#define GRL_NUM_POLLED_KEYS (sizeof(grl_polled_keys) / sizeof(grl_polled_keys[0]))

/**
 * grl_window_poll_input:
 * @self: A #GrlWindow.
 *
 * Polls for input events and emits signals for any detected input.
 * Call this once per frame in your custom game loop to get input signals.
 *
 * The grl_window_run() method calls this automatically. You only need
 * to call this directly if you're writing a custom game loop.
 */
void
grl_window_poll_input (GrlWindow *self)
{
    GrlWindowPrivate *priv;
    gint mouse_x, mouse_y;
    gint delta_x, delta_y;
    Vector2 wheel;
    gint gamepad, button, axis;
    gfloat axis_value;
    gsize i;

    g_return_if_fail (GRL_IS_WINDOW (self));

    priv = grl_window_get_instance_private (self);

    /* Initialize mouse position on first poll */
    if (!priv->input_initialized)
    {
        priv->prev_mouse_x = GetMouseX ();
        priv->prev_mouse_y = GetMouseY ();
        priv->input_initialized = TRUE;
    }

    /*
     * Keyboard input
     */
    for (i = 0; i < GRL_NUM_POLLED_KEYS; i++)
    {
        GrlKey key = grl_polled_keys[i];

        if (IsKeyPressed (key))
        {
            g_signal_emit (self, signals[SIGNAL_KEY_PRESSED], 0, key);
        }

        if (IsKeyReleased (key))
        {
            g_signal_emit (self, signals[SIGNAL_KEY_RELEASED], 0, key);
        }
    }

    /*
     * Mouse button input
     */
    mouse_x = GetMouseX ();
    mouse_y = GetMouseY ();

    for (button = GRL_MOUSE_BUTTON_LEFT; button <= GRL_MOUSE_BUTTON_BACK; button++)
    {
        if (IsMouseButtonPressed (button))
        {
            g_signal_emit (self, signals[SIGNAL_MOUSE_BUTTON_PRESSED], 0,
                           (GrlMouseButton)button, mouse_x, mouse_y);
        }

        if (IsMouseButtonReleased (button))
        {
            g_signal_emit (self, signals[SIGNAL_MOUSE_BUTTON_RELEASED], 0,
                           (GrlMouseButton)button, mouse_x, mouse_y);
        }
    }

    /*
     * Mouse movement
     */
    delta_x = mouse_x - priv->prev_mouse_x;
    delta_y = mouse_y - priv->prev_mouse_y;

    if (delta_x != 0 || delta_y != 0)
    {
        g_signal_emit (self, signals[SIGNAL_MOUSE_MOVED], 0,
                       mouse_x, mouse_y, delta_x, delta_y);
    }

    priv->prev_mouse_x = mouse_x;
    priv->prev_mouse_y = mouse_y;

    /*
     * Mouse wheel
     */
    wheel = GetMouseWheelMoveV ();
    if (wheel.x != 0.0f || wheel.y != 0.0f)
    {
        g_signal_emit (self, signals[SIGNAL_MOUSE_WHEEL], 0, wheel.x, wheel.y);
    }

    /*
     * Gamepad input
     */
    for (gamepad = 0; gamepad < GRL_MAX_GAMEPADS; gamepad++)
    {
        if (!IsGamepadAvailable (gamepad))
        {
            continue;
        }

        /* Gamepad buttons */
        for (button = GRL_GAMEPAD_BUTTON_LEFT_FACE_UP;
             button <= GRL_GAMEPAD_BUTTON_RIGHT_THUMB;
             button++)
        {
            if (IsGamepadButtonPressed (gamepad, button))
            {
                g_signal_emit (self, signals[SIGNAL_GAMEPAD_BUTTON_PRESSED], 0,
                               gamepad, (GrlGamepadButton)button);
            }

            if (IsGamepadButtonReleased (gamepad, button))
            {
                g_signal_emit (self, signals[SIGNAL_GAMEPAD_BUTTON_RELEASED], 0,
                               gamepad, (GrlGamepadButton)button);
            }
        }

        /* Gamepad axes */
        for (axis = GRL_GAMEPAD_AXIS_LEFT_X;
             axis <= GRL_GAMEPAD_AXIS_RIGHT_TRIGGER;
             axis++)
        {
            axis_value = GetGamepadAxisMovement (gamepad, axis);

            /* Apply deadzone */
            if (fabsf (axis_value) < GRL_AXIS_DEADZONE)
            {
                axis_value = 0.0f;
            }

            /* Check if axis value changed significantly */
            if (fabsf (axis_value - priv->prev_axis[gamepad][axis]) > 0.01f)
            {
                g_signal_emit (self, signals[SIGNAL_GAMEPAD_AXIS_MOVED], 0,
                               gamepad, (GrlGamepadAxis)axis, axis_value);
                priv->prev_axis[gamepad][axis] = axis_value;
            }
        }
    }
}

/*
 * Public API - Main loop helper
 */

/**
 * grl_window_run:
 * @self: A #GrlWindow.
 *
 * Runs the window's main loop. This calls the update and draw
 * virtual methods/signals each frame until the window should close.
 *
 * This is a convenience method. You can also write your own loop:
 * |[<!-- language="C" -->
 * while (!grl_window_should_close (window))
 * {
 *     // Update game state
 *     grl_window_begin_drawing (window);
 *     grl_window_clear_background (window, bg_color);
 *     // Draw content
 *     grl_window_end_drawing (window);
 * }
 * ]|
 */
void
grl_window_run (GrlWindow *self)
{
    GrlWindowClass *klass;
    gfloat delta_time;
    gint prev_width, prev_height;
    gint new_width, new_height;

    g_return_if_fail (GRL_IS_WINDOW (self));

    klass = GRL_WINDOW_GET_CLASS (self);
    prev_width = GetScreenWidth ();
    prev_height = GetScreenHeight ();

    while (!WindowShouldClose ())
    {
        delta_time = GetFrameTime ();

        /* Check for resize */
        if (IsWindowResized ())
        {
            new_width = GetScreenWidth ();
            new_height = GetScreenHeight ();

            if (new_width != prev_width || new_height != prev_height)
            {
                g_signal_emit (self, signals[SIGNAL_RESIZE], 0,
                               new_width, new_height);
                prev_width = new_width;
                prev_height = new_height;
            }
        }

        /* Poll input and emit signals */
        grl_window_poll_input (self);

        /* Update phase */
        g_signal_emit (self, signals[SIGNAL_UPDATE], 0, delta_time);

        /* Draw phase */
        BeginDrawing ();
        g_signal_emit (self, signals[SIGNAL_DRAW], 0);
        EndDrawing ();
    }

    /* Emit close-request signal */
    g_signal_emit (self, signals[SIGNAL_CLOSE_REQUEST], 0);
}

/*
 * Public API - Clipboard
 */

/**
 * grl_window_set_clipboard_text:
 * @self: A #GrlWindow.
 * @text: Text to copy to clipboard.
 *
 * Copies text to the system clipboard.
 */
void
grl_window_set_clipboard_text (GrlWindow   *self,
                               const gchar *text)
{
    g_return_if_fail (GRL_IS_WINDOW (self));
    g_return_if_fail (text != NULL);

    SetClipboardText (text);
}

/**
 * grl_window_get_clipboard_text:
 * @self: A #GrlWindow.
 *
 * Gets text from the system clipboard.
 *
 * Returns: (transfer full): The clipboard text, or %NULL.
 */
gchar *
grl_window_get_clipboard_text (GrlWindow *self)
{
    const char *text;

    g_return_val_if_fail (GRL_IS_WINDOW (self), NULL);

    text = GetClipboardText ();
    return text ? g_strdup (text) : NULL;
}
