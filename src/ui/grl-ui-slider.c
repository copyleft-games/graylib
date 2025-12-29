/* grl-ui-slider.c
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#include "config.h"
#include "grl-ui-slider.h"

#include <raygui.h>

struct _GrlUiSlider
{
    GrlUiControl    parent_instance;

    gfloat          value;
    gfloat          min_value;
    gfloat          max_value;
    gchar          *text_left;
    gchar          *text_right;
    gboolean        show_bar;
};

G_DEFINE_TYPE (GrlUiSlider, grl_ui_slider, GRL_TYPE_UI_CONTROL)

enum
{
    PROP_0,
    PROP_VALUE,
    PROP_MIN_VALUE,
    PROP_MAX_VALUE,
    PROP_TEXT_LEFT,
    PROP_TEXT_RIGHT,
    PROP_SHOW_BAR,
    N_PROPS
};

enum
{
    SIGNAL_VALUE_CHANGED,
    N_SIGNALS
};

static GParamSpec *properties[N_PROPS];
static guint signals[N_SIGNALS];

static void
grl_ui_slider_finalize (GObject *object)
{
    GrlUiSlider *self = GRL_UI_SLIDER (object);

    g_clear_pointer (&self->text_left, g_free);
    g_clear_pointer (&self->text_right, g_free);

    G_OBJECT_CLASS (grl_ui_slider_parent_class)->finalize (object);
}

static void
grl_ui_slider_get_property (GObject    *object,
                            guint       prop_id,
                            GValue     *value,
                            GParamSpec *pspec)
{
    GrlUiSlider *self = GRL_UI_SLIDER (object);

    switch (prop_id)
    {
    case PROP_VALUE:
        g_value_set_float (value, self->value);
        break;
    case PROP_MIN_VALUE:
        g_value_set_float (value, self->min_value);
        break;
    case PROP_MAX_VALUE:
        g_value_set_float (value, self->max_value);
        break;
    case PROP_TEXT_LEFT:
        g_value_set_string (value, self->text_left);
        break;
    case PROP_TEXT_RIGHT:
        g_value_set_string (value, self->text_right);
        break;
    case PROP_SHOW_BAR:
        g_value_set_boolean (value, self->show_bar);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
grl_ui_slider_set_property (GObject      *object,
                            guint         prop_id,
                            const GValue *value,
                            GParamSpec   *pspec)
{
    GrlUiSlider *self = GRL_UI_SLIDER (object);

    switch (prop_id)
    {
    case PROP_VALUE:
        grl_ui_slider_set_value (self, g_value_get_float (value));
        break;
    case PROP_MIN_VALUE:
        grl_ui_slider_set_min_value (self, g_value_get_float (value));
        break;
    case PROP_MAX_VALUE:
        grl_ui_slider_set_max_value (self, g_value_get_float (value));
        break;
    case PROP_TEXT_LEFT:
        grl_ui_slider_set_text_left (self, g_value_get_string (value));
        break;
    case PROP_TEXT_RIGHT:
        grl_ui_slider_set_text_right (self, g_value_get_string (value));
        break;
    case PROP_SHOW_BAR:
        grl_ui_slider_set_show_bar (self, g_value_get_boolean (value));
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
grl_ui_slider_draw (GrlUiControl *control)
{
    GrlUiSlider *self = GRL_UI_SLIDER (control);
    g_autoptr(GrlRectangle) bounds = NULL;
    Rectangle rl_bounds;
    float value;
    gfloat old_value;
    int result;

    bounds = grl_ui_control_get_bounds (control);

    rl_bounds.x = bounds->x;
    rl_bounds.y = bounds->y;
    rl_bounds.width = bounds->width;
    rl_bounds.height = bounds->height;

    /* Store old value and pass current value to raygui */
    old_value = self->value;
    value = self->value;

    /* Use GuiSlider or GuiSliderBar depending on show_bar */
    if (self->show_bar)
    {
        result = GuiSliderBar (rl_bounds,
                               self->text_left,
                               self->text_right,
                               &value,
                               self->min_value,
                               self->max_value);
    }
    else
    {
        result = GuiSlider (rl_bounds,
                            self->text_left,
                            self->text_right,
                            &value,
                            self->min_value,
                            self->max_value);
    }

    /* Check if value changed */
    if (result != 0 || value != old_value)
    {
        if (value != old_value)
        {
            self->value = value;
            g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_VALUE]);
            g_signal_emit (self, signals[SIGNAL_VALUE_CHANGED], 0, self->value);
        }
    }
}

static gboolean
grl_ui_slider_handle_input (GrlUiControl *control)
{
    /*
     * Slider input is handled in draw() via raygui's immediate mode.
     * Return FALSE to indicate we don't exclusively consume input.
     */
    return FALSE;
}

static void
grl_ui_slider_get_preferred_size (GrlUiControl *control,
                                  gfloat       *width,
                                  gfloat       *height)
{
    if (width != NULL)
        *width = 200.0f;  /* Default slider width */
    if (height != NULL)
        *height = 20.0f;  /* Default slider height */
}

static void
grl_ui_slider_class_init (GrlUiSliderClass *klass)
{
    GObjectClass      *object_class = G_OBJECT_CLASS (klass);
    GrlUiControlClass *control_class = GRL_UI_CONTROL_CLASS (klass);

    object_class->finalize = grl_ui_slider_finalize;
    object_class->get_property = grl_ui_slider_get_property;
    object_class->set_property = grl_ui_slider_set_property;

    control_class->draw = grl_ui_slider_draw;
    control_class->handle_input = grl_ui_slider_handle_input;
    control_class->get_preferred_size = grl_ui_slider_get_preferred_size;

    /**
     * GrlUiSlider:value:
     *
     * The current slider value.
     */
    properties[PROP_VALUE] =
        g_param_spec_float ("value",
                            "Value",
                            "The current slider value",
                            -G_MAXFLOAT, G_MAXFLOAT, 0.0f,
                            G_PARAM_READWRITE |
                            G_PARAM_STATIC_STRINGS |
                            G_PARAM_EXPLICIT_NOTIFY);

    /**
     * GrlUiSlider:min-value:
     *
     * The minimum slider value.
     */
    properties[PROP_MIN_VALUE] =
        g_param_spec_float ("min-value",
                            "Minimum Value",
                            "The minimum slider value",
                            -G_MAXFLOAT, G_MAXFLOAT, 0.0f,
                            G_PARAM_READWRITE |
                            G_PARAM_STATIC_STRINGS |
                            G_PARAM_EXPLICIT_NOTIFY);

    /**
     * GrlUiSlider:max-value:
     *
     * The maximum slider value.
     */
    properties[PROP_MAX_VALUE] =
        g_param_spec_float ("max-value",
                            "Maximum Value",
                            "The maximum slider value",
                            -G_MAXFLOAT, G_MAXFLOAT, 100.0f,
                            G_PARAM_READWRITE |
                            G_PARAM_STATIC_STRINGS |
                            G_PARAM_EXPLICIT_NOTIFY);

    /**
     * GrlUiSlider:text-left:
     *
     * The text label displayed to the left of the slider.
     */
    properties[PROP_TEXT_LEFT] =
        g_param_spec_string ("text-left",
                             "Left Text",
                             "The left label text",
                             NULL,
                             G_PARAM_READWRITE |
                             G_PARAM_STATIC_STRINGS |
                             G_PARAM_EXPLICIT_NOTIFY);

    /**
     * GrlUiSlider:text-right:
     *
     * The text label displayed to the right of the slider.
     */
    properties[PROP_TEXT_RIGHT] =
        g_param_spec_string ("text-right",
                             "Right Text",
                             "The right label text",
                             NULL,
                             G_PARAM_READWRITE |
                             G_PARAM_STATIC_STRINGS |
                             G_PARAM_EXPLICIT_NOTIFY);

    /**
     * GrlUiSlider:show-bar:
     *
     * Whether to show the slider as a filled bar.
     */
    properties[PROP_SHOW_BAR] =
        g_param_spec_boolean ("show-bar",
                              "Show Bar",
                              "Whether to show as a filled bar",
                              FALSE,
                              G_PARAM_READWRITE |
                              G_PARAM_STATIC_STRINGS |
                              G_PARAM_EXPLICIT_NOTIFY);

    g_object_class_install_properties (object_class, N_PROPS, properties);

    /**
     * GrlUiSlider::value-changed:
     * @self: The #GrlUiSlider whose value changed
     * @value: The new value
     *
     * Emitted when the slider value changes.
     */
    signals[SIGNAL_VALUE_CHANGED] =
        g_signal_new ("value-changed",
                      G_TYPE_FROM_CLASS (klass),
                      G_SIGNAL_RUN_LAST,
                      0,
                      NULL, NULL,
                      NULL,
                      G_TYPE_NONE, 1,
                      G_TYPE_FLOAT);
}

static void
grl_ui_slider_init (GrlUiSlider *self)
{
    self->value = 0.0f;
    self->min_value = 0.0f;
    self->max_value = 100.0f;
    self->text_left = NULL;
    self->text_right = NULL;
    self->show_bar = FALSE;
}

/**
 * grl_ui_slider_new:
 * @min_value: Minimum value
 * @max_value: Maximum value
 *
 * Creates a new slider control.
 *
 * Returns: (transfer full): A new #GrlUiSlider
 */
GrlUiSlider *
grl_ui_slider_new (gfloat min_value,
                   gfloat max_value)
{
    return g_object_new (GRL_TYPE_UI_SLIDER,
                         "min-value", min_value,
                         "max-value", max_value,
                         NULL);
}

/**
 * grl_ui_slider_new_with_bounds:
 * @x: X position
 * @y: Y position
 * @width: Width of the slider
 * @height: Height of the slider
 * @min_value: Minimum value
 * @max_value: Maximum value
 *
 * Creates a new slider control with specified bounds.
 *
 * Returns: (transfer full): A new #GrlUiSlider
 */
GrlUiSlider *
grl_ui_slider_new_with_bounds (gfloat x,
                               gfloat y,
                               gfloat width,
                               gfloat height,
                               gfloat min_value,
                               gfloat max_value)
{
    GrlUiSlider *slider;
    g_autoptr(GrlRectangle) bounds = NULL;

    bounds = grl_rectangle_new (x, y, width, height);
    slider = g_object_new (GRL_TYPE_UI_SLIDER,
                           "min-value", min_value,
                           "max-value", max_value,
                           "bounds", bounds,
                           NULL);

    return slider;
}

/**
 * grl_ui_slider_get_value:
 * @self: A #GrlUiSlider
 *
 * Gets the current slider value.
 *
 * Returns: The current value
 */
gfloat
grl_ui_slider_get_value (GrlUiSlider *self)
{
    g_return_val_if_fail (GRL_IS_UI_SLIDER (self), 0.0f);

    return self->value;
}

/**
 * grl_ui_slider_set_value:
 * @self: A #GrlUiSlider
 * @value: The new value
 *
 * Sets the slider value. The value will be clamped to the
 * min/max range.
 */
void
grl_ui_slider_set_value (GrlUiSlider *self,
                         gfloat       value)
{
    g_return_if_fail (GRL_IS_UI_SLIDER (self));

    /* Clamp value to range */
    if (value < self->min_value)
        value = self->min_value;
    if (value > self->max_value)
        value = self->max_value;

    if (self->value == value)
        return;

    self->value = value;

    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_VALUE]);
}

/**
 * grl_ui_slider_get_min_value:
 * @self: A #GrlUiSlider
 *
 * Gets the minimum slider value.
 *
 * Returns: The minimum value
 */
gfloat
grl_ui_slider_get_min_value (GrlUiSlider *self)
{
    g_return_val_if_fail (GRL_IS_UI_SLIDER (self), 0.0f);

    return self->min_value;
}

/**
 * grl_ui_slider_set_min_value:
 * @self: A #GrlUiSlider
 * @min_value: The new minimum value
 *
 * Sets the minimum slider value.
 */
void
grl_ui_slider_set_min_value (GrlUiSlider *self,
                             gfloat       min_value)
{
    g_return_if_fail (GRL_IS_UI_SLIDER (self));

    if (self->min_value == min_value)
        return;

    self->min_value = min_value;

    /* Clamp current value if needed */
    if (self->value < min_value)
    {
        self->value = min_value;
        g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_VALUE]);
    }

    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_MIN_VALUE]);
}

/**
 * grl_ui_slider_get_max_value:
 * @self: A #GrlUiSlider
 *
 * Gets the maximum slider value.
 *
 * Returns: The maximum value
 */
gfloat
grl_ui_slider_get_max_value (GrlUiSlider *self)
{
    g_return_val_if_fail (GRL_IS_UI_SLIDER (self), 100.0f);

    return self->max_value;
}

/**
 * grl_ui_slider_set_max_value:
 * @self: A #GrlUiSlider
 * @max_value: The new maximum value
 *
 * Sets the maximum slider value.
 */
void
grl_ui_slider_set_max_value (GrlUiSlider *self,
                             gfloat       max_value)
{
    g_return_if_fail (GRL_IS_UI_SLIDER (self));

    if (self->max_value == max_value)
        return;

    self->max_value = max_value;

    /* Clamp current value if needed */
    if (self->value > max_value)
    {
        self->value = max_value;
        g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_VALUE]);
    }

    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_MAX_VALUE]);
}

/**
 * grl_ui_slider_get_text_left:
 * @self: A #GrlUiSlider
 *
 * Gets the left label text.
 *
 * Returns: (transfer none) (nullable): The left label text
 */
const gchar *
grl_ui_slider_get_text_left (GrlUiSlider *self)
{
    g_return_val_if_fail (GRL_IS_UI_SLIDER (self), NULL);

    return self->text_left;
}

/**
 * grl_ui_slider_set_text_left:
 * @self: A #GrlUiSlider
 * @text: (nullable): The left label text
 *
 * Sets the left label text.
 */
void
grl_ui_slider_set_text_left (GrlUiSlider *self,
                             const gchar *text)
{
    g_return_if_fail (GRL_IS_UI_SLIDER (self));

    if (g_strcmp0 (self->text_left, text) == 0)
        return;

    g_free (self->text_left);
    self->text_left = g_strdup (text);

    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_TEXT_LEFT]);
}

/**
 * grl_ui_slider_get_text_right:
 * @self: A #GrlUiSlider
 *
 * Gets the right label text.
 *
 * Returns: (transfer none) (nullable): The right label text
 */
const gchar *
grl_ui_slider_get_text_right (GrlUiSlider *self)
{
    g_return_val_if_fail (GRL_IS_UI_SLIDER (self), NULL);

    return self->text_right;
}

/**
 * grl_ui_slider_set_text_right:
 * @self: A #GrlUiSlider
 * @text: (nullable): The right label text
 *
 * Sets the right label text.
 */
void
grl_ui_slider_set_text_right (GrlUiSlider *self,
                              const gchar *text)
{
    g_return_if_fail (GRL_IS_UI_SLIDER (self));

    if (g_strcmp0 (self->text_right, text) == 0)
        return;

    g_free (self->text_right);
    self->text_right = g_strdup (text);

    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_TEXT_RIGHT]);
}

/**
 * grl_ui_slider_get_show_bar:
 * @self: A #GrlUiSlider
 *
 * Gets whether the slider shows as a bar (filled from left).
 *
 * Returns: %TRUE if showing as bar
 */
gboolean
grl_ui_slider_get_show_bar (GrlUiSlider *self)
{
    g_return_val_if_fail (GRL_IS_UI_SLIDER (self), FALSE);

    return self->show_bar;
}

/**
 * grl_ui_slider_set_show_bar:
 * @self: A #GrlUiSlider
 * @show_bar: Whether to show as a bar
 *
 * Sets whether the slider shows as a bar (filled from left).
 */
void
grl_ui_slider_set_show_bar (GrlUiSlider *self,
                            gboolean     show_bar)
{
    g_return_if_fail (GRL_IS_UI_SLIDER (self));

    show_bar = !!show_bar;

    if (self->show_bar == show_bar)
        return;

    self->show_bar = show_bar;

    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_SHOW_BAR]);
}
