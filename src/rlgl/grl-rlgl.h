/* grl-rlgl.h
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * GObject/GLib wrappers for raylib's rlgl low-level OpenGL abstraction layer.
 * Provides immediate mode rendering, matrix operations, render state management,
 * texture binding, buffer operations, framebuffer control, and shader management.
 */

#pragma once

#if !defined(GRAYLIB_INSIDE) && !defined(GRAYLIB_COMPILATION)
#error "Only <graylib.h> can be included directly."
#endif

#include <glib-object.h>
#include "../grl-version.h"
#include "../grl-enums.h"
#include "../math/grl-matrix.h"

G_BEGIN_DECLS

/**
 * SECTION:grl-rlgl
 * @title: RLGL Functions
 * @short_description: Low-level OpenGL abstraction layer
 *
 * The rlgl module provides GObject/GLib wrappers around raylib's low-level
 * OpenGL abstraction layer. This includes:
 *
 * - Matrix stack operations (push, pop, translate, rotate, scale)
 * - Immediate mode vertex drawing (begin, end, vertex, color, normal)
 * - Render state management (depth, blend, cull, scissor)
 * - Texture binding and parameters
 * - Vertex buffer object (VBO) and vertex array object (VAO) operations
 * - Framebuffer object (FBO) operations
 * - Low-level shader operations
 *
 * These functions are intended for advanced users who need direct control
 * over the OpenGL pipeline. For most use cases, the higher-level drawing
 * functions in grl-draw.h are recommended.
 */

/*
 * =============================================================================
 * Core / Utility Functions
 * =============================================================================
 */

GRL_AVAILABLE_IN_ALL
void                grl_rlgl_init                       (gint width,
                                                         gint height);

GRL_AVAILABLE_IN_ALL
void                grl_rlgl_close                      (void);

GRL_AVAILABLE_IN_ALL
GrlRlglGlVersion    grl_rlgl_get_version                (void);

GRL_AVAILABLE_IN_ALL
void                grl_rlgl_set_framebuffer_width      (gint width);

GRL_AVAILABLE_IN_ALL
gint                grl_rlgl_get_framebuffer_width      (void);

GRL_AVAILABLE_IN_ALL
void                grl_rlgl_set_framebuffer_height     (gint height);

GRL_AVAILABLE_IN_ALL
gint                grl_rlgl_get_framebuffer_height     (void);

GRL_AVAILABLE_IN_ALL
guint               grl_rlgl_get_texture_id_default     (void);

GRL_AVAILABLE_IN_ALL
guint               grl_rlgl_get_shader_id_default      (void);

GRL_AVAILABLE_IN_ALL
void                grl_rlgl_load_draw_cube             (void);

GRL_AVAILABLE_IN_ALL
void                grl_rlgl_load_draw_quad             (void);

/*
 * =============================================================================
 * Matrix Operations
 * =============================================================================
 */

GRL_AVAILABLE_IN_ALL
void                grl_rlgl_matrix_mode                (GrlRlglMatrixMode mode);

GRL_AVAILABLE_IN_ALL
void                grl_rlgl_push_matrix                (void);

GRL_AVAILABLE_IN_ALL
void                grl_rlgl_pop_matrix                 (void);

GRL_AVAILABLE_IN_ALL
void                grl_rlgl_load_identity              (void);

GRL_AVAILABLE_IN_ALL
void                grl_rlgl_translatef                 (gfloat x,
                                                         gfloat y,
                                                         gfloat z);

GRL_AVAILABLE_IN_ALL
void                grl_rlgl_rotatef                    (gfloat angle,
                                                         gfloat x,
                                                         gfloat y,
                                                         gfloat z);

GRL_AVAILABLE_IN_ALL
void                grl_rlgl_scalef                     (gfloat x,
                                                         gfloat y,
                                                         gfloat z);

GRL_AVAILABLE_IN_ALL
void                grl_rlgl_mult_matrixf               (const gfloat *matf);

GRL_AVAILABLE_IN_ALL
void                grl_rlgl_frustum                    (gdouble left,
                                                         gdouble right,
                                                         gdouble bottom,
                                                         gdouble top,
                                                         gdouble znear,
                                                         gdouble zfar);

GRL_AVAILABLE_IN_ALL
void                grl_rlgl_ortho                      (gdouble left,
                                                         gdouble right,
                                                         gdouble bottom,
                                                         gdouble top,
                                                         gdouble znear,
                                                         gdouble zfar);

GRL_AVAILABLE_IN_ALL
void                grl_rlgl_viewport                   (gint x,
                                                         gint y,
                                                         gint width,
                                                         gint height);

GRL_AVAILABLE_IN_ALL
void                grl_rlgl_set_clip_planes            (gdouble near_plane,
                                                         gdouble far_plane);

GRL_AVAILABLE_IN_ALL
gdouble             grl_rlgl_get_cull_distance_near     (void);

GRL_AVAILABLE_IN_ALL
gdouble             grl_rlgl_get_cull_distance_far      (void);

GRL_AVAILABLE_IN_ALL
GrlMatrix *         grl_rlgl_get_matrix_modelview       (void);

GRL_AVAILABLE_IN_ALL
GrlMatrix *         grl_rlgl_get_matrix_projection      (void);

GRL_AVAILABLE_IN_ALL
GrlMatrix *         grl_rlgl_get_matrix_transform       (void);

GRL_AVAILABLE_IN_ALL
void                grl_rlgl_set_matrix_projection      (const GrlMatrix *proj);

GRL_AVAILABLE_IN_ALL
void                grl_rlgl_set_matrix_modelview       (const GrlMatrix *view);

/*
 * =============================================================================
 * Immediate Mode Vertex Operations
 * =============================================================================
 */

GRL_AVAILABLE_IN_ALL
void                grl_rlgl_begin                      (GrlRlglDrawMode mode);

GRL_AVAILABLE_IN_ALL
void                grl_rlgl_end                        (void);

GRL_AVAILABLE_IN_ALL
void                grl_rlgl_vertex2i                   (gint x,
                                                         gint y);

GRL_AVAILABLE_IN_ALL
void                grl_rlgl_vertex2f                   (gfloat x,
                                                         gfloat y);

GRL_AVAILABLE_IN_ALL
void                grl_rlgl_vertex3f                   (gfloat x,
                                                         gfloat y,
                                                         gfloat z);

GRL_AVAILABLE_IN_ALL
void                grl_rlgl_tex_coord2f                (gfloat x,
                                                         gfloat y);

GRL_AVAILABLE_IN_ALL
void                grl_rlgl_normal3f                   (gfloat x,
                                                         gfloat y,
                                                         gfloat z);

GRL_AVAILABLE_IN_ALL
void                grl_rlgl_color4ub                   (guint8 r,
                                                         guint8 g,
                                                         guint8 b,
                                                         guint8 a);

GRL_AVAILABLE_IN_ALL
void                grl_rlgl_color3f                    (gfloat r,
                                                         gfloat g,
                                                         gfloat b);

GRL_AVAILABLE_IN_ALL
void                grl_rlgl_color4f                    (gfloat r,
                                                         gfloat g,
                                                         gfloat b,
                                                         gfloat a);

/*
 * =============================================================================
 * Render State Management
 * =============================================================================
 */

GRL_AVAILABLE_IN_ALL
void                grl_rlgl_enable_color_blend         (void);

GRL_AVAILABLE_IN_ALL
void                grl_rlgl_disable_color_blend        (void);

GRL_AVAILABLE_IN_ALL
void                grl_rlgl_enable_depth_test          (void);

GRL_AVAILABLE_IN_ALL
void                grl_rlgl_disable_depth_test         (void);

GRL_AVAILABLE_IN_ALL
void                grl_rlgl_enable_depth_mask          (void);

GRL_AVAILABLE_IN_ALL
void                grl_rlgl_disable_depth_mask         (void);

GRL_AVAILABLE_IN_ALL
void                grl_rlgl_enable_backface_culling    (void);

GRL_AVAILABLE_IN_ALL
void                grl_rlgl_disable_backface_culling   (void);

GRL_AVAILABLE_IN_ALL
void                grl_rlgl_color_mask                 (gboolean r,
                                                         gboolean g,
                                                         gboolean b,
                                                         gboolean a);

GRL_AVAILABLE_IN_ALL
void                grl_rlgl_set_cull_face              (GrlRlglCullMode mode);

GRL_AVAILABLE_IN_ALL
void                grl_rlgl_enable_scissor_test        (void);

GRL_AVAILABLE_IN_ALL
void                grl_rlgl_disable_scissor_test       (void);

GRL_AVAILABLE_IN_ALL
void                grl_rlgl_scissor                    (gint x,
                                                         gint y,
                                                         gint width,
                                                         gint height);

GRL_AVAILABLE_IN_ALL
void                grl_rlgl_enable_wire_mode           (void);

GRL_AVAILABLE_IN_ALL
void                grl_rlgl_enable_point_mode          (void);

GRL_AVAILABLE_IN_ALL
void                grl_rlgl_disable_wire_mode          (void);

GRL_AVAILABLE_IN_ALL
void                grl_rlgl_set_line_width             (gfloat width);

GRL_AVAILABLE_IN_ALL
gfloat              grl_rlgl_get_line_width             (void);

GRL_AVAILABLE_IN_ALL
void                grl_rlgl_enable_smooth_lines        (void);

GRL_AVAILABLE_IN_ALL
void                grl_rlgl_disable_smooth_lines       (void);

GRL_AVAILABLE_IN_ALL
void                grl_rlgl_enable_stereo_render       (void);

GRL_AVAILABLE_IN_ALL
void                grl_rlgl_disable_stereo_render      (void);

GRL_AVAILABLE_IN_ALL
gboolean            grl_rlgl_is_stereo_render_enabled   (void);

GRL_AVAILABLE_IN_ALL
void                grl_rlgl_clear_color                (guint8 r,
                                                         guint8 g,
                                                         guint8 b,
                                                         guint8 a);

GRL_AVAILABLE_IN_ALL
void                grl_rlgl_clear_screen_buffers       (void);

GRL_AVAILABLE_IN_ALL
void                grl_rlgl_check_errors               (void);

GRL_AVAILABLE_IN_ALL
void                grl_rlgl_set_blend_mode             (GrlBlendMode mode);

GRL_AVAILABLE_IN_ALL
void                grl_rlgl_set_blend_factors          (gint src_factor,
                                                         gint dst_factor,
                                                         gint equation);

GRL_AVAILABLE_IN_ALL
void                grl_rlgl_set_blend_factors_separate (gint src_rgb,
                                                         gint dst_rgb,
                                                         gint src_alpha,
                                                         gint dst_alpha,
                                                         gint eq_rgb,
                                                         gint eq_alpha);

/*
 * =============================================================================
 * Texture Binding
 * =============================================================================
 */

GRL_AVAILABLE_IN_ALL
void                grl_rlgl_active_texture_slot        (gint slot);

GRL_AVAILABLE_IN_ALL
void                grl_rlgl_enable_texture             (guint id);

GRL_AVAILABLE_IN_ALL
void                grl_rlgl_disable_texture            (void);

GRL_AVAILABLE_IN_ALL
void                grl_rlgl_enable_texture_cubemap     (guint id);

GRL_AVAILABLE_IN_ALL
void                grl_rlgl_disable_texture_cubemap    (void);

GRL_AVAILABLE_IN_ALL
void                grl_rlgl_texture_parameters         (guint id,
                                                         gint param,
                                                         gint value);

GRL_AVAILABLE_IN_ALL
void                grl_rlgl_cubemap_parameters         (guint id,
                                                         gint param,
                                                         gint value);

/*
 * =============================================================================
 * VBO/VAO Operations
 * =============================================================================
 */

GRL_AVAILABLE_IN_ALL
guint               grl_rlgl_load_vertex_array          (void);

GRL_AVAILABLE_IN_ALL
guint               grl_rlgl_load_vertex_buffer         (gconstpointer buffer,
                                                         gint size,
                                                         gboolean dynamic);

GRL_AVAILABLE_IN_ALL
guint               grl_rlgl_load_vertex_buffer_element (gconstpointer buffer,
                                                         gint size,
                                                         gboolean dynamic);

GRL_AVAILABLE_IN_ALL
void                grl_rlgl_update_vertex_buffer       (guint buffer_id,
                                                         gconstpointer data,
                                                         gint data_size,
                                                         gint offset);

GRL_AVAILABLE_IN_ALL
void                grl_rlgl_update_vertex_buffer_elements (guint id,
                                                            gconstpointer data,
                                                            gint data_size,
                                                            gint offset);

GRL_AVAILABLE_IN_ALL
void                grl_rlgl_unload_vertex_array        (guint vao_id);

GRL_AVAILABLE_IN_ALL
void                grl_rlgl_unload_vertex_buffer       (guint vbo_id);

GRL_AVAILABLE_IN_ALL
gboolean            grl_rlgl_enable_vertex_array        (guint vao_id);

GRL_AVAILABLE_IN_ALL
void                grl_rlgl_disable_vertex_array       (void);

GRL_AVAILABLE_IN_ALL
void                grl_rlgl_enable_vertex_buffer       (guint id);

GRL_AVAILABLE_IN_ALL
void                grl_rlgl_disable_vertex_buffer      (void);

GRL_AVAILABLE_IN_ALL
void                grl_rlgl_enable_vertex_buffer_element (guint id);

GRL_AVAILABLE_IN_ALL
void                grl_rlgl_disable_vertex_buffer_element (void);

GRL_AVAILABLE_IN_ALL
void                grl_rlgl_enable_vertex_attribute    (guint index);

GRL_AVAILABLE_IN_ALL
void                grl_rlgl_disable_vertex_attribute   (guint index);

GRL_AVAILABLE_IN_ALL
void                grl_rlgl_draw_vertex_array          (gint offset,
                                                         gint count);

GRL_AVAILABLE_IN_ALL
void                grl_rlgl_draw_vertex_array_elements (gint offset,
                                                         gint count,
                                                         gconstpointer buffer);

GRL_AVAILABLE_IN_ALL
void                grl_rlgl_draw_vertex_array_instanced (gint offset,
                                                          gint count,
                                                          gint instances);

GRL_AVAILABLE_IN_ALL
void                grl_rlgl_draw_vertex_array_elements_instanced (gint offset,
                                                                   gint count,
                                                                   gconstpointer buffer,
                                                                   gint instances);

/*
 * =============================================================================
 * Framebuffer Operations
 * =============================================================================
 */

GRL_AVAILABLE_IN_ALL
guint               grl_rlgl_load_framebuffer           (void);

GRL_AVAILABLE_IN_ALL
void                grl_rlgl_framebuffer_attach         (guint fbo_id,
                                                         guint tex_id,
                                                         GrlRlglFramebufferAttachType attach_type,
                                                         GrlRlglFramebufferTexType tex_type,
                                                         gint mip_level);

GRL_AVAILABLE_IN_ALL
gboolean            grl_rlgl_framebuffer_complete       (guint id);

GRL_AVAILABLE_IN_ALL
void                grl_rlgl_unload_framebuffer         (guint id);

GRL_AVAILABLE_IN_ALL
void                grl_rlgl_enable_framebuffer         (guint id);

GRL_AVAILABLE_IN_ALL
void                grl_rlgl_disable_framebuffer        (void);

GRL_AVAILABLE_IN_ALL
guint               grl_rlgl_get_active_framebuffer     (void);

GRL_AVAILABLE_IN_ALL
void                grl_rlgl_active_draw_buffers        (gint count);

GRL_AVAILABLE_IN_ALL
void                grl_rlgl_blit_framebuffer           (gint src_x,
                                                         gint src_y,
                                                         gint src_width,
                                                         gint src_height,
                                                         gint dst_x,
                                                         gint dst_y,
                                                         gint dst_width,
                                                         gint dst_height,
                                                         gint buffer_mask);

/*
 * =============================================================================
 * Low-level Shader Operations
 * =============================================================================
 */

GRL_AVAILABLE_IN_ALL
guint               grl_rlgl_load_shader_code           (const gchar *vs_code,
                                                         const gchar *fs_code);

GRL_AVAILABLE_IN_ALL
guint               grl_rlgl_compile_shader             (const gchar *shader_code,
                                                         GrlRlglShaderType type);

GRL_AVAILABLE_IN_ALL
guint               grl_rlgl_load_shader_program        (guint vs_id,
                                                         guint fs_id);

GRL_AVAILABLE_IN_ALL
void                grl_rlgl_unload_shader_program      (guint id);

GRL_AVAILABLE_IN_ALL
gint                grl_rlgl_get_location_uniform       (guint shader_id,
                                                         const gchar *uniform_name);

GRL_AVAILABLE_IN_ALL
gint                grl_rlgl_get_location_attrib        (guint shader_id,
                                                         const gchar *attrib_name);

GRL_AVAILABLE_IN_ALL
void                grl_rlgl_set_uniform                (gint loc_index,
                                                         gconstpointer value,
                                                         gint uniform_type,
                                                         gint count);

GRL_AVAILABLE_IN_ALL
void                grl_rlgl_set_uniform_matrix         (gint loc_index,
                                                         const GrlMatrix *mat);

GRL_AVAILABLE_IN_ALL
void                grl_rlgl_set_uniform_sampler        (gint loc_index,
                                                         guint texture_id);

GRL_AVAILABLE_IN_ALL
void                grl_rlgl_enable_shader              (guint id);

GRL_AVAILABLE_IN_ALL
void                grl_rlgl_disable_shader             (void);

G_END_DECLS
