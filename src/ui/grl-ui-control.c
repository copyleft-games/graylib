/* grl-ui-control.c
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Base class implementation for UI controls.
 */

#include "config.h"
#include "grl-ui-control.h"
#include "grl-ui-control-private.h"
#include <raylib.h>

/**
 * SECTION:grl-ui-control
 * @title: GrlUiControl
 * @short_description: Base class for UI controls
 *
 * #GrlUiControl is the abstract base class for all UI controls in the
 * Graylib UI system. It provides common properties like bounds, enabled
 * state, and visibility, as well as virtual methods for drawing and
 * input handling.
 *
 * Subclasses should implement the draw() and handle_input() virtual
 * methods to provide their specific behavior.
 */

typedef struct
{
    GrlRectangle   *bounds;
    gboolean        enabled;
    gboolean        visible;
    GrlUiState      state;
    gchar          *tooltip;
} GrlUiControlPrivate;

G_DEFINE_ABSTRACT_TYPE_WITH_PRIVATE (GrlUiControl, grl_ui_control, G_TYPE_OBJECT)

enum
{
    PROP_0,
    PROP_BOUNDS,
    PROP_X,
    PROP_Y,
    PROP_WIDTH,
    PROP_HEIGHT,
    PROP_ENABLED,
    PROP_VISIBLE,
    PROP_STATE,
    PROP_TOOLTIP,
    N_PROPS
};

static GParamSpec *properties[N_PROPS];

static void
grl_ui_control_finalize (GObject *object)
{
    GrlUiControl        *self = GRL_UI_CONTROL (object);
    GrlUiControlPrivate *priv = grl_ui_control_get_instance_private (self);

    g_clear_pointer (&priv->bounds, grl_rectangle_free);
    g_clear_pointer (&priv->tooltip, g_free);

    G_OBJECT_CLASS (grl_ui_control_parent_class)->finalize (object);
}

static void
grl_ui_control_get_property (GObject    *object,
                             guint       prop_id,
                             GValue     *value,
                             GParamSpec *pspec)
{
    GrlUiControl        *self = GRL_UI_CONTROL (object);
    GrlUiControlPrivate *priv = grl_ui_control_get_instance_private (self);

    switch (prop_id)
    {
    case PROP_BOUNDS:
        g_value_set_boxed (value, priv->bounds);
        break;

    case PROP_X:
        g_value_set_float (value, priv->bounds ? priv->bounds->x : 0.0f);
        break;

    case PROP_Y:
        g_value_set_float (value, priv->bounds ? priv->bounds->y : 0.0f);
        break;

    case PROP_WIDTH:
        g_value_set_float (value, priv->bounds ? priv->bounds->width : 0.0f);
        break;

    case PROP_HEIGHT:
        g_value_set_float (value, priv->bounds ? priv->bounds->height : 0.0f);
        break;

    case PROP_ENABLED:
        g_value_set_boolean (value, priv->enabled);
        break;

    case PROP_VISIBLE:
        g_value_set_boolean (value, priv->visible);
        break;

    case PROP_STATE:
        g_value_set_enum (value, priv->state);
        break;

    case PROP_TOOLTIP:
        g_value_set_string (value, priv->tooltip);
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
grl_ui_control_set_property (GObject      *object,
                             guint         prop_id,
                             const GValue *value,
                             GParamSpec   *pspec)
{
    GrlUiControl *self = GRL_UI_CONTROL (object);

    switch (prop_id)
    {
    case PROP_BOUNDS:
        grl_ui_control_set_bounds (self, g_value_get_boxed (value));
        break;

    case PROP_X:
        grl_ui_control_set_x (self, g_value_get_float (value));
        break;

    case PROP_Y:
        grl_ui_control_set_y (self, g_value_get_float (value));
        break;

    case PROP_WIDTH:
        grl_ui_control_set_width (self, g_value_get_float (value));
        break;

    case PROP_HEIGHT:
        grl_ui_control_set_height (self, g_value_get_float (value));
        break;

    case PROP_ENABLED:
        grl_ui_control_set_enabled (self, g_value_get_boolean (value));
        break;

    case PROP_VISIBLE:
        grl_ui_control_set_visible (self, g_value_get_boolean (value));
        break;

    case PROP_TOOLTIP:
        grl_ui_control_set_tooltip (self, g_value_get_string (value));
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

/* Default implementations for virtual methods */
static void
grl_ui_control_real_draw (GrlUiControl *self)
{
    /* Default implementation does nothing - subclasses must override */
}

static gboolean
grl_ui_control_real_handle_input (GrlUiControl *self)
{
    /* Default implementation does nothing */
    return FALSE;
}

static void
grl_ui_control_real_get_preferred_size (GrlUiControl *self,
                                        gfloat       *width,
                                        gfloat       *height)
{
    GrlUiControlPrivate *priv = grl_ui_control_get_instance_private (self);

    /* Default: return current bounds size */
    if (width != NULL)
        *width = priv->bounds ? priv->bounds->width : 100.0f;
    if (height != NULL)
        *height = priv->bounds ? priv->bounds->height : 30.0f;
}

static void
grl_ui_control_class_init (GrlUiControlClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->finalize = grl_ui_control_finalize;
    object_class->get_property = grl_ui_control_get_property;
    object_class->set_property = grl_ui_control_set_property;

    /* Set default implementations */
    klass->draw = grl_ui_control_real_draw;
    klass->handle_input = grl_ui_control_real_handle_input;
    klass->get_preferred_size = grl_ui_control_real_get_preferred_size;

    /**
     * GrlUiControl:bounds:
     *
     * The bounds (position and size) of the control.
     */
    properties[PROP_BOUNDS] =
        g_param_spec_boxed ("bounds",
                            "Bounds",
                            "The control bounds",
                            GRL_TYPE_RECTANGLE,
                            G_PARAM_READWRITE |
                            G_PARAM_STATIC_STRINGS |
                            G_PARAM_EXPLICIT_NOTIFY);

    /**
     * GrlUiControl:x:
     *
     * The X position of the control.
     */
    properties[PROP_X] =
        g_param_spec_float ("x",
                            "X",
                            "X position",
                            -G_MAXFLOAT, G_MAXFLOAT, 0.0f,
                            G_PARAM_READWRITE |
                            G_PARAM_STATIC_STRINGS |
                            G_PARAM_EXPLICIT_NOTIFY);

    /**
     * GrlUiControl:y:
     *
     * The Y position of the control.
     */
    properties[PROP_Y] =
        g_param_spec_float ("y",
                            "Y",
                            "Y position",
                            -G_MAXFLOAT, G_MAXFLOAT, 0.0f,
                            G_PARAM_READWRITE |
                            G_PARAM_STATIC_STRINGS |
                            G_PARAM_EXPLICIT_NOTIFY);

    /**
     * GrlUiControl:width:
     *
     * The width of the control.
     */
    properties[PROP_WIDTH] =
        g_param_spec_float ("width",
                            "Width",
                            "Control width",
                            0.0f, G_MAXFLOAT, 100.0f,
                            G_PARAM_READWRITE |
                            G_PARAM_STATIC_STRINGS |
                            G_PARAM_EXPLICIT_NOTIFY);

    /**
     * GrlUiControl:height:
     *
     * The height of the control.
     */
    properties[PROP_HEIGHT] =
        g_param_spec_float ("height",
                            "Height",
                            "Control height",
                            0.0f, G_MAXFLOAT, 30.0f,
                            G_PARAM_READWRITE |
                            G_PARAM_STATIC_STRINGS |
                            G_PARAM_EXPLICIT_NOTIFY);

    /**
     * GrlUiControl:enabled:
     *
     * Whether the control accepts input.
     */
    properties[PROP_ENABLED] =
        g_param_spec_boolean ("enabled",
                              "Enabled",
                              "Whether the control accepts input",
                              TRUE,
                              G_PARAM_READWRITE |
                              G_PARAM_STATIC_STRINGS |
                              G_PARAM_EXPLICIT_NOTIFY);

    /**
     * GrlUiControl:visible:
     *
     * Whether the control is drawn.
     */
    properties[PROP_VISIBLE] =
        g_param_spec_boolean ("visible",
                              "Visible",
                              "Whether the control is drawn",
                              TRUE,
                              G_PARAM_READWRITE |
                              G_PARAM_STATIC_STRINGS |
                              G_PARAM_EXPLICIT_NOTIFY);

    /**
     * GrlUiControl:state:
     *
     * The current visual state of the control.
     */
    properties[PROP_STATE] =
        g_param_spec_enum ("state",
                           "State",
                           "The current visual state",
                           GRL_TYPE_UI_STATE,
                           GRL_UI_STATE_NORMAL,
                           G_PARAM_READABLE |
                           G_PARAM_STATIC_STRINGS);

    /**
     * GrlUiControl:tooltip:
     *
     * The tooltip text for the control.
     */
    properties[PROP_TOOLTIP] =
        g_param_spec_string ("tooltip",
                             "Tooltip",
                             "The tooltip text",
                             NULL,
                             G_PARAM_READWRITE |
                             G_PARAM_STATIC_STRINGS |
                             G_PARAM_EXPLICIT_NOTIFY);

    g_object_class_install_properties (object_class, N_PROPS, properties);
}

static void
grl_ui_control_init (GrlUiControl *self)
{
    GrlUiControlPrivate *priv = grl_ui_control_get_instance_private (self);

    priv->bounds = grl_rectangle_new (0.0f, 0.0f, 100.0f, 30.0f);
    priv->enabled = TRUE;
    priv->visible = TRUE;
    priv->state = GRL_UI_STATE_NORMAL;
    priv->tooltip = NULL;
}

/**
 * grl_ui_control_get_bounds:
 * @self: A #GrlUiControl
 *
 * Gets the bounds (position and size) of the control.
 *
 * Returns: (transfer full): The control bounds as a #GrlRectangle
 */
GrlRectangle *
grl_ui_control_get_bounds (GrlUiControl *self)
{
    GrlUiControlPrivate *priv;

    g_return_val_if_fail (GRL_IS_UI_CONTROL (self), NULL);

    priv = grl_ui_control_get_instance_private (self);
    return grl_rectangle_copy (priv->bounds);
}

/**
 * grl_ui_control_set_bounds:
 * @self: A #GrlUiControl
 * @bounds: The new bounds
 *
 * Sets the bounds (position and size) of the control.
 */
void
grl_ui_control_set_bounds (GrlUiControl *self,
                           GrlRectangle *bounds)
{
    GrlUiControlPrivate *priv;

    g_return_if_fail (GRL_IS_UI_CONTROL (self));
    g_return_if_fail (bounds != NULL);

    priv = grl_ui_control_get_instance_private (self);

    if (priv->bounds != NULL)
        grl_rectangle_free (priv->bounds);

    priv->bounds = grl_rectangle_copy (bounds);

    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_BOUNDS]);
    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_X]);
    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_Y]);
    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_WIDTH]);
    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_HEIGHT]);
}

/**
 * grl_ui_control_get_x:
 * @self: A #GrlUiControl
 *
 * Gets the X position of the control.
 *
 * Returns: The X position
 */
gfloat
grl_ui_control_get_x (GrlUiControl *self)
{
    GrlUiControlPrivate *priv;

    g_return_val_if_fail (GRL_IS_UI_CONTROL (self), 0.0f);

    priv = grl_ui_control_get_instance_private (self);
    return priv->bounds ? priv->bounds->x : 0.0f;
}

/**
 * grl_ui_control_set_x:
 * @self: A #GrlUiControl
 * @x: The new X position
 *
 * Sets the X position of the control.
 */
void
grl_ui_control_set_x (GrlUiControl *self,
                      gfloat        x)
{
    GrlUiControlPrivate *priv;

    g_return_if_fail (GRL_IS_UI_CONTROL (self));

    priv = grl_ui_control_get_instance_private (self);

    if (priv->bounds == NULL)
        priv->bounds = grl_rectangle_new (x, 0.0f, 100.0f, 30.0f);
    else if (priv->bounds->x != x)
    {
        priv->bounds->x = x;
        g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_X]);
        g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_BOUNDS]);
    }
}

/**
 * grl_ui_control_get_y:
 * @self: A #GrlUiControl
 *
 * Gets the Y position of the control.
 *
 * Returns: The Y position
 */
gfloat
grl_ui_control_get_y (GrlUiControl *self)
{
    GrlUiControlPrivate *priv;

    g_return_val_if_fail (GRL_IS_UI_CONTROL (self), 0.0f);

    priv = grl_ui_control_get_instance_private (self);
    return priv->bounds ? priv->bounds->y : 0.0f;
}

/**
 * grl_ui_control_set_y:
 * @self: A #GrlUiControl
 * @y: The new Y position
 *
 * Sets the Y position of the control.
 */
void
grl_ui_control_set_y (GrlUiControl *self,
                      gfloat        y)
{
    GrlUiControlPrivate *priv;

    g_return_if_fail (GRL_IS_UI_CONTROL (self));

    priv = grl_ui_control_get_instance_private (self);

    if (priv->bounds == NULL)
        priv->bounds = grl_rectangle_new (0.0f, y, 100.0f, 30.0f);
    else if (priv->bounds->y != y)
    {
        priv->bounds->y = y;
        g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_Y]);
        g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_BOUNDS]);
    }
}

/**
 * grl_ui_control_get_width:
 * @self: A #GrlUiControl
 *
 * Gets the width of the control.
 *
 * Returns: The width
 */
gfloat
grl_ui_control_get_width (GrlUiControl *self)
{
    GrlUiControlPrivate *priv;

    g_return_val_if_fail (GRL_IS_UI_CONTROL (self), 0.0f);

    priv = grl_ui_control_get_instance_private (self);
    return priv->bounds ? priv->bounds->width : 0.0f;
}

/**
 * grl_ui_control_set_width:
 * @self: A #GrlUiControl
 * @width: The new width
 *
 * Sets the width of the control.
 */
void
grl_ui_control_set_width (GrlUiControl *self,
                          gfloat        width)
{
    GrlUiControlPrivate *priv;

    g_return_if_fail (GRL_IS_UI_CONTROL (self));
    g_return_if_fail (width >= 0.0f);

    priv = grl_ui_control_get_instance_private (self);

    if (priv->bounds == NULL)
        priv->bounds = grl_rectangle_new (0.0f, 0.0f, width, 30.0f);
    else if (priv->bounds->width != width)
    {
        priv->bounds->width = width;
        g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_WIDTH]);
        g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_BOUNDS]);
    }
}

/**
 * grl_ui_control_get_height:
 * @self: A #GrlUiControl
 *
 * Gets the height of the control.
 *
 * Returns: The height
 */
gfloat
grl_ui_control_get_height (GrlUiControl *self)
{
    GrlUiControlPrivate *priv;

    g_return_val_if_fail (GRL_IS_UI_CONTROL (self), 0.0f);

    priv = grl_ui_control_get_instance_private (self);
    return priv->bounds ? priv->bounds->height : 0.0f;
}

/**
 * grl_ui_control_set_height:
 * @self: A #GrlUiControl
 * @height: The new height
 *
 * Sets the height of the control.
 */
void
grl_ui_control_set_height (GrlUiControl *self,
                           gfloat        height)
{
    GrlUiControlPrivate *priv;

    g_return_if_fail (GRL_IS_UI_CONTROL (self));
    g_return_if_fail (height >= 0.0f);

    priv = grl_ui_control_get_instance_private (self);

    if (priv->bounds == NULL)
        priv->bounds = grl_rectangle_new (0.0f, 0.0f, 100.0f, height);
    else if (priv->bounds->height != height)
    {
        priv->bounds->height = height;
        g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_HEIGHT]);
        g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_BOUNDS]);
    }
}

/**
 * grl_ui_control_get_enabled:
 * @self: A #GrlUiControl
 *
 * Gets whether the control is enabled (accepts input).
 *
 * Returns: %TRUE if the control is enabled
 */
gboolean
grl_ui_control_get_enabled (GrlUiControl *self)
{
    GrlUiControlPrivate *priv;

    g_return_val_if_fail (GRL_IS_UI_CONTROL (self), FALSE);

    priv = grl_ui_control_get_instance_private (self);
    return priv->enabled;
}

/**
 * grl_ui_control_set_enabled:
 * @self: A #GrlUiControl
 * @enabled: Whether the control should be enabled
 *
 * Sets whether the control is enabled (accepts input).
 */
void
grl_ui_control_set_enabled (GrlUiControl *self,
                            gboolean      enabled)
{
    GrlUiControlPrivate *priv;

    g_return_if_fail (GRL_IS_UI_CONTROL (self));

    priv = grl_ui_control_get_instance_private (self);

    if (priv->enabled != enabled)
    {
        priv->enabled = enabled;

        /* Update state when disabled */
        if (!enabled)
            priv->state = GRL_UI_STATE_DISABLED;
        else if (priv->state == GRL_UI_STATE_DISABLED)
            priv->state = GRL_UI_STATE_NORMAL;

        g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_ENABLED]);
        g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_STATE]);
    }
}

/**
 * grl_ui_control_get_visible:
 * @self: A #GrlUiControl
 *
 * Gets whether the control is visible (is drawn).
 *
 * Returns: %TRUE if the control is visible
 */
gboolean
grl_ui_control_get_visible (GrlUiControl *self)
{
    GrlUiControlPrivate *priv;

    g_return_val_if_fail (GRL_IS_UI_CONTROL (self), FALSE);

    priv = grl_ui_control_get_instance_private (self);
    return priv->visible;
}

/**
 * grl_ui_control_set_visible:
 * @self: A #GrlUiControl
 * @visible: Whether the control should be visible
 *
 * Sets whether the control is visible (is drawn).
 */
void
grl_ui_control_set_visible (GrlUiControl *self,
                            gboolean      visible)
{
    GrlUiControlPrivate *priv;

    g_return_if_fail (GRL_IS_UI_CONTROL (self));

    priv = grl_ui_control_get_instance_private (self);

    if (priv->visible != visible)
    {
        priv->visible = visible;
        g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_VISIBLE]);
    }
}

/**
 * grl_ui_control_get_state:
 * @self: A #GrlUiControl
 *
 * Gets the current visual state of the control.
 *
 * Returns: The current #GrlUiState
 */
GrlUiState
grl_ui_control_get_state (GrlUiControl *self)
{
    GrlUiControlPrivate *priv;

    g_return_val_if_fail (GRL_IS_UI_CONTROL (self), GRL_UI_STATE_NORMAL);

    priv = grl_ui_control_get_instance_private (self);
    return priv->state;
}

/**
 * grl_ui_control_get_tooltip:
 * @self: A #GrlUiControl
 *
 * Gets the tooltip text for the control.
 *
 * Returns: (nullable) (transfer none): The tooltip text, or %NULL
 */
const gchar *
grl_ui_control_get_tooltip (GrlUiControl *self)
{
    GrlUiControlPrivate *priv;

    g_return_val_if_fail (GRL_IS_UI_CONTROL (self), NULL);

    priv = grl_ui_control_get_instance_private (self);
    return priv->tooltip;
}

/**
 * grl_ui_control_set_tooltip:
 * @self: A #GrlUiControl
 * @tooltip: (nullable): The tooltip text, or %NULL
 *
 * Sets the tooltip text for the control.
 */
void
grl_ui_control_set_tooltip (GrlUiControl *self,
                            const gchar  *tooltip)
{
    GrlUiControlPrivate *priv;

    g_return_if_fail (GRL_IS_UI_CONTROL (self));

    priv = grl_ui_control_get_instance_private (self);

    if (g_strcmp0 (priv->tooltip, tooltip) != 0)
    {
        g_free (priv->tooltip);
        priv->tooltip = g_strdup (tooltip);
        g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_TOOLTIP]);
    }
}

/**
 * grl_ui_control_draw:
 * @self: A #GrlUiControl
 *
 * Draws the control. This calls the virtual draw() method
 * if the control is visible.
 */
void
grl_ui_control_draw (GrlUiControl *self)
{
    GrlUiControlPrivate *priv;
    GrlUiControlClass   *klass;

    g_return_if_fail (GRL_IS_UI_CONTROL (self));

    priv = grl_ui_control_get_instance_private (self);

    if (!priv->visible)
        return;

    klass = GRL_UI_CONTROL_GET_CLASS (self);
    if (klass->draw != NULL)
        klass->draw (self);
}

/**
 * grl_ui_control_handle_input:
 * @self: A #GrlUiControl
 *
 * Processes input for the control. This calls the virtual
 * handle_input() method if the control is enabled.
 *
 * Returns: %TRUE if the input was handled
 */
gboolean
grl_ui_control_handle_input (GrlUiControl *self)
{
    GrlUiControlPrivate *priv;
    GrlUiControlClass   *klass;

    g_return_val_if_fail (GRL_IS_UI_CONTROL (self), FALSE);

    priv = grl_ui_control_get_instance_private (self);

    if (!priv->enabled)
        return FALSE;

    klass = GRL_UI_CONTROL_GET_CLASS (self);
    if (klass->handle_input != NULL)
        return klass->handle_input (self);

    return FALSE;
}

/**
 * grl_ui_control_get_preferred_size:
 * @self: A #GrlUiControl
 * @width: (out) (nullable): Return location for preferred width
 * @height: (out) (nullable): Return location for preferred height
 *
 * Gets the preferred size of the control based on its content.
 */
void
grl_ui_control_get_preferred_size (GrlUiControl *self,
                                   gfloat       *width,
                                   gfloat       *height)
{
    GrlUiControlClass *klass;

    g_return_if_fail (GRL_IS_UI_CONTROL (self));

    klass = GRL_UI_CONTROL_GET_CLASS (self);
    if (klass->get_preferred_size != NULL)
        klass->get_preferred_size (self, width, height);
}

/**
 * grl_ui_control_contains_point:
 * @self: A #GrlUiControl
 * @x: X coordinate to test
 * @y: Y coordinate to test
 *
 * Tests whether a point is within the control's bounds.
 *
 * Returns: %TRUE if the point is inside the control
 */
gboolean
grl_ui_control_contains_point (GrlUiControl *self,
                               gfloat        x,
                               gfloat        y)
{
    GrlUiControlPrivate *priv;

    g_return_val_if_fail (GRL_IS_UI_CONTROL (self), FALSE);

    priv = grl_ui_control_get_instance_private (self);

    if (priv->bounds == NULL)
        return FALSE;

    return (x >= priv->bounds->x &&
            x < priv->bounds->x + priv->bounds->width &&
            y >= priv->bounds->y &&
            y < priv->bounds->y + priv->bounds->height);
}

/*
 * Private API for internal UI control use
 */

/* Global edit mode tracking */
GrlUiControl *grl_ui_current_edit_control = NULL;

/**
 * grl_ui_control_set_editing:
 * @self: (nullable): A #GrlUiControl, or %NULL to clear
 * @editing: Whether the control is entering edit mode
 *
 * Sets or clears the global edit mode for a control.
 */
void
grl_ui_control_set_editing (GrlUiControl *self,
                            gboolean      editing)
{
    if (editing)
    {
        /* Exit current edit control if different */
        if (grl_ui_current_edit_control != NULL &&
            grl_ui_current_edit_control != self)
        {
            /* The control being replaced should handle its own cleanup
             * by checking grl_ui_control_is_editing() */
            grl_ui_current_edit_control = NULL;
        }

        grl_ui_current_edit_control = self;
    }
    else
    {
        if (grl_ui_current_edit_control == self)
            grl_ui_current_edit_control = NULL;
    }
}

/**
 * grl_ui_control_is_editing:
 * @self: A #GrlUiControl
 *
 * Checks if this control is currently in edit mode.
 *
 * Returns: %TRUE if @self is the current edit control
 */
gboolean
grl_ui_control_is_editing (GrlUiControl *self)
{
    g_return_val_if_fail (self == NULL || GRL_IS_UI_CONTROL (self), FALSE);

    return (grl_ui_current_edit_control == self);
}

/**
 * grl_ui_control_get_current_edit:
 *
 * Gets the control currently in edit mode, if any.
 *
 * Returns: (nullable) (transfer none): The current edit control, or %NULL
 */
GrlUiControl *
grl_ui_control_get_current_edit (void)
{
    return grl_ui_current_edit_control;
}

/**
 * grl_ui_control_clear_edit_mode:
 *
 * Clears the current edit mode without notifying the control.
 */
void
grl_ui_control_clear_edit_mode (void)
{
    grl_ui_current_edit_control = NULL;
}

/**
 * grl_ui_control_update_state:
 * @self: A #GrlUiControl
 *
 * Updates the control's visual state based on mouse position and input.
 */
void
grl_ui_control_update_state (GrlUiControl *self)
{
    GrlUiControlPrivate *priv;
    GrlUiState           old_state;
    GrlUiState           new_state;
    Vector2              mouse_pos;
    gboolean             is_hover;

    g_return_if_fail (GRL_IS_UI_CONTROL (self));

    priv = grl_ui_control_get_instance_private (self);
    old_state = priv->state;

    if (!priv->enabled)
    {
        new_state = GRL_UI_STATE_DISABLED;
    }
    else
    {
        mouse_pos = GetMousePosition ();
        is_hover = grl_ui_control_contains_point (self, mouse_pos.x, mouse_pos.y);

        if (is_hover)
        {
            unsigned char raw = IsMouseButtonDown (MOUSE_LEFT_BUTTON);
            if (raw != 0)
                new_state = GRL_UI_STATE_PRESSED;
            else
                new_state = GRL_UI_STATE_FOCUSED;
        }
        else
        {
            new_state = GRL_UI_STATE_NORMAL;
        }
    }

    if (new_state != old_state)
    {
        priv->state = new_state;
        g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_STATE]);
    }
}

/**
 * grl_ui_control_set_state:
 * @self: A #GrlUiControl
 * @state: The new state
 *
 * Directly sets the control's visual state.
 */
void
grl_ui_control_set_state (GrlUiControl *self,
                          GrlUiState    state)
{
    GrlUiControlPrivate *priv;

    g_return_if_fail (GRL_IS_UI_CONTROL (self));

    priv = grl_ui_control_get_instance_private (self);

    if (priv->state != state)
    {
        priv->state = state;
        g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_STATE]);
    }
}
