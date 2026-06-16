/* grl-rlgl-shader.c
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Low-level shader operations for rlgl.
 */

#include "config.h"
#include "grl-rlgl.h"
#include <rlgl.h>

/**
 * grl_rlgl_load_shader_code:
 * @vs_code: Vertex shader source code
 * @fs_code: Fragment shader source code
 *
 * Compiles and links a shader program from source code.
 *
 * Returns: Shader program ID, or 0 on failure
 */
guint
grl_rlgl_load_shader_code (const gchar *vs_code,
                           const gchar *fs_code)
{
    /* raylib 6.0 renamed rlLoadShaderCode -> rlLoadShaderProgram (string form) */
    return rlLoadShaderProgram (vs_code, fs_code);
}

/**
 * grl_rlgl_compile_shader:
 * @shader_code: Shader source code
 * @type: Shader type (vertex, fragment, or compute)
 *
 * Compiles a single shader from source code.
 *
 * Returns: Shader ID, or 0 on failure
 */
guint
grl_rlgl_compile_shader (const gchar *shader_code,
                         GrlRlglShaderType type)
{
    g_return_val_if_fail (shader_code != NULL, 0);

    /* raylib 6.0 renamed rlCompileShader -> rlLoadShader */
    return rlLoadShader (shader_code, (int)type);
}

/**
 * grl_rlgl_load_shader_program:
 * @vs_id: Compiled vertex shader ID
 * @fs_id: Compiled fragment shader ID
 *
 * Links a shader program from compiled shaders.
 *
 * Returns: Shader program ID, or 0 on failure
 */
guint
grl_rlgl_load_shader_program (guint vs_id,
                              guint fs_id)
{
    /* raylib 6.0: id-based linking is now rlLoadShaderProgramEx
     * (rlLoadShaderProgram now takes code strings) */
    return rlLoadShaderProgramEx (vs_id, fs_id);
}

/**
 * grl_rlgl_unload_shader_program:
 * @id: Shader program ID to unload
 *
 * Deletes a shader program.
 */
void
grl_rlgl_unload_shader_program (guint id)
{
    rlUnloadShaderProgram (id);
}

/**
 * grl_rlgl_get_location_uniform:
 * @shader_id: Shader program ID
 * @uniform_name: Name of the uniform
 *
 * Gets the location of a uniform variable.
 *
 * Returns: Uniform location, or -1 if not found
 */
gint
grl_rlgl_get_location_uniform (guint shader_id,
                               const gchar *uniform_name)
{
    g_return_val_if_fail (uniform_name != NULL, -1);

    return rlGetLocationUniform (shader_id, uniform_name);
}

/**
 * grl_rlgl_get_location_attrib:
 * @shader_id: Shader program ID
 * @attrib_name: Name of the attribute
 *
 * Gets the location of a vertex attribute.
 *
 * Returns: Attribute location, or -1 if not found
 */
gint
grl_rlgl_get_location_attrib (guint shader_id,
                              const gchar *attrib_name)
{
    g_return_val_if_fail (attrib_name != NULL, -1);

    return rlGetLocationAttrib (shader_id, attrib_name);
}

/**
 * grl_rlgl_set_uniform:
 * @loc_index: Uniform location
 * @value: Pointer to value data
 * @uniform_type: Type of uniform (FLOAT, VEC2, VEC3, VEC4, INT, etc.)
 * @count: Number of elements (for arrays)
 *
 * Sets a uniform value in the current shader.
 */
void
grl_rlgl_set_uniform (gint loc_index,
                      gconstpointer value,
                      gint uniform_type,
                      gint count)
{
    g_return_if_fail (value != NULL);

    rlSetUniform (loc_index, value, uniform_type, count);
}

/**
 * grl_rlgl_set_uniform_matrix:
 * @loc_index: Uniform location
 * @mat: Matrix value
 *
 * Sets a matrix uniform value.
 */
void
grl_rlgl_set_uniform_matrix (gint loc_index,
                             const GrlMatrix *mat)
{
    Matrix m;

    g_return_if_fail (mat != NULL);

    m.m0 = mat->m0;
    m.m1 = mat->m1;
    m.m2 = mat->m2;
    m.m3 = mat->m3;
    m.m4 = mat->m4;
    m.m5 = mat->m5;
    m.m6 = mat->m6;
    m.m7 = mat->m7;
    m.m8 = mat->m8;
    m.m9 = mat->m9;
    m.m10 = mat->m10;
    m.m11 = mat->m11;
    m.m12 = mat->m12;
    m.m13 = mat->m13;
    m.m14 = mat->m14;
    m.m15 = mat->m15;

    rlSetUniformMatrix (loc_index, m);
}

/**
 * grl_rlgl_set_uniform_sampler:
 * @loc_index: Sampler uniform location
 * @texture_id: Texture GPU ID
 *
 * Sets a sampler uniform to a texture.
 */
void
grl_rlgl_set_uniform_sampler (gint loc_index,
                              guint texture_id)
{
    rlSetUniformSampler (loc_index, texture_id);
}

/**
 * grl_rlgl_enable_shader:
 * @id: Shader program ID
 *
 * Binds a shader program for use.
 */
void
grl_rlgl_enable_shader (guint id)
{
    rlEnableShader (id);
}

/**
 * grl_rlgl_disable_shader:
 *
 * Unbinds the current shader program.
 */
void
grl_rlgl_disable_shader (void)
{
    rlDisableShader ();
}
