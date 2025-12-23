/* grl-window.h
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Window management wrapper for raylib.
 */

#pragma once

#if !defined(GRAYLIB_INSIDE) && !defined(GRAYLIB_COMPILATION)
#error "Only <graylib.h> can be included directly."
#endif

#include <glib-object.h>
#include "grl-version.h"
#include "grl-enums.h"
#include "../math/grl-vector2.h"
#include "../math/grl-color.h"

G_BEGIN_DECLS

#define GRL_TYPE_WINDOW (grl_window_get_type ())

GRL_AVAILABLE_IN_ALL
G_DECLARE_DERIVABLE_TYPE (GrlWindow, grl_window, GRL, WINDOW, GObject)

/**
 * GrlWindowClass:
 * @parent_class: The parent class
 * @draw: Called each frame during the drawing phase.
 *        Subclasses can override to perform custom drawing.
 * @update: Called each frame during the update phase.
 *          Subclasses can override to update game state.
 * @on_resize: Called when the window is resized.
 * @on_close: Called when the window is about to close.
 *            Return %FALSE to cancel the close.
 * @on_key_pressed: Called when a key is pressed.
 * @on_key_released: Called when a key is released.
 * @on_mouse_button_pressed: Called when a mouse button is pressed.
 * @on_mouse_button_released: Called when a mouse button is released.
 * @on_mouse_moved: Called when the mouse moves.
 * @on_mouse_wheel: Called when the mouse wheel is scrolled.
 * @on_gamepad_button_pressed: Called when a gamepad button is pressed.
 * @on_gamepad_button_released: Called when a gamepad button is released.
 * @on_gamepad_axis_moved: Called when a gamepad axis moves.
 *
 * Class structure for #GrlWindow.
 */
struct _GrlWindowClass
{
    GObjectClass parent_class;

    /*< public >*/

    /* Virtual methods */
    void     (*draw)      (GrlWindow *self);
    void     (*update)    (GrlWindow *self,
                           gfloat     delta_time);
    void     (*on_resize) (GrlWindow *self,
                           gint       width,
                           gint       height);
    gboolean (*on_close)  (GrlWindow *self);

    /* Input virtual methods */
    void     (*on_key_pressed)             (GrlWindow       *self,
                                            GrlKey           key);
    void     (*on_key_released)            (GrlWindow       *self,
                                            GrlKey           key);
    void     (*on_mouse_button_pressed)    (GrlWindow       *self,
                                            GrlMouseButton   button,
                                            gint             x,
                                            gint             y);
    void     (*on_mouse_button_released)   (GrlWindow       *self,
                                            GrlMouseButton   button,
                                            gint             x,
                                            gint             y);
    void     (*on_mouse_moved)             (GrlWindow       *self,
                                            gint             x,
                                            gint             y,
                                            gint             delta_x,
                                            gint             delta_y);
    void     (*on_mouse_wheel)             (GrlWindow       *self,
                                            gfloat           x,
                                            gfloat           y);
    void     (*on_gamepad_button_pressed)  (GrlWindow       *self,
                                            gint             gamepad,
                                            GrlGamepadButton button);
    void     (*on_gamepad_button_released) (GrlWindow       *self,
                                            gint             gamepad,
                                            GrlGamepadButton button);
    void     (*on_gamepad_axis_moved)      (GrlWindow       *self,
                                            gint             gamepad,
                                            GrlGamepadAxis   axis,
                                            gfloat           value);

    /*< private >*/
    gpointer _reserved[8];
};

/*
 * Constructors
 */

GRL_AVAILABLE_IN_ALL
GrlWindow *         grl_window_new              (gint                width,
                                                 gint                height,
                                                 const gchar        *title);

/*
 * Window state
 */

GRL_AVAILABLE_IN_ALL
gboolean            grl_window_should_close     (GrlWindow          *self);

GRL_AVAILABLE_IN_ALL
void                grl_window_set_should_close (GrlWindow          *self,
                                                 gboolean            should_close);

GRL_AVAILABLE_IN_ALL
gboolean            grl_window_is_ready         (GrlWindow          *self);

GRL_AVAILABLE_IN_ALL
gboolean            grl_window_is_fullscreen    (GrlWindow          *self);

GRL_AVAILABLE_IN_ALL
gboolean            grl_window_is_hidden        (GrlWindow          *self);

GRL_AVAILABLE_IN_ALL
gboolean            grl_window_is_minimized     (GrlWindow          *self);

GRL_AVAILABLE_IN_ALL
gboolean            grl_window_is_maximized     (GrlWindow          *self);

GRL_AVAILABLE_IN_ALL
gboolean            grl_window_is_focused       (GrlWindow          *self);

GRL_AVAILABLE_IN_ALL
gboolean            grl_window_is_resized       (GrlWindow          *self);

GRL_AVAILABLE_IN_ALL
gboolean            grl_window_has_state        (GrlWindow          *self,
                                                 GrlConfigFlags      flag);

/*
 * Window configuration
 */

GRL_AVAILABLE_IN_ALL
void                grl_window_set_state        (GrlWindow          *self,
                                                 GrlConfigFlags      flags);

GRL_AVAILABLE_IN_ALL
void                grl_window_clear_state      (GrlWindow          *self,
                                                 GrlConfigFlags      flags);

GRL_AVAILABLE_IN_ALL
void                grl_window_toggle_fullscreen (GrlWindow         *self);

GRL_AVAILABLE_IN_ALL
void                grl_window_toggle_borderless (GrlWindow         *self);

GRL_AVAILABLE_IN_ALL
void                grl_window_maximize         (GrlWindow          *self);

GRL_AVAILABLE_IN_ALL
void                grl_window_minimize         (GrlWindow          *self);

GRL_AVAILABLE_IN_ALL
void                grl_window_restore          (GrlWindow          *self);

GRL_AVAILABLE_IN_ALL
void                grl_window_set_title        (GrlWindow          *self,
                                                 const gchar        *title);

GRL_AVAILABLE_IN_ALL
const gchar *       grl_window_get_title        (GrlWindow          *self);

GRL_AVAILABLE_IN_ALL
void                grl_window_set_position     (GrlWindow          *self,
                                                 gint                x,
                                                 gint                y);

GRL_AVAILABLE_IN_ALL
GrlVector2 *        grl_window_get_position     (GrlWindow          *self);

GRL_AVAILABLE_IN_ALL
void                grl_window_set_size         (GrlWindow          *self,
                                                 gint                width,
                                                 gint                height);

GRL_AVAILABLE_IN_ALL
gint                grl_window_get_width        (GrlWindow          *self);

GRL_AVAILABLE_IN_ALL
gint                grl_window_get_height       (GrlWindow          *self);

GRL_AVAILABLE_IN_ALL
void                grl_window_set_min_size     (GrlWindow          *self,
                                                 gint                width,
                                                 gint                height);

GRL_AVAILABLE_IN_ALL
void                grl_window_set_max_size     (GrlWindow          *self,
                                                 gint                width,
                                                 gint                height);

GRL_AVAILABLE_IN_ALL
void                grl_window_set_opacity      (GrlWindow          *self,
                                                 gfloat              opacity);

GRL_AVAILABLE_IN_ALL
void                grl_window_set_monitor      (GrlWindow          *self,
                                                 gint                monitor);

GRL_AVAILABLE_IN_ALL
void                grl_window_focus            (GrlWindow          *self);

/*
 * Rendering dimensions
 */

GRL_AVAILABLE_IN_ALL
gint                grl_window_get_render_width  (GrlWindow         *self);

GRL_AVAILABLE_IN_ALL
gint                grl_window_get_render_height (GrlWindow         *self);

GRL_AVAILABLE_IN_ALL
GrlVector2 *        grl_window_get_scale_dpi    (GrlWindow          *self);

/*
 * Monitor info
 */

GRL_AVAILABLE_IN_ALL
gint                grl_window_get_monitor_count (GrlWindow         *self);

GRL_AVAILABLE_IN_ALL
gint                grl_window_get_current_monitor (GrlWindow       *self);

GRL_AVAILABLE_IN_ALL
gint                grl_window_get_monitor_width (GrlWindow         *self,
                                                  gint               monitor);

GRL_AVAILABLE_IN_ALL
gint                grl_window_get_monitor_height (GrlWindow        *self,
                                                   gint              monitor);

GRL_AVAILABLE_IN_ALL
gint                grl_window_get_monitor_refresh_rate (GrlWindow  *self,
                                                         gint        monitor);

GRL_AVAILABLE_IN_ALL
const gchar *       grl_window_get_monitor_name (GrlWindow          *self,
                                                 gint                monitor);

/*
 * Cursor
 */

GRL_AVAILABLE_IN_ALL
void                grl_window_show_cursor      (GrlWindow          *self);

GRL_AVAILABLE_IN_ALL
void                grl_window_hide_cursor      (GrlWindow          *self);

GRL_AVAILABLE_IN_ALL
gboolean            grl_window_is_cursor_hidden (GrlWindow          *self);

GRL_AVAILABLE_IN_ALL
void                grl_window_enable_cursor    (GrlWindow          *self);

GRL_AVAILABLE_IN_ALL
void                grl_window_disable_cursor   (GrlWindow          *self);

GRL_AVAILABLE_IN_ALL
gboolean            grl_window_is_cursor_on_screen (GrlWindow       *self);

GRL_AVAILABLE_IN_ALL
void                grl_window_set_cursor       (GrlWindow          *self,
                                                 GrlMouseCursor      cursor);

/*
 * Drawing
 */

GRL_AVAILABLE_IN_ALL
void                grl_window_begin_drawing    (GrlWindow          *self);

GRL_AVAILABLE_IN_ALL
void                grl_window_end_drawing      (GrlWindow          *self);

GRL_AVAILABLE_IN_ALL
void                grl_window_clear_background (GrlWindow          *self,
                                                 const GrlColor     *color);

/*
 * Frame timing
 */

GRL_AVAILABLE_IN_ALL
void                grl_window_set_target_fps   (GrlWindow          *self,
                                                 gint                fps);

GRL_AVAILABLE_IN_ALL
gint                grl_window_get_fps          (GrlWindow          *self);

GRL_AVAILABLE_IN_ALL
gfloat              grl_window_get_frame_time   (GrlWindow          *self);

GRL_AVAILABLE_IN_ALL
gdouble             grl_window_get_time         (GrlWindow          *self);

/*
 * Main loop helper
 */

GRL_AVAILABLE_IN_ALL
void                grl_window_run              (GrlWindow          *self);

/**
 * grl_window_poll_input:
 * @self: A #GrlWindow.
 *
 * Polls for input events and emits signals for any detected input.
 * Call this once per frame in your custom game loop to get input signals.
 *
 * The #GrlWindow::run method calls this automatically. You only need
 * to call this directly if you're writing a custom game loop.
 *
 * Input signals that may be emitted:
 * - #GrlWindow::key-pressed
 * - #GrlWindow::key-released
 * - #GrlWindow::mouse-button-pressed
 * - #GrlWindow::mouse-button-released
 * - #GrlWindow::mouse-moved
 * - #GrlWindow::mouse-wheel
 * - #GrlWindow::gamepad-button-pressed
 * - #GrlWindow::gamepad-button-released
 * - #GrlWindow::gamepad-axis-moved
 */
GRL_AVAILABLE_IN_ALL
void                grl_window_poll_input       (GrlWindow          *self);

/*
 * Clipboard
 */

GRL_AVAILABLE_IN_ALL
void                grl_window_set_clipboard_text (GrlWindow        *self,
                                                   const gchar      *text);

GRL_AVAILABLE_IN_ALL
gchar *             grl_window_get_clipboard_text (GrlWindow        *self);

G_END_DECLS
