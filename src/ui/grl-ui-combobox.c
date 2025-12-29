/* grl-ui-combobox.c
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#include "config.h"
#include "grl-ui-combobox.h"
#include "../math/grl-rectangle.h"

#define RAYGUI_STATIC
#include <raygui.h>

struct _GrlUiComboBox
{
    GrlUiControl  parent_instance;

    gchar        *items;
    gint          active;
    gint          item_count;
};

G_DEFINE_TYPE (GrlUiComboBox, grl_ui_combobox, GRL_TYPE_UI_CONTROL)

enum
{
    PROP_0,
    PROP_ITEMS,
    PROP_ACTIVE,
    N_PROPS
};

static GParamSpec *properties[N_PROPS];

enum
{
    SIGNAL_SELECTION_CHANGED,
    N_SIGNALS
};

static guint signals[N_SIGNALS];

/*
 * Helper function to count items
 */
static gint
count_items (const gchar *items)
{
    gint count;
    const gchar *p;

    if (items == NULL || items[0] == '\0')
        return 0;

    count = 1;  /* At least one item if string is not empty */
    for (p = items; *p != '\0'; p++)
    {
        if (*p == ';')
            count++;
    }

    return count;
}

/*
 * Virtual method implementations
 */

static void
grl_ui_combobox_draw_impl (GrlUiControl *control)
{
    GrlUiComboBox    *self;
    GrlRectangle     *bounds;
    Rectangle         raygui_bounds;
    gboolean          enabled;
    gboolean          visible;
    gint              old_active;
    int               active;

    self = GRL_UI_COMBOBOX (control);

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

    old_active = self->active;
    active = self->active;

    GuiComboBox (raygui_bounds,
                 self->items != NULL ? self->items : "",
                 &active);

    if (!enabled)
        GuiSetState (STATE_NORMAL);

    /* Check if selection changed */
    if (active != old_active)
    {
        self->active = active;
        g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_ACTIVE]);
        g_signal_emit (self, signals[SIGNAL_SELECTION_CHANGED], 0, self->active);
    }
}

static void
grl_ui_combobox_get_preferred_size_impl (GrlUiControl *control,
                                         gfloat       *width,
                                         gfloat       *height)
{
    GrlUiComboBox *self;
    int            max_width;
    int            text_width;
    gchar        **items_array;
    gint           i;

    self = GRL_UI_COMBOBOX (control);
    max_width = 80;  /* Default minimum */

    /* Find the widest item */
    if (self->items != NULL && self->items[0] != '\0')
    {
        items_array = g_strsplit (self->items, ";", -1);
        for (i = 0; items_array[i] != NULL; i++)
        {
            text_width = GuiGetTextWidth (items_array[i]);
            if (text_width > max_width)
                max_width = text_width;
        }
        g_strfreev (items_array);
    }

    *width = (gfloat)(max_width + 40);  /* Add space for dropdown button */
    *height = 24.0f;  /* Default height */
}

/*
 * GObject implementation
 */

static void
grl_ui_combobox_finalize (GObject *object)
{
    GrlUiComboBox *self = GRL_UI_COMBOBOX (object);

    g_clear_pointer (&self->items, g_free);

    G_OBJECT_CLASS (grl_ui_combobox_parent_class)->finalize (object);
}

static void
grl_ui_combobox_get_property (GObject    *object,
                              guint       prop_id,
                              GValue     *value,
                              GParamSpec *pspec)
{
    GrlUiComboBox *self = GRL_UI_COMBOBOX (object);

    switch (prop_id)
    {
    case PROP_ITEMS:
        g_value_set_string (value, self->items);
        break;
    case PROP_ACTIVE:
        g_value_set_int (value, self->active);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
grl_ui_combobox_set_property (GObject      *object,
                              guint         prop_id,
                              const GValue *value,
                              GParamSpec   *pspec)
{
    GrlUiComboBox *self = GRL_UI_COMBOBOX (object);

    switch (prop_id)
    {
    case PROP_ITEMS:
        grl_ui_combobox_set_items (self, g_value_get_string (value));
        break;
    case PROP_ACTIVE:
        grl_ui_combobox_set_active (self, g_value_get_int (value));
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
grl_ui_combobox_class_init (GrlUiComboBoxClass *klass)
{
    GObjectClass      *object_class;
    GrlUiControlClass *control_class;

    object_class = G_OBJECT_CLASS (klass);
    control_class = GRL_UI_CONTROL_CLASS (klass);

    object_class->finalize = grl_ui_combobox_finalize;
    object_class->get_property = grl_ui_combobox_get_property;
    object_class->set_property = grl_ui_combobox_set_property;

    control_class->draw = grl_ui_combobox_draw_impl;
    control_class->get_preferred_size = grl_ui_combobox_get_preferred_size_impl;

    /**
     * GrlUiComboBox:items:
     *
     * The semicolon-separated list of items.
     */
    properties[PROP_ITEMS] =
        g_param_spec_string ("items",
                             "Items",
                             "Semicolon-separated list of items",
                             NULL,
                             G_PARAM_READWRITE |
                             G_PARAM_STATIC_STRINGS |
                             G_PARAM_EXPLICIT_NOTIFY);

    /**
     * GrlUiComboBox:active:
     *
     * The index of the currently selected item.
     */
    properties[PROP_ACTIVE] =
        g_param_spec_int ("active",
                          "Active",
                          "The index of the currently selected item",
                          0, G_MAXINT, 0,
                          G_PARAM_READWRITE |
                          G_PARAM_STATIC_STRINGS |
                          G_PARAM_EXPLICIT_NOTIFY);

    g_object_class_install_properties (object_class, N_PROPS, properties);

    /**
     * GrlUiComboBox::selection-changed:
     * @self: The combo box
     * @active: The new active item index
     *
     * Emitted when the selection changes.
     */
    signals[SIGNAL_SELECTION_CHANGED] =
        g_signal_new ("selection-changed",
                      G_TYPE_FROM_CLASS (klass),
                      G_SIGNAL_RUN_LAST,
                      0,
                      NULL, NULL,
                      NULL,
                      G_TYPE_NONE,
                      1,
                      G_TYPE_INT);
}

static void
grl_ui_combobox_init (GrlUiComboBox *self)
{
    self->items = NULL;
    self->active = 0;
    self->item_count = 0;
}

/*
 * Public API
 */

GrlUiComboBox *
grl_ui_combobox_new (const gchar *items)
{
    return g_object_new (GRL_TYPE_UI_COMBOBOX,
                         "items", items,
                         NULL);
}

GrlUiComboBox *
grl_ui_combobox_new_with_bounds (gfloat       x,
                                 gfloat       y,
                                 gfloat       width,
                                 gfloat       height,
                                 const gchar *items)
{
    g_autoptr(GrlRectangle) bounds = NULL;

    bounds = grl_rectangle_new (x, y, width, height);

    return g_object_new (GRL_TYPE_UI_COMBOBOX,
                         "bounds", bounds,
                         "items", items,
                         NULL);
}

const gchar *
grl_ui_combobox_get_items (GrlUiComboBox *self)
{
    g_return_val_if_fail (GRL_IS_UI_COMBOBOX (self), NULL);

    return self->items;
}

void
grl_ui_combobox_set_items (GrlUiComboBox *self,
                           const gchar   *items)
{
    g_return_if_fail (GRL_IS_UI_COMBOBOX (self));

    if (g_strcmp0 (self->items, items) == 0)
        return;

    g_free (self->items);
    self->items = g_strdup (items);
    self->item_count = count_items (items);

    /* Reset active if out of range */
    if (self->active >= self->item_count)
    {
        self->active = self->item_count > 0 ? self->item_count - 1 : 0;
        g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_ACTIVE]);
    }

    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_ITEMS]);
}

gint
grl_ui_combobox_get_active (GrlUiComboBox *self)
{
    g_return_val_if_fail (GRL_IS_UI_COMBOBOX (self), 0);

    return self->active;
}

void
grl_ui_combobox_set_active (GrlUiComboBox *self,
                            gint           active)
{
    g_return_if_fail (GRL_IS_UI_COMBOBOX (self));

    if (active < 0)
        active = 0;
    else if (self->item_count > 0 && active >= self->item_count)
        active = self->item_count - 1;

    if (self->active == active)
        return;

    self->active = active;

    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_ACTIVE]);
}

gint
grl_ui_combobox_get_item_count (GrlUiComboBox *self)
{
    g_return_val_if_fail (GRL_IS_UI_COMBOBOX (self), 0);

    return self->item_count;
}

gchar *
grl_ui_combobox_get_active_text (GrlUiComboBox *self)
{
    gchar **items_array;
    gchar  *result;

    g_return_val_if_fail (GRL_IS_UI_COMBOBOX (self), NULL);

    if (self->items == NULL || self->items[0] == '\0')
        return NULL;

    items_array = g_strsplit (self->items, ";", -1);
    if (self->active >= 0 && items_array[self->active] != NULL)
        result = g_strdup (items_array[self->active]);
    else
        result = NULL;

    g_strfreev (items_array);
    return result;
}
