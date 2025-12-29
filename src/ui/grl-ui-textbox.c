/* grl-ui-textbox.c
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#include "config.h"
#include "grl-ui-textbox.h"

#include <raygui.h>
#include <string.h>

#define DEFAULT_MAX_LENGTH 256

struct _GrlUiTextBox
{
    GrlUiControl    parent_instance;

    gchar          *text;
    gint            max_length;
    gboolean        edit_mode;
    gboolean        read_only;
    gchar          *previous_text;  /* For change detection */
};

G_DEFINE_TYPE (GrlUiTextBox, grl_ui_textbox, GRL_TYPE_UI_CONTROL)

enum
{
    PROP_0,
    PROP_TEXT,
    PROP_MAX_LENGTH,
    PROP_EDIT_MODE,
    PROP_READ_ONLY,
    N_PROPS
};

enum
{
    SIGNAL_TEXT_CHANGED,
    SIGNAL_EDITING_STARTED,
    SIGNAL_EDITING_FINISHED,
    N_SIGNALS
};

static GParamSpec *properties[N_PROPS];
static guint signals[N_SIGNALS];

static void
grl_ui_textbox_finalize (GObject *object)
{
    GrlUiTextBox *self = GRL_UI_TEXTBOX (object);

    g_clear_pointer (&self->text, g_free);
    g_clear_pointer (&self->previous_text, g_free);

    G_OBJECT_CLASS (grl_ui_textbox_parent_class)->finalize (object);
}

static void
grl_ui_textbox_get_property (GObject    *object,
                             guint       prop_id,
                             GValue     *value,
                             GParamSpec *pspec)
{
    GrlUiTextBox *self = GRL_UI_TEXTBOX (object);

    switch (prop_id)
    {
    case PROP_TEXT:
        g_value_set_string (value, self->text);
        break;
    case PROP_MAX_LENGTH:
        g_value_set_int (value, self->max_length);
        break;
    case PROP_EDIT_MODE:
        g_value_set_boolean (value, self->edit_mode);
        break;
    case PROP_READ_ONLY:
        g_value_set_boolean (value, self->read_only);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
grl_ui_textbox_set_property (GObject      *object,
                             guint         prop_id,
                             const GValue *value,
                             GParamSpec   *pspec)
{
    GrlUiTextBox *self = GRL_UI_TEXTBOX (object);

    switch (prop_id)
    {
    case PROP_TEXT:
        grl_ui_textbox_set_text (self, g_value_get_string (value));
        break;
    case PROP_MAX_LENGTH:
        grl_ui_textbox_set_max_length (self, g_value_get_int (value));
        break;
    case PROP_EDIT_MODE:
        grl_ui_textbox_set_edit_mode (self, g_value_get_boolean (value));
        break;
    case PROP_READ_ONLY:
        grl_ui_textbox_set_read_only (self, g_value_get_boolean (value));
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
grl_ui_textbox_draw (GrlUiControl *control)
{
    GrlUiTextBox *self = GRL_UI_TEXTBOX (control);
    g_autoptr(GrlRectangle) bounds = NULL;
    Rectangle rl_bounds;
    gboolean old_edit_mode;
    gboolean text_changed;
    int result;
    int old_readonly;

    bounds = grl_ui_control_get_bounds (control);

    rl_bounds.x = bounds->x;
    rl_bounds.y = bounds->y;
    rl_bounds.width = bounds->width;
    rl_bounds.height = bounds->height;

    /* Store old edit mode */
    old_edit_mode = self->edit_mode;

    /* Set read-only style if needed */
    old_readonly = GuiGetStyle (TEXTBOX, TEXT_READONLY);
    if (self->read_only)
        GuiSetStyle (TEXTBOX, TEXT_READONLY, 1);
    else
        GuiSetStyle (TEXTBOX, TEXT_READONLY, 0);

    /* Call raygui - it modifies text buffer and returns edit mode change */
    result = GuiTextBox (rl_bounds,
                         self->text,
                         self->max_length,
                         self->edit_mode);

    /* Restore read-only style */
    GuiSetStyle (TEXTBOX, TEXT_READONLY, old_readonly);

    /* Check if edit mode changed (result is 1 when clicked to toggle edit mode) */
    if (result != 0)
    {
        self->edit_mode = !self->edit_mode;

        g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_EDIT_MODE]);

        if (self->edit_mode && !old_edit_mode)
        {
            /* Started editing */
            g_signal_emit (self, signals[SIGNAL_EDITING_STARTED], 0);
        }
        else if (!self->edit_mode && old_edit_mode)
        {
            /* Finished editing */
            g_signal_emit (self, signals[SIGNAL_EDITING_FINISHED], 0);
        }
    }

    /* Check if text changed */
    text_changed = g_strcmp0 (self->text, self->previous_text) != 0;
    if (text_changed)
    {
        g_free (self->previous_text);
        self->previous_text = g_strdup (self->text);

        g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_TEXT]);
        g_signal_emit (self, signals[SIGNAL_TEXT_CHANGED], 0, self->text);
    }
}

static gboolean
grl_ui_textbox_handle_input (GrlUiControl *control)
{
    GrlUiTextBox *self = GRL_UI_TEXTBOX (control);

    /*
     * TextBox consumes input when in edit mode.
     * Return TRUE if in edit mode to indicate exclusive input handling.
     */
    return self->edit_mode;
}

static void
grl_ui_textbox_get_preferred_size (GrlUiControl *control,
                                   gfloat       *width,
                                   gfloat       *height)
{
    if (width != NULL)
        *width = 200.0f;  /* Default textbox width */
    if (height != NULL)
        *height = 28.0f;  /* Default textbox height */
}

static void
grl_ui_textbox_class_init (GrlUiTextBoxClass *klass)
{
    GObjectClass      *object_class = G_OBJECT_CLASS (klass);
    GrlUiControlClass *control_class = GRL_UI_CONTROL_CLASS (klass);

    object_class->finalize = grl_ui_textbox_finalize;
    object_class->get_property = grl_ui_textbox_get_property;
    object_class->set_property = grl_ui_textbox_set_property;

    control_class->draw = grl_ui_textbox_draw;
    control_class->handle_input = grl_ui_textbox_handle_input;
    control_class->get_preferred_size = grl_ui_textbox_get_preferred_size;

    /**
     * GrlUiTextBox:text:
     *
     * The text content of the textbox.
     */
    properties[PROP_TEXT] =
        g_param_spec_string ("text",
                             "Text",
                             "The textbox text content",
                             "",
                             G_PARAM_READWRITE |
                             G_PARAM_STATIC_STRINGS |
                             G_PARAM_EXPLICIT_NOTIFY);

    /**
     * GrlUiTextBox:max-length:
     *
     * The maximum number of characters allowed.
     */
    properties[PROP_MAX_LENGTH] =
        g_param_spec_int ("max-length",
                          "Maximum Length",
                          "The maximum number of characters",
                          1, G_MAXINT, DEFAULT_MAX_LENGTH,
                          G_PARAM_READWRITE |
                          G_PARAM_CONSTRUCT |
                          G_PARAM_STATIC_STRINGS |
                          G_PARAM_EXPLICIT_NOTIFY);

    /**
     * GrlUiTextBox:edit-mode:
     *
     * Whether the textbox is in edit mode (accepting keyboard input).
     */
    properties[PROP_EDIT_MODE] =
        g_param_spec_boolean ("edit-mode",
                              "Edit Mode",
                              "Whether in edit mode",
                              FALSE,
                              G_PARAM_READWRITE |
                              G_PARAM_STATIC_STRINGS |
                              G_PARAM_EXPLICIT_NOTIFY);

    /**
     * GrlUiTextBox:read-only:
     *
     * Whether the textbox is read-only.
     */
    properties[PROP_READ_ONLY] =
        g_param_spec_boolean ("read-only",
                              "Read Only",
                              "Whether the textbox is read-only",
                              FALSE,
                              G_PARAM_READWRITE |
                              G_PARAM_STATIC_STRINGS |
                              G_PARAM_EXPLICIT_NOTIFY);

    g_object_class_install_properties (object_class, N_PROPS, properties);

    /**
     * GrlUiTextBox::text-changed:
     * @self: The #GrlUiTextBox whose text changed
     * @text: The new text
     *
     * Emitted when the textbox content changes.
     */
    signals[SIGNAL_TEXT_CHANGED] =
        g_signal_new ("text-changed",
                      G_TYPE_FROM_CLASS (klass),
                      G_SIGNAL_RUN_LAST,
                      0,
                      NULL, NULL,
                      NULL,
                      G_TYPE_NONE, 1,
                      G_TYPE_STRING);

    /**
     * GrlUiTextBox::editing-started:
     * @self: The #GrlUiTextBox that started editing
     *
     * Emitted when the textbox enters edit mode.
     */
    signals[SIGNAL_EDITING_STARTED] =
        g_signal_new ("editing-started",
                      G_TYPE_FROM_CLASS (klass),
                      G_SIGNAL_RUN_LAST,
                      0,
                      NULL, NULL,
                      NULL,
                      G_TYPE_NONE, 0);

    /**
     * GrlUiTextBox::editing-finished:
     * @self: The #GrlUiTextBox that finished editing
     *
     * Emitted when the textbox exits edit mode.
     */
    signals[SIGNAL_EDITING_FINISHED] =
        g_signal_new ("editing-finished",
                      G_TYPE_FROM_CLASS (klass),
                      G_SIGNAL_RUN_LAST,
                      0,
                      NULL, NULL,
                      NULL,
                      G_TYPE_NONE, 0);
}

static void
grl_ui_textbox_init (GrlUiTextBox *self)
{
    self->max_length = DEFAULT_MAX_LENGTH;
    self->text = g_malloc0 (self->max_length + 1);
    self->text[0] = '\0';
    self->previous_text = g_strdup ("");
    self->edit_mode = FALSE;
    self->read_only = FALSE;
}

/**
 * grl_ui_textbox_new:
 * @max_length: Maximum number of characters
 *
 * Creates a new textbox control.
 *
 * Returns: (transfer full): A new #GrlUiTextBox
 */
GrlUiTextBox *
grl_ui_textbox_new (gint max_length)
{
    return g_object_new (GRL_TYPE_UI_TEXTBOX,
                         "max-length", max_length,
                         NULL);
}

/**
 * grl_ui_textbox_new_with_bounds:
 * @x: X position
 * @y: Y position
 * @width: Width of the textbox
 * @height: Height of the textbox
 * @max_length: Maximum number of characters
 *
 * Creates a new textbox control with specified bounds.
 *
 * Returns: (transfer full): A new #GrlUiTextBox
 */
GrlUiTextBox *
grl_ui_textbox_new_with_bounds (gfloat x,
                                gfloat y,
                                gfloat width,
                                gfloat height,
                                gint   max_length)
{
    GrlUiTextBox *textbox;
    g_autoptr(GrlRectangle) bounds = NULL;

    bounds = grl_rectangle_new (x, y, width, height);
    textbox = g_object_new (GRL_TYPE_UI_TEXTBOX,
                            "max-length", max_length,
                            "bounds", bounds,
                            NULL);

    return textbox;
}

/**
 * grl_ui_textbox_get_text:
 * @self: A #GrlUiTextBox
 *
 * Gets the textbox text content.
 *
 * Returns: (transfer none): The text content
 */
const gchar *
grl_ui_textbox_get_text (GrlUiTextBox *self)
{
    g_return_val_if_fail (GRL_IS_UI_TEXTBOX (self), "");

    return self->text;
}

/**
 * grl_ui_textbox_set_text:
 * @self: A #GrlUiTextBox
 * @text: (nullable): The new text content
 *
 * Sets the textbox text content.
 */
void
grl_ui_textbox_set_text (GrlUiTextBox *self,
                         const gchar  *text)
{
    gsize len;

    g_return_if_fail (GRL_IS_UI_TEXTBOX (self));

    if (text == NULL)
        text = "";

    if (g_strcmp0 (self->text, text) == 0)
        return;

    /* Copy text, truncating if necessary */
    len = strlen (text);
    if (len >= (gsize)self->max_length)
        len = self->max_length - 1;

    memcpy (self->text, text, len);
    self->text[len] = '\0';

    /* Update previous text for change tracking */
    g_free (self->previous_text);
    self->previous_text = g_strdup (self->text);

    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_TEXT]);
}

/**
 * grl_ui_textbox_get_max_length:
 * @self: A #GrlUiTextBox
 *
 * Gets the maximum text length.
 *
 * Returns: The maximum number of characters
 */
gint
grl_ui_textbox_get_max_length (GrlUiTextBox *self)
{
    g_return_val_if_fail (GRL_IS_UI_TEXTBOX (self), DEFAULT_MAX_LENGTH);

    return self->max_length;
}

/**
 * grl_ui_textbox_set_max_length:
 * @self: A #GrlUiTextBox
 * @max_length: The maximum number of characters
 *
 * Sets the maximum text length.
 */
void
grl_ui_textbox_set_max_length (GrlUiTextBox *self,
                               gint          max_length)
{
    gchar *new_text;
    gsize current_len;

    g_return_if_fail (GRL_IS_UI_TEXTBOX (self));
    g_return_if_fail (max_length > 0);

    if (self->max_length == max_length)
        return;

    /* Reallocate text buffer */
    new_text = g_malloc0 (max_length + 1);

    /* Copy existing text, truncating if necessary */
    current_len = strlen (self->text);
    if (current_len >= (gsize)max_length)
        current_len = max_length - 1;

    memcpy (new_text, self->text, current_len);
    new_text[current_len] = '\0';

    g_free (self->text);
    self->text = new_text;
    self->max_length = max_length;

    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_MAX_LENGTH]);
}

/**
 * grl_ui_textbox_get_edit_mode:
 * @self: A #GrlUiTextBox
 *
 * Gets whether the textbox is in edit mode.
 *
 * Returns: %TRUE if in edit mode
 */
gboolean
grl_ui_textbox_get_edit_mode (GrlUiTextBox *self)
{
    g_return_val_if_fail (GRL_IS_UI_TEXTBOX (self), FALSE);

    return self->edit_mode;
}

/**
 * grl_ui_textbox_set_edit_mode:
 * @self: A #GrlUiTextBox
 * @edit_mode: Whether to enter edit mode
 *
 * Sets whether the textbox is in edit mode.
 */
void
grl_ui_textbox_set_edit_mode (GrlUiTextBox *self,
                              gboolean      edit_mode)
{
    gboolean old_edit_mode;

    g_return_if_fail (GRL_IS_UI_TEXTBOX (self));

    edit_mode = !!edit_mode;

    if (self->edit_mode == edit_mode)
        return;

    old_edit_mode = self->edit_mode;
    self->edit_mode = edit_mode;

    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_EDIT_MODE]);

    /* Emit appropriate signal */
    if (edit_mode && !old_edit_mode)
        g_signal_emit (self, signals[SIGNAL_EDITING_STARTED], 0);
    else if (!edit_mode && old_edit_mode)
        g_signal_emit (self, signals[SIGNAL_EDITING_FINISHED], 0);
}

/**
 * grl_ui_textbox_get_read_only:
 * @self: A #GrlUiTextBox
 *
 * Gets whether the textbox is read-only.
 *
 * Returns: %TRUE if read-only
 */
gboolean
grl_ui_textbox_get_read_only (GrlUiTextBox *self)
{
    g_return_val_if_fail (GRL_IS_UI_TEXTBOX (self), FALSE);

    return self->read_only;
}

/**
 * grl_ui_textbox_set_read_only:
 * @self: A #GrlUiTextBox
 * @read_only: Whether the textbox should be read-only
 *
 * Sets whether the textbox is read-only.
 */
void
grl_ui_textbox_set_read_only (GrlUiTextBox *self,
                              gboolean      read_only)
{
    g_return_if_fail (GRL_IS_UI_TEXTBOX (self));

    read_only = !!read_only;

    if (self->read_only == read_only)
        return;

    self->read_only = read_only;

    g_object_notify_by_pspec (G_OBJECT (self), properties[PROP_READ_ONLY]);
}

/**
 * grl_ui_textbox_clear:
 * @self: A #GrlUiTextBox
 *
 * Clears the textbox content.
 */
void
grl_ui_textbox_clear (GrlUiTextBox *self)
{
    g_return_if_fail (GRL_IS_UI_TEXTBOX (self));

    grl_ui_textbox_set_text (self, "");
}

/**
 * grl_ui_textbox_get_text_length:
 * @self: A #GrlUiTextBox
 *
 * Gets the current text length.
 *
 * Returns: The number of characters in the text
 */
gint
grl_ui_textbox_get_text_length (GrlUiTextBox *self)
{
    g_return_val_if_fail (GRL_IS_UI_TEXTBOX (self), 0);

    return (gint)strlen (self->text);
}
