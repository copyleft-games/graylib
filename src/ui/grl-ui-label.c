/* grl-ui-label.c
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#include "config.h"
#include "grl-ui-label.h"

#define RAYGUI_IMPLEMENTATION
#include <raygui.h>

struct _GrlUiLabel
{
    GrlUiControl        parent_instance;

    gchar              *text;
    GrlUiTextAlignment  text_alignment;
};

G_DEFINE_TYPE (GrlUiLabel, grl_ui_label, GRL_TYPE_UI_CONTROL)

enum
{
    PROP_0,
    PROP_TEXT,
    PROP_TEXT_ALIGNMENT,
    N_PROPS
};

static GParamSpec *properties[N_PROPS];

static void
grl_ui_label_finalize (GObject *object)
{
    GrlUiLabel *self = GRL_UI_LABEL (object);

    g_clear_pointer (&self->text, g_free);

    G_OBJECT_CLASS (grl_ui_label_parent_class)->finalize (object);
}

static void
grl_ui_label_get_property (GObject    *object,
                           guint       prop_id,
                           GValue     *value,
                           GParamSpec *pspec)
{
    GrlUiLabel *self = GRL_UI_LABEL (object);

    switch (prop_id)
    {
    case PROP_TEXT:
        g_value_set_string (value, self->text);
        break;
    case PROP_TEXT_ALIGNMENT:
        g_value_set_enum (value, self->text_alignment);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
grl_ui_label_set_property (GObject      *object,
                           guint         prop_id,
                           const GValue *value,
                           GParamSpec   *pspec)
{
    GrlUiLabel *self = GRL_UI_LABEL (object);

    switch (prop_id)
    {
    case PROP_TEXT:
        grl_ui_label_set_text (self, g_value_get_string (value));
        break;
    case PROP_TEXT_ALIGNMENT:
        grl_ui_label_set_text_alignment (self, g_value_get_enum (value));
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
grl_ui_label_draw (GrlUiControl *control)
{
    GrlUiLabel *self = GRL_UI_LABEL (control);
    g_autoptr(GrlRectangle) bounds = NULL;
    Rectangle rl_bounds;
    int old_alignment;

    bounds = grl_ui_control_get_bounds (control);

    rl_bounds.x = bounds->x;
    rl_bounds.y = bounds->y;
    rl_bounds.width = bounds->width;
    rl_bounds.height = bounds->height;

    /* Save old alignment, set our alignment, draw, restore */
    old_alignment = GuiGetStyle (LABEL, TEXT_ALIGNMENT);
    GuiSetStyle (LABEL, TEXT_ALIGNMENT, (int)self->text_alignment);

    GuiLabel (rl_bounds, self->text);

    GuiSetStyle (LABEL, TEXT_ALIGNMENT, old_alignment);
}

static void
grl_ui_label_get_preferred_size (GrlUiControl *control,
                                 gfloat       *width,
                                 gfloat       *height)
{
    GrlUiLabel *self = GRL_UI_LABEL (control);
    int text_width;

    /* Get text width from raygui */
    text_width = GuiGetTextWidth (self->text != NULL ? self->text : "");

    if (width != NULL)
        *width = (gfloat)text_width + 8.0f;  /* Add some padding */
    if (height != NULL)
        *height = 24.0f;  /* Default label height */
}

static void
grl_ui_label_class_init (GrlUiLabelClass *klass)
{
    GObjectClass      *object_class = G_OBJECT_CLASS (klass);
    GrlUiControlClass *control_class = GRL_UI_CONTROL_CLASS (klass);

    object_class->finalize = grl_ui_label_finalize;
    object_class->get_property = grl_ui_label_get_property;
    object_class->set_property = grl_ui_label_set_property;

    control_class->draw = grl_ui_label_draw;
    control_class->get_preferred_size = grl_ui_label_get_preferred_size;

    /**
     * GrlUiLabel:text:
     *
     * The text displayed by the label.
     */
    properties[PROP_TEXT] =
        g_param_spec_string ("text",
                             "Text",
                             "The label text",
                             NULL,
                             G_PARAM_READWRITE |
                             G_PARAM_STATIC_STRINGS |
                             G_PARAM_EXPLICIT_NOTIFY);

    /**
     * GrlUiLabel:text-alignment:
     *
     * The horizontal text alignment.
     */
    properties[PROP_TEXT_ALIGNMENT] =
        g_param_spec_enum ("text-alignment",
                           "Text Alignment",
                           "The horizontal text alignment",
                           GRL_TYPE_UI_TEXT_ALIGNMENT,
                           GRL_UI_TEXT_ALIGN_LEFT,
                           G_PARAM_READWRITE |
                           G_PARAM_STATIC_STRINGS |
                           G_PARAM_EXPLICIT_NOTIFY);

    g_object_class_install_properties (object_class, N_PROPS, properties);
}

static void
grl_ui_label_init (GrlUiLabel *self)
{
    self->text = NULL;
    self->text_alignment = GRL_UI_TEXT_ALIGN_LEFT;
}

/**
 * grl_ui_label_new:
 * @text: (nullable): The label text
 *
 * Creates a new label control.
 *
 * Returns: (transfer full): A new #GrlUiLabel
 */
GrlUiLabel *
grl_ui_label_new (const gchar *text)
{
    return g_object_new (GRL_TYPE_UI_LABEL,
                         "text", text,
                         NULL);
}

/**
 * grl_ui_label_new_with_bounds:
 * @x: X position
 * @y: Y position
 * @width: Width of the label
 * @height: Height of the label
 * @text: (nullable): The label text
 *
 * Creates a new label control with specified bounds.
 *
 * Returns: (transfer full): A new #GrlUiLabel
 */
GrlUiLabel *
grl_ui_label_new_with_bounds (gfloat       x,
                              gfloat       y,
                              gfloat       width,
                              gfloat       height,
                              const gchar *text)
{
    GrlUiLabel *label;
    g_autoptr(GrlRectangle) bounds = NULL;

    bounds = grl_rectangle_new (x, y, width, height);
    label = g_object_new (GRL_TYPE_UI_LABEL,
                          "text", text,
                          "bounds", bounds,
                          NULL);

    return label;
}

/**
 * grl_ui_label_get_text:
 * @self: A #GrlUiLabel
 *
 * Gets the label text.
 *
 * Returns: (transfer none) (nullable): The label text
 */
const gchar *
grl_ui_label_get_text (GrlUiLabel *self)
{
    g_return_val_if_fail (GRL_IS_UI_LABEL (self), NULL);

    return self->text;
}

/**
 * grl_ui_label_set_text:
 * @self: A #GrlUiLabel
 * @text: (nullable): The new text
 *
 * Sets the label text.
 */
void
grl_ui_label_set_text (GrlUiLabel  *self,
                       const gchar *text)
{
    g_return_if_fail (GRL_IS_UI_LABEL (self));

    if (g_strcmp0 (self->text, text) == 0)
        return;

    g_free (self->text);
    self->text = g_strdup (text);

    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_TEXT]);
}

/**
 * grl_ui_label_get_text_alignment:
 * @self: A #GrlUiLabel
 *
 * Gets the horizontal text alignment.
 *
 * Returns: The #GrlUiTextAlignment
 */
GrlUiTextAlignment
grl_ui_label_get_text_alignment (GrlUiLabel *self)
{
    g_return_val_if_fail (GRL_IS_UI_LABEL (self), GRL_UI_TEXT_ALIGN_LEFT);

    return self->text_alignment;
}

/**
 * grl_ui_label_set_text_alignment:
 * @self: A #GrlUiLabel
 * @alignment: The horizontal text alignment
 *
 * Sets the horizontal text alignment.
 */
void
grl_ui_label_set_text_alignment (GrlUiLabel         *self,
                                 GrlUiTextAlignment  alignment)
{
    g_return_if_fail (GRL_IS_UI_LABEL (self));

    if (self->text_alignment == alignment)
        return;

    self->text_alignment = alignment;

    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_TEXT_ALIGNMENT]);
}
