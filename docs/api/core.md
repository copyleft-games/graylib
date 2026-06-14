# Core Types

The core module provides the fundamental types for managing a Graylib application.

## GrlApplication

A singleton that manages the overall application lifecycle.

### Creating an Application

```c
g_autoptr(GrlApplication) app = grl_application_new ("com.example.MyGame");
```

### Accessing the Default Application

```c
GrlApplication *app = grl_application_get_default ();
gboolean exists = grl_application_is_initialized ();
```

### Configuration (Before Running)

```c
/* Set raylib configuration flags */
grl_application_set_config_flags (app,
    GRL_CONFIG_FLAG_VSYNC_HINT |
    GRL_CONFIG_FLAG_WINDOW_RESIZABLE);

/* Set log level */
grl_application_set_log_level (app, GRL_LOG_LEVEL_WARNING);

/* Set target FPS */
grl_application_set_target_fps (app, 60);
```

### Running the Application

```c
/* Runs the application lifecycle */
gint status = grl_application_run (app);

/* Request quit */
grl_application_quit (app);

/* Check if running */
gboolean running = grl_application_is_running (app);
```

### Timing Information

```c
gfloat dt = grl_application_get_frame_time (app);
gint fps = grl_application_get_fps (app);
gdouble time = grl_application_get_time (app);
guint64 frames = grl_application_get_frame_count (app);
```

### Signals

| Signal | Parameters | Description |
|--------|------------|-------------|
| `startup` | None | Emitted during startup, before windows |
| `activate` | None | Create and show main window here |
| `shutdown` | None | Emitted during shutdown, after main loop |

### Subclassing

```c
#define MY_TYPE_APPLICATION (my_application_get_type ())
G_DECLARE_FINAL_TYPE (MyApplication, my_application, MY, APPLICATION, GrlApplication)

struct _MyApplication
{
    GrlApplication parent_instance;
    GrlWindow *window;
};

G_DEFINE_TYPE (MyApplication, my_application, GRL_TYPE_APPLICATION)

static void
my_application_activate (GrlApplication *app)
{
    MyApplication *self = MY_APPLICATION (app);

    self->window = grl_window_new (800, 600, "My Game");
    grl_window_run (self->window);
}

static void
my_application_class_init (MyApplicationClass *klass)
{
    GrlApplicationClass *app_class = GRL_APPLICATION_CLASS (klass);
    app_class->activate = my_application_activate;
}
```

## GrlWindow

Manages a window and its graphics context.

### Creating a Window

```c
g_autoptr(GrlWindow) window = grl_window_new (800, 600, "Window Title");
```

### Window State

```c
gboolean should_close = grl_window_should_close (window);
gboolean ready = grl_window_is_ready (window);
gboolean fullscreen = grl_window_is_fullscreen (window);
gboolean hidden = grl_window_is_hidden (window);
gboolean minimized = grl_window_is_minimized (window);
gboolean maximized = grl_window_is_maximized (window);
gboolean focused = grl_window_is_focused (window);
gboolean resized = grl_window_is_resized (window);
```

### Window Configuration

```c
/* Creation-time config flags -- call BEFORE creating a window (raylib applies
   them in InitWindow).  Used for hints that cannot change after creation, such
   as GRL_FLAG_WINDOW_TRANSPARENT (transparent framebuffer) or
   GRL_FLAG_MSAA_4X_HINT.  Unlike grl_window_set_state(), this is a free
   function (global, like raylib's SetConfigFlags) with no GrlWindow argument. */
grl_window_set_config_flags (GRL_FLAG_WINDOW_TRANSPARENT);
GrlWindow *window = grl_window_new (800, 600, "Title");  /* now transparent */

/* Toggle modes */
grl_window_toggle_fullscreen (window);
grl_window_toggle_borderless (window);
grl_window_maximize (window);
grl_window_minimize (window);
grl_window_restore (window);

/* Set properties */
grl_window_set_title (window, "New Title");
grl_window_set_position (window, 100, 100);
grl_window_set_size (window, 1024, 768);
grl_window_set_min_size (window, 640, 480);
grl_window_set_max_size (window, 1920, 1080);
grl_window_set_opacity (window, 0.9f);
grl_window_set_monitor (window, 0);
grl_window_focus (window);
```

### Window Information

```c
const gchar *title = grl_window_get_title (window);
g_autoptr(GrlVector2) pos = grl_window_get_position (window);
gint width = grl_window_get_width (window);
gint height = grl_window_get_height (window);
gint render_width = grl_window_get_render_width (window);
gint render_height = grl_window_get_render_height (window);
g_autoptr(GrlVector2) scale = grl_window_get_scale_dpi (window);
```

### Monitor Information

```c
gint count = grl_window_get_monitor_count (window);
gint current = grl_window_get_current_monitor (window);
gint mon_width = grl_window_get_monitor_width (window, 0);
gint mon_height = grl_window_get_monitor_height (window, 0);
gint refresh = grl_window_get_monitor_refresh_rate (window, 0);
const gchar *name = grl_window_get_monitor_name (window, 0);
```

### Cursor Control

```c
grl_window_show_cursor (window);
grl_window_hide_cursor (window);
grl_window_enable_cursor (window);
grl_window_disable_cursor (window);
gboolean hidden = grl_window_is_cursor_hidden (window);
gboolean on_screen = grl_window_is_cursor_on_screen (window);
grl_window_set_cursor (window, GRL_MOUSE_CURSOR_CROSSHAIR);
```

### Drawing

```c
/* Basic loop structure */
grl_window_begin_drawing (window);
grl_window_clear_background (window, bg_color);
/* ... draw content ... */
grl_window_end_drawing (window);
```

### Frame Timing

```c
grl_window_set_target_fps (window, 60);
gint fps = grl_window_get_fps (window);
gfloat dt = grl_window_get_frame_time (window);
gdouble time = grl_window_get_time (window);
```

### Running the Window Loop

```c
/* Option 1: Built-in loop (uses signals) */
grl_window_run (window);

/* Option 2: Manual loop */
while (!grl_window_should_close (window))
{
    grl_window_begin_drawing (window);
    grl_window_clear_background (window, bg_color);
    /* Draw content */
    grl_window_end_drawing (window);
}
```

### Clipboard

```c
grl_window_set_clipboard_text (window, "Hello!");
g_autofree gchar *text = grl_window_get_clipboard_text (window);
```

### Signals

| Signal | Parameters | Description |
|--------|------------|-------------|
| `draw` | None | Emitted each frame for drawing |
| `update` | `delta_time` (gfloat) | Emitted each frame for logic |
| `resize` | `width`, `height` (gint) | Emitted when window resizes |
| `close-request` | None | Emitted on close request (return FALSE to cancel) |

### Properties

| Property | Type | Description |
|----------|------|-------------|
| `title` | string | Window title |
| `width` | int | Window width (construct-only) |
| `height` | int | Window height (construct-only) |
| `target-fps` | int | Target frames per second |
| `is-initialized` | boolean | Whether window is ready (read-only) |

### Subclassing

```c
#define MY_TYPE_WINDOW (my_window_get_type ())
G_DECLARE_FINAL_TYPE (MyWindow, my_window, MY, WINDOW, GrlWindow)

struct _MyWindow
{
    GrlWindow parent_instance;
    GrlColor *bg_color;
};

G_DEFINE_TYPE (MyWindow, my_window, GRL_TYPE_WINDOW)

static void
my_window_draw (GrlWindow *window)
{
    MyWindow *self = MY_WINDOW (window);

    grl_window_clear_background (window, self->bg_color);
    /* Draw game content */
}

static void
my_window_update (GrlWindow *window,
                  gfloat     delta_time)
{
    /* Update game state */
}

static void
my_window_class_init (MyWindowClass *klass)
{
    GrlWindowClass *window_class = GRL_WINDOW_CLASS (klass);
    window_class->draw = my_window_draw;
    window_class->update = my_window_update;
}
```
