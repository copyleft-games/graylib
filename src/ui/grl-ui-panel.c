/* grl-ui-panel.c
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#include "config.h"
#include "grl-ui-panel.h"
#include "../math/grl-rectangle.h"

#define RAYGUI_STATIC
#include <raygui.h>

struct _GrlUiPanel
{
    GrlUiControl  parent_instance;

    gchar        *text;
};

G_DEFINE_TYPE (GrlUiPanel, grl_ui_panel, GRL_TYPE_UI_CONTROL)

enum
{
    PROP_0,
    PROP_TEXT,
    N_PROPS
};

static GParamSpec *properties[N_PROPS];

/*
 * Virtual method implementations
 */

static void
grl_ui_panel_draw_impl (GrlUiControl *control)
{
    GrlUiPanel   *self;
    GrlRectangle *bounds;
    Rectangle     raygui_bounds;
    gboolean      enabled;
    gboolean      visible;

    self = GRL_UI_PANEL (control);

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

    GuiPanel (raygui_bounds, self->text);

    if (!enabled)
        GuiSetState (STATE_NORMAL);
}

static void
grl_ui_panel_get_preferred_size_impl (GrlUiControl *control,
                                      gfloat       *width,
                                      gfloat       *height)
{
    (void)control;

    /* Default panel size */
    *width = 200.0f;
    *height = 150.0f;
}

/*
 * GObject implementation
 */

static void
grl_ui_panel_finalize (GObject *object)
{
    GrlUiPanel *self = GRL_UI_PANEL (object);

    g_clear_pointer (&self->text, g_free);

    G_OBJECT_CLASS (grl_ui_panel_parent_class)->finalize (object);
}

static void
grl_ui_panel_get_property (GObject    *object,
                           guint       prop_id,
                           GValue     *value,
                           GParamSpec *pspec)
{
    GrlUiPanel *self = GRL_UI_PANEL (object);

    switch (prop_id)
    {
    case PROP_TEXT:
        g_value_set_string (value, self->text);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
grl_ui_panel_set_property (GObject      *object,
                           guint         prop_id,
                           const GValue *value,
                           GParamSpec   *pspec)
{
    GrlUiPanel *self = GRL_UI_PANEL (object);

    switch (prop_id)
    {
    case PROP_TEXT:
        grl_ui_panel_set_text (self, g_value_get_string (value));
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
grl_ui_panel_class_init (GrlUiPanelClass *klass)
{
    GObjectClass      *object_class;
    GrlUiControlClass *control_class;

    object_class = G_OBJECT_CLASS (klass);
    control_class = GRL_UI_CONTROL_CLASS (klass);

    object_class->finalize = grl_ui_panel_finalize;
    object_class->get_property = grl_ui_panel_get_property;
    object_class->set_property = grl_ui_panel_set_property;

    control_class->draw = grl_ui_panel_draw_impl;
    control_class->get_preferred_size = grl_ui_panel_get_preferred_size_impl;

    /**
     * GrlUiPanel:text:
     *
     * The panel header text.
     */
    properties[PROP_TEXT] =
        g_param_spec_string ("text",
                             "Text",
                             "The panel header text",
                             NULL,
                             G_PARAM_READWRITE |
                             G_PARAM_STATIC_STRINGS |
                             G_PARAM_EXPLICIT_NOTIFY);

    g_object_class_install_properties (object_class, N_PROPS, properties);
}

static void
grl_ui_panel_init (GrlUiPanel *self)
{
    self->text = NULL;
}

/*
 * Public API
 */

/**
 * grl_ui_panel_new:
 * @text: (nullable): Optional header text
 *
 * Creates a new panel control.
 *
 * Returns: (transfer full): A new #GrlUiPanel
 */
GrlUiPanel *
grl_ui_panel_new (const gchar *text)
{
    return g_object_new (GRL_TYPE_UI_PANEL,
                         "text", text,
                         NULL);
}

/**
 * grl_ui_panel_new_with_bounds:
 * @x: X position
 * @y: Y position
 * @width: Width of the panel
 * @height: Height of the panel
 * @text: (nullable): Optional header text
 *
 * Creates a new panel control with specified bounds.
 *
 * Returns: (transfer full): A new #GrlUiPanel
 */
GrlUiPanel *
grl_ui_panel_new_with_bounds (gfloat       x,
                              gfloat       y,
                              gfloat       width,
                              gfloat       height,
                              const gchar *text)
{
    g_autoptr(GrlRectangle) bounds = NULL;

    bounds = grl_rectangle_new (x, y, width, height);

    return g_object_new (GRL_TYPE_UI_PANEL,
                         "bounds", bounds,
                         "text", text,
                         NULL);
}

/**
 * grl_ui_panel_get_text:
 * @self: A #GrlUiPanel
 *
 * Gets the panel header text.
 *
 * Returns: (transfer none) (nullable): The header text
 */
const gchar *
grl_ui_panel_get_text (GrlUiPanel *self)
{
    g_return_val_if_fail (GRL_IS_UI_PANEL (self), NULL);

    return self->text;
}

/**
 * grl_ui_panel_set_text:
 * @self: A #GrlUiPanel
 * @text: (nullable): The header text to display
 *
 * Sets the panel header text.
 */
void
grl_ui_panel_set_text (GrlUiPanel  *self,
                       const gchar *text)
{
    g_return_if_fail (GRL_IS_UI_PANEL (self));

    if (g_strcmp0 (self->text, text) == 0)
        return;

    g_free (self->text);
    self->text = g_strdup (text);

    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_TEXT]);
}

/**
 * grl_ui_panel_get_content_bounds:
 * @self: A #GrlUiPanel
 *
 * Gets the bounds of the content area (inside the panel).
 * This is useful for positioning child controls.
 *
 * Returns: (transfer full): The content area bounds
 */
GrlRectangle *
grl_ui_panel_get_content_bounds (GrlUiPanel *self)
{
    GrlRectangle *bounds;
    gfloat        header_height;
    gfloat        padding;

    g_return_val_if_fail (GRL_IS_UI_PANEL (self), NULL);

    bounds = grl_ui_control_get_bounds (GRL_UI_CONTROL (self));
    if (bounds == NULL)
        return NULL;

    /* Account for header if text is present */
    header_height = (self->text != NULL && self->text[0] != '\0') ? 24.0f : 0.0f;
    padding = 4.0f;

    bounds->x += padding;
    bounds->y += header_height + padding;
    bounds->width -= 2.0f * padding;
    bounds->height -= header_height + 2.0f * padding;

    return bounds;
}
