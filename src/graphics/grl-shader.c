/* grl-shader.c
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#include "grl-shader.h"
#include "grl-texture.h"
#include <gio/gio.h>
#include <raylib.h>

/**
 * SECTION:grl-shader
 * @Title: GrlShader
 * @Short_description: Shader programs for custom rendering
 *
 * #GrlShader provides shader functionality for custom rendering effects.
 * Shaders can be loaded from files or from code strings.
 *
 * # Example Usage
 *
 * |[<!-- language="C" -->
 * g_autoptr(GError) error = NULL;
 * g_autoptr(GrlShader) shader = NULL;
 *
 * shader = grl_shader_new_from_file (NULL, "effects/blur.fs", &error);
 * if (shader == NULL)
 * {
 *     g_printerr ("Failed to load shader: %s\n", error->message);
 *     return;
 * }
 *
 * // In draw loop
 * grl_shader_begin (shader);
 * // Draw objects with shader
 * grl_shader_end (shader);
 * ]|
 */

typedef struct
{
    Shader   shader;
    gboolean valid;
} GrlShaderPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (GrlShader, grl_shader, G_TYPE_OBJECT)

enum
{
    PROP_0,
    PROP_VALID,
    N_PROPS
};

static GParamSpec *properties[N_PROPS];

static void
grl_shader_finalize (GObject *object)
{
    GrlShader *self = GRL_SHADER (object);
    GrlShaderPrivate *priv = grl_shader_get_instance_private (self);

    if (priv->valid)
    {
        UnloadShader (priv->shader);
        priv->valid = FALSE;
    }

    G_OBJECT_CLASS (grl_shader_parent_class)->finalize (object);
}

static void
grl_shader_get_property (GObject    *object,
                         guint       prop_id,
                         GValue     *value,
                         GParamSpec *pspec)
{
    GrlShader *self = GRL_SHADER (object);
    GrlShaderPrivate *priv = grl_shader_get_instance_private (self);

    switch (prop_id)
    {
    case PROP_VALID:
        g_value_set_boolean (value, priv->valid);
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
grl_shader_class_init (GrlShaderClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->finalize = grl_shader_finalize;
    object_class->get_property = grl_shader_get_property;

    /**
     * GrlShader:valid:
     *
     * Whether the shader is valid and ready to use.
     */
    properties[PROP_VALID] =
        g_param_spec_boolean ("valid",
                              "Valid",
                              "Whether shader is valid",
                              FALSE,
                              G_PARAM_READABLE |
                              G_PARAM_STATIC_STRINGS);

    g_object_class_install_properties (object_class, N_PROPS, properties);
}

static void
grl_shader_init (GrlShader *self)
{
    GrlShaderPrivate *priv = grl_shader_get_instance_private (self);

    priv->valid = FALSE;
    priv->shader.id = 0;
    priv->shader.locs = NULL;
}

/**
 * grl_shader_new_from_file:
 * @vs_filename: (nullable) (type filename): Vertex shader file, or %NULL
 * @fs_filename: (nullable) (type filename): Fragment shader file, or %NULL
 * @error: (nullable): Return location for error, or %NULL
 *
 * Loads a shader from files.
 *
 * Returns: (transfer full) (nullable): A new #GrlShader, or %NULL on error
 */
GrlShader *
grl_shader_new_from_file (const gchar  *vs_filename,
                          const gchar  *fs_filename,
                          GError      **error)
{
    GrlShader *self;
    GrlShaderPrivate *priv;
    Shader shader;

    self = g_object_new (GRL_TYPE_SHADER, NULL);
    priv = grl_shader_get_instance_private (self);

    shader = LoadShader (vs_filename, fs_filename);

    if (shader.id == 0)
    {
        g_set_error (error,
                     G_IO_ERROR,
                     G_IO_ERROR_FAILED,
                     "Failed to load shader from files: vs=%s, fs=%s",
                     vs_filename ? vs_filename : "(default)",
                     fs_filename ? fs_filename : "(default)");
        g_object_unref (self);
        return NULL;
    }

    priv->shader = shader;
    priv->valid = TRUE;

    return self;
}

/**
 * grl_shader_new_from_memory:
 * @vs_code: (nullable): Vertex shader code, or %NULL
 * @fs_code: (nullable): Fragment shader code, or %NULL
 * @error: (nullable): Return location for error, or %NULL
 *
 * Creates a shader from code strings.
 *
 * Returns: (transfer full) (nullable): A new #GrlShader, or %NULL on error
 */
GrlShader *
grl_shader_new_from_memory (const gchar  *vs_code,
                            const gchar  *fs_code,
                            GError      **error)
{
    GrlShader *self;
    GrlShaderPrivate *priv;
    Shader shader;

    self = g_object_new (GRL_TYPE_SHADER, NULL);
    priv = grl_shader_get_instance_private (self);

    shader = LoadShaderFromMemory (vs_code, fs_code);

    if (shader.id == 0)
    {
        g_set_error (error,
                     G_IO_ERROR,
                     G_IO_ERROR_FAILED,
                     "Failed to compile shader from memory");
        g_object_unref (self);
        return NULL;
    }

    priv->shader = shader;
    priv->valid = TRUE;

    return self;
}

/**
 * grl_shader_is_valid:
 * @self: A #GrlShader
 *
 * Checks if the shader is valid.
 *
 * Returns: %TRUE if valid
 */
gboolean
grl_shader_is_valid (GrlShader *self)
{
    GrlShaderPrivate *priv;

    g_return_val_if_fail (GRL_IS_SHADER (self), FALSE);

    priv = grl_shader_get_instance_private (self);
    return priv->valid;
}

/**
 * grl_shader_begin:
 * @self: A #GrlShader
 *
 * Begins custom shader mode.
 */
void
grl_shader_begin (GrlShader *self)
{
    GrlShaderPrivate *priv;

    g_return_if_fail (GRL_IS_SHADER (self));

    priv = grl_shader_get_instance_private (self);

    if (priv->valid)
        BeginShaderMode (priv->shader);
}

/**
 * grl_shader_end:
 * @self: A #GrlShader
 *
 * Ends custom shader mode.
 */
void
grl_shader_end (GrlShader *self)
{
    g_return_if_fail (GRL_IS_SHADER (self));

    EndShaderMode ();
}

/**
 * grl_shader_get_location:
 * @self: A #GrlShader
 * @uniform_name: The uniform name
 *
 * Gets a uniform location.
 *
 * Returns: The location, or -1 if not found
 */
gint
grl_shader_get_location (GrlShader   *self,
                         const gchar *uniform_name)
{
    GrlShaderPrivate *priv;

    g_return_val_if_fail (GRL_IS_SHADER (self), -1);
    g_return_val_if_fail (uniform_name != NULL, -1);

    priv = grl_shader_get_instance_private (self);

    if (!priv->valid)
        return -1;

    return GetShaderLocation (priv->shader, uniform_name);
}

/**
 * grl_shader_get_location_attrib:
 * @self: A #GrlShader
 * @attrib_name: The attribute name
 *
 * Gets an attribute location.
 *
 * Returns: The location, or -1 if not found
 */
gint
grl_shader_get_location_attrib (GrlShader   *self,
                                const gchar *attrib_name)
{
    GrlShaderPrivate *priv;

    g_return_val_if_fail (GRL_IS_SHADER (self), -1);
    g_return_val_if_fail (attrib_name != NULL, -1);

    priv = grl_shader_get_instance_private (self);

    if (!priv->valid)
        return -1;

    return GetShaderLocationAttrib (priv->shader, attrib_name);
}

/**
 * grl_shader_set_value_float:
 * @self: A #GrlShader
 * @loc_index: The uniform location
 * @value: The float value
 *
 * Sets a float uniform value.
 */
void
grl_shader_set_value_float (GrlShader *self,
                            gint       loc_index,
                            gfloat     value)
{
    GrlShaderPrivate *priv;

    g_return_if_fail (GRL_IS_SHADER (self));

    priv = grl_shader_get_instance_private (self);

    if (priv->valid && loc_index >= 0)
        SetShaderValue (priv->shader, loc_index, &value, SHADER_UNIFORM_FLOAT);
}

/**
 * grl_shader_set_value_int:
 * @self: A #GrlShader
 * @loc_index: The uniform location
 * @value: The integer value
 *
 * Sets an integer uniform value.
 */
void
grl_shader_set_value_int (GrlShader *self,
                          gint       loc_index,
                          gint       value)
{
    GrlShaderPrivate *priv;

    g_return_if_fail (GRL_IS_SHADER (self));

    priv = grl_shader_get_instance_private (self);

    if (priv->valid && loc_index >= 0)
        SetShaderValue (priv->shader, loc_index, &value, SHADER_UNIFORM_INT);
}

/**
 * grl_shader_set_value_vec2:
 * @self: A #GrlShader
 * @loc_index: The uniform location
 * @x: X component
 * @y: Y component
 *
 * Sets a vec2 uniform value.
 */
void
grl_shader_set_value_vec2 (GrlShader *self,
                           gint       loc_index,
                           gfloat     x,
                           gfloat     y)
{
    GrlShaderPrivate *priv;
    gfloat value[2];

    g_return_if_fail (GRL_IS_SHADER (self));

    priv = grl_shader_get_instance_private (self);

    if (priv->valid && loc_index >= 0)
    {
        value[0] = x;
        value[1] = y;
        SetShaderValue (priv->shader, loc_index, value, SHADER_UNIFORM_VEC2);
    }
}

/**
 * grl_shader_set_value_vec3:
 * @self: A #GrlShader
 * @loc_index: The uniform location
 * @x: X component
 * @y: Y component
 * @z: Z component
 *
 * Sets a vec3 uniform value.
 */
void
grl_shader_set_value_vec3 (GrlShader *self,
                           gint       loc_index,
                           gfloat     x,
                           gfloat     y,
                           gfloat     z)
{
    GrlShaderPrivate *priv;
    gfloat value[3];

    g_return_if_fail (GRL_IS_SHADER (self));

    priv = grl_shader_get_instance_private (self);

    if (priv->valid && loc_index >= 0)
    {
        value[0] = x;
        value[1] = y;
        value[2] = z;
        SetShaderValue (priv->shader, loc_index, value, SHADER_UNIFORM_VEC3);
    }
}

/**
 * grl_shader_set_value_vec4:
 * @self: A #GrlShader
 * @loc_index: The uniform location
 * @x: X component
 * @y: Y component
 * @z: Z component
 * @w: W component
 *
 * Sets a vec4 uniform value.
 */
void
grl_shader_set_value_vec4 (GrlShader *self,
                           gint       loc_index,
                           gfloat     x,
                           gfloat     y,
                           gfloat     z,
                           gfloat     w)
{
    GrlShaderPrivate *priv;
    gfloat value[4];

    g_return_if_fail (GRL_IS_SHADER (self));

    priv = grl_shader_get_instance_private (self);

    if (priv->valid && loc_index >= 0)
    {
        value[0] = x;
        value[1] = y;
        value[2] = z;
        value[3] = w;
        SetShaderValue (priv->shader, loc_index, value, SHADER_UNIFORM_VEC4);
    }
}

/**
 * grl_shader_set_value_texture:
 * @self: A #GrlShader
 * @loc_index: The uniform location
 * @texture: The texture to bind
 *
 * Sets a sampler2D uniform to a texture.
 */
void
grl_shader_set_value_texture (GrlShader  *self,
                              gint        loc_index,
                              GrlTexture *texture)
{
    GrlShaderPrivate *priv;
    Texture2D rl_texture;

    g_return_if_fail (GRL_IS_SHADER (self));
    g_return_if_fail (GRL_IS_TEXTURE (texture));

    priv = grl_shader_get_instance_private (self);

    if (!priv->valid || loc_index < 0)
        return;

    /* Get the internal raylib texture from GrlTexture */
    rl_texture = *(Texture2D *)grl_texture_get_handle (texture);
    SetShaderValueTexture (priv->shader, loc_index, rl_texture);
}

/**
 * grl_shader_get_handle:
 * @self: A #GrlShader
 *
 * Gets the internal raylib Shader pointer.
 * This is for internal use and advanced users only.
 *
 * Returns: (transfer none): Internal shader pointer
 */
gpointer
grl_shader_get_handle (GrlShader *self)
{
    GrlShaderPrivate *priv;

    g_return_val_if_fail (GRL_IS_SHADER (self), NULL);

    priv = grl_shader_get_instance_private (self);

    return &priv->shader;
}
