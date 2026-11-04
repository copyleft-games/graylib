/* grl-ui-tabbar.c
 *
 * Copyright 2026 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * GrlUiTabBar wraps raygui 5's GuiTabBarEx(). raygui keeps the tab layout
 * in its TABBAR style (tab width, close buttons, line side), which is
 * global; the control sets its own values around each draw and restores
 * the previous ones, so several tab bars with different settings coexist.
 *
 * raygui reports a close request (close button or middle click) as
 * RESULT_TAB_CLOSE but not which tab it was for: a middle click leaves the
 * active tab alone. The control hit-tests the pointer against the same
 * layout raygui used to find the tab.
 */

#include "config.h"
#include "grl-ui-tabbar.h"
#include "../math/grl-rectangle.h"

#define RAYGUI_STATIC
#include <raygui.h>

/* Gap raygui leaves between two tabs (GuiTabBarEx(): tabItemsWidth + 4) */
#define GRL_UI_TABBAR_TAB_GAP (4)

struct _GrlUiTabBar
{
    GrlUiControl  parent_instance;

    gchar        *items;
    gint          item_count;
    gint          active;
    gint          tab_width;            /* 0: use the style's */
    gboolean      show_close_buttons;
    gboolean      line_on_top;
};

G_DEFINE_TYPE (GrlUiTabBar, grl_ui_tabbar, GRL_TYPE_UI_CONTROL)

enum
{
    PROP_0,
    PROP_ITEMS,
    PROP_ACTIVE,
    PROP_TAB_WIDTH,
    PROP_SHOW_CLOSE_BUTTONS,
    PROP_LINE_ON_TOP,
    N_PROPS
};

static GParamSpec *properties[N_PROPS];

enum
{
    SIGNAL_SELECTION_CHANGED,
    SIGNAL_TAB_CLOSE_REQUESTED,
    N_SIGNALS
};

static guint signals[N_SIGNALS];

/*
 * count_items:
 * Number of semicolon-separated entries in @items (0 for NULL or "").
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
 * effective_tab_width:
 * The tab width raygui will use: ours, or the TABBAR style's.
 */
static gint
effective_tab_width (GrlUiTabBar *self)
{
    if (self->tab_width > 0)
        return self->tab_width;
    return GuiGetStyle (TABBAR, TAB_ITEMS_WIDTH);
}

/*
 * scroll_offset:
 * The horizontal offset GuiTabBarEx() applies to every tab when the
 * active one would start past the right edge of the screen.
 */
static gint
scroll_offset (GrlUiTabBar *self,
               gint         tab_width)
{
    unsigned char ready;
    gint          offset;

    ready = IsWindowReady ();
    if (ready == 0)
        return 0;

    offset = self->active * tab_width - GetScreenWidth ();
    return offset < 0 ? 0 : offset;
}

/*
 * tab_index_at:
 * Hit-tests (@x, @y) against the tab rectangles of @bounds.
 */
static gint
tab_index_at (GrlUiTabBar     *self,
              const Rectangle *bounds,
              gfloat           x,
              gfloat           y)
{
    gint   tab_width;
    gint   index;
    gfloat local_x;

    if (self->item_count <= 0 || y < bounds->y || y >= bounds->y + bounds->height)
        return -1;

    tab_width = effective_tab_width (self);
    if (tab_width <= 0)
        return -1;

    local_x = x - bounds->x - (gfloat)scroll_offset (self, tab_width);
    if (local_x < 0.0f)
        return -1;

    index = (gint)(local_x / (gfloat)(tab_width + GRL_UI_TABBAR_TAB_GAP));
    if (index >= self->item_count)
        return -1;

    /* the gap between two tabs belongs to neither */
    if (local_x - (gfloat)(index * (tab_width + GRL_UI_TABBAR_TAB_GAP)) >= (gfloat)tab_width)
        return -1;

    return index;
}

/*
 * Virtual method implementations
 */

static void
grl_ui_tabbar_draw_impl (GrlUiControl *control)
{
    GrlUiTabBar  *self;
    GrlRectangle *bounds;
    Rectangle     raygui_bounds;
    gboolean      enabled;
    gboolean      visible;
    gchar       **titles;
    Vector2       pointer;
    int           old_width;
    int           old_close;
    int           old_side;
    int           active;
    int           hscroll;
    int           result;
    gint          old_active;
    gint          closed;

    self = GRL_UI_TABBAR (control);

    g_object_get (G_OBJECT (control),
                  "visible", &visible,
                  "enabled", &enabled,
                  NULL);

    if (!visible || self->item_count <= 0)
        return;

    bounds = grl_ui_control_get_bounds (control);
    if (bounds == NULL)
        return;

    raygui_bounds.x = bounds->x;
    raygui_bounds.y = bounds->y;
    raygui_bounds.width = bounds->width;
    raygui_bounds.height = bounds->height;
    grl_rectangle_free (bounds);

    /* Apply this bar's layout to the global TABBAR style */
    old_width = GuiGetStyle (TABBAR, TAB_ITEMS_WIDTH);
    old_close = GuiGetStyle (TABBAR, TAB_CLOSE_BUTTON);
    old_side = GuiGetStyle (TABBAR, TAB_LINE_SIDE);
    if (self->tab_width > 0)
        GuiSetStyle (TABBAR, TAB_ITEMS_WIDTH, self->tab_width);
    GuiSetStyle (TABBAR, TAB_CLOSE_BUTTON, self->show_close_buttons ? 1 : 0);
    GuiSetStyle (TABBAR, TAB_LINE_SIDE, self->line_on_top ? 1 : 0);

    if (!enabled)
        GuiSetState (STATE_DISABLED);

    /* raygui wants a mutable array of titles; it does not modify them */
    titles = g_strsplit (self->items, ";", -1);
    old_active = self->active;
    active = self->active;
    hscroll = 0;
    pointer = GetMousePosition ();

    result = GuiTabBarEx (raygui_bounds, titles, self->item_count,
                          &hscroll, &active, NULL);

    /* Find the tab a close request was for before restoring the style */
    closed = -1;
    if (result == RESULT_TAB_CLOSE)
        closed = tab_index_at (self, &raygui_bounds, pointer.x, pointer.y);

    if (!enabled)
        GuiSetState (STATE_NORMAL);

    GuiSetStyle (TABBAR, TAB_ITEMS_WIDTH, old_width);
    GuiSetStyle (TABBAR, TAB_CLOSE_BUTTON, old_close);
    GuiSetStyle (TABBAR, TAB_LINE_SIDE, old_side);
    g_strfreev (titles);

    /* Report a selection change, then a close request */
    if (active != old_active && active >= 0 && active < self->item_count)
    {
        self->active = active;
        g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_ACTIVE]);
        g_signal_emit (self, signals[SIGNAL_SELECTION_CHANGED], 0, self->active);
    }

    if (closed >= 0)
        g_signal_emit (self, signals[SIGNAL_TAB_CLOSE_REQUESTED], 0, closed);
}

static void
grl_ui_tabbar_get_preferred_size_impl (GrlUiControl *control,
                                       gfloat       *width,
                                       gfloat       *height)
{
    GrlUiTabBar *self;
    gint         tab_width;

    self = GRL_UI_TABBAR (control);
    tab_width = effective_tab_width (self);

    /* every tab is tab_width wide, with raygui's gap between them */
    if (self->item_count > 0)
        *width = (gfloat)(self->item_count * tab_width +
                          (self->item_count - 1) * GRL_UI_TABBAR_TAB_GAP);
    else
        *width = (gfloat)tab_width;
    *height = 24.0f;
}

/*
 * GObject implementation
 */

static void
grl_ui_tabbar_finalize (GObject *object)
{
    GrlUiTabBar *self = GRL_UI_TABBAR (object);

    g_clear_pointer (&self->items, g_free);

    G_OBJECT_CLASS (grl_ui_tabbar_parent_class)->finalize (object);
}

static void
grl_ui_tabbar_get_property (GObject    *object,
                            guint       prop_id,
                            GValue     *value,
                            GParamSpec *pspec)
{
    GrlUiTabBar *self = GRL_UI_TABBAR (object);

    switch (prop_id)
    {
    case PROP_ITEMS:
        g_value_set_string (value, self->items);
        break;
    case PROP_ACTIVE:
        g_value_set_int (value, self->active);
        break;
    case PROP_TAB_WIDTH:
        g_value_set_int (value, self->tab_width);
        break;
    case PROP_SHOW_CLOSE_BUTTONS:
        g_value_set_boolean (value, self->show_close_buttons);
        break;
    case PROP_LINE_ON_TOP:
        g_value_set_boolean (value, self->line_on_top);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
grl_ui_tabbar_set_property (GObject      *object,
                            guint         prop_id,
                            const GValue *value,
                            GParamSpec   *pspec)
{
    GrlUiTabBar *self = GRL_UI_TABBAR (object);

    switch (prop_id)
    {
    case PROP_ITEMS:
        grl_ui_tabbar_set_items (self, g_value_get_string (value));
        break;
    case PROP_ACTIVE:
        grl_ui_tabbar_set_active (self, g_value_get_int (value));
        break;
    case PROP_TAB_WIDTH:
        grl_ui_tabbar_set_tab_width (self, g_value_get_int (value));
        break;
    case PROP_SHOW_CLOSE_BUTTONS:
        grl_ui_tabbar_set_show_close_buttons (self, g_value_get_boolean (value));
        break;
    case PROP_LINE_ON_TOP:
        grl_ui_tabbar_set_line_on_top (self, g_value_get_boolean (value));
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
grl_ui_tabbar_class_init (GrlUiTabBarClass *klass)
{
    GObjectClass      *object_class;
    GrlUiControlClass *control_class;

    object_class = G_OBJECT_CLASS (klass);
    control_class = GRL_UI_CONTROL_CLASS (klass);

    object_class->finalize = grl_ui_tabbar_finalize;
    object_class->get_property = grl_ui_tabbar_get_property;
    object_class->set_property = grl_ui_tabbar_set_property;

    control_class->draw = grl_ui_tabbar_draw_impl;
    control_class->get_preferred_size = grl_ui_tabbar_get_preferred_size_impl;

    /**
     * GrlUiTabBar:items:
     *
     * The semicolon-separated tab titles.
     */
    properties[PROP_ITEMS] =
        g_param_spec_string ("items", "Items",
                             "Semicolon-separated tab titles",
                             NULL,
                             G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS |
                             G_PARAM_EXPLICIT_NOTIFY);

    /**
     * GrlUiTabBar:active:
     *
     * The index of the active tab.
     */
    properties[PROP_ACTIVE] =
        g_param_spec_int ("active", "Active",
                          "Index of the active tab",
                          0, G_MAXINT, 0,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS |
                          G_PARAM_EXPLICIT_NOTIFY);

    /**
     * GrlUiTabBar:tab-width:
     *
     * The width of each tab in pixels; 0 uses the TABBAR style's
     * TAB_ITEMS_WIDTH (160 in the default style).
     */
    properties[PROP_TAB_WIDTH] =
        g_param_spec_int ("tab-width", "Tab width",
                          "Width of each tab, 0 for the style's",
                          0, G_MAXINT, 0,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS |
                          G_PARAM_EXPLICIT_NOTIFY);

    /**
     * GrlUiTabBar:show-close-buttons:
     *
     * Whether every tab shows a close button.
     */
    properties[PROP_SHOW_CLOSE_BUTTONS] =
        g_param_spec_boolean ("show-close-buttons", "Show close buttons",
                              "Whether every tab shows a close button",
                              FALSE,
                              G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS |
                              G_PARAM_EXPLICIT_NOTIFY);

    /**
     * GrlUiTabBar:line-on-top:
     *
     * Whether the bar's line is drawn above the tabs instead of below.
     */
    properties[PROP_LINE_ON_TOP] =
        g_param_spec_boolean ("line-on-top", "Line on top",
                              "Whether the bar line is drawn above the tabs",
                              FALSE,
                              G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS |
                              G_PARAM_EXPLICIT_NOTIFY);

    g_object_class_install_properties (object_class, N_PROPS, properties);

    /**
     * GrlUiTabBar::selection-changed:
     * @self: The tab bar
     * @active: The new active tab index
     *
     * Emitted when the user activates another tab.
     */
    signals[SIGNAL_SELECTION_CHANGED] =
        g_signal_new ("selection-changed",
                      G_TYPE_FROM_CLASS (klass),
                      G_SIGNAL_RUN_LAST,
                      0, NULL, NULL, NULL,
                      G_TYPE_NONE, 1, G_TYPE_INT);

    /**
     * GrlUiTabBar::tab-close-requested:
     * @self: The tab bar
     * @index: The tab the user asked to close
     *
     * Emitted when the user clicks a tab's close button or middle-clicks a
     * tab. The tab is not removed: update #GrlUiTabBar:items to close it.
     */
    signals[SIGNAL_TAB_CLOSE_REQUESTED] =
        g_signal_new ("tab-close-requested",
                      G_TYPE_FROM_CLASS (klass),
                      G_SIGNAL_RUN_LAST,
                      0, NULL, NULL, NULL,
                      G_TYPE_NONE, 1, G_TYPE_INT);
}

static void
grl_ui_tabbar_init (GrlUiTabBar *self)
{
    self->items = NULL;
    self->item_count = 0;
    self->active = 0;
    self->tab_width = 0;
    self->show_close_buttons = FALSE;
    self->line_on_top = FALSE;
}

/*
 * Public API
 */

GrlUiTabBar *
grl_ui_tabbar_new (const gchar *items)
{
    return g_object_new (GRL_TYPE_UI_TABBAR, "items", items, NULL);
}

GrlUiTabBar *
grl_ui_tabbar_new_with_bounds (gfloat       x,
                               gfloat       y,
                               gfloat       width,
                               gfloat       height,
                               const gchar *items)
{
    g_autoptr(GrlRectangle) bounds = NULL;

    bounds = grl_rectangle_new (x, y, width, height);

    return g_object_new (GRL_TYPE_UI_TABBAR,
                         "bounds", bounds,
                         "items", items,
                         NULL);
}

const gchar *
grl_ui_tabbar_get_items (GrlUiTabBar *self)
{
    g_return_val_if_fail (GRL_IS_UI_TABBAR (self), NULL);

    return self->items;
}

void
grl_ui_tabbar_set_items (GrlUiTabBar *self,
                         const gchar *items)
{
    g_return_if_fail (GRL_IS_UI_TABBAR (self));

    if (g_strcmp0 (self->items, items) == 0)
        return;

    g_free (self->items);
    self->items = g_strdup (items);
    self->item_count = count_items (items);

    /* keep the active index pointing at an existing tab */
    if (self->active >= self->item_count)
    {
        self->active = self->item_count > 0 ? self->item_count - 1 : 0;
        g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_ACTIVE]);
    }

    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_ITEMS]);
}

gint
grl_ui_tabbar_get_item_count (GrlUiTabBar *self)
{
    g_return_val_if_fail (GRL_IS_UI_TABBAR (self), 0);

    return self->item_count;
}

gint
grl_ui_tabbar_get_active (GrlUiTabBar *self)
{
    g_return_val_if_fail (GRL_IS_UI_TABBAR (self), 0);

    return self->active;
}

void
grl_ui_tabbar_set_active (GrlUiTabBar *self,
                          gint         active)
{
    g_return_if_fail (GRL_IS_UI_TABBAR (self));

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
grl_ui_tabbar_get_tab_width (GrlUiTabBar *self)
{
    g_return_val_if_fail (GRL_IS_UI_TABBAR (self), 0);

    return self->tab_width;
}

void
grl_ui_tabbar_set_tab_width (GrlUiTabBar *self,
                             gint         tab_width)
{
    g_return_if_fail (GRL_IS_UI_TABBAR (self));

    if (tab_width < 0)
        tab_width = 0;
    if (self->tab_width == tab_width)
        return;

    self->tab_width = tab_width;
    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_TAB_WIDTH]);
}

gboolean
grl_ui_tabbar_get_show_close_buttons (GrlUiTabBar *self)
{
    g_return_val_if_fail (GRL_IS_UI_TABBAR (self), FALSE);

    return self->show_close_buttons;
}

void
grl_ui_tabbar_set_show_close_buttons (GrlUiTabBar *self,
                                      gboolean     show)
{
    g_return_if_fail (GRL_IS_UI_TABBAR (self));

    show = !!show;
    if (self->show_close_buttons == show)
        return;

    self->show_close_buttons = show;
    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_SHOW_CLOSE_BUTTONS]);
}

gboolean
grl_ui_tabbar_get_line_on_top (GrlUiTabBar *self)
{
    g_return_val_if_fail (GRL_IS_UI_TABBAR (self), FALSE);

    return self->line_on_top;
}

void
grl_ui_tabbar_set_line_on_top (GrlUiTabBar *self,
                               gboolean     on_top)
{
    g_return_if_fail (GRL_IS_UI_TABBAR (self));

    on_top = !!on_top;
    if (self->line_on_top == on_top)
        return;

    self->line_on_top = on_top;
    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_LINE_ON_TOP]);
}

gchar *
grl_ui_tabbar_get_active_text (GrlUiTabBar *self)
{
    g_auto(GStrv) titles = NULL;

    g_return_val_if_fail (GRL_IS_UI_TABBAR (self), NULL);

    if (self->item_count <= 0)
        return NULL;

    titles = g_strsplit (self->items, ";", -1);
    if (self->active < 0 || self->active >= (gint)g_strv_length (titles))
        return NULL;

    return g_strdup (titles[self->active]);
}

gint
grl_ui_tabbar_tab_at (GrlUiTabBar *self,
                      gfloat       x,
                      gfloat       y)
{
    GrlRectangle *bounds;
    Rectangle     rec;

    g_return_val_if_fail (GRL_IS_UI_TABBAR (self), -1);

    bounds = grl_ui_control_get_bounds (GRL_UI_CONTROL (self));
    if (bounds == NULL)
        return -1;

    rec.x = bounds->x;
    rec.y = bounds->y;
    rec.width = bounds->width;
    rec.height = bounds->height;
    grl_rectangle_free (bounds);

    return tab_index_at (self, &rec, x, y);
}
