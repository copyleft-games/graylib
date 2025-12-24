/* grl-shader.h
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Shader programs for custom rendering effects.
 */

#pragma once

#if !defined(GRAYLIB_INSIDE) && !defined(GRAYLIB_COMPILATION)
#error "Only <graylib.h> can be included directly."
#endif

#include <glib-object.h>
#include "../grl-version.h"
#include "../grl-types.h"
#include "../grl-enums.h"

G_BEGIN_DECLS

#define GRL_TYPE_SHADER (grl_shader_get_type())

GRL_AVAILABLE_IN_ALL
G_DECLARE_DERIVABLE_TYPE (GrlShader, grl_shader, GRL, SHADER, GObject)

/**
 * GrlShaderClass:
 * @parent_class: The parent class
 *
 * The class structure for #GrlShader.
 */
struct _GrlShaderClass
{
    GObjectClass parent_class;

    /*< private >*/
    gpointer _reserved[8];
};

/**
 * grl_shader_new_from_file:
 * @vs_filename: (nullable) (type filename): Vertex shader file, or %NULL for default
 * @fs_filename: (nullable) (type filename): Fragment shader file, or %NULL for default
 * @error: (nullable): Return location for error, or %NULL
 *
 * Loads a shader from files. Pass %NULL for either parameter to use
 * the default vertex or fragment shader.
 *
 * Returns: (transfer full) (nullable): A new #GrlShader, or %NULL on error
 */
GRL_AVAILABLE_IN_ALL
GrlShader *         grl_shader_new_from_file            (const gchar        *vs_filename,
                                                         const gchar        *fs_filename,
                                                         GError            **error);

/**
 * grl_shader_new_from_memory:
 * @vs_code: (nullable): Vertex shader code, or %NULL for default
 * @fs_code: (nullable): Fragment shader code, or %NULL for default
 * @error: (nullable): Return location for error, or %NULL
 *
 * Creates a shader from code strings. Pass %NULL for either parameter
 * to use the default vertex or fragment shader.
 *
 * Returns: (transfer full) (nullable): A new #GrlShader, or %NULL on error
 */
GRL_AVAILABLE_IN_ALL
GrlShader *         grl_shader_new_from_memory          (const gchar        *vs_code,
                                                         const gchar        *fs_code,
                                                         GError            **error);

/**
 * grl_shader_is_valid:
 * @self: A #GrlShader
 *
 * Checks if the shader is valid (successfully loaded).
 *
 * Returns: %TRUE if the shader is valid
 */
GRL_AVAILABLE_IN_ALL
gboolean            grl_shader_is_valid                 (GrlShader          *self);

/**
 * grl_shader_begin:
 * @self: A #GrlShader
 *
 * Begins custom shader mode. All drawing operations until
 * grl_shader_end() is called will use this shader.
 */
GRL_AVAILABLE_IN_ALL
void                grl_shader_begin                    (GrlShader          *self);

/**
 * grl_shader_end:
 * @self: A #GrlShader
 *
 * Ends custom shader mode and returns to the default shader.
 */
GRL_AVAILABLE_IN_ALL
void                grl_shader_end                      (GrlShader          *self);

/**
 * grl_shader_get_location:
 * @self: A #GrlShader
 * @uniform_name: The name of the uniform variable
 *
 * Gets the location of a uniform variable in the shader.
 *
 * Returns: The uniform location, or -1 if not found
 */
GRL_AVAILABLE_IN_ALL
gint                grl_shader_get_location             (GrlShader          *self,
                                                         const gchar        *uniform_name);

/**
 * grl_shader_get_location_attrib:
 * @self: A #GrlShader
 * @attrib_name: The name of the attribute
 *
 * Gets the location of an attribute in the shader.
 *
 * Returns: The attribute location, or -1 if not found
 */
GRL_AVAILABLE_IN_ALL
gint                grl_shader_get_location_attrib      (GrlShader          *self,
                                                         const gchar        *attrib_name);

/**
 * grl_shader_set_value_float:
 * @self: A #GrlShader
 * @loc_index: The uniform location
 * @value: The float value to set
 *
 * Sets a float uniform value.
 */
GRL_AVAILABLE_IN_ALL
void                grl_shader_set_value_float          (GrlShader          *self,
                                                         gint                loc_index,
                                                         gfloat              value);

/**
 * grl_shader_set_value_int:
 * @self: A #GrlShader
 * @loc_index: The uniform location
 * @value: The integer value to set
 *
 * Sets an integer uniform value.
 */
GRL_AVAILABLE_IN_ALL
void                grl_shader_set_value_int            (GrlShader          *self,
                                                         gint                loc_index,
                                                         gint                value);

/**
 * grl_shader_set_value_vec2:
 * @self: A #GrlShader
 * @loc_index: The uniform location
 * @x: X component
 * @y: Y component
 *
 * Sets a vec2 uniform value.
 */
GRL_AVAILABLE_IN_ALL
void                grl_shader_set_value_vec2           (GrlShader          *self,
                                                         gint                loc_index,
                                                         gfloat              x,
                                                         gfloat              y);

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
GRL_AVAILABLE_IN_ALL
void                grl_shader_set_value_vec3           (GrlShader          *self,
                                                         gint                loc_index,
                                                         gfloat              x,
                                                         gfloat              y,
                                                         gfloat              z);

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
GRL_AVAILABLE_IN_ALL
void                grl_shader_set_value_vec4           (GrlShader          *self,
                                                         gint                loc_index,
                                                         gfloat              x,
                                                         gfloat              y,
                                                         gfloat              z,
                                                         gfloat              w);

/**
 * grl_shader_set_value_texture:
 * @self: A #GrlShader
 * @loc_index: The uniform location
 * @texture: The texture to bind
 *
 * Sets a sampler2D uniform to a texture.
 */
GRL_AVAILABLE_IN_ALL
void                grl_shader_set_value_texture        (GrlShader          *self,
                                                         gint                loc_index,
                                                         GrlTexture         *texture);

/**
 * grl_shader_get_handle:
 * @self: A #GrlShader
 *
 * Gets the internal raylib Shader pointer.
 * This is for internal use and advanced users.
 *
 * Returns: (transfer none): Internal shader pointer
 */
GRL_AVAILABLE_IN_ALL
gpointer            grl_shader_get_handle               (GrlShader          *self);

G_END_DECLS
