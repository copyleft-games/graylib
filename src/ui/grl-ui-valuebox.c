/* grl-ui-valuebox.c
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#include "config.h"
#include "grl-ui-valuebox.h"
#include "../math/grl-rectangle.h"

#define RAYGUI_STATIC
#include <raygui.h>

struct _GrlUiValueBox
{
    GrlUiControl  parent_instance;

    gint          value;
    gint          min_value;
    gint          max_value;
    gchar        *text;
    gboolean      edit_mode;
};

G_DEFINE_TYPE (GrlUiValueBox, grl_ui_valuebox, GRL_TYPE_UI_CONTROL)

enum
{
    PROP_0,
    PROP_VALUE,
    PROP_MIN_VALUE,
    PROP_MAX_VALUE,
    PROP_TEXT,
    PROP_EDIT_MODE,
    N_PROPS
};

static GParamSpec *properties[N_PROPS];

enum
{
    SIGNAL_VALUE_CHANGED,
    SIGNAL_EDITING_STARTED,
    SIGNAL_EDITING_FINISHED,
    N_SIGNALS
};

static guint signals[N_SIGNALS];

/*
 * Virtual method implementations
 */

static void
grl_ui_valuebox_draw_impl (GrlUiControl *control)
{
    GrlUiValueBox    *self;
    GrlRectangle     *bounds;
    Rectangle         raygui_bounds;
    gboolean          was_edit_mode;
    gboolean          enabled;
    gboolean          visible;
    gint              old_value;
    int               value;
    int               result;

    self = GRL_UI_VALUEBOX (control);

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

    was_edit_mode = self->edit_mode;
    old_value = self->value;
    value = self->value;

    result = GuiValueBox (raygui_bounds,
                          self->text,
                          &value,
                          self->min_value,
                          self->max_value,
                          self->edit_mode);

    if (!enabled)
        GuiSetState (STATE_NORMAL);

    /* Update value if changed */
    if (value != old_value)
    {
        self->value = value;
        g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_VALUE]);
        g_signal_emit (self, signals[SIGNAL_VALUE_CHANGED], 0, self->value);
    }

    /* Handle edit mode toggle (result indicates if edit mode should toggle) */
    if (result != 0)
    {
        self->edit_mode = !self->edit_mode;
        g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_EDIT_MODE]);

        if (self->edit_mode && !was_edit_mode)
        {
            g_signal_emit (self, signals[SIGNAL_EDITING_STARTED], 0);
        }
        else if (!self->edit_mode && was_edit_mode)
        {
            g_signal_emit (self, signals[SIGNAL_EDITING_FINISHED], 0);
        }
    }
}

static gboolean
grl_ui_valuebox_handle_input_impl (GrlUiControl *control)
{
    GrlUiValueBox *self = GRL_UI_VALUEBOX (control);

    /* Return TRUE if we're in edit mode (exclusive input) */
    return self->edit_mode;
}

static void
grl_ui_valuebox_get_preferred_size_impl (GrlUiControl *control,
                                         gfloat       *width,
                                         gfloat       *height)
{
    GrlUiValueBox *self;
    int            text_width;

    self = GRL_UI_VALUEBOX (control);

    if (self->text != NULL && self->text[0] != '\0')
    {
        text_width = GuiGetTextWidth (self->text);
        *width = (gfloat)(text_width + 60);  /* Add space for value display */
    }
    else
    {
        *width = 80.0f;  /* Default value box width */
    }

    *height = 24.0f;  /* Default value box height */
}

/*
 * GObject implementation
 */

static void
grl_ui_valuebox_finalize (GObject *object)
{
    GrlUiValueBox *self = GRL_UI_VALUEBOX (object);

    g_clear_pointer (&self->text, g_free);

    G_OBJECT_CLASS (grl_ui_valuebox_parent_class)->finalize (object);
}

static void
grl_ui_valuebox_get_property (GObject    *object,
                              guint       prop_id,
                              GValue     *value,
                              GParamSpec *pspec)
{
    GrlUiValueBox *self = GRL_UI_VALUEBOX (object);

    switch (prop_id)
    {
    case PROP_VALUE:
        g_value_set_int (value, self->value);
        break;
    case PROP_MIN_VALUE:
        g_value_set_int (value, self->min_value);
        break;
    case PROP_MAX_VALUE:
        g_value_set_int (value, self->max_value);
        break;
    case PROP_TEXT:
        g_value_set_string (value, self->text);
        break;
    case PROP_EDIT_MODE:
        g_value_set_boolean (value, self->edit_mode);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
grl_ui_valuebox_set_property (GObject      *object,
                              guint         prop_id,
                              const GValue *value,
                              GParamSpec   *pspec)
{
    GrlUiValueBox *self = GRL_UI_VALUEBOX (object);

    switch (prop_id)
    {
    case PROP_VALUE:
        grl_ui_valuebox_set_value (self, g_value_get_int (value));
        break;
    case PROP_MIN_VALUE:
        grl_ui_valuebox_set_min_value (self, g_value_get_int (value));
        break;
    case PROP_MAX_VALUE:
        grl_ui_valuebox_set_max_value (self, g_value_get_int (value));
        break;
    case PROP_TEXT:
        grl_ui_valuebox_set_text (self, g_value_get_string (value));
        break;
    case PROP_EDIT_MODE:
        grl_ui_valuebox_set_edit_mode (self, g_value_get_boolean (value));
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
grl_ui_valuebox_class_init (GrlUiValueBoxClass *klass)
{
    GObjectClass      *object_class;
    GrlUiControlClass *control_class;

    object_class = G_OBJECT_CLASS (klass);
    control_class = GRL_UI_CONTROL_CLASS (klass);

    object_class->finalize = grl_ui_valuebox_finalize;
    object_class->get_property = grl_ui_valuebox_get_property;
    object_class->set_property = grl_ui_valuebox_set_property;

    control_class->draw = grl_ui_valuebox_draw_impl;
    control_class->handle_input = grl_ui_valuebox_handle_input_impl;
    control_class->get_preferred_size = grl_ui_valuebox_get_preferred_size_impl;

    /**
     * GrlUiValueBox:value:
     *
     * The current value.
     */
    properties[PROP_VALUE] =
        g_param_spec_int ("value",
                          "Value",
                          "The current value",
                          G_MININT, G_MAXINT, 0,
                          G_PARAM_READWRITE |
                          G_PARAM_STATIC_STRINGS |
                          G_PARAM_EXPLICIT_NOTIFY);

    /**
     * GrlUiValueBox:min-value:
     *
     * The minimum value.
     */
    properties[PROP_MIN_VALUE] =
        g_param_spec_int ("min-value",
                          "Min Value",
                          "The minimum value",
                          G_MININT, G_MAXINT, 0,
                          G_PARAM_READWRITE |
                          G_PARAM_STATIC_STRINGS |
                          G_PARAM_EXPLICIT_NOTIFY);

    /**
     * GrlUiValueBox:max-value:
     *
     * The maximum value.
     */
    properties[PROP_MAX_VALUE] =
        g_param_spec_int ("max-value",
                          "Max Value",
                          "The maximum value",
                          G_MININT, G_MAXINT, 100,
                          G_PARAM_READWRITE |
                          G_PARAM_STATIC_STRINGS |
                          G_PARAM_EXPLICIT_NOTIFY);

    /**
     * GrlUiValueBox:text:
     *
     * The value box label text.
     */
    properties[PROP_TEXT] =
        g_param_spec_string ("text",
                             "Text",
                             "The value box label text",
                             NULL,
                             G_PARAM_READWRITE |
                             G_PARAM_STATIC_STRINGS |
                             G_PARAM_EXPLICIT_NOTIFY);

    /**
     * GrlUiValueBox:edit-mode:
     *
     * Whether the value box is in edit mode.
     */
    properties[PROP_EDIT_MODE] =
        g_param_spec_boolean ("edit-mode",
                              "Edit Mode",
                              "Whether the value box is in edit mode",
                              FALSE,
                              G_PARAM_READWRITE |
                              G_PARAM_STATIC_STRINGS |
                              G_PARAM_EXPLICIT_NOTIFY);

    g_object_class_install_properties (object_class, N_PROPS, properties);

    /**
     * GrlUiValueBox::value-changed:
     * @self: The value box
     * @value: The new value
     *
     * Emitted when the value changes.
     */
    signals[SIGNAL_VALUE_CHANGED] =
        g_signal_new ("value-changed",
                      G_TYPE_FROM_CLASS (klass),
                      G_SIGNAL_RUN_LAST,
                      0,
                      NULL, NULL,
                      NULL,
                      G_TYPE_NONE,
                      1,
                      G_TYPE_INT);

    /**
     * GrlUiValueBox::editing-started:
     * @self: The value box
     *
     * Emitted when editing mode is started.
     */
    signals[SIGNAL_EDITING_STARTED] =
        g_signal_new ("editing-started",
                      G_TYPE_FROM_CLASS (klass),
                      G_SIGNAL_RUN_LAST,
                      0,
                      NULL, NULL,
                      NULL,
                      G_TYPE_NONE,
                      0);

    /**
     * GrlUiValueBox::editing-finished:
     * @self: The value box
     *
     * Emitted when editing mode is finished.
     */
    signals[SIGNAL_EDITING_FINISHED] =
        g_signal_new ("editing-finished",
                      G_TYPE_FROM_CLASS (klass),
                      G_SIGNAL_RUN_LAST,
                      0,
                      NULL, NULL,
                      NULL,
                      G_TYPE_NONE,
                      0);
}

static void
grl_ui_valuebox_init (GrlUiValueBox *self)
{
    self->value = 0;
    self->min_value = 0;
    self->max_value = 100;
    self->text = NULL;
    self->edit_mode = FALSE;
}

/*
 * Public API
 */

/**
 * grl_ui_valuebox_new:
 * @min_value: Minimum value
 * @max_value: Maximum value
 *
 * Creates a new value box control.
 *
 * Returns: (transfer full): A new #GrlUiValueBox
 */
GrlUiValueBox *
grl_ui_valuebox_new (gint min_value,
                     gint max_value)
{
    return g_object_new (GRL_TYPE_UI_VALUEBOX,
                         "min-value", min_value,
                         "max-value", max_value,
                         NULL);
}

/**
 * grl_ui_valuebox_new_with_bounds:
 * @x: X position
 * @y: Y position
 * @width: Width of the value box
 * @height: Height of the value box
 * @min_value: Minimum value
 * @max_value: Maximum value
 *
 * Creates a new value box control with specified bounds.
 *
 * Returns: (transfer full): A new #GrlUiValueBox
 */
GrlUiValueBox *
grl_ui_valuebox_new_with_bounds (gfloat x,
                                 gfloat y,
                                 gfloat width,
                                 gfloat height,
                                 gint   min_value,
                                 gint   max_value)
{
    g_autoptr(GrlRectangle) bounds = NULL;

    bounds = grl_rectangle_new (x, y, width, height);

    return g_object_new (GRL_TYPE_UI_VALUEBOX,
                         "bounds", bounds,
                         "min-value", min_value,
                         "max-value", max_value,
                         NULL);
}

/**
 * grl_ui_valuebox_get_value:
 * @self: A #GrlUiValueBox
 *
 * Gets the current value.
 *
 * Returns: The current value
 */
gint
grl_ui_valuebox_get_value (GrlUiValueBox *self)
{
    g_return_val_if_fail (GRL_IS_UI_VALUEBOX (self), 0);

    return self->value;
}

/**
 * grl_ui_valuebox_set_value:
 * @self: A #GrlUiValueBox
 * @value: The new value
 *
 * Sets the current value.
 */
void
grl_ui_valuebox_set_value (GrlUiValueBox *self,
                           gint           value)
{
    g_return_if_fail (GRL_IS_UI_VALUEBOX (self));

    /* Clamp to range */
    if (value < self->min_value)
        value = self->min_value;
    else if (value > self->max_value)
        value = self->max_value;

    if (self->value == value)
        return;

    self->value = value;

    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_VALUE]);
}

/**
 * grl_ui_valuebox_get_min_value:
 * @self: A #GrlUiValueBox
 *
 * Gets the minimum value.
 *
 * Returns: The minimum value
 */
gint
grl_ui_valuebox_get_min_value (GrlUiValueBox *self)
{
    g_return_val_if_fail (GRL_IS_UI_VALUEBOX (self), 0);

    return self->min_value;
}

/**
 * grl_ui_valuebox_set_min_value:
 * @self: A #GrlUiValueBox
 * @min_value: The new minimum value
 *
 * Sets the minimum value.
 */
void
grl_ui_valuebox_set_min_value (GrlUiValueBox *self,
                               gint           min_value)
{
    g_return_if_fail (GRL_IS_UI_VALUEBOX (self));

    if (self->min_value == min_value)
        return;

    self->min_value = min_value;

    /* Adjust current value if needed */
    if (self->value < min_value)
    {
        self->value = min_value;
        g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_VALUE]);
    }

    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_MIN_VALUE]);
}

/**
 * grl_ui_valuebox_get_max_value:
 * @self: A #GrlUiValueBox
 *
 * Gets the maximum value.
 *
 * Returns: The maximum value
 */
gint
grl_ui_valuebox_get_max_value (GrlUiValueBox *self)
{
    g_return_val_if_fail (GRL_IS_UI_VALUEBOX (self), 100);

    return self->max_value;
}

/**
 * grl_ui_valuebox_set_max_value:
 * @self: A #GrlUiValueBox
 * @max_value: The new maximum value
 *
 * Sets the maximum value.
 */
void
grl_ui_valuebox_set_max_value (GrlUiValueBox *self,
                               gint           max_value)
{
    g_return_if_fail (GRL_IS_UI_VALUEBOX (self));

    if (self->max_value == max_value)
        return;

    self->max_value = max_value;

    /* Adjust current value if needed */
    if (self->value > max_value)
    {
        self->value = max_value;
        g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_VALUE]);
    }

    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_MAX_VALUE]);
}

/**
 * grl_ui_valuebox_get_text:
 * @self: A #GrlUiValueBox
 *
 * Gets the value box label text.
 *
 * Returns: (transfer none) (nullable): The label text
 */
const gchar *
grl_ui_valuebox_get_text (GrlUiValueBox *self)
{
    g_return_val_if_fail (GRL_IS_UI_VALUEBOX (self), NULL);

    return self->text;
}

/**
 * grl_ui_valuebox_set_text:
 * @self: A #GrlUiValueBox
 * @text: (nullable): The new label text
 *
 * Sets the value box label text.
 */
void
grl_ui_valuebox_set_text (GrlUiValueBox *self,
                          const gchar   *text)
{
    g_return_if_fail (GRL_IS_UI_VALUEBOX (self));

    if (g_strcmp0 (self->text, text) == 0)
        return;

    g_free (self->text);
    self->text = g_strdup (text);

    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_TEXT]);
}

/**
 * grl_ui_valuebox_get_edit_mode:
 * @self: A #GrlUiValueBox
 *
 * Gets whether the value box is in edit mode.
 *
 * Returns: %TRUE if in edit mode
 */
gboolean
grl_ui_valuebox_get_edit_mode (GrlUiValueBox *self)
{
    g_return_val_if_fail (GRL_IS_UI_VALUEBOX (self), FALSE);

    return self->edit_mode;
}

/**
 * grl_ui_valuebox_set_edit_mode:
 * @self: A #GrlUiValueBox
 * @edit_mode: Whether to enter edit mode
 *
 * Sets whether the value box is in edit mode.
 */
void
grl_ui_valuebox_set_edit_mode (GrlUiValueBox *self,
                               gboolean       edit_mode)
{
    g_return_if_fail (GRL_IS_UI_VALUEBOX (self));

    edit_mode = !!edit_mode;  /* Normalize to TRUE/FALSE */

    if (self->edit_mode == edit_mode)
        return;

    self->edit_mode = edit_mode;

    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_EDIT_MODE]);
}
