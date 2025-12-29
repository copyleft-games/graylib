/* grl-ui-toggle.c
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#include "config.h"
#include "grl-ui-toggle.h"
#include "../math/grl-rectangle.h"

#define RAYGUI_STATIC
#include <raygui.h>

struct _GrlUiToggle
{
    GrlUiControl  parent_instance;

    gchar        *text;
    gboolean      active;
};

G_DEFINE_TYPE (GrlUiToggle, grl_ui_toggle, GRL_TYPE_UI_CONTROL)

enum
{
    PROP_0,
    PROP_TEXT,
    PROP_ACTIVE,
    N_PROPS
};

static GParamSpec *properties[N_PROPS];

enum
{
    SIGNAL_TOGGLED,
    N_SIGNALS
};

static guint signals[N_SIGNALS];

/*
 * Virtual method implementations
 */

static void
grl_ui_toggle_draw_impl (GrlUiControl *control)
{
    GrlUiToggle      *self;
    GrlRectangle     *bounds;
    Rectangle         raygui_bounds;
    gboolean          was_active;
    gboolean          enabled;
    gboolean          visible;
    int               result;
    bool              active_state;

    self = GRL_UI_TOGGLE (control);

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

    was_active = self->active;
    active_state = self->active ? true : false;

    result = GuiToggle (raygui_bounds,
                        self->text != NULL ? self->text : "",
                        &active_state);

    /* Convert bool back to gboolean safely */
    self->active = active_state != 0;

    if (!enabled)
        GuiSetState (STATE_NORMAL);

    /* Check if toggled (GuiToggle returns 1 when active state changes) */
    if (result != 0 && self->active != was_active)
    {
        g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_ACTIVE]);
        g_signal_emit (self, signals[SIGNAL_TOGGLED], 0, self->active);
    }
}

static void
grl_ui_toggle_get_preferred_size_impl (GrlUiControl *control,
                                       gfloat       *width,
                                       gfloat       *height)
{
    GrlUiToggle *self;
    int          text_width;

    self = GRL_UI_TOGGLE (control);

    if (self->text != NULL && self->text[0] != '\0')
    {
        text_width = GuiGetTextWidth (self->text);
        *width = (gfloat)(text_width + 20);  /* Padding for toggle styling */
    }
    else
    {
        *width = 80.0f;  /* Default toggle width */
    }

    *height = 24.0f;  /* Default toggle height */
}

/*
 * GObject implementation
 */

static void
grl_ui_toggle_finalize (GObject *object)
{
    GrlUiToggle *self = GRL_UI_TOGGLE (object);

    g_clear_pointer (&self->text, g_free);

    G_OBJECT_CLASS (grl_ui_toggle_parent_class)->finalize (object);
}

static void
grl_ui_toggle_get_property (GObject    *object,
                            guint       prop_id,
                            GValue     *value,
                            GParamSpec *pspec)
{
    GrlUiToggle *self = GRL_UI_TOGGLE (object);

    switch (prop_id)
    {
    case PROP_TEXT:
        g_value_set_string (value, self->text);
        break;
    case PROP_ACTIVE:
        g_value_set_boolean (value, self->active);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
grl_ui_toggle_set_property (GObject      *object,
                            guint         prop_id,
                            const GValue *value,
                            GParamSpec   *pspec)
{
    GrlUiToggle *self = GRL_UI_TOGGLE (object);

    switch (prop_id)
    {
    case PROP_TEXT:
        grl_ui_toggle_set_text (self, g_value_get_string (value));
        break;
    case PROP_ACTIVE:
        grl_ui_toggle_set_active (self, g_value_get_boolean (value));
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
grl_ui_toggle_class_init (GrlUiToggleClass *klass)
{
    GObjectClass      *object_class;
    GrlUiControlClass *control_class;

    object_class = G_OBJECT_CLASS (klass);
    control_class = GRL_UI_CONTROL_CLASS (klass);

    object_class->finalize = grl_ui_toggle_finalize;
    object_class->get_property = grl_ui_toggle_get_property;
    object_class->set_property = grl_ui_toggle_set_property;

    control_class->draw = grl_ui_toggle_draw_impl;
    control_class->get_preferred_size = grl_ui_toggle_get_preferred_size_impl;

    /**
     * GrlUiToggle:text:
     *
     * The toggle label text.
     */
    properties[PROP_TEXT] =
        g_param_spec_string ("text",
                             "Text",
                             "The toggle label text",
                             NULL,
                             G_PARAM_READWRITE |
                             G_PARAM_STATIC_STRINGS |
                             G_PARAM_EXPLICIT_NOTIFY);

    /**
     * GrlUiToggle:active:
     *
     * Whether the toggle is active (on).
     */
    properties[PROP_ACTIVE] =
        g_param_spec_boolean ("active",
                              "Active",
                              "Whether the toggle is active (on)",
                              FALSE,
                              G_PARAM_READWRITE |
                              G_PARAM_STATIC_STRINGS |
                              G_PARAM_EXPLICIT_NOTIFY);

    g_object_class_install_properties (object_class, N_PROPS, properties);

    /**
     * GrlUiToggle::toggled:
     * @self: The toggle that was toggled
     * @active: The new active state
     *
     * Emitted when the toggle is activated or deactivated.
     */
    signals[SIGNAL_TOGGLED] =
        g_signal_new ("toggled",
                      G_TYPE_FROM_CLASS (klass),
                      G_SIGNAL_RUN_LAST,
                      0,
                      NULL, NULL,
                      NULL,
                      G_TYPE_NONE,
                      1,
                      G_TYPE_BOOLEAN);
}

static void
grl_ui_toggle_init (GrlUiToggle *self)
{
    self->text = NULL;
    self->active = FALSE;
}

/*
 * Public API
 */

/**
 * grl_ui_toggle_new:
 * @text: (nullable): The toggle label text
 *
 * Creates a new toggle button control.
 *
 * Returns: (transfer full): A new #GrlUiToggle
 */
GrlUiToggle *
grl_ui_toggle_new (const gchar *text)
{
    return g_object_new (GRL_TYPE_UI_TOGGLE,
                         "text", text,
                         NULL);
}

/**
 * grl_ui_toggle_new_with_bounds:
 * @x: X position
 * @y: Y position
 * @width: Width of the toggle
 * @height: Height of the toggle
 * @text: (nullable): The toggle label text
 *
 * Creates a new toggle button control with specified bounds.
 *
 * Returns: (transfer full): A new #GrlUiToggle
 */
GrlUiToggle *
grl_ui_toggle_new_with_bounds (gfloat       x,
                               gfloat       y,
                               gfloat       width,
                               gfloat       height,
                               const gchar *text)
{
    g_autoptr(GrlRectangle) bounds = NULL;

    bounds = grl_rectangle_new (x, y, width, height);

    return g_object_new (GRL_TYPE_UI_TOGGLE,
                         "bounds", bounds,
                         "text", text,
                         NULL);
}

/**
 * grl_ui_toggle_get_text:
 * @self: A #GrlUiToggle
 *
 * Gets the toggle label text.
 *
 * Returns: (transfer none) (nullable): The label text
 */
const gchar *
grl_ui_toggle_get_text (GrlUiToggle *self)
{
    g_return_val_if_fail (GRL_IS_UI_TOGGLE (self), NULL);

    return self->text;
}

/**
 * grl_ui_toggle_set_text:
 * @self: A #GrlUiToggle
 * @text: (nullable): The new label text
 *
 * Sets the toggle label text.
 */
void
grl_ui_toggle_set_text (GrlUiToggle *self,
                        const gchar *text)
{
    g_return_if_fail (GRL_IS_UI_TOGGLE (self));

    if (g_strcmp0 (self->text, text) == 0)
        return;

    g_free (self->text);
    self->text = g_strdup (text);

    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_TEXT]);
}

/**
 * grl_ui_toggle_get_active:
 * @self: A #GrlUiToggle
 *
 * Gets whether the toggle is active (on).
 *
 * Returns: %TRUE if active
 */
gboolean
grl_ui_toggle_get_active (GrlUiToggle *self)
{
    g_return_val_if_fail (GRL_IS_UI_TOGGLE (self), FALSE);

    return self->active;
}

/**
 * grl_ui_toggle_set_active:
 * @self: A #GrlUiToggle
 * @active: Whether the toggle should be active
 *
 * Sets whether the toggle is active (on).
 */
void
grl_ui_toggle_set_active (GrlUiToggle *self,
                          gboolean     active)
{
    g_return_if_fail (GRL_IS_UI_TOGGLE (self));

    active = !!active;  /* Normalize to TRUE/FALSE */

    if (self->active == active)
        return;

    self->active = active;

    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_ACTIVE]);
}

/**
 * grl_ui_toggle_toggle:
 * @self: A #GrlUiToggle
 *
 * Toggles the active state.
 */
void
grl_ui_toggle_toggle (GrlUiToggle *self)
{
    g_return_if_fail (GRL_IS_UI_TOGGLE (self));

    self->active = !self->active;

    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_ACTIVE]);
    g_signal_emit (self, signals[SIGNAL_TOGGLED], 0, self->active);
}
