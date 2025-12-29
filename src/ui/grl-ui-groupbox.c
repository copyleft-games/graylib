/* grl-ui-groupbox.c
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#include "config.h"
#include "grl-ui-groupbox.h"
#include "../math/grl-rectangle.h"

#define RAYGUI_STATIC
#include <raygui.h>

struct _GrlUiGroupBox
{
    GrlUiControl  parent_instance;

    gchar        *text;
};

G_DEFINE_TYPE (GrlUiGroupBox, grl_ui_groupbox, GRL_TYPE_UI_CONTROL)

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
grl_ui_groupbox_draw_impl (GrlUiControl *control)
{
    GrlUiGroupBox *self;
    GrlRectangle  *bounds;
    Rectangle      raygui_bounds;
    gboolean       enabled;
    gboolean       visible;

    self = GRL_UI_GROUPBOX (control);

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

    GuiGroupBox (raygui_bounds, self->text);

    if (!enabled)
        GuiSetState (STATE_NORMAL);
}

static void
grl_ui_groupbox_get_preferred_size_impl (GrlUiControl *control,
                                         gfloat       *width,
                                         gfloat       *height)
{
    (void)control;

    /* Default group box size */
    *width = 200.0f;
    *height = 100.0f;
}

/*
 * GObject implementation
 */

static void
grl_ui_groupbox_finalize (GObject *object)
{
    GrlUiGroupBox *self = GRL_UI_GROUPBOX (object);

    g_clear_pointer (&self->text, g_free);

    G_OBJECT_CLASS (grl_ui_groupbox_parent_class)->finalize (object);
}

static void
grl_ui_groupbox_get_property (GObject    *object,
                              guint       prop_id,
                              GValue     *value,
                              GParamSpec *pspec)
{
    GrlUiGroupBox *self = GRL_UI_GROUPBOX (object);

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
grl_ui_groupbox_set_property (GObject      *object,
                              guint         prop_id,
                              const GValue *value,
                              GParamSpec   *pspec)
{
    GrlUiGroupBox *self = GRL_UI_GROUPBOX (object);

    switch (prop_id)
    {
    case PROP_TEXT:
        grl_ui_groupbox_set_text (self, g_value_get_string (value));
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
grl_ui_groupbox_class_init (GrlUiGroupBoxClass *klass)
{
    GObjectClass      *object_class;
    GrlUiControlClass *control_class;

    object_class = G_OBJECT_CLASS (klass);
    control_class = GRL_UI_CONTROL_CLASS (klass);

    object_class->finalize = grl_ui_groupbox_finalize;
    object_class->get_property = grl_ui_groupbox_get_property;
    object_class->set_property = grl_ui_groupbox_set_property;

    control_class->draw = grl_ui_groupbox_draw_impl;
    control_class->get_preferred_size = grl_ui_groupbox_get_preferred_size_impl;

    /**
     * GrlUiGroupBox:text:
     *
     * The group box label text.
     */
    properties[PROP_TEXT] =
        g_param_spec_string ("text",
                             "Text",
                             "The group box label text",
                             NULL,
                             G_PARAM_READWRITE |
                             G_PARAM_STATIC_STRINGS |
                             G_PARAM_EXPLICIT_NOTIFY);

    g_object_class_install_properties (object_class, N_PROPS, properties);
}

static void
grl_ui_groupbox_init (GrlUiGroupBox *self)
{
    self->text = NULL;
}

/*
 * Public API
 */

/**
 * grl_ui_groupbox_new:
 * @text: (nullable): Label text for the group box
 *
 * Creates a new group box control.
 *
 * Returns: (transfer full): A new #GrlUiGroupBox
 */
GrlUiGroupBox *
grl_ui_groupbox_new (const gchar *text)
{
    return g_object_new (GRL_TYPE_UI_GROUPBOX,
                         "text", text,
                         NULL);
}

/**
 * grl_ui_groupbox_new_with_bounds:
 * @x: X position
 * @y: Y position
 * @width: Width of the group box
 * @height: Height of the group box
 * @text: (nullable): Label text for the group box
 *
 * Creates a new group box control with specified bounds.
 *
 * Returns: (transfer full): A new #GrlUiGroupBox
 */
GrlUiGroupBox *
grl_ui_groupbox_new_with_bounds (gfloat       x,
                                 gfloat       y,
                                 gfloat       width,
                                 gfloat       height,
                                 const gchar *text)
{
    g_autoptr(GrlRectangle) bounds = NULL;

    bounds = grl_rectangle_new (x, y, width, height);

    return g_object_new (GRL_TYPE_UI_GROUPBOX,
                         "bounds", bounds,
                         "text", text,
                         NULL);
}

/**
 * grl_ui_groupbox_get_text:
 * @self: A #GrlUiGroupBox
 *
 * Gets the group box label text.
 *
 * Returns: (transfer none) (nullable): The label text
 */
const gchar *
grl_ui_groupbox_get_text (GrlUiGroupBox *self)
{
    g_return_val_if_fail (GRL_IS_UI_GROUPBOX (self), NULL);

    return self->text;
}

/**
 * grl_ui_groupbox_set_text:
 * @self: A #GrlUiGroupBox
 * @text: (nullable): The label text to display
 *
 * Sets the group box label text.
 */
void
grl_ui_groupbox_set_text (GrlUiGroupBox *self,
                          const gchar   *text)
{
    g_return_if_fail (GRL_IS_UI_GROUPBOX (self));

    if (g_strcmp0 (self->text, text) == 0)
        return;

    g_free (self->text);
    self->text = g_strdup (text);

    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_TEXT]);
}

/**
 * grl_ui_groupbox_get_content_bounds:
 * @self: A #GrlUiGroupBox
 *
 * Gets the bounds of the content area (inside the group box).
 * This is useful for positioning child controls.
 *
 * Returns: (transfer full): The content area bounds
 */
GrlRectangle *
grl_ui_groupbox_get_content_bounds (GrlUiGroupBox *self)
{
    GrlRectangle *bounds;
    gfloat        label_height;
    gfloat        border_width;

    g_return_val_if_fail (GRL_IS_UI_GROUPBOX (self), NULL);

    bounds = grl_ui_control_get_bounds (GRL_UI_CONTROL (self));
    if (bounds == NULL)
        return NULL;

    /* Account for border and label */
    label_height = 12.0f;  /* Height of label area */
    border_width = 2.0f;   /* Border padding */

    bounds->x += border_width;
    bounds->y += label_height + border_width;
    bounds->width -= 2.0f * border_width;
    bounds->height -= label_height + 2.0f * border_width;

    return bounds;
}
