/* grl-ui-style.c
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * UI style manager implementation.
 */

#include "config.h"
#include "grl-ui-style.h"
#include <raygui.h>

/**
 * SECTION:grl-ui-style
 * @title: GrlUiStyle
 * @short_description: UI style manager for theming controls
 *
 * #GrlUiStyle is a singleton that manages the visual appearance of all
 * UI controls. It wraps the raygui style system and provides methods
 * for loading style files, setting individual style properties, and
 * controlling global UI state.
 *
 * Use grl_ui_style_get_default() to get the singleton instance.
 *
 * ## Style Files
 *
 * Style files (.rgs format) can define colors, fonts, and other visual
 * properties for all control types. They can be created using the
 * rGuiStyler tool from the raygui project.
 *
 * ## Example
 *
 * |[<!-- language="C" -->
 * GrlUiStyle *style = grl_ui_style_get_default ();
 *
 * // Load a custom style
 * grl_ui_style_load_file (style, "dark_theme.rgs", NULL);
 *
 * // Or modify individual properties
 * grl_ui_style_set_property (style, GRL_UI_CONTROL_BUTTON,
 *                            GRL_UI_PROP_BASE_COLOR_NORMAL, 0x404040FF);
 *
 * // Set global transparency
 * grl_ui_style_set_alpha (style, 0.9f);
 * ]|
 */

struct _GrlUiStyle
{
    GObject parent_instance;

    gfloat  alpha;
};

G_DEFINE_TYPE (GrlUiStyle, grl_ui_style, G_TYPE_OBJECT)

static GrlUiStyle *default_style = NULL;

enum
{
    PROP_0,
    PROP_ALPHA,
    PROP_LOCKED,
    N_PROPS
};

static GParamSpec *properties[N_PROPS];

static void
grl_ui_style_get_property (GObject    *object,
                           guint       prop_id,
                           GValue     *value,
                           GParamSpec *pspec)
{
    GrlUiStyle *self = GRL_UI_STYLE (object);

    switch (prop_id)
    {
    case PROP_ALPHA:
        g_value_set_float (value, self->alpha);
        break;

    case PROP_LOCKED:
        {
            unsigned char raw = GuiIsLocked ();
            g_value_set_boolean (value, raw != 0);
        }
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
grl_ui_style_set_property (GObject      *object,
                           guint         prop_id,
                           const GValue *value,
                           GParamSpec   *pspec)
{
    GrlUiStyle *self = GRL_UI_STYLE (object);

    switch (prop_id)
    {
    case PROP_ALPHA:
        grl_ui_style_set_alpha (self, g_value_get_float (value));
        break;

    case PROP_LOCKED:
        if (g_value_get_boolean (value))
            grl_ui_style_lock (self);
        else
            grl_ui_style_unlock (self);
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
grl_ui_style_class_init (GrlUiStyleClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->get_property = grl_ui_style_get_property;
    object_class->set_property = grl_ui_style_set_property;

    /**
     * GrlUiStyle:alpha:
     *
     * Global alpha (transparency) for all UI controls.
     * Range is 0.0 (transparent) to 1.0 (opaque).
     */
    properties[PROP_ALPHA] =
        g_param_spec_float ("alpha",
                            "Alpha",
                            "Global transparency for UI controls",
                            0.0f, 1.0f, 1.0f,
                            G_PARAM_READWRITE |
                            G_PARAM_STATIC_STRINGS);

    /**
     * GrlUiStyle:locked:
     *
     * Whether the UI is locked (not responding to input).
     */
    properties[PROP_LOCKED] =
        g_param_spec_boolean ("locked",
                              "Locked",
                              "Whether the UI is locked",
                              FALSE,
                              G_PARAM_READWRITE |
                              G_PARAM_STATIC_STRINGS);

    g_object_class_install_properties (object_class, N_PROPS, properties);
}

static void
grl_ui_style_init (GrlUiStyle *self)
{
    self->alpha = 1.0f;
}

/**
 * grl_ui_style_get_default:
 *
 * Gets the default UI style singleton instance.
 *
 * Returns: (transfer none): The default #GrlUiStyle instance
 */
GrlUiStyle *
grl_ui_style_get_default (void)
{
    if (default_style == NULL)
    {
        default_style = g_object_new (GRL_TYPE_UI_STYLE, NULL);
    }

    return default_style;
}

/**
 * grl_ui_style_load_file:
 * @self: A #GrlUiStyle
 * @filename: (type filename): Path to a .rgs style file
 * @error: (nullable): Return location for error, or %NULL
 *
 * Loads a raygui style file (.rgs format).
 *
 * Returns: %TRUE on success, %FALSE on error
 */
gboolean
grl_ui_style_load_file (GrlUiStyle   *self,
                        const gchar  *filename,
                        GError      **error)
{
    g_return_val_if_fail (GRL_IS_UI_STYLE (self), FALSE);
    g_return_val_if_fail (filename != NULL, FALSE);
    g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

    if (!g_file_test (filename, G_FILE_TEST_EXISTS))
    {
        g_set_error (error,
                     G_FILE_ERROR,
                     G_FILE_ERROR_NOENT,
                     "Style file not found: %s",
                     filename);
        return FALSE;
    }

    GuiLoadStyle (filename);

    return TRUE;
}

/**
 * grl_ui_style_load_default:
 * @self: A #GrlUiStyle
 *
 * Resets the style to the default raygui theme.
 */
void
grl_ui_style_load_default (GrlUiStyle *self)
{
    g_return_if_fail (GRL_IS_UI_STYLE (self));

    GuiLoadStyleDefault ();
}

/**
 * grl_ui_style_set_control_property:
 * @self: A #GrlUiStyle
 * @control: The control type
 * @property: The property to set
 * @value: The value to set
 *
 * Sets a style property for a specific control type.
 */
void
grl_ui_style_set_control_property (GrlUiStyle *self,
                                   gint        control,
                                   gint        property,
                                   gint        value)
{
    g_return_if_fail (GRL_IS_UI_STYLE (self));

    GuiSetStyle (control, property, value);
}

/**
 * grl_ui_style_get_control_property:
 * @self: A #GrlUiStyle
 * @control: The control type
 * @property: The property to get
 *
 * Gets a style property for a specific control type.
 *
 * Returns: The property value
 */
gint
grl_ui_style_get_control_property (GrlUiStyle *self,
                                   gint        control,
                                   gint        property)
{
    g_return_val_if_fail (GRL_IS_UI_STYLE (self), 0);

    return GuiGetStyle (control, property);
}

/**
 * grl_ui_style_set_font:
 * @self: A #GrlUiStyle
 * @font: (nullable): The font to use, or %NULL for default
 *
 * Sets the font used for all UI controls.
 *
 * Note: The font pointer should be a raylib Font structure.
 * This is a low-level function; prefer using GrlFont when possible.
 */
void
grl_ui_style_set_font (GrlUiStyle *self,
                       gpointer    font)
{
    g_return_if_fail (GRL_IS_UI_STYLE (self));

    if (font != NULL)
    {
        GuiSetFont (*(Font *)font);
    }
    else
    {
        GuiLoadStyleDefault ();
    }
}

/**
 * grl_ui_style_enable:
 * @self: A #GrlUiStyle
 *
 * Enables the UI state (controls respond to input).
 */
void
grl_ui_style_enable (GrlUiStyle *self)
{
    g_return_if_fail (GRL_IS_UI_STYLE (self));

    GuiEnable ();
}

/**
 * grl_ui_style_disable:
 * @self: A #GrlUiStyle
 *
 * Disables the UI state (controls don't respond to input).
 */
void
grl_ui_style_disable (GrlUiStyle *self)
{
    g_return_if_fail (GRL_IS_UI_STYLE (self));

    GuiDisable ();
}

/**
 * grl_ui_style_lock:
 * @self: A #GrlUiStyle
 *
 * Locks the UI from processing input.
 */
void
grl_ui_style_lock (GrlUiStyle *self)
{
    g_return_if_fail (GRL_IS_UI_STYLE (self));

    GuiLock ();
    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_LOCKED]);
}

/**
 * grl_ui_style_unlock:
 * @self: A #GrlUiStyle
 *
 * Unlocks the UI to resume processing input.
 */
void
grl_ui_style_unlock (GrlUiStyle *self)
{
    g_return_if_fail (GRL_IS_UI_STYLE (self));

    GuiUnlock ();
    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_LOCKED]);
}

/**
 * grl_ui_style_is_locked:
 * @self: A #GrlUiStyle
 *
 * Checks if the UI is locked.
 *
 * Returns: %TRUE if the UI is locked
 */
gboolean
grl_ui_style_is_locked (GrlUiStyle *self)
{
    unsigned char raw;

    g_return_val_if_fail (GRL_IS_UI_STYLE (self), FALSE);

    raw = GuiIsLocked ();
    return raw != 0;
}

/**
 * grl_ui_style_set_alpha:
 * @self: A #GrlUiStyle
 * @alpha: Alpha value (0.0 = transparent, 1.0 = opaque)
 *
 * Sets the global transparency for all UI controls.
 */
void
grl_ui_style_set_alpha (GrlUiStyle *self,
                        gfloat      alpha)
{
    g_return_if_fail (GRL_IS_UI_STYLE (self));

    alpha = CLAMP (alpha, 0.0f, 1.0f);

    if (self->alpha != alpha)
    {
        self->alpha = alpha;
        GuiSetAlpha (alpha);
        g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_ALPHA]);
    }
}

/**
 * grl_ui_style_get_alpha:
 * @self: A #GrlUiStyle
 *
 * Gets the global transparency for UI controls.
 *
 * Returns: The current alpha value
 */
gfloat
grl_ui_style_get_alpha (GrlUiStyle *self)
{
    g_return_val_if_fail (GRL_IS_UI_STYLE (self), 1.0f);

    return self->alpha;
}
