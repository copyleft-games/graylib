/* grl-ui-colorpicker.c
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#include "config.h"
#include "grl-ui-colorpicker.h"
#include "../math/grl-rectangle.h"

#define RAYGUI_STATIC
#include <raygui.h>

struct _GrlUiColorPicker
{
    GrlUiControl  parent_instance;

    GrlColor     *color;
    gchar        *text;
};

G_DEFINE_TYPE (GrlUiColorPicker, grl_ui_colorpicker, GRL_TYPE_UI_CONTROL)

enum
{
    PROP_0,
    PROP_COLOR,
    PROP_TEXT,
    N_PROPS
};

static GParamSpec *properties[N_PROPS];

enum
{
    SIGNAL_COLOR_CHANGED,
    N_SIGNALS
};

static guint signals[N_SIGNALS];

/*
 * Helper to compare colors
 */
static gboolean
colors_equal (GrlColor *a,
              GrlColor *b)
{
    if (a == NULL && b == NULL)
        return TRUE;
    if (a == NULL || b == NULL)
        return FALSE;
    return (a->r == b->r && a->g == b->g && a->b == b->b && a->a == b->a);
}

/*
 * Virtual method implementations
 */

static void
grl_ui_colorpicker_draw_impl (GrlUiControl *control)
{
    GrlUiColorPicker *self;
    GrlRectangle     *bounds;
    Rectangle         raygui_bounds;
    gboolean          enabled;
    gboolean          visible;
    Color             old_color;
    Color             raygui_color;

    self = GRL_UI_COLORPICKER (control);

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

    /* Store old color for comparison */
    old_color.r = self->color->r;
    old_color.g = self->color->g;
    old_color.b = self->color->b;
    old_color.a = self->color->a;

    /* Set current color */
    raygui_color.r = self->color->r;
    raygui_color.g = self->color->g;
    raygui_color.b = self->color->b;
    raygui_color.a = self->color->a;

    GuiColorPicker (raygui_bounds,
                    self->text,
                    &raygui_color);

    if (!enabled)
        GuiSetState (STATE_NORMAL);

    /* Check if color changed */
    if (raygui_color.r != old_color.r ||
        raygui_color.g != old_color.g ||
        raygui_color.b != old_color.b ||
        raygui_color.a != old_color.a)
    {
        self->color->r = raygui_color.r;
        self->color->g = raygui_color.g;
        self->color->b = raygui_color.b;
        self->color->a = raygui_color.a;

        g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_COLOR]);
        g_signal_emit (self, signals[SIGNAL_COLOR_CHANGED], 0, self->color);
    }
}

static void
grl_ui_colorpicker_get_preferred_size_impl (GrlUiControl *control,
                                            gfloat       *width,
                                            gfloat       *height)
{
    (void)control;

    /* Default size for color picker - includes color panel and sliders */
    *width = 200.0f;
    *height = 200.0f;
}

/*
 * GObject implementation
 */

static void
grl_ui_colorpicker_finalize (GObject *object)
{
    GrlUiColorPicker *self = GRL_UI_COLORPICKER (object);

    g_clear_pointer (&self->color, grl_color_free);
    g_clear_pointer (&self->text, g_free);

    G_OBJECT_CLASS (grl_ui_colorpicker_parent_class)->finalize (object);
}

static void
grl_ui_colorpicker_get_property (GObject    *object,
                                 guint       prop_id,
                                 GValue     *value,
                                 GParamSpec *pspec)
{
    GrlUiColorPicker *self = GRL_UI_COLORPICKER (object);

    switch (prop_id)
    {
    case PROP_COLOR:
        g_value_set_boxed (value, self->color);
        break;
    case PROP_TEXT:
        g_value_set_string (value, self->text);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
grl_ui_colorpicker_set_property (GObject      *object,
                                 guint         prop_id,
                                 const GValue *value,
                                 GParamSpec   *pspec)
{
    GrlUiColorPicker *self = GRL_UI_COLORPICKER (object);

    switch (prop_id)
    {
    case PROP_COLOR:
        grl_ui_colorpicker_set_color (self, g_value_get_boxed (value));
        break;
    case PROP_TEXT:
        grl_ui_colorpicker_set_text (self, g_value_get_string (value));
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
grl_ui_colorpicker_class_init (GrlUiColorPickerClass *klass)
{
    GObjectClass      *object_class;
    GrlUiControlClass *control_class;

    object_class = G_OBJECT_CLASS (klass);
    control_class = GRL_UI_CONTROL_CLASS (klass);

    object_class->finalize = grl_ui_colorpicker_finalize;
    object_class->get_property = grl_ui_colorpicker_get_property;
    object_class->set_property = grl_ui_colorpicker_set_property;

    control_class->draw = grl_ui_colorpicker_draw_impl;
    control_class->get_preferred_size = grl_ui_colorpicker_get_preferred_size_impl;

    /**
     * GrlUiColorPicker:color:
     *
     * The currently selected color.
     */
    properties[PROP_COLOR] =
        g_param_spec_boxed ("color",
                            "Color",
                            "The currently selected color",
                            GRL_TYPE_COLOR,
                            G_PARAM_READWRITE |
                            G_PARAM_STATIC_STRINGS |
                            G_PARAM_EXPLICIT_NOTIFY);

    /**
     * GrlUiColorPicker:text:
     *
     * The label text displayed above the color picker.
     */
    properties[PROP_TEXT] =
        g_param_spec_string ("text",
                             "Text",
                             "The label text displayed above the color picker",
                             NULL,
                             G_PARAM_READWRITE |
                             G_PARAM_STATIC_STRINGS |
                             G_PARAM_EXPLICIT_NOTIFY);

    g_object_class_install_properties (object_class, N_PROPS, properties);

    /**
     * GrlUiColorPicker::color-changed:
     * @self: The color picker
     * @color: The new color
     *
     * Emitted when the selected color changes.
     */
    signals[SIGNAL_COLOR_CHANGED] =
        g_signal_new ("color-changed",
                      G_TYPE_FROM_CLASS (klass),
                      G_SIGNAL_RUN_LAST,
                      0,
                      NULL, NULL,
                      NULL,
                      G_TYPE_NONE,
                      1,
                      GRL_TYPE_COLOR);
}

static void
grl_ui_colorpicker_init (GrlUiColorPicker *self)
{
    /* Default to red */
    self->color = grl_color_new (255, 0, 0, 255);
    self->text = NULL;
}

/*
 * Public API
 */

/**
 * grl_ui_colorpicker_new:
 * @color: (nullable): Initial color, or %NULL for red
 *
 * Creates a new color picker control.
 *
 * Returns: (transfer full): A new #GrlUiColorPicker
 */
GrlUiColorPicker *
grl_ui_colorpicker_new (GrlColor *color)
{
    GrlUiColorPicker *picker;

    picker = g_object_new (GRL_TYPE_UI_COLORPICKER, NULL);

    if (color != NULL)
        grl_ui_colorpicker_set_color (picker, color);

    return picker;
}

/**
 * grl_ui_colorpicker_new_with_bounds:
 * @x: X position
 * @y: Y position
 * @width: Width of the color picker
 * @height: Height of the color picker
 * @color: (nullable): Initial color, or %NULL for red
 *
 * Creates a new color picker control with specified bounds.
 *
 * Returns: (transfer full): A new #GrlUiColorPicker
 */
GrlUiColorPicker *
grl_ui_colorpicker_new_with_bounds (gfloat    x,
                                    gfloat    y,
                                    gfloat    width,
                                    gfloat    height,
                                    GrlColor *color)
{
    g_autoptr(GrlRectangle) bounds = NULL;
    GrlUiColorPicker       *picker;

    bounds = grl_rectangle_new (x, y, width, height);

    picker = g_object_new (GRL_TYPE_UI_COLORPICKER,
                           "bounds", bounds,
                           NULL);

    if (color != NULL)
        grl_ui_colorpicker_set_color (picker, color);

    return picker;
}

/**
 * grl_ui_colorpicker_get_color:
 * @self: A #GrlUiColorPicker
 *
 * Gets the currently selected color.
 *
 * Returns: (transfer full): The current color
 */
GrlColor *
grl_ui_colorpicker_get_color (GrlUiColorPicker *self)
{
    g_return_val_if_fail (GRL_IS_UI_COLORPICKER (self), NULL);

    return grl_color_copy (self->color);
}

/**
 * grl_ui_colorpicker_set_color:
 * @self: A #GrlUiColorPicker
 * @color: The color to set
 *
 * Sets the currently selected color.
 */
void
grl_ui_colorpicker_set_color (GrlUiColorPicker *self,
                              GrlColor         *color)
{
    g_return_if_fail (GRL_IS_UI_COLORPICKER (self));
    g_return_if_fail (color != NULL);

    if (colors_equal (self->color, color))
        return;

    grl_color_free (self->color);
    self->color = grl_color_copy (color);

    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_COLOR]);
}

/**
 * grl_ui_colorpicker_get_text:
 * @self: A #GrlUiColorPicker
 *
 * Gets the label text displayed above the color picker.
 *
 * Returns: (transfer none) (nullable): The label text
 */
const gchar *
grl_ui_colorpicker_get_text (GrlUiColorPicker *self)
{
    g_return_val_if_fail (GRL_IS_UI_COLORPICKER (self), NULL);

    return self->text;
}

/**
 * grl_ui_colorpicker_set_text:
 * @self: A #GrlUiColorPicker
 * @text: (nullable): The label text to display
 *
 * Sets the label text displayed above the color picker.
 */
void
grl_ui_colorpicker_set_text (GrlUiColorPicker *self,
                             const gchar      *text)
{
    g_return_if_fail (GRL_IS_UI_COLORPICKER (self));

    if (g_strcmp0 (self->text, text) == 0)
        return;

    g_free (self->text);
    self->text = g_strdup (text);

    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_TEXT]);
}

/**
 * grl_ui_colorpicker_get_rgb:
 * @self: A #GrlUiColorPicker
 * @r: (out): Red component (0-255)
 * @g: (out): Green component (0-255)
 * @b: (out): Blue component (0-255)
 *
 * Gets the RGB components of the current color.
 */
void
grl_ui_colorpicker_get_rgb (GrlUiColorPicker *self,
                            guint8           *r,
                            guint8           *g,
                            guint8           *b)
{
    g_return_if_fail (GRL_IS_UI_COLORPICKER (self));

    if (r != NULL)
        *r = self->color->r;
    if (g != NULL)
        *g = self->color->g;
    if (b != NULL)
        *b = self->color->b;
}

/**
 * grl_ui_colorpicker_set_rgb:
 * @self: A #GrlUiColorPicker
 * @r: Red component (0-255)
 * @g: Green component (0-255)
 * @b: Blue component (0-255)
 *
 * Sets the current color using RGB components.
 */
void
grl_ui_colorpicker_set_rgb (GrlUiColorPicker *self,
                            guint8            r,
                            guint8            g,
                            guint8            b)
{
    g_return_if_fail (GRL_IS_UI_COLORPICKER (self));

    if (self->color->r == r && self->color->g == g && self->color->b == b)
        return;

    self->color->r = r;
    self->color->g = g;
    self->color->b = b;

    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_COLOR]);
}

/**
 * grl_ui_colorpicker_get_rgba:
 * @self: A #GrlUiColorPicker
 * @r: (out): Red component (0-255)
 * @g: (out): Green component (0-255)
 * @b: (out): Blue component (0-255)
 * @a: (out): Alpha component (0-255)
 *
 * Gets the RGBA components of the current color.
 */
void
grl_ui_colorpicker_get_rgba (GrlUiColorPicker *self,
                             guint8           *r,
                             guint8           *g,
                             guint8           *b,
                             guint8           *a)
{
    g_return_if_fail (GRL_IS_UI_COLORPICKER (self));

    if (r != NULL)
        *r = self->color->r;
    if (g != NULL)
        *g = self->color->g;
    if (b != NULL)
        *b = self->color->b;
    if (a != NULL)
        *a = self->color->a;
}

/**
 * grl_ui_colorpicker_set_rgba:
 * @self: A #GrlUiColorPicker
 * @r: Red component (0-255)
 * @g: Green component (0-255)
 * @b: Blue component (0-255)
 * @a: Alpha component (0-255)
 *
 * Sets the current color using RGBA components.
 */
void
grl_ui_colorpicker_set_rgba (GrlUiColorPicker *self,
                             guint8            r,
                             guint8            g,
                             guint8            b,
                             guint8            a)
{
    g_return_if_fail (GRL_IS_UI_COLORPICKER (self));

    if (self->color->r == r && self->color->g == g &&
        self->color->b == b && self->color->a == a)
        return;

    self->color->r = r;
    self->color->g = g;
    self->color->b = b;
    self->color->a = a;

    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_COLOR]);
}
