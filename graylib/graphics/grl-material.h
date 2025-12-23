/* grl-material.h
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Material for 3D rendering with textures and shader.
 */

#pragma once

#if !defined(GRAYLIB_INSIDE) && !defined(GRAYLIB_COMPILATION)
#error "Only <graylib.h> can be included directly."
#endif

#include <glib-object.h>
#include "../grl-version.h"
#include "grl-texture.h"
#include "grl-shader.h"
#include "../math/grl-color.h"

G_BEGIN_DECLS

#define GRL_TYPE_MATERIAL (grl_material_get_type())

GRL_AVAILABLE_IN_ALL
G_DECLARE_DERIVABLE_TYPE (GrlMaterial, grl_material, GRL, MATERIAL, GObject)

struct _GrlMaterialClass
{
    GObjectClass parent_class;

    gpointer _reserved[8];
};

/**
 * GrlMaterialMapType:
 * @GRL_MATERIAL_MAP_ALBEDO: Albedo/diffuse color map (default texture)
 * @GRL_MATERIAL_MAP_METALNESS: Metalness map
 * @GRL_MATERIAL_MAP_NORMAL: Normal map
 * @GRL_MATERIAL_MAP_ROUGHNESS: Roughness map
 * @GRL_MATERIAL_MAP_OCCLUSION: Ambient occlusion map
 * @GRL_MATERIAL_MAP_EMISSION: Emission map
 * @GRL_MATERIAL_MAP_HEIGHT: Height/displacement map
 * @GRL_MATERIAL_MAP_CUBEMAP: Cubemap for reflections
 * @GRL_MATERIAL_MAP_IRRADIANCE: Irradiance map for IBL
 * @GRL_MATERIAL_MAP_PREFILTER: Prefiltered environment map
 * @GRL_MATERIAL_MAP_BRDF: BRDF lookup texture
 *
 * Types of texture maps that can be assigned to materials.
 */
typedef enum
{
    GRL_MATERIAL_MAP_ALBEDO = 0,
    GRL_MATERIAL_MAP_METALNESS,
    GRL_MATERIAL_MAP_NORMAL,
    GRL_MATERIAL_MAP_ROUGHNESS,
    GRL_MATERIAL_MAP_OCCLUSION,
    GRL_MATERIAL_MAP_EMISSION,
    GRL_MATERIAL_MAP_HEIGHT,
    GRL_MATERIAL_MAP_CUBEMAP,
    GRL_MATERIAL_MAP_IRRADIANCE,
    GRL_MATERIAL_MAP_PREFILTER,
    GRL_MATERIAL_MAP_BRDF,
    GRL_MATERIAL_MAP_COUNT
} GrlMaterialMapType;

/**
 * grl_material_new:
 *
 * Creates a new material with default values.
 *
 * Returns: (transfer full): A new #GrlMaterial
 */
GRL_AVAILABLE_IN_ALL
GrlMaterial *       grl_material_new                (void);

/**
 * grl_material_new_default:
 *
 * Creates a new material loaded with default raylib material settings.
 *
 * Returns: (transfer full): A new #GrlMaterial with defaults
 */
GRL_AVAILABLE_IN_ALL
GrlMaterial *       grl_material_new_default        (void);

/**
 * grl_material_set_shader:
 * @self: A #GrlMaterial
 * @shader: (nullable): Shader to use
 *
 * Sets the shader for this material.
 * Pass %NULL to use the default shader.
 */
GRL_AVAILABLE_IN_ALL
void                grl_material_set_shader         (GrlMaterial        *self,
                                                     GrlShader          *shader);

/**
 * grl_material_get_shader:
 * @self: A #GrlMaterial
 *
 * Gets the shader used by this material.
 *
 * Returns: (transfer none) (nullable): The shader, or %NULL
 */
GRL_AVAILABLE_IN_ALL
GrlShader *         grl_material_get_shader         (GrlMaterial        *self);

/**
 * grl_material_set_texture:
 * @self: A #GrlMaterial
 * @map_type: Which texture map to set
 * @texture: (nullable): The texture to use
 *
 * Sets a texture for the specified map type.
 * Pass %NULL to clear the texture.
 */
GRL_AVAILABLE_IN_ALL
void                grl_material_set_texture        (GrlMaterial        *self,
                                                     GrlMaterialMapType  map_type,
                                                     GrlTexture         *texture);

/**
 * grl_material_get_texture:
 * @self: A #GrlMaterial
 * @map_type: Which texture map to get
 *
 * Gets the texture for the specified map type.
 *
 * Returns: (transfer none) (nullable): The texture, or %NULL
 */
GRL_AVAILABLE_IN_ALL
GrlTexture *        grl_material_get_texture        (GrlMaterial        *self,
                                                     GrlMaterialMapType  map_type);

/**
 * grl_material_set_map_color:
 * @self: A #GrlMaterial
 * @map_type: Which texture map to set color for
 * @color: The color value
 *
 * Sets the color tint for a texture map.
 */
GRL_AVAILABLE_IN_ALL
void                grl_material_set_map_color      (GrlMaterial        *self,
                                                     GrlMaterialMapType  map_type,
                                                     GrlColor           *color);

/**
 * grl_material_get_map_color:
 * @self: A #GrlMaterial
 * @map_type: Which texture map to get color for
 *
 * Gets the color tint for a texture map.
 *
 * Returns: (transfer full): The map color
 */
GRL_AVAILABLE_IN_ALL
GrlColor *          grl_material_get_map_color      (GrlMaterial        *self,
                                                     GrlMaterialMapType  map_type);

/**
 * grl_material_set_map_value:
 * @self: A #GrlMaterial
 * @map_type: Which texture map to set value for
 * @value: The scalar value
 *
 * Sets a scalar value for a texture map (e.g., metalness, roughness).
 */
GRL_AVAILABLE_IN_ALL
void                grl_material_set_map_value      (GrlMaterial        *self,
                                                     GrlMaterialMapType  map_type,
                                                     gfloat              value);

/**
 * grl_material_get_map_value:
 * @self: A #GrlMaterial
 * @map_type: Which texture map to get value for
 *
 * Gets the scalar value for a texture map.
 *
 * Returns: The map value
 */
GRL_AVAILABLE_IN_ALL
gfloat              grl_material_get_map_value      (GrlMaterial        *self,
                                                     GrlMaterialMapType  map_type);

/**
 * grl_material_is_valid:
 * @self: A #GrlMaterial
 *
 * Checks if the material is valid.
 *
 * Returns: %TRUE if valid
 */
GRL_AVAILABLE_IN_ALL
gboolean            grl_material_is_valid           (GrlMaterial        *self);

/**
 * grl_material_get_handle:
 * @self: A #GrlMaterial
 *
 * Gets the internal raylib Material pointer.
 * This is for internal use and advanced users.
 *
 * Returns: (transfer none): Internal material pointer
 */
GRL_AVAILABLE_IN_ALL
gpointer            grl_material_get_handle         (GrlMaterial        *self);

G_END_DECLS
