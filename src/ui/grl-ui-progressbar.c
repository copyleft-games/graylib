/* grl-ui-progressbar.c
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#include "config.h"
#include "grl-ui-progressbar.h"
#include "../math/grl-rectangle.h"

#define RAYGUI_STATIC
#include <raygui.h>

struct _GrlUiProgressBar
{
    GrlUiControl  parent_instance;

    gfloat        value;
    gfloat        min_value;
    gfloat        max_value;
    gchar        *text_left;
    gchar        *text_right;
};

G_DEFINE_TYPE (GrlUiProgressBar, grl_ui_progressbar, GRL_TYPE_UI_CONTROL)

enum
{
    PROP_0,
    PROP_VALUE,
    PROP_MIN_VALUE,
    PROP_MAX_VALUE,
    PROP_TEXT_LEFT,
    PROP_TEXT_RIGHT,
    N_PROPS
};

static GParamSpec *properties[N_PROPS];

/*
 * Virtual method implementations
 */

static void
grl_ui_progressbar_draw_impl (GrlUiControl *control)
{
    GrlUiProgressBar *self;
    GrlRectangle     *bounds;
    Rectangle         raygui_bounds;
    gboolean          enabled;
    gboolean          visible;
    float             value;

    self = GRL_UI_PROGRESSBAR (control);

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

    value = self->value;

    GuiProgressBar (raygui_bounds,
                    self->text_left,
                    self->text_right,
                    &value,
                    self->min_value,
                    self->max_value);

    if (!enabled)
        GuiSetState (STATE_NORMAL);
}

static void
grl_ui_progressbar_get_preferred_size_impl (GrlUiControl *control,
                                            gfloat       *width,
                                            gfloat       *height)
{
    (void)control;

    *width = 200.0f;   /* Default progress bar width */
    *height = 20.0f;   /* Default progress bar height */
}

/*
 * GObject implementation
 */

static void
grl_ui_progressbar_finalize (GObject *object)
{
    GrlUiProgressBar *self = GRL_UI_PROGRESSBAR (object);

    g_clear_pointer (&self->text_left, g_free);
    g_clear_pointer (&self->text_right, g_free);

    G_OBJECT_CLASS (grl_ui_progressbar_parent_class)->finalize (object);
}

static void
grl_ui_progressbar_get_property (GObject    *object,
                                 guint       prop_id,
                                 GValue     *value,
                                 GParamSpec *pspec)
{
    GrlUiProgressBar *self = GRL_UI_PROGRESSBAR (object);

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
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
grl_ui_progressbar_set_property (GObject      *object,
                                 guint         prop_id,
                                 const GValue *value,
                                 GParamSpec   *pspec)
{
    GrlUiProgressBar *self = GRL_UI_PROGRESSBAR (object);

    switch (prop_id)
    {
    case PROP_VALUE:
        grl_ui_progressbar_set_value (self, g_value_get_float (value));
        break;
    case PROP_MIN_VALUE:
        grl_ui_progressbar_set_min_value (self, g_value_get_float (value));
        break;
    case PROP_MAX_VALUE:
        grl_ui_progressbar_set_max_value (self, g_value_get_float (value));
        break;
    case PROP_TEXT_LEFT:
        grl_ui_progressbar_set_text_left (self, g_value_get_string (value));
        break;
    case PROP_TEXT_RIGHT:
        grl_ui_progressbar_set_text_right (self, g_value_get_string (value));
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
grl_ui_progressbar_class_init (GrlUiProgressBarClass *klass)
{
    GObjectClass      *object_class;
    GrlUiControlClass *control_class;

    object_class = G_OBJECT_CLASS (klass);
    control_class = GRL_UI_CONTROL_CLASS (klass);

    object_class->finalize = grl_ui_progressbar_finalize;
    object_class->get_property = grl_ui_progressbar_get_property;
    object_class->set_property = grl_ui_progressbar_set_property;

    control_class->draw = grl_ui_progressbar_draw_impl;
    control_class->get_preferred_size = grl_ui_progressbar_get_preferred_size_impl;

    /**
     * GrlUiProgressBar:value:
     *
     * The current progress value.
     */
    properties[PROP_VALUE] =
        g_param_spec_float ("value",
                            "Value",
                            "The current progress value",
                            -G_MAXFLOAT, G_MAXFLOAT, 0.0f,
                            G_PARAM_READWRITE |
                            G_PARAM_STATIC_STRINGS |
                            G_PARAM_EXPLICIT_NOTIFY);

    /**
     * GrlUiProgressBar:min-value:
     *
     * The minimum value.
     */
    properties[PROP_MIN_VALUE] =
        g_param_spec_float ("min-value",
                            "Min Value",
                            "The minimum value",
                            -G_MAXFLOAT, G_MAXFLOAT, 0.0f,
                            G_PARAM_READWRITE |
                            G_PARAM_STATIC_STRINGS |
                            G_PARAM_EXPLICIT_NOTIFY);

    /**
     * GrlUiProgressBar:max-value:
     *
     * The maximum value.
     */
    properties[PROP_MAX_VALUE] =
        g_param_spec_float ("max-value",
                            "Max Value",
                            "The maximum value",
                            -G_MAXFLOAT, G_MAXFLOAT, 100.0f,
                            G_PARAM_READWRITE |
                            G_PARAM_STATIC_STRINGS |
                            G_PARAM_EXPLICIT_NOTIFY);

    /**
     * GrlUiProgressBar:text-left:
     *
     * The left text label.
     */
    properties[PROP_TEXT_LEFT] =
        g_param_spec_string ("text-left",
                             "Text Left",
                             "The left text label",
                             NULL,
                             G_PARAM_READWRITE |
                             G_PARAM_STATIC_STRINGS |
                             G_PARAM_EXPLICIT_NOTIFY);

    /**
     * GrlUiProgressBar:text-right:
     *
     * The right text label.
     */
    properties[PROP_TEXT_RIGHT] =
        g_param_spec_string ("text-right",
                             "Text Right",
                             "The right text label",
                             NULL,
                             G_PARAM_READWRITE |
                             G_PARAM_STATIC_STRINGS |
                             G_PARAM_EXPLICIT_NOTIFY);

    g_object_class_install_properties (object_class, N_PROPS, properties);
}

static void
grl_ui_progressbar_init (GrlUiProgressBar *self)
{
    self->value = 0.0f;
    self->min_value = 0.0f;
    self->max_value = 100.0f;
    self->text_left = NULL;
    self->text_right = NULL;
}

/*
 * Public API
 */

/**
 * grl_ui_progressbar_new:
 * @min_value: Minimum value
 * @max_value: Maximum value
 *
 * Creates a new progress bar control.
 *
 * Returns: (transfer full): A new #GrlUiProgressBar
 */
GrlUiProgressBar *
grl_ui_progressbar_new (gfloat min_value,
                        gfloat max_value)
{
    return g_object_new (GRL_TYPE_UI_PROGRESSBAR,
                         "min-value", min_value,
                         "max-value", max_value,
                         NULL);
}

/**
 * grl_ui_progressbar_new_with_bounds:
 * @x: X position
 * @y: Y position
 * @width: Width of the progress bar
 * @height: Height of the progress bar
 * @min_value: Minimum value
 * @max_value: Maximum value
 *
 * Creates a new progress bar control with specified bounds.
 *
 * Returns: (transfer full): A new #GrlUiProgressBar
 */
GrlUiProgressBar *
grl_ui_progressbar_new_with_bounds (gfloat x,
                                    gfloat y,
                                    gfloat width,
                                    gfloat height,
                                    gfloat min_value,
                                    gfloat max_value)
{
    g_autoptr(GrlRectangle) bounds = NULL;

    bounds = grl_rectangle_new (x, y, width, height);

    return g_object_new (GRL_TYPE_UI_PROGRESSBAR,
                         "bounds", bounds,
                         "min-value", min_value,
                         "max-value", max_value,
                         NULL);
}

/**
 * grl_ui_progressbar_get_value:
 * @self: A #GrlUiProgressBar
 *
 * Gets the current progress value.
 *
 * Returns: The current value
 */
gfloat
grl_ui_progressbar_get_value (GrlUiProgressBar *self)
{
    g_return_val_if_fail (GRL_IS_UI_PROGRESSBAR (self), 0.0f);

    return self->value;
}

/**
 * grl_ui_progressbar_set_value:
 * @self: A #GrlUiProgressBar
 * @value: The new value
 *
 * Sets the current progress value.
 */
void
grl_ui_progressbar_set_value (GrlUiProgressBar *self,
                              gfloat            value)
{
    g_return_if_fail (GRL_IS_UI_PROGRESSBAR (self));

    if (self->value == value)
        return;

    self->value = value;

    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_VALUE]);
}

/**
 * grl_ui_progressbar_get_min_value:
 * @self: A #GrlUiProgressBar
 *
 * Gets the minimum value.
 *
 * Returns: The minimum value
 */
gfloat
grl_ui_progressbar_get_min_value (GrlUiProgressBar *self)
{
    g_return_val_if_fail (GRL_IS_UI_PROGRESSBAR (self), 0.0f);

    return self->min_value;
}

/**
 * grl_ui_progressbar_set_min_value:
 * @self: A #GrlUiProgressBar
 * @min_value: The new minimum value
 *
 * Sets the minimum value.
 */
void
grl_ui_progressbar_set_min_value (GrlUiProgressBar *self,
                                  gfloat            min_value)
{
    g_return_if_fail (GRL_IS_UI_PROGRESSBAR (self));

    if (self->min_value == min_value)
        return;

    self->min_value = min_value;

    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_MIN_VALUE]);
}

/**
 * grl_ui_progressbar_get_max_value:
 * @self: A #GrlUiProgressBar
 *
 * Gets the maximum value.
 *
 * Returns: The maximum value
 */
gfloat
grl_ui_progressbar_get_max_value (GrlUiProgressBar *self)
{
    g_return_val_if_fail (GRL_IS_UI_PROGRESSBAR (self), 100.0f);

    return self->max_value;
}

/**
 * grl_ui_progressbar_set_max_value:
 * @self: A #GrlUiProgressBar
 * @max_value: The new maximum value
 *
 * Sets the maximum value.
 */
void
grl_ui_progressbar_set_max_value (GrlUiProgressBar *self,
                                  gfloat            max_value)
{
    g_return_if_fail (GRL_IS_UI_PROGRESSBAR (self));

    if (self->max_value == max_value)
        return;

    self->max_value = max_value;

    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_MAX_VALUE]);
}

/**
 * grl_ui_progressbar_get_text_left:
 * @self: A #GrlUiProgressBar
 *
 * Gets the left text label.
 *
 * Returns: (transfer none) (nullable): The left text label
 */
const gchar *
grl_ui_progressbar_get_text_left (GrlUiProgressBar *self)
{
    g_return_val_if_fail (GRL_IS_UI_PROGRESSBAR (self), NULL);

    return self->text_left;
}

/**
 * grl_ui_progressbar_set_text_left:
 * @self: A #GrlUiProgressBar
 * @text: (nullable): The new left text label
 *
 * Sets the left text label.
 */
void
grl_ui_progressbar_set_text_left (GrlUiProgressBar *self,
                                  const gchar      *text)
{
    g_return_if_fail (GRL_IS_UI_PROGRESSBAR (self));

    if (g_strcmp0 (self->text_left, text) == 0)
        return;

    g_free (self->text_left);
    self->text_left = g_strdup (text);

    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_TEXT_LEFT]);
}

/**
 * grl_ui_progressbar_get_text_right:
 * @self: A #GrlUiProgressBar
 *
 * Gets the right text label.
 *
 * Returns: (transfer none) (nullable): The right text label
 */
const gchar *
grl_ui_progressbar_get_text_right (GrlUiProgressBar *self)
{
    g_return_val_if_fail (GRL_IS_UI_PROGRESSBAR (self), NULL);

    return self->text_right;
}

/**
 * grl_ui_progressbar_set_text_right:
 * @self: A #GrlUiProgressBar
 * @text: (nullable): The new right text label
 *
 * Sets the right text label.
 */
void
grl_ui_progressbar_set_text_right (GrlUiProgressBar *self,
                                   const gchar      *text)
{
    g_return_if_fail (GRL_IS_UI_PROGRESSBAR (self));

    if (g_strcmp0 (self->text_right, text) == 0)
        return;

    g_free (self->text_right);
    self->text_right = g_strdup (text);

    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_TEXT_RIGHT]);
}

/**
 * grl_ui_progressbar_get_progress:
 * @self: A #GrlUiProgressBar
 *
 * Gets the progress as a percentage (0.0 to 1.0).
 *
 * Returns: The progress percentage
 */
gfloat
grl_ui_progressbar_get_progress (GrlUiProgressBar *self)
{
    gfloat range;

    g_return_val_if_fail (GRL_IS_UI_PROGRESSBAR (self), 0.0f);

    range = self->max_value - self->min_value;
    if (range <= 0.0f)
        return 0.0f;

    return (self->value - self->min_value) / range;
}

/**
 * grl_ui_progressbar_set_progress:
 * @self: A #GrlUiProgressBar
 * @progress: The progress as a percentage (0.0 to 1.0)
 *
 * Sets the progress as a percentage, mapping to the min/max range.
 */
void
grl_ui_progressbar_set_progress (GrlUiProgressBar *self,
                                 gfloat            progress)
{
    gfloat range;
    gfloat value;

    g_return_if_fail (GRL_IS_UI_PROGRESSBAR (self));

    /* Clamp progress to [0, 1] */
    if (progress < 0.0f)
        progress = 0.0f;
    else if (progress > 1.0f)
        progress = 1.0f;

    range = self->max_value - self->min_value;
    value = self->min_value + (progress * range);

    grl_ui_progressbar_set_value (self, value);
}
