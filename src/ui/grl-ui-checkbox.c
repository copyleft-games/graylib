/* grl-ui-checkbox.c
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#include "config.h"
#include "grl-ui-checkbox.h"

#include <raygui.h>

struct _GrlUiCheckBox
{
    GrlUiControl    parent_instance;

    gchar          *text;
    gboolean        checked;
};

G_DEFINE_TYPE (GrlUiCheckBox, grl_ui_checkbox, GRL_TYPE_UI_CONTROL)

enum
{
    PROP_0,
    PROP_TEXT,
    PROP_CHECKED,
    N_PROPS
};

enum
{
    SIGNAL_TOGGLED,
    N_SIGNALS
};

static GParamSpec *properties[N_PROPS];
static guint signals[N_SIGNALS];

static void
grl_ui_checkbox_finalize (GObject *object)
{
    GrlUiCheckBox *self = GRL_UI_CHECKBOX (object);

    g_clear_pointer (&self->text, g_free);

    G_OBJECT_CLASS (grl_ui_checkbox_parent_class)->finalize (object);
}

static void
grl_ui_checkbox_get_property (GObject    *object,
                              guint       prop_id,
                              GValue     *value,
                              GParamSpec *pspec)
{
    GrlUiCheckBox *self = GRL_UI_CHECKBOX (object);

    switch (prop_id)
    {
    case PROP_TEXT:
        g_value_set_string (value, self->text);
        break;
    case PROP_CHECKED:
        g_value_set_boolean (value, self->checked);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
grl_ui_checkbox_set_property (GObject      *object,
                              guint         prop_id,
                              const GValue *value,
                              GParamSpec   *pspec)
{
    GrlUiCheckBox *self = GRL_UI_CHECKBOX (object);

    switch (prop_id)
    {
    case PROP_TEXT:
        grl_ui_checkbox_set_text (self, g_value_get_string (value));
        break;
    case PROP_CHECKED:
        grl_ui_checkbox_set_checked (self, g_value_get_boolean (value));
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
grl_ui_checkbox_draw (GrlUiControl *control)
{
    GrlUiCheckBox *self = GRL_UI_CHECKBOX (control);
    g_autoptr(GrlRectangle) bounds = NULL;
    Rectangle rl_bounds;
    bool checked;
    gboolean old_checked;
    int result;

    bounds = grl_ui_control_get_bounds (control);

    rl_bounds.x = bounds->x;
    rl_bounds.y = bounds->y;
    rl_bounds.width = bounds->width;
    rl_bounds.height = bounds->height;

    /* Store old state and pass current state to raygui */
    old_checked = self->checked;
    checked = self->checked ? true : false;

    result = GuiCheckBox (rl_bounds,
                          self->text != NULL ? self->text : "",
                          &checked);

    /*
     * Handle bool/gboolean ABI mismatch: read as unsigned char
     * raygui modifies checked directly, so we need to read it properly
     */
    if (result != 0)
    {
        unsigned char raw = (unsigned char)checked;
        gboolean new_checked = raw != 0;

        if (new_checked != old_checked)
        {
            self->checked = new_checked;
            g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_CHECKED]);
            g_signal_emit (self, signals[SIGNAL_TOGGLED], 0, self->checked);
        }
    }
}

static gboolean
grl_ui_checkbox_handle_input (GrlUiControl *control)
{
    /*
     * CheckBox input is handled in draw() via raygui's immediate mode.
     * Return FALSE to indicate we don't exclusively consume input.
     */
    return FALSE;
}

static void
grl_ui_checkbox_get_preferred_size (GrlUiControl *control,
                                    gfloat       *width,
                                    gfloat       *height)
{
    GrlUiCheckBox *self = GRL_UI_CHECKBOX (control);
    int text_width;
    int checkbox_size;

    /* Get text width from raygui */
    text_width = GuiGetTextWidth (self->text != NULL ? self->text : "");

    /* Checkbox is typically square, default size is 16 */
    checkbox_size = GuiGetStyle (CHECKBOX, CHECK_PADDING) + 16;

    if (width != NULL)
        *width = (gfloat)(checkbox_size + 8 + text_width);  /* checkbox + spacing + text */
    if (height != NULL)
        *height = (gfloat)checkbox_size;
}

static void
grl_ui_checkbox_class_init (GrlUiCheckBoxClass *klass)
{
    GObjectClass      *object_class = G_OBJECT_CLASS (klass);
    GrlUiControlClass *control_class = GRL_UI_CONTROL_CLASS (klass);

    object_class->finalize = grl_ui_checkbox_finalize;
    object_class->get_property = grl_ui_checkbox_get_property;
    object_class->set_property = grl_ui_checkbox_set_property;

    control_class->draw = grl_ui_checkbox_draw;
    control_class->handle_input = grl_ui_checkbox_handle_input;
    control_class->get_preferred_size = grl_ui_checkbox_get_preferred_size;

    /**
     * GrlUiCheckBox:text:
     *
     * The label text displayed next to the checkbox.
     */
    properties[PROP_TEXT] =
        g_param_spec_string ("text",
                             "Text",
                             "The checkbox label text",
                             NULL,
                             G_PARAM_READWRITE |
                             G_PARAM_STATIC_STRINGS |
                             G_PARAM_EXPLICIT_NOTIFY);

    /**
     * GrlUiCheckBox:checked:
     *
     * Whether the checkbox is checked.
     */
    properties[PROP_CHECKED] =
        g_param_spec_boolean ("checked",
                              "Checked",
                              "Whether the checkbox is checked",
                              FALSE,
                              G_PARAM_READWRITE |
                              G_PARAM_STATIC_STRINGS |
                              G_PARAM_EXPLICIT_NOTIFY);

    g_object_class_install_properties (object_class, N_PROPS, properties);

    /**
     * GrlUiCheckBox::toggled:
     * @self: The #GrlUiCheckBox that was toggled
     * @checked: The new checked state
     *
     * Emitted when the checkbox state changes.
     */
    signals[SIGNAL_TOGGLED] =
        g_signal_new ("toggled",
                      G_TYPE_FROM_CLASS (klass),
                      G_SIGNAL_RUN_LAST,
                      0,
                      NULL, NULL,
                      NULL,
                      G_TYPE_NONE, 1,
                      G_TYPE_BOOLEAN);
}

static void
grl_ui_checkbox_init (GrlUiCheckBox *self)
{
    self->text = NULL;
    self->checked = FALSE;
}

/**
 * grl_ui_checkbox_new:
 * @text: (nullable): The checkbox label text
 *
 * Creates a new checkbox control.
 *
 * Returns: (transfer full): A new #GrlUiCheckBox
 */
GrlUiCheckBox *
grl_ui_checkbox_new (const gchar *text)
{
    return g_object_new (GRL_TYPE_UI_CHECKBOX,
                         "text", text,
                         NULL);
}

/**
 * grl_ui_checkbox_new_with_bounds:
 * @x: X position
 * @y: Y position
 * @width: Width of the checkbox
 * @height: Height of the checkbox
 * @text: (nullable): The checkbox label text
 *
 * Creates a new checkbox control with specified bounds.
 *
 * Returns: (transfer full): A new #GrlUiCheckBox
 */
GrlUiCheckBox *
grl_ui_checkbox_new_with_bounds (gfloat       x,
                                 gfloat       y,
                                 gfloat       width,
                                 gfloat       height,
                                 const gchar *text)
{
    GrlUiCheckBox *checkbox;
    g_autoptr(GrlRectangle) bounds = NULL;

    bounds = grl_rectangle_new (x, y, width, height);
    checkbox = g_object_new (GRL_TYPE_UI_CHECKBOX,
                             "text", text,
                             "bounds", bounds,
                             NULL);

    return checkbox;
}

/**
 * grl_ui_checkbox_get_text:
 * @self: A #GrlUiCheckBox
 *
 * Gets the checkbox label text.
 *
 * Returns: (transfer none) (nullable): The checkbox text
 */
const gchar *
grl_ui_checkbox_get_text (GrlUiCheckBox *self)
{
    g_return_val_if_fail (GRL_IS_UI_CHECKBOX (self), NULL);

    return self->text;
}

/**
 * grl_ui_checkbox_set_text:
 * @self: A #GrlUiCheckBox
 * @text: (nullable): The new text
 *
 * Sets the checkbox label text.
 */
void
grl_ui_checkbox_set_text (GrlUiCheckBox *self,
                          const gchar   *text)
{
    g_return_if_fail (GRL_IS_UI_CHECKBOX (self));

    if (g_strcmp0 (self->text, text) == 0)
        return;

    g_free (self->text);
    self->text = g_strdup (text);

    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_TEXT]);
}

/**
 * grl_ui_checkbox_get_checked:
 * @self: A #GrlUiCheckBox
 *
 * Gets whether the checkbox is checked.
 *
 * Returns: %TRUE if checked, %FALSE otherwise
 */
gboolean
grl_ui_checkbox_get_checked (GrlUiCheckBox *self)
{
    g_return_val_if_fail (GRL_IS_UI_CHECKBOX (self), FALSE);

    return self->checked;
}

/**
 * grl_ui_checkbox_set_checked:
 * @self: A #GrlUiCheckBox
 * @checked: Whether the checkbox should be checked
 *
 * Sets whether the checkbox is checked.
 */
void
grl_ui_checkbox_set_checked (GrlUiCheckBox *self,
                             gboolean       checked)
{
    g_return_if_fail (GRL_IS_UI_CHECKBOX (self));

    checked = !!checked;

    if (self->checked == checked)
        return;

    self->checked = checked;

    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_CHECKED]);
}

/**
 * grl_ui_checkbox_toggle:
 * @self: A #GrlUiCheckBox
 *
 * Toggles the checkbox state.
 */
void
grl_ui_checkbox_toggle (GrlUiCheckBox *self)
{
    g_return_if_fail (GRL_IS_UI_CHECKBOX (self));

    grl_ui_checkbox_set_checked (self, !self->checked);
    g_signal_emit (self, signals[SIGNAL_TOGGLED], 0, self->checked);
}
