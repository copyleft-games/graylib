/* grl-material.c
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#include "grl-material.h"
#include <raylib.h>

/**
 * SECTION:grl-material
 * @Title: GrlMaterial
 * @Short_description: Material for 3D rendering
 *
 * #GrlMaterial defines the visual properties of 3D surfaces including
 * textures, colors, and shaders. Materials support physically-based
 * rendering (PBR) with various texture maps.
 *
 * # Texture Maps
 *
 * Materials can have multiple texture maps:
 * - Albedo: Base color/diffuse texture
 * - Metalness: Metal vs non-metal surface
 * - Normal: Surface detail normals
 * - Roughness: Surface roughness
 * - Occlusion: Ambient occlusion
 * - Emission: Self-illumination
 */

typedef struct
{
    Material     material;
    gboolean     valid;
    GrlShader   *shader;
    GrlTexture  *textures[GRL_MATERIAL_MAP_COUNT];
} GrlMaterialPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (GrlMaterial, grl_material, G_TYPE_OBJECT)

enum
{
    PROP_0,
    PROP_VALID,
    N_PROPS
};

static GParamSpec *properties[N_PROPS];

static void
grl_material_dispose (GObject *object)
{
    GrlMaterial *self = GRL_MATERIAL (object);
    GrlMaterialPrivate *priv = grl_material_get_instance_private (self);
    gint i;

    g_clear_object (&priv->shader);

    for (i = 0; i < GRL_MATERIAL_MAP_COUNT; i++)
        g_clear_object (&priv->textures[i]);

    G_OBJECT_CLASS (grl_material_parent_class)->dispose (object);
}

static void
grl_material_finalize (GObject *object)
{
    GrlMaterial *self = GRL_MATERIAL (object);
    GrlMaterialPrivate *priv = grl_material_get_instance_private (self);

    if (priv->valid)
    {
        /* Note: Don't unload the shader here, it's managed separately */
        /* Also don't unload textures, they're managed by GrlTexture objects */
        priv->valid = FALSE;
    }

    G_OBJECT_CLASS (grl_material_parent_class)->finalize (object);
}

static void
grl_material_get_property (GObject    *object,
                           guint       prop_id,
                           GValue     *value,
                           GParamSpec *pspec)
{
    GrlMaterial *self = GRL_MATERIAL (object);
    GrlMaterialPrivate *priv = grl_material_get_instance_private (self);

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
grl_material_class_init (GrlMaterialClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->dispose = grl_material_dispose;
    object_class->finalize = grl_material_finalize;
    object_class->get_property = grl_material_get_property;

    /**
     * GrlMaterial:valid:
     *
     * Whether the material is valid.
     */
    properties[PROP_VALID] =
        g_param_spec_boolean ("valid",
                              "Valid",
                              "Whether material is valid",
                              FALSE,
                              G_PARAM_READABLE |
                              G_PARAM_STATIC_STRINGS);

    g_object_class_install_properties (object_class, N_PROPS, properties);
}

static void
grl_material_init (GrlMaterial *self)
{
    GrlMaterialPrivate *priv = grl_material_get_instance_private (self);
    gint i;

    priv->valid = FALSE;
    priv->shader = NULL;
    memset (&priv->material, 0, sizeof (Material));

    for (i = 0; i < GRL_MATERIAL_MAP_COUNT; i++)
        priv->textures[i] = NULL;
}

/**
 * grl_material_new:
 *
 * Creates a new material with default values.
 * The material will have no shader or textures assigned.
 *
 * Returns: (transfer full): A new #GrlMaterial
 */
GrlMaterial *
grl_material_new (void)
{
    GrlMaterial *self;
    GrlMaterialPrivate *priv;

    self = g_object_new (GRL_TYPE_MATERIAL, NULL);
    priv = grl_material_get_instance_private (self);

    /* Initialize with identity shader (default) */
    priv->material.shader.id = 0;
    priv->valid = TRUE;

    return self;
}

/**
 * grl_material_new_default:
 *
 * Creates a new material with raylib's default material settings.
 * This includes the default shader and a white default texture.
 *
 * Returns: (transfer full): A new #GrlMaterial with defaults
 */
GrlMaterial *
grl_material_new_default (void)
{
    GrlMaterial *self;
    GrlMaterialPrivate *priv;

    self = g_object_new (GRL_TYPE_MATERIAL, NULL);
    priv = grl_material_get_instance_private (self);

    priv->material = LoadMaterialDefault ();
    priv->valid = TRUE;

    return self;
}

/**
 * grl_material_set_shader:
 * @self: A #GrlMaterial
 * @shader: (nullable): Shader to use
 *
 * Sets the shader for this material.
 * Pass %NULL to use the default shader.
 */
void
grl_material_set_shader (GrlMaterial *self,
                         GrlShader   *shader)
{
    GrlMaterialPrivate *priv;

    g_return_if_fail (GRL_IS_MATERIAL (self));
    g_return_if_fail (shader == NULL || GRL_IS_SHADER (shader));

    priv = grl_material_get_instance_private (self);

    if (priv->shader == shader)
        return;

    g_clear_object (&priv->shader);

    if (shader != NULL)
    {
        priv->shader = g_object_ref (shader);
        priv->material.shader = *(Shader *)grl_shader_get_handle (shader);
    }
    else
    {
        /* Use default shader */
        priv->material.shader = (Shader){ 0 };
    }
}

/**
 * grl_material_get_shader:
 * @self: A #GrlMaterial
 *
 * Gets the shader used by this material.
 *
 * Returns: (transfer none) (nullable): The shader, or %NULL
 */
GrlShader *
grl_material_get_shader (GrlMaterial *self)
{
    GrlMaterialPrivate *priv;

    g_return_val_if_fail (GRL_IS_MATERIAL (self), NULL);

    priv = grl_material_get_instance_private (self);

    return priv->shader;
}

/**
 * grl_material_set_texture:
 * @self: A #GrlMaterial
 * @map_type: Which texture map to set
 * @texture: (nullable): The texture to use
 *
 * Sets a texture for the specified map type.
 * Pass %NULL to clear the texture.
 */
void
grl_material_set_texture (GrlMaterial        *self,
                          GrlMaterialMapType  map_type,
                          GrlTexture         *texture)
{
    GrlMaterialPrivate *priv;

    g_return_if_fail (GRL_IS_MATERIAL (self));
    g_return_if_fail (map_type >= 0 && map_type < GRL_MATERIAL_MAP_COUNT);
    g_return_if_fail (texture == NULL || GRL_IS_TEXTURE (texture));

    priv = grl_material_get_instance_private (self);

    if (priv->textures[map_type] == texture)
        return;

    g_clear_object (&priv->textures[map_type]);

    if (texture != NULL)
    {
        priv->textures[map_type] = g_object_ref (texture);
        priv->material.maps[map_type].texture =
            *(Texture2D *)grl_texture_get_handle (texture);
    }
    else
    {
        priv->material.maps[map_type].texture = (Texture2D){ 0 };
    }
}

/**
 * grl_material_get_texture:
 * @self: A #GrlMaterial
 * @map_type: Which texture map to get
 *
 * Gets the texture for the specified map type.
 *
 * Returns: (transfer none) (nullable): The texture, or %NULL
 */
GrlTexture *
grl_material_get_texture (GrlMaterial        *self,
                          GrlMaterialMapType  map_type)
{
    GrlMaterialPrivate *priv;

    g_return_val_if_fail (GRL_IS_MATERIAL (self), NULL);
    g_return_val_if_fail (map_type >= 0 && map_type < GRL_MATERIAL_MAP_COUNT, NULL);

    priv = grl_material_get_instance_private (self);

    return priv->textures[map_type];
}

/**
 * grl_material_set_map_color:
 * @self: A #GrlMaterial
 * @map_type: Which texture map to set color for
 * @color: The color value
 *
 * Sets the color tint for a texture map.
 * This color is multiplied with the texture color during rendering.
 */
void
grl_material_set_map_color (GrlMaterial        *self,
                            GrlMaterialMapType  map_type,
                            GrlColor           *color)
{
    GrlMaterialPrivate *priv;

    g_return_if_fail (GRL_IS_MATERIAL (self));
    g_return_if_fail (map_type >= 0 && map_type < GRL_MATERIAL_MAP_COUNT);
    g_return_if_fail (color != NULL);

    priv = grl_material_get_instance_private (self);

    priv->material.maps[map_type].color.r = grl_color_get_r (color);
    priv->material.maps[map_type].color.g = grl_color_get_g (color);
    priv->material.maps[map_type].color.b = grl_color_get_b (color);
    priv->material.maps[map_type].color.a = grl_color_get_a (color);
}

/**
 * grl_material_get_map_color:
 * @self: A #GrlMaterial
 * @map_type: Which texture map to get color for
 *
 * Gets the color tint for a texture map.
 *
 * Returns: (transfer full): The map color
 */
GrlColor *
grl_material_get_map_color (GrlMaterial        *self,
                            GrlMaterialMapType  map_type)
{
    GrlMaterialPrivate *priv;
    Color c;

    g_return_val_if_fail (GRL_IS_MATERIAL (self), NULL);
    g_return_val_if_fail (map_type >= 0 && map_type < GRL_MATERIAL_MAP_COUNT, NULL);

    priv = grl_material_get_instance_private (self);
    c = priv->material.maps[map_type].color;

    return grl_color_new (c.r, c.g, c.b, c.a);
}

/**
 * grl_material_set_map_value:
 * @self: A #GrlMaterial
 * @map_type: Which texture map to set value for
 * @value: The scalar value
 *
 * Sets a scalar value for a texture map.
 * Used for properties like metalness and roughness when no texture is set.
 */
void
grl_material_set_map_value (GrlMaterial        *self,
                            GrlMaterialMapType  map_type,
                            gfloat              value)
{
    GrlMaterialPrivate *priv;

    g_return_if_fail (GRL_IS_MATERIAL (self));
    g_return_if_fail (map_type >= 0 && map_type < GRL_MATERIAL_MAP_COUNT);

    priv = grl_material_get_instance_private (self);

    priv->material.maps[map_type].value = value;
}

/**
 * grl_material_get_map_value:
 * @self: A #GrlMaterial
 * @map_type: Which texture map to get value for
 *
 * Gets the scalar value for a texture map.
 *
 * Returns: The map value
 */
gfloat
grl_material_get_map_value (GrlMaterial        *self,
                            GrlMaterialMapType  map_type)
{
    GrlMaterialPrivate *priv;

    g_return_val_if_fail (GRL_IS_MATERIAL (self), 0.0f);
    g_return_val_if_fail (map_type >= 0 && map_type < GRL_MATERIAL_MAP_COUNT, 0.0f);

    priv = grl_material_get_instance_private (self);

    return priv->material.maps[map_type].value;
}

/**
 * grl_material_is_valid:
 * @self: A #GrlMaterial
 *
 * Checks if the material is valid and ready for use.
 *
 * Returns: %TRUE if valid
 */
gboolean
grl_material_is_valid (GrlMaterial *self)
{
    GrlMaterialPrivate *priv;

    g_return_val_if_fail (GRL_IS_MATERIAL (self), FALSE);

    priv = grl_material_get_instance_private (self);

    return priv->valid;
}

/**
 * grl_material_get_handle:
 * @self: A #GrlMaterial
 *
 * Gets the internal raylib Material pointer.
 * This is for internal use and advanced users only.
 *
 * Returns: (transfer none): Internal material pointer
 */
gpointer
grl_material_get_handle (GrlMaterial *self)
{
    GrlMaterialPrivate *priv;

    g_return_val_if_fail (GRL_IS_MATERIAL (self), NULL);

    priv = grl_material_get_instance_private (self);

    return &priv->material;
}
