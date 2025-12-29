/* grl-ui-togglegroup.c
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#include "config.h"
#include "grl-ui-togglegroup.h"
#include "../math/grl-rectangle.h"

#define RAYGUI_STATIC
#include <raygui.h>

struct _GrlUiToggleGroup
{
    GrlUiControl  parent_instance;

    gchar        *items;
    gint          active;
    gint          item_count;
};

G_DEFINE_TYPE (GrlUiToggleGroup, grl_ui_togglegroup, GRL_TYPE_UI_CONTROL)

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

    count = 1;
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
grl_ui_togglegroup_draw_impl (GrlUiControl *control)
{
    GrlUiToggleGroup *self;
    GrlRectangle     *bounds;
    Rectangle         raygui_bounds;
    gboolean          enabled;
    gboolean          visible;
    gint              old_active;
    int               active;

    self = GRL_UI_TOGGLEGROUP (control);

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

    GuiToggleGroup (raygui_bounds,
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
grl_ui_togglegroup_get_preferred_size_impl (GrlUiControl *control,
                                            gfloat       *width,
                                            gfloat       *height)
{
    GrlUiToggleGroup *self;
    int               total_width;
    int               text_width;
    int               padding;
    gchar           **items_array;
    gint              i;

    self = GRL_UI_TOGGLEGROUP (control);
    total_width = 0;
    padding = GuiGetStyle (TOGGLE, GROUP_PADDING);

    if (self->items != NULL && self->items[0] != '\0')
    {
        items_array = g_strsplit (self->items, ";", -1);
        for (i = 0; items_array[i] != NULL; i++)
        {
            text_width = GuiGetTextWidth (items_array[i]);
            total_width += text_width + 20;  /* Add padding for each toggle */
            if (i > 0)
                total_width += padding;  /* Add spacing between toggles */
        }
        g_strfreev (items_array);
    }
    else
    {
        total_width = 80;
    }

    *width = (gfloat)total_width;
    *height = 24.0f;
}

/*
 * GObject implementation
 */

static void
grl_ui_togglegroup_finalize (GObject *object)
{
    GrlUiToggleGroup *self = GRL_UI_TOGGLEGROUP (object);

    g_clear_pointer (&self->items, g_free);

    G_OBJECT_CLASS (grl_ui_togglegroup_parent_class)->finalize (object);
}

static void
grl_ui_togglegroup_get_property (GObject    *object,
                                 guint       prop_id,
                                 GValue     *value,
                                 GParamSpec *pspec)
{
    GrlUiToggleGroup *self = GRL_UI_TOGGLEGROUP (object);

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
grl_ui_togglegroup_set_property (GObject      *object,
                                 guint         prop_id,
                                 const GValue *value,
                                 GParamSpec   *pspec)
{
    GrlUiToggleGroup *self = GRL_UI_TOGGLEGROUP (object);

    switch (prop_id)
    {
    case PROP_ITEMS:
        grl_ui_togglegroup_set_items (self, g_value_get_string (value));
        break;
    case PROP_ACTIVE:
        grl_ui_togglegroup_set_active (self, g_value_get_int (value));
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
grl_ui_togglegroup_class_init (GrlUiToggleGroupClass *klass)
{
    GObjectClass      *object_class;
    GrlUiControlClass *control_class;

    object_class = G_OBJECT_CLASS (klass);
    control_class = GRL_UI_CONTROL_CLASS (klass);

    object_class->finalize = grl_ui_togglegroup_finalize;
    object_class->get_property = grl_ui_togglegroup_get_property;
    object_class->set_property = grl_ui_togglegroup_set_property;

    control_class->draw = grl_ui_togglegroup_draw_impl;
    control_class->get_preferred_size = grl_ui_togglegroup_get_preferred_size_impl;

    /**
     * GrlUiToggleGroup:items:
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
     * GrlUiToggleGroup:active:
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
     * GrlUiToggleGroup::selection-changed:
     * @self: The toggle group
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
grl_ui_togglegroup_init (GrlUiToggleGroup *self)
{
    self->items = NULL;
    self->active = 0;
    self->item_count = 0;
}

/*
 * Public API
 */

GrlUiToggleGroup *
grl_ui_togglegroup_new (const gchar *items)
{
    return g_object_new (GRL_TYPE_UI_TOGGLEGROUP,
                         "items", items,
                         NULL);
}

GrlUiToggleGroup *
grl_ui_togglegroup_new_with_bounds (gfloat       x,
                                    gfloat       y,
                                    gfloat       width,
                                    gfloat       height,
                                    const gchar *items)
{
    g_autoptr(GrlRectangle) bounds = NULL;

    bounds = grl_rectangle_new (x, y, width, height);

    return g_object_new (GRL_TYPE_UI_TOGGLEGROUP,
                         "bounds", bounds,
                         "items", items,
                         NULL);
}

const gchar *
grl_ui_togglegroup_get_items (GrlUiToggleGroup *self)
{
    g_return_val_if_fail (GRL_IS_UI_TOGGLEGROUP (self), NULL);

    return self->items;
}

void
grl_ui_togglegroup_set_items (GrlUiToggleGroup *self,
                              const gchar      *items)
{
    g_return_if_fail (GRL_IS_UI_TOGGLEGROUP (self));

    if (g_strcmp0 (self->items, items) == 0)
        return;

    g_free (self->items);
    self->items = g_strdup (items);
    self->item_count = count_items (items);

    if (self->active >= self->item_count)
    {
        self->active = self->item_count > 0 ? self->item_count - 1 : 0;
        g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_ACTIVE]);
    }

    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_ITEMS]);
}

gint
grl_ui_togglegroup_get_active (GrlUiToggleGroup *self)
{
    g_return_val_if_fail (GRL_IS_UI_TOGGLEGROUP (self), 0);

    return self->active;
}

void
grl_ui_togglegroup_set_active (GrlUiToggleGroup *self,
                               gint              active)
{
    g_return_if_fail (GRL_IS_UI_TOGGLEGROUP (self));

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
grl_ui_togglegroup_get_item_count (GrlUiToggleGroup *self)
{
    g_return_val_if_fail (GRL_IS_UI_TOGGLEGROUP (self), 0);

    return self->item_count;
}

gchar *
grl_ui_togglegroup_get_active_text (GrlUiToggleGroup *self)
{
    gchar **items_array;
    gchar  *result;

    g_return_val_if_fail (GRL_IS_UI_TOGGLEGROUP (self), NULL);

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
