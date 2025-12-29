/* grl-ui-windowbox.c
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#include "config.h"
#include "grl-ui-windowbox.h"
#include "../math/grl-rectangle.h"

#define RAYGUI_STATIC
#include <raygui.h>

struct _GrlUiWindowBox
{
    GrlUiControl  parent_instance;

    gchar        *title;
    gboolean      open;
};

G_DEFINE_TYPE (GrlUiWindowBox, grl_ui_windowbox, GRL_TYPE_UI_CONTROL)

enum
{
    PROP_0,
    PROP_TITLE,
    PROP_OPEN,
    N_PROPS
};

static GParamSpec *properties[N_PROPS];

enum
{
    SIGNAL_CLOSED,
    SIGNAL_OPENED,
    N_SIGNALS
};

static guint signals[N_SIGNALS];

/*
 * Virtual method implementations
 */

static void
grl_ui_windowbox_draw_impl (GrlUiControl *control)
{
    GrlUiWindowBox *self;
    GrlRectangle   *bounds;
    Rectangle       raygui_bounds;
    gboolean        enabled;
    gboolean        visible;
    int             close_clicked;

    self = GRL_UI_WINDOWBOX (control);

    /* Don't draw if closed */
    if (!self->open)
        return;

    g_object_get (G_OBJECT (control),
                  "visible", &visible,
                  "enabled", &enabled,
                  NULL);

    if (!visible)
        return;

    bounds = grl_ui_control_get_bounds (control);
    if (bounds == NULL)
        return;

    raygui_bounds.x = bounds->x;
    raygui_bounds.y = bounds->y;
    raygui_bounds.width = bounds->width;
    raygui_bounds.height = bounds->height;

    grl_rectangle_free (bounds);

    if (!enabled)
        GuiSetState (STATE_DISABLED);

    close_clicked = GuiWindowBox (raygui_bounds, self->title);

    if (!enabled)
        GuiSetState (STATE_NORMAL);

    /* Check if close button was clicked */
    if (close_clicked)
    {
        self->open = FALSE;
        g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_OPEN]);
        g_signal_emit (self, signals[SIGNAL_CLOSED], 0);
    }
}

static void
grl_ui_windowbox_get_preferred_size_impl (GrlUiControl *control,
                                          gfloat       *width,
                                          gfloat       *height)
{
    (void)control;

    /* Default window box size */
    *width = 300.0f;
    *height = 200.0f;
}

/*
 * GObject implementation
 */

static void
grl_ui_windowbox_finalize (GObject *object)
{
    GrlUiWindowBox *self = GRL_UI_WINDOWBOX (object);

    g_clear_pointer (&self->title, g_free);

    G_OBJECT_CLASS (grl_ui_windowbox_parent_class)->finalize (object);
}

static void
grl_ui_windowbox_get_property (GObject    *object,
                               guint       prop_id,
                               GValue     *value,
                               GParamSpec *pspec)
{
    GrlUiWindowBox *self = GRL_UI_WINDOWBOX (object);

    switch (prop_id)
    {
    case PROP_TITLE:
        g_value_set_string (value, self->title);
        break;
    case PROP_OPEN:
        g_value_set_boolean (value, self->open);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
grl_ui_windowbox_set_property (GObject      *object,
                               guint         prop_id,
                               const GValue *value,
                               GParamSpec   *pspec)
{
    GrlUiWindowBox *self = GRL_UI_WINDOWBOX (object);

    switch (prop_id)
    {
    case PROP_TITLE:
        grl_ui_windowbox_set_title (self, g_value_get_string (value));
        break;
    case PROP_OPEN:
        grl_ui_windowbox_set_open (self, g_value_get_boolean (value));
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
grl_ui_windowbox_class_init (GrlUiWindowBoxClass *klass)
{
    GObjectClass      *object_class;
    GrlUiControlClass *control_class;

    object_class = G_OBJECT_CLASS (klass);
    control_class = GRL_UI_CONTROL_CLASS (klass);

    object_class->finalize = grl_ui_windowbox_finalize;
    object_class->get_property = grl_ui_windowbox_get_property;
    object_class->set_property = grl_ui_windowbox_set_property;

    control_class->draw = grl_ui_windowbox_draw_impl;
    control_class->get_preferred_size = grl_ui_windowbox_get_preferred_size_impl;

    /**
     * GrlUiWindowBox:title:
     *
     * The window title.
     */
    properties[PROP_TITLE] =
        g_param_spec_string ("title",
                             "Title",
                             "The window title",
                             NULL,
                             G_PARAM_READWRITE |
                             G_PARAM_STATIC_STRINGS |
                             G_PARAM_EXPLICIT_NOTIFY);

    /**
     * GrlUiWindowBox:open:
     *
     * Whether the window is open (visible).
     */
    properties[PROP_OPEN] =
        g_param_spec_boolean ("open",
                              "Open",
                              "Whether the window is open",
                              TRUE,
                              G_PARAM_READWRITE |
                              G_PARAM_STATIC_STRINGS |
                              G_PARAM_EXPLICIT_NOTIFY);

    g_object_class_install_properties (object_class, N_PROPS, properties);

    /**
     * GrlUiWindowBox::closed:
     * @self: The window box
     *
     * Emitted when the window is closed (close button clicked).
     */
    signals[SIGNAL_CLOSED] =
        g_signal_new ("closed",
                      G_TYPE_FROM_CLASS (klass),
                      G_SIGNAL_RUN_LAST,
                      0,
                      NULL, NULL,
                      NULL,
                      G_TYPE_NONE,
                      0);

    /**
     * GrlUiWindowBox::opened:
     * @self: The window box
     *
     * Emitted when the window is opened.
     */
    signals[SIGNAL_OPENED] =
        g_signal_new ("opened",
                      G_TYPE_FROM_CLASS (klass),
                      G_SIGNAL_RUN_LAST,
                      0,
                      NULL, NULL,
                      NULL,
                      G_TYPE_NONE,
                      0);
}

static void
grl_ui_windowbox_init (GrlUiWindowBox *self)
{
    self->title = NULL;
    self->open = TRUE;
}

/*
 * Public API
 */

/**
 * grl_ui_windowbox_new:
 * @title: (nullable): Window title
 *
 * Creates a new window box control.
 *
 * Returns: (transfer full): A new #GrlUiWindowBox
 */
GrlUiWindowBox *
grl_ui_windowbox_new (const gchar *title)
{
    return g_object_new (GRL_TYPE_UI_WINDOWBOX,
                         "title", title,
                         NULL);
}

/**
 * grl_ui_windowbox_new_with_bounds:
 * @x: X position
 * @y: Y position
 * @width: Width of the window box
 * @height: Height of the window box
 * @title: (nullable): Window title
 *
 * Creates a new window box control with specified bounds.
 *
 * Returns: (transfer full): A new #GrlUiWindowBox
 */
GrlUiWindowBox *
grl_ui_windowbox_new_with_bounds (gfloat       x,
                                  gfloat       y,
                                  gfloat       width,
                                  gfloat       height,
                                  const gchar *title)
{
    g_autoptr(GrlRectangle) bounds = NULL;

    bounds = grl_rectangle_new (x, y, width, height);

    return g_object_new (GRL_TYPE_UI_WINDOWBOX,
                         "bounds", bounds,
                         "title", title,
                         NULL);
}

/**
 * grl_ui_windowbox_get_title:
 * @self: A #GrlUiWindowBox
 *
 * Gets the window title.
 *
 * Returns: (transfer none) (nullable): The window title
 */
const gchar *
grl_ui_windowbox_get_title (GrlUiWindowBox *self)
{
    g_return_val_if_fail (GRL_IS_UI_WINDOWBOX (self), NULL);

    return self->title;
}

/**
 * grl_ui_windowbox_set_title:
 * @self: A #GrlUiWindowBox
 * @title: (nullable): The window title
 *
 * Sets the window title.
 */
void
grl_ui_windowbox_set_title (GrlUiWindowBox *self,
                            const gchar    *title)
{
    g_return_if_fail (GRL_IS_UI_WINDOWBOX (self));

    if (g_strcmp0 (self->title, title) == 0)
        return;

    g_free (self->title);
    self->title = g_strdup (title);

    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_TITLE]);
}

/**
 * grl_ui_windowbox_get_open:
 * @self: A #GrlUiWindowBox
 *
 * Gets whether the window is open (visible).
 *
 * Returns: %TRUE if the window is open
 */
gboolean
grl_ui_windowbox_get_open (GrlUiWindowBox *self)
{
    g_return_val_if_fail (GRL_IS_UI_WINDOWBOX (self), FALSE);

    return self->open;
}

/**
 * grl_ui_windowbox_set_open:
 * @self: A #GrlUiWindowBox
 * @open: %TRUE to open the window, %FALSE to close it
 *
 * Sets whether the window is open.
 */
void
grl_ui_windowbox_set_open (GrlUiWindowBox *self,
                           gboolean        open)
{
    g_return_if_fail (GRL_IS_UI_WINDOWBOX (self));

    open = !!open;  /* Normalize to TRUE/FALSE */

    if (self->open == open)
        return;

    self->open = open;

    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_OPEN]);

    if (open)
        g_signal_emit (self, signals[SIGNAL_OPENED], 0);
    else
        g_signal_emit (self, signals[SIGNAL_CLOSED], 0);
}

/**
 * grl_ui_windowbox_close:
 * @self: A #GrlUiWindowBox
 *
 * Closes the window box.
 */
void
grl_ui_windowbox_close (GrlUiWindowBox *self)
{
    g_return_if_fail (GRL_IS_UI_WINDOWBOX (self));

    grl_ui_windowbox_set_open (self, FALSE);
}

/**
 * grl_ui_windowbox_open:
 * @self: A #GrlUiWindowBox
 *
 * Opens the window box.
 */
void
grl_ui_windowbox_open (GrlUiWindowBox *self)
{
    g_return_if_fail (GRL_IS_UI_WINDOWBOX (self));

    grl_ui_windowbox_set_open (self, TRUE);
}

/**
 * grl_ui_windowbox_toggle:
 * @self: A #GrlUiWindowBox
 *
 * Toggles the window box open/closed state.
 */
void
grl_ui_windowbox_toggle (GrlUiWindowBox *self)
{
    g_return_if_fail (GRL_IS_UI_WINDOWBOX (self));

    grl_ui_windowbox_set_open (self, !self->open);
}

/**
 * grl_ui_windowbox_get_content_bounds:
 * @self: A #GrlUiWindowBox
 *
 * Gets the bounds of the content area (inside the window).
 * This is useful for positioning child controls.
 *
 * Returns: (transfer full): The content area bounds
 */
GrlRectangle *
grl_ui_windowbox_get_content_bounds (GrlUiWindowBox *self)
{
    GrlRectangle *bounds;
    gfloat        title_bar_height;
    gfloat        padding;

    g_return_val_if_fail (GRL_IS_UI_WINDOWBOX (self), NULL);

    bounds = grl_ui_control_get_bounds (GRL_UI_CONTROL (self));
    if (bounds == NULL)
        return NULL;

    /* Account for title bar and padding */
    title_bar_height = 24.0f;  /* Standard title bar height */
    padding = 4.0f;

    bounds->x += padding;
    bounds->y += title_bar_height + padding;
    bounds->width -= 2.0f * padding;
    bounds->height -= title_bar_height + 2.0f * padding;

    return bounds;
}
