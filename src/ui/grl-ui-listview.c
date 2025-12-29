/* grl-ui-listview.c
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#include "config.h"
#include "grl-ui-listview.h"
#include "../math/grl-rectangle.h"

#define RAYGUI_STATIC
#include <raygui.h>

struct _GrlUiListView
{
    GrlUiControl  parent_instance;

    gchar        *items;
    gint          active;
    gint          scroll_index;
    gint          item_count;
};

G_DEFINE_TYPE (GrlUiListView, grl_ui_listview, GRL_TYPE_UI_CONTROL)

enum
{
    PROP_0,
    PROP_ITEMS,
    PROP_ACTIVE,
    PROP_SCROLL_INDEX,
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
    gint         count;
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
grl_ui_listview_draw_impl (GrlUiControl *control)
{
    GrlUiListView *self;
    GrlRectangle  *bounds;
    Rectangle      raygui_bounds;
    gboolean       enabled;
    gboolean       visible;
    gint           old_active;
    gint           old_scroll;
    int            active;
    int            scroll_index;

    self = GRL_UI_LISTVIEW (control);

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
    old_scroll = self->scroll_index;
    active = self->active;
    scroll_index = self->scroll_index;

    GuiListView (raygui_bounds,
                 self->items != NULL ? self->items : "",
                 &scroll_index,
                 &active);

    if (!enabled)
        GuiSetState (STATE_NORMAL);

    /* Check if scroll index changed */
    if (scroll_index != old_scroll)
    {
        self->scroll_index = scroll_index;
        g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_SCROLL_INDEX]);
    }

    /* Check if selection changed */
    if (active != old_active)
    {
        self->active = active;
        g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_ACTIVE]);
        g_signal_emit (self, signals[SIGNAL_SELECTION_CHANGED], 0, self->active);
    }
}

static void
grl_ui_listview_get_preferred_size_impl (GrlUiControl *control,
                                         gfloat       *width,
                                         gfloat       *height)
{
    GrlUiListView *self;
    int            max_width;
    int            text_width;
    gchar        **items_array;
    gint           i;

    self = GRL_UI_LISTVIEW (control);
    max_width = 0;

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

    /* Add padding and scrollbar width */
    *width = (gfloat)(max_width + 40);
    /* Default height for ~5 visible items */
    *height = 120.0f;
}

/*
 * GObject implementation
 */

static void
grl_ui_listview_finalize (GObject *object)
{
    GrlUiListView *self = GRL_UI_LISTVIEW (object);

    g_clear_pointer (&self->items, g_free);

    G_OBJECT_CLASS (grl_ui_listview_parent_class)->finalize (object);
}

static void
grl_ui_listview_get_property (GObject    *object,
                              guint       prop_id,
                              GValue     *value,
                              GParamSpec *pspec)
{
    GrlUiListView *self = GRL_UI_LISTVIEW (object);

    switch (prop_id)
    {
    case PROP_ITEMS:
        g_value_set_string (value, self->items);
        break;
    case PROP_ACTIVE:
        g_value_set_int (value, self->active);
        break;
    case PROP_SCROLL_INDEX:
        g_value_set_int (value, self->scroll_index);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
grl_ui_listview_set_property (GObject      *object,
                              guint         prop_id,
                              const GValue *value,
                              GParamSpec   *pspec)
{
    GrlUiListView *self = GRL_UI_LISTVIEW (object);

    switch (prop_id)
    {
    case PROP_ITEMS:
        grl_ui_listview_set_items (self, g_value_get_string (value));
        break;
    case PROP_ACTIVE:
        grl_ui_listview_set_active (self, g_value_get_int (value));
        break;
    case PROP_SCROLL_INDEX:
        grl_ui_listview_set_scroll_index (self, g_value_get_int (value));
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
grl_ui_listview_class_init (GrlUiListViewClass *klass)
{
    GObjectClass      *object_class;
    GrlUiControlClass *control_class;

    object_class = G_OBJECT_CLASS (klass);
    control_class = GRL_UI_CONTROL_CLASS (klass);

    object_class->finalize = grl_ui_listview_finalize;
    object_class->get_property = grl_ui_listview_get_property;
    object_class->set_property = grl_ui_listview_set_property;

    control_class->draw = grl_ui_listview_draw_impl;
    control_class->get_preferred_size = grl_ui_listview_get_preferred_size_impl;

    /**
     * GrlUiListView:items:
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
     * GrlUiListView:active:
     *
     * The index of the currently selected item, or -1 if no selection.
     */
    properties[PROP_ACTIVE] =
        g_param_spec_int ("active",
                          "Active",
                          "The index of the currently selected item",
                          -1, G_MAXINT, -1,
                          G_PARAM_READWRITE |
                          G_PARAM_STATIC_STRINGS |
                          G_PARAM_EXPLICIT_NOTIFY);

    /**
     * GrlUiListView:scroll-index:
     *
     * The current scroll position.
     */
    properties[PROP_SCROLL_INDEX] =
        g_param_spec_int ("scroll-index",
                          "Scroll Index",
                          "The current scroll position",
                          0, G_MAXINT, 0,
                          G_PARAM_READWRITE |
                          G_PARAM_STATIC_STRINGS |
                          G_PARAM_EXPLICIT_NOTIFY);

    g_object_class_install_properties (object_class, N_PROPS, properties);

    /**
     * GrlUiListView::selection-changed:
     * @self: The list view
     * @active: The new active item index, or -1 if no selection
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
grl_ui_listview_init (GrlUiListView *self)
{
    self->items = NULL;
    self->active = -1;
    self->scroll_index = 0;
    self->item_count = 0;
}

/*
 * Public API
 */

/**
 * grl_ui_listview_new:
 * @items: (nullable): Semicolon-separated list of items
 *
 * Creates a new list view control.
 *
 * Returns: (transfer full): A new #GrlUiListView
 */
GrlUiListView *
grl_ui_listview_new (const gchar *items)
{
    return g_object_new (GRL_TYPE_UI_LISTVIEW,
                         "items", items,
                         NULL);
}

/**
 * grl_ui_listview_new_with_bounds:
 * @x: X position
 * @y: Y position
 * @width: Width of the list view
 * @height: Height of the list view
 * @items: (nullable): Semicolon-separated list of items
 *
 * Creates a new list view control with specified bounds.
 *
 * Returns: (transfer full): A new #GrlUiListView
 */
GrlUiListView *
grl_ui_listview_new_with_bounds (gfloat       x,
                                 gfloat       y,
                                 gfloat       width,
                                 gfloat       height,
                                 const gchar *items)
{
    g_autoptr(GrlRectangle) bounds = NULL;

    bounds = grl_rectangle_new (x, y, width, height);

    return g_object_new (GRL_TYPE_UI_LISTVIEW,
                         "bounds", bounds,
                         "items", items,
                         NULL);
}

/**
 * grl_ui_listview_get_items:
 * @self: A #GrlUiListView
 *
 * Gets the semicolon-separated list of items.
 *
 * Returns: (transfer none) (nullable): The items string
 */
const gchar *
grl_ui_listview_get_items (GrlUiListView *self)
{
    g_return_val_if_fail (GRL_IS_UI_LISTVIEW (self), NULL);

    return self->items;
}

/**
 * grl_ui_listview_set_items:
 * @self: A #GrlUiListView
 * @items: (nullable): Semicolon-separated list of items
 *
 * Sets the semicolon-separated list of items.
 */
void
grl_ui_listview_set_items (GrlUiListView *self,
                           const gchar   *items)
{
    g_return_if_fail (GRL_IS_UI_LISTVIEW (self));

    if (g_strcmp0 (self->items, items) == 0)
        return;

    g_free (self->items);
    self->items = g_strdup (items);
    self->item_count = count_items (items);

    /* Adjust active if out of range */
    if (self->active >= self->item_count)
    {
        self->active = self->item_count > 0 ? self->item_count - 1 : -1;
        g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_ACTIVE]);
    }

    /* Reset scroll if items changed */
    if (self->scroll_index > 0)
    {
        self->scroll_index = 0;
        g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_SCROLL_INDEX]);
    }

    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_ITEMS]);
}

/**
 * grl_ui_listview_get_active:
 * @self: A #GrlUiListView
 *
 * Gets the index of the currently selected item.
 *
 * Returns: The active item index (0-based), or -1 if no selection
 */
gint
grl_ui_listview_get_active (GrlUiListView *self)
{
    g_return_val_if_fail (GRL_IS_UI_LISTVIEW (self), -1);

    return self->active;
}

/**
 * grl_ui_listview_set_active:
 * @self: A #GrlUiListView
 * @active: The index of the item to select, or -1 for no selection
 *
 * Sets the currently selected item by index.
 */
void
grl_ui_listview_set_active (GrlUiListView *self,
                            gint           active)
{
    g_return_if_fail (GRL_IS_UI_LISTVIEW (self));

    if (active < -1)
        active = -1;
    else if (self->item_count > 0 && active >= self->item_count)
        active = self->item_count - 1;

    if (self->active == active)
        return;

    self->active = active;

    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_ACTIVE]);
}

/**
 * grl_ui_listview_get_scroll_index:
 * @self: A #GrlUiListView
 *
 * Gets the current scroll position.
 *
 * Returns: The scroll index
 */
gint
grl_ui_listview_get_scroll_index (GrlUiListView *self)
{
    g_return_val_if_fail (GRL_IS_UI_LISTVIEW (self), 0);

    return self->scroll_index;
}

/**
 * grl_ui_listview_set_scroll_index:
 * @self: A #GrlUiListView
 * @scroll_index: The scroll position
 *
 * Sets the current scroll position.
 */
void
grl_ui_listview_set_scroll_index (GrlUiListView *self,
                                  gint           scroll_index)
{
    g_return_if_fail (GRL_IS_UI_LISTVIEW (self));

    if (scroll_index < 0)
        scroll_index = 0;

    if (self->scroll_index == scroll_index)
        return;

    self->scroll_index = scroll_index;

    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_SCROLL_INDEX]);
}

/**
 * grl_ui_listview_get_item_count:
 * @self: A #GrlUiListView
 *
 * Gets the number of items in the list view.
 *
 * Returns: The number of items
 */
gint
grl_ui_listview_get_item_count (GrlUiListView *self)
{
    g_return_val_if_fail (GRL_IS_UI_LISTVIEW (self), 0);

    return self->item_count;
}

/**
 * grl_ui_listview_get_active_text:
 * @self: A #GrlUiListView
 *
 * Gets the text of the currently selected item.
 *
 * Returns: (transfer full) (nullable): The active item text, or %NULL if no selection
 */
gchar *
grl_ui_listview_get_active_text (GrlUiListView *self)
{
    gchar **items_array;
    gchar  *result;

    g_return_val_if_fail (GRL_IS_UI_LISTVIEW (self), NULL);

    if (self->items == NULL || self->items[0] == '\0')
        return NULL;

    if (self->active < 0)
        return NULL;

    items_array = g_strsplit (self->items, ";", -1);
    if (self->active < self->item_count && items_array[self->active] != NULL)
        result = g_strdup (items_array[self->active]);
    else
        result = NULL;

    g_strfreev (items_array);
    return result;
}

/**
 * grl_ui_listview_scroll_to_active:
 * @self: A #GrlUiListView
 *
 * Scrolls the list to make the active item visible.
 */
void
grl_ui_listview_scroll_to_active (GrlUiListView *self)
{
    g_return_if_fail (GRL_IS_UI_LISTVIEW (self));

    if (self->active < 0)
        return;

    /* Simple approach: set scroll index to active item */
    if (self->scroll_index != self->active)
    {
        self->scroll_index = self->active;
        g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_SCROLL_INDEX]);
    }
}

/**
 * grl_ui_listview_clear_selection:
 * @self: A #GrlUiListView
 *
 * Clears the current selection.
 */
void
grl_ui_listview_clear_selection (GrlUiListView *self)
{
    g_return_if_fail (GRL_IS_UI_LISTVIEW (self));

    if (self->active == -1)
        return;

    self->active = -1;

    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_ACTIVE]);
    g_signal_emit (self, signals[SIGNAL_SELECTION_CHANGED], 0, self->active);
}
