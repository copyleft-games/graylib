/* grl-rlgl-buffer.c
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * VBO/VAO buffer operations for rlgl.
 */

#include "config.h"
#include "grl-rlgl.h"
#include <rlgl.h>

/**
 * grl_rlgl_load_vertex_array:
 *
 * Creates a new vertex array object (VAO).
 *
 * Returns: VAO ID
 */
guint
grl_rlgl_load_vertex_array (void)
{
    return rlLoadVertexArray ();
}

/**
 * grl_rlgl_load_vertex_buffer:
 * @buffer: Pointer to vertex data
 * @size: Size of data in bytes
 * @dynamic: %TRUE for dynamic buffer, %FALSE for static
 *
 * Creates a new vertex buffer object (VBO).
 *
 * Returns: VBO ID
 */
guint
grl_rlgl_load_vertex_buffer (gconstpointer buffer,
                             gint size,
                             gboolean dynamic)
{
    return rlLoadVertexBuffer (buffer, size, dynamic);
}

/**
 * grl_rlgl_load_vertex_buffer_element:
 * @buffer: Pointer to element data
 * @size: Size of data in bytes
 * @dynamic: %TRUE for dynamic buffer, %FALSE for static
 *
 * Creates a new element buffer object (EBO/IBO).
 *
 * Returns: Buffer ID
 */
guint
grl_rlgl_load_vertex_buffer_element (gconstpointer buffer,
                                     gint size,
                                     gboolean dynamic)
{
    return rlLoadVertexBufferElement (buffer, size, dynamic);
}

/**
 * grl_rlgl_update_vertex_buffer:
 * @buffer_id: VBO ID
 * @data: New data to upload
 * @data_size: Size of data in bytes
 * @offset: Offset in buffer to start writing
 *
 * Updates data in an existing vertex buffer.
 */
void
grl_rlgl_update_vertex_buffer (guint buffer_id,
                               gconstpointer data,
                               gint data_size,
                               gint offset)
{
    rlUpdateVertexBuffer (buffer_id, data, data_size, offset);
}

/**
 * grl_rlgl_update_vertex_buffer_elements:
 * @id: Element buffer ID
 * @data: New element data
 * @data_size: Size of data in bytes
 * @offset: Offset in buffer to start writing
 *
 * Updates data in an existing element buffer.
 */
void
grl_rlgl_update_vertex_buffer_elements (guint id,
                                        gconstpointer data,
                                        gint data_size,
                                        gint offset)
{
    rlUpdateVertexBufferElements (id, data, data_size, offset);
}

/**
 * grl_rlgl_unload_vertex_array:
 * @vao_id: VAO ID to unload
 *
 * Deletes a vertex array object.
 */
void
grl_rlgl_unload_vertex_array (guint vao_id)
{
    rlUnloadVertexArray (vao_id);
}

/**
 * grl_rlgl_unload_vertex_buffer:
 * @vbo_id: VBO ID to unload
 *
 * Deletes a vertex buffer object.
 */
void
grl_rlgl_unload_vertex_buffer (guint vbo_id)
{
    rlUnloadVertexBuffer (vbo_id);
}

/**
 * grl_rlgl_enable_vertex_array:
 * @vao_id: VAO ID to enable
 *
 * Binds a vertex array object.
 *
 * Returns: %TRUE if successful
 */
gboolean
grl_rlgl_enable_vertex_array (guint vao_id)
{
    /* Fix bool/gboolean ABI mismatch */
    unsigned char raw = rlEnableVertexArray (vao_id);
    return raw != 0;
}

/**
 * grl_rlgl_disable_vertex_array:
 *
 * Unbinds the current vertex array object.
 */
void
grl_rlgl_disable_vertex_array (void)
{
    rlDisableVertexArray ();
}

/**
 * grl_rlgl_enable_vertex_buffer:
 * @id: VBO ID to enable
 *
 * Binds a vertex buffer object.
 */
void
grl_rlgl_enable_vertex_buffer (guint id)
{
    rlEnableVertexBuffer (id);
}

/**
 * grl_rlgl_disable_vertex_buffer:
 *
 * Unbinds the current vertex buffer object.
 */
void
grl_rlgl_disable_vertex_buffer (void)
{
    rlDisableVertexBuffer ();
}

/**
 * grl_rlgl_enable_vertex_buffer_element:
 * @id: Element buffer ID to enable
 *
 * Binds an element buffer object.
 */
void
grl_rlgl_enable_vertex_buffer_element (guint id)
{
    rlEnableVertexBufferElement (id);
}

/**
 * grl_rlgl_disable_vertex_buffer_element:
 *
 * Unbinds the current element buffer object.
 */
void
grl_rlgl_disable_vertex_buffer_element (void)
{
    rlDisableVertexBufferElement ();
}

/**
 * grl_rlgl_enable_vertex_attribute:
 * @index: Attribute index to enable
 *
 * Enables a vertex attribute.
 */
void
grl_rlgl_enable_vertex_attribute (guint index)
{
    rlEnableVertexAttribute (index);
}

/**
 * grl_rlgl_disable_vertex_attribute:
 * @index: Attribute index to disable
 *
 * Disables a vertex attribute.
 */
void
grl_rlgl_disable_vertex_attribute (guint index)
{
    rlDisableVertexAttribute (index);
}

/**
 * grl_rlgl_draw_vertex_array:
 * @offset: Starting vertex index
 * @count: Number of vertices to draw
 *
 * Draws vertices from the current VAO.
 */
void
grl_rlgl_draw_vertex_array (gint offset,
                            gint count)
{
    rlDrawVertexArray (offset, count);
}

/**
 * grl_rlgl_draw_vertex_array_elements:
 * @offset: Starting element index
 * @count: Number of elements to draw
 * @buffer: Element buffer (can be NULL if EBO is bound)
 *
 * Draws indexed vertices.
 */
void
grl_rlgl_draw_vertex_array_elements (gint offset,
                                     gint count,
                                     gconstpointer buffer)
{
    rlDrawVertexArrayElements (offset, count, buffer);
}

/**
 * grl_rlgl_draw_vertex_array_instanced:
 * @offset: Starting vertex index
 * @count: Number of vertices per instance
 * @instances: Number of instances to draw
 *
 * Draws instanced vertices.
 */
void
grl_rlgl_draw_vertex_array_instanced (gint offset,
                                      gint count,
                                      gint instances)
{
    rlDrawVertexArrayInstanced (offset, count, instances);
}

/**
 * grl_rlgl_draw_vertex_array_elements_instanced:
 * @offset: Starting element index
 * @count: Number of elements per instance
 * @buffer: Element buffer (can be NULL if EBO is bound)
 * @instances: Number of instances to draw
 *
 * Draws instanced indexed vertices.
 */
void
grl_rlgl_draw_vertex_array_elements_instanced (gint offset,
                                               gint count,
                                               gconstpointer buffer,
                                               gint instances)
{
    rlDrawVertexArrayElementsInstanced (offset, count, buffer, instances);
}
