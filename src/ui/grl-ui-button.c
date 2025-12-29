/* grl-ui-button.c
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#include "config.h"
#include "grl-ui-button.h"

#include <raygui.h>

struct _GrlUiButton
{
    GrlUiControl    parent_instance;

    gchar          *text;
    gint            icon_id;
};

G_DEFINE_TYPE (GrlUiButton, grl_ui_button, GRL_TYPE_UI_CONTROL)

enum
{
    PROP_0,
    PROP_TEXT,
    PROP_ICON,
    N_PROPS
};

enum
{
    SIGNAL_CLICKED,
    N_SIGNALS
};

static GParamSpec *properties[N_PROPS];
static guint signals[N_SIGNALS];

static void
grl_ui_button_finalize (GObject *object)
{
    GrlUiButton *self = GRL_UI_BUTTON (object);

    g_clear_pointer (&self->text, g_free);

    G_OBJECT_CLASS (grl_ui_button_parent_class)->finalize (object);
}

static void
grl_ui_button_get_property (GObject    *object,
                            guint       prop_id,
                            GValue     *value,
                            GParamSpec *pspec)
{
    GrlUiButton *self = GRL_UI_BUTTON (object);

    switch (prop_id)
    {
    case PROP_TEXT:
        g_value_set_string (value, self->text);
        break;
    case PROP_ICON:
        g_value_set_int (value, self->icon_id);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
grl_ui_button_set_property (GObject      *object,
                            guint         prop_id,
                            const GValue *value,
                            GParamSpec   *pspec)
{
    GrlUiButton *self = GRL_UI_BUTTON (object);

    switch (prop_id)
    {
    case PROP_TEXT:
        grl_ui_button_set_text (self, g_value_get_string (value));
        break;
    case PROP_ICON:
        grl_ui_button_set_icon (self, g_value_get_int (value));
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
grl_ui_button_draw (GrlUiControl *control)
{
    GrlUiButton *self = GRL_UI_BUTTON (control);
    g_autoptr(GrlRectangle) bounds = NULL;
    Rectangle rl_bounds;
    const char *display_text;
    int result;

    bounds = grl_ui_control_get_bounds (control);

    rl_bounds.x = bounds->x;
    rl_bounds.y = bounds->y;
    rl_bounds.width = bounds->width;
    rl_bounds.height = bounds->height;

    /* Build display text with optional icon */
    if (self->icon_id >= 0)
        display_text = GuiIconText (self->icon_id, self->text != NULL ? self->text : "");
    else
        display_text = self->text != NULL ? self->text : "";

    result = GuiButton (rl_bounds, display_text);

    /* Emit clicked signal if button was pressed */
    if (result != 0)
    {
        g_signal_emit (self, signals[SIGNAL_CLICKED], 0);
    }
}

static gboolean
grl_ui_button_handle_input (GrlUiControl *control)
{
    /*
     * Button input is handled in draw() via raygui's immediate mode.
     * Return FALSE to indicate we don't exclusively consume input.
     */
    return FALSE;
}

static void
grl_ui_button_get_preferred_size (GrlUiControl *control,
                                  gfloat       *width,
                                  gfloat       *height)
{
    GrlUiButton *self = GRL_UI_BUTTON (control);
    const char *display_text;
    int text_width;

    /* Build display text with optional icon */
    if (self->icon_id >= 0)
        display_text = GuiIconText (self->icon_id, self->text != NULL ? self->text : "");
    else
        display_text = self->text != NULL ? self->text : "";

    text_width = GuiGetTextWidth (display_text);

    if (width != NULL)
        *width = (gfloat)text_width + 24.0f;  /* Add padding for button borders */
    if (height != NULL)
        *height = 28.0f;  /* Default button height */
}

static void
grl_ui_button_class_init (GrlUiButtonClass *klass)
{
    GObjectClass      *object_class = G_OBJECT_CLASS (klass);
    GrlUiControlClass *control_class = GRL_UI_CONTROL_CLASS (klass);

    object_class->finalize = grl_ui_button_finalize;
    object_class->get_property = grl_ui_button_get_property;
    object_class->set_property = grl_ui_button_set_property;

    control_class->draw = grl_ui_button_draw;
    control_class->handle_input = grl_ui_button_handle_input;
    control_class->get_preferred_size = grl_ui_button_get_preferred_size;

    /**
     * GrlUiButton:text:
     *
     * The text displayed on the button.
     */
    properties[PROP_TEXT] =
        g_param_spec_string ("text",
                             "Text",
                             "The button text",
                             NULL,
                             G_PARAM_READWRITE |
                             G_PARAM_STATIC_STRINGS |
                             G_PARAM_EXPLICIT_NOTIFY);

    /**
     * GrlUiButton:icon:
     *
     * The icon ID to display on the button (-1 for no icon).
     */
    properties[PROP_ICON] =
        g_param_spec_int ("icon",
                          "Icon",
                          "The button icon ID",
                          -1, G_MAXINT, -1,
                          G_PARAM_READWRITE |
                          G_PARAM_STATIC_STRINGS |
                          G_PARAM_EXPLICIT_NOTIFY);

    g_object_class_install_properties (object_class, N_PROPS, properties);

    /**
     * GrlUiButton::clicked:
     * @self: The #GrlUiButton that was clicked
     *
     * Emitted when the button is clicked.
     */
    signals[SIGNAL_CLICKED] =
        g_signal_new ("clicked",
                      G_TYPE_FROM_CLASS (klass),
                      G_SIGNAL_RUN_LAST,
                      0,
                      NULL, NULL,
                      NULL,
                      G_TYPE_NONE, 0);
}

static void
grl_ui_button_init (GrlUiButton *self)
{
    self->text = NULL;
    self->icon_id = -1;
}

/**
 * grl_ui_button_new:
 * @text: (nullable): The button text
 *
 * Creates a new button control.
 *
 * Returns: (transfer full): A new #GrlUiButton
 */
GrlUiButton *
grl_ui_button_new (const gchar *text)
{
    return g_object_new (GRL_TYPE_UI_BUTTON,
                         "text", text,
                         NULL);
}

/**
 * grl_ui_button_new_with_bounds:
 * @x: X position
 * @y: Y position
 * @width: Width of the button
 * @height: Height of the button
 * @text: (nullable): The button text
 *
 * Creates a new button control with specified bounds.
 *
 * Returns: (transfer full): A new #GrlUiButton
 */
GrlUiButton *
grl_ui_button_new_with_bounds (gfloat       x,
                               gfloat       y,
                               gfloat       width,
                               gfloat       height,
                               const gchar *text)
{
    GrlUiButton *button;
    g_autoptr(GrlRectangle) bounds = NULL;

    bounds = grl_rectangle_new (x, y, width, height);
    button = g_object_new (GRL_TYPE_UI_BUTTON,
                           "text", text,
                           "bounds", bounds,
                           NULL);

    return button;
}

/**
 * grl_ui_button_get_text:
 * @self: A #GrlUiButton
 *
 * Gets the button text.
 *
 * Returns: (transfer none) (nullable): The button text
 */
const gchar *
grl_ui_button_get_text (GrlUiButton *self)
{
    g_return_val_if_fail (GRL_IS_UI_BUTTON (self), NULL);

    return self->text;
}

/**
 * grl_ui_button_set_text:
 * @self: A #GrlUiButton
 * @text: (nullable): The new text
 *
 * Sets the button text.
 */
void
grl_ui_button_set_text (GrlUiButton *self,
                        const gchar *text)
{
    g_return_if_fail (GRL_IS_UI_BUTTON (self));

    if (g_strcmp0 (self->text, text) == 0)
        return;

    g_free (self->text);
    self->text = g_strdup (text);

    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_TEXT]);
}

/**
 * grl_ui_button_get_icon:
 * @self: A #GrlUiButton
 *
 * Gets the button icon ID.
 *
 * Returns: The icon ID, or -1 if no icon
 */
gint
grl_ui_button_get_icon (GrlUiButton *self)
{
    g_return_val_if_fail (GRL_IS_UI_BUTTON (self), -1);

    return self->icon_id;
}

/**
 * grl_ui_button_set_icon:
 * @self: A #GrlUiButton
 * @icon_id: The icon ID, or -1 for no icon
 *
 * Sets the button icon. The icon is prepended to the text.
 */
void
grl_ui_button_set_icon (GrlUiButton *self,
                        gint         icon_id)
{
    g_return_if_fail (GRL_IS_UI_BUTTON (self));

    if (self->icon_id == icon_id)
        return;

    self->icon_id = icon_id;

    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_ICON]);
}
