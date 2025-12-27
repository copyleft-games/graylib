# RLGL - Low-Level OpenGL Abstraction

The RLGL module provides GObject wrappers for raylib's low-level OpenGL abstraction layer. These functions give direct access to OpenGL-like operations for advanced rendering techniques.

**Warning:** Most applications should use the higher-level drawing functions. RLGL functions are for advanced users who need direct control over the rendering pipeline.

## Initialization and Cleanup

```c
/* Initialize rlgl internal buffers */
grl_rlgl_init (width, height);

/* Clean up rlgl internal buffers */
grl_rlgl_close ();

/* Get current OpenGL version */
GrlRlglGlVersion version = grl_rlgl_get_version ();
```

## Framebuffer Configuration

```c
/* Set/get internal framebuffer dimensions */
grl_rlgl_set_framebuffer_width (width);
grl_rlgl_set_framebuffer_height (height);
gint width = grl_rlgl_get_framebuffer_width ();
gint height = grl_rlgl_get_framebuffer_height ();

/* Get default resources */
guint tex_id = grl_rlgl_get_texture_id_default ();
guint shader_id = grl_rlgl_get_shader_id_default ();
```

## Matrix Operations

### Matrix Stack

```c
/* Push/pop matrix stack */
grl_rlgl_push_matrix ();
grl_rlgl_pop_matrix ();

/* Load identity matrix */
grl_rlgl_load_identity ();

/* Set matrix mode */
grl_rlgl_matrix_mode (GRL_RLGL_MODELVIEW);
grl_rlgl_matrix_mode (GRL_RLGL_PROJECTION);
grl_rlgl_matrix_mode (GRL_RLGL_TEXTURE);
```

### Transformations

```c
/* Translate */
grl_rlgl_translatef (x, y, z);

/* Rotate (angle in degrees) */
grl_rlgl_rotatef (angle, x, y, z);

/* Scale */
grl_rlgl_scalef (x, y, z);

/* Multiply by matrix */
g_autoptr(GrlMatrix) mat = grl_matrix_new ();
grl_rlgl_mult_matrixf (mat);
```

### Projection Setup

```c
/* Set up perspective frustum */
grl_rlgl_frustum (left, right, bottom, top, znear, zfar);

/* Set up orthographic projection */
grl_rlgl_ortho (left, right, bottom, top, znear, zfar);

/* Set viewport */
grl_rlgl_viewport (x, y, width, height);

/* Set clip planes */
grl_rlgl_set_clip_planes (near, far);
gfloat near = grl_rlgl_get_cull_distance_near ();
gfloat far = grl_rlgl_get_cull_distance_far ();
```

### Get/Set Matrices

```c
/* Get current matrices (returns newly allocated GrlMatrix) */
g_autoptr(GrlMatrix) modelview = grl_rlgl_get_matrix_modelview ();
g_autoptr(GrlMatrix) projection = grl_rlgl_get_matrix_projection ();
g_autoptr(GrlMatrix) transform = grl_rlgl_get_matrix_transform ();
g_autoptr(GrlMatrix) proj_stereo = grl_rlgl_get_matrix_projection_stereo (eye);
g_autoptr(GrlMatrix) view_offset = grl_rlgl_get_matrix_view_offset_stereo (eye);

/* Set matrices */
grl_rlgl_set_matrix_modelview (mat);
grl_rlgl_set_matrix_projection (mat);
grl_rlgl_set_matrix_projection_stereo (right_mat, left_mat);
grl_rlgl_set_matrix_view_offset_stereo (right_mat, left_mat);
```

## Immediate Mode Vertex Operations

Similar to OpenGL immediate mode:

```c
/* Begin drawing primitives */
grl_rlgl_begin (GRL_RLGL_TRIANGLES);  /* or GRL_RLGL_LINES, GRL_RLGL_QUADS */

/* Define vertices */
grl_rlgl_vertex2f (x, y);
grl_rlgl_vertex3f (x, y, z);
grl_rlgl_vertex2i (x, y);

/* Set vertex attributes */
grl_rlgl_tex_coord2f (u, v);
grl_rlgl_normal3f (nx, ny, nz);
grl_rlgl_color4ub (r, g, b, a);
grl_rlgl_color3f (r, g, b);
grl_rlgl_color4f (r, g, b, a);

/* End drawing */
grl_rlgl_end ();
```

### Example: Drawing a Triangle

```c
grl_rlgl_begin (GRL_RLGL_TRIANGLES);
    grl_rlgl_color4ub (255, 0, 0, 255);
    grl_rlgl_vertex2f (100.0f, 100.0f);

    grl_rlgl_color4ub (0, 255, 0, 255);
    grl_rlgl_vertex2f (200.0f, 100.0f);

    grl_rlgl_color4ub (0, 0, 255, 255);
    grl_rlgl_vertex2f (150.0f, 200.0f);
grl_rlgl_end ();
```

## Render State Management

### Depth Testing

```c
grl_rlgl_enable_depth_test ();
grl_rlgl_disable_depth_test ();

/* Depth buffer writing */
grl_rlgl_enable_depth_mask ();
grl_rlgl_disable_depth_mask ();
```

### Blending

```c
grl_rlgl_enable_color_blend ();
grl_rlgl_disable_color_blend ();

/* Set blend mode */
grl_rlgl_set_blend_mode (GRL_BLEND_MODE_ALPHA);

/* Custom blend factors */
grl_rlgl_set_blend_factors (src_factor, dst_factor, equation);
grl_rlgl_set_blend_factors_separate (src_rgb, dst_rgb, src_alpha, dst_alpha, eq_rgb, eq_alpha);
```

### Face Culling

```c
grl_rlgl_enable_backface_culling ();
grl_rlgl_disable_backface_culling ();

/* Set which face to cull */
grl_rlgl_set_cull_face (GRL_RLGL_CULL_BACK);
grl_rlgl_set_cull_face (GRL_RLGL_CULL_FRONT);
```

### Scissor Test

```c
grl_rlgl_enable_scissor_test ();
grl_rlgl_scissor (x, y, width, height);
grl_rlgl_disable_scissor_test ();
```

### Wireframe and Point Mode

```c
grl_rlgl_enable_wire_mode ();   /* Wireframe rendering */
grl_rlgl_enable_point_mode ();  /* Point cloud rendering */
grl_rlgl_disable_wire_mode ();  /* Back to solid fill */
```

### Line Drawing

```c
grl_rlgl_set_line_width (2.0f);
gfloat width = grl_rlgl_get_line_width ();

grl_rlgl_enable_smooth_lines ();   /* Antialiased lines */
grl_rlgl_disable_smooth_lines ();
```

### Stereo Rendering

```c
grl_rlgl_enable_stereo_render ();
grl_rlgl_disable_stereo_render ();
gboolean stereo = grl_rlgl_is_stereo_render_enabled ();
```

### Color Mask

```c
/* Control which color channels are written */
grl_rlgl_color_mask (red, green, blue, alpha);  /* gboolean for each */
```

### Clear Operations

```c
/* Set clear color */
grl_rlgl_clear_color (r, g, b, a);  /* guint8 values 0-255 */

/* Clear color and depth buffers */
grl_rlgl_clear_screen_buffers ();

/* Check for OpenGL errors */
grl_rlgl_check_errors ();
```

## Texture Binding

```c
/* Activate texture slot (0-7) */
grl_rlgl_active_texture_slot (slot);

/* Bind/unbind 2D texture */
grl_rlgl_enable_texture (texture_id);
grl_rlgl_disable_texture ();

/* Bind/unbind cubemap texture */
grl_rlgl_enable_texture_cubemap (cubemap_id);
grl_rlgl_disable_texture_cubemap ();

/* Set texture parameters */
grl_rlgl_texture_parameters (texture_id, param, value);
grl_rlgl_cubemap_parameters (cubemap_id, param, value);
```

## VBO/VAO Buffer Operations

### Vertex Array Objects (VAO)

```c
/* Create VAO */
guint vao_id = grl_rlgl_load_vertex_array ();

/* Bind/unbind VAO */
gboolean success = grl_rlgl_enable_vertex_array (vao_id);
grl_rlgl_disable_vertex_array ();

/* Delete VAO */
grl_rlgl_unload_vertex_array (vao_id);
```

### Vertex Buffer Objects (VBO)

```c
/* Create VBO */
guint vbo_id = grl_rlgl_load_vertex_buffer (data, size, dynamic);

/* Create element buffer (indices) */
guint ebo_id = grl_rlgl_load_vertex_buffer_element (indices, size, dynamic);

/* Bind/unbind VBO */
grl_rlgl_enable_vertex_buffer (vbo_id);
grl_rlgl_disable_vertex_buffer ();

/* Bind/unbind element buffer */
grl_rlgl_enable_vertex_buffer_element (ebo_id);
grl_rlgl_disable_vertex_buffer_element ();

/* Update buffer data */
grl_rlgl_update_vertex_buffer (vbo_id, data, size, offset);
grl_rlgl_update_vertex_buffer_elements (ebo_id, data, size, offset);

/* Delete buffers */
grl_rlgl_unload_vertex_buffer (vbo_id);
```

### Vertex Attributes

```c
grl_rlgl_enable_vertex_attribute (index);
grl_rlgl_disable_vertex_attribute (index);
```

### Drawing

```c
/* Draw from vertex array */
grl_rlgl_draw_vertex_array (offset, count);
grl_rlgl_draw_vertex_array_elements (offset, count, buffer);

/* Instanced drawing */
grl_rlgl_draw_vertex_array_instanced (offset, count, instances);
grl_rlgl_draw_vertex_array_elements_instanced (offset, count, buffer, instances);
```

## Framebuffer Operations

```c
/* Create framebuffer */
guint fbo_id = grl_rlgl_load_framebuffer ();

/* Bind/unbind framebuffer */
grl_rlgl_enable_framebuffer (fbo_id);
grl_rlgl_disable_framebuffer ();  /* Back to default */

/* Get active framebuffer */
guint active = grl_rlgl_get_active_framebuffer ();

/* Attach texture to framebuffer */
grl_rlgl_framebuffer_attach (fbo_id, tex_id,
                             GRL_RLGL_FRAMEBUFFER_ATTACH_COLOR_CHANNEL0,
                             GRL_RLGL_FRAMEBUFFER_TEX_TEXTURE2D,
                             mip_level);

/* Check framebuffer completeness */
gboolean complete = grl_rlgl_framebuffer_complete (fbo_id);

/* Delete framebuffer */
grl_rlgl_unload_framebuffer (fbo_id);

/* Multiple render targets */
grl_rlgl_active_draw_buffers (count);

/* Blit between framebuffers */
grl_rlgl_blit_framebuffer (src_x, src_y, src_width, src_height,
                           dst_x, dst_y, dst_width, dst_height,
                           buffer_mask);
```

### Framebuffer Attachment Types

- `GRL_RLGL_FRAMEBUFFER_ATTACH_COLOR_CHANNEL0` through `COLOR_CHANNEL7`
- `GRL_RLGL_FRAMEBUFFER_ATTACH_DEPTH`
- `GRL_RLGL_FRAMEBUFFER_ATTACH_STENCIL`

### Framebuffer Texture Types

- `GRL_RLGL_FRAMEBUFFER_TEX_CUBEMAP_POSITIVE_X/Y/Z`
- `GRL_RLGL_FRAMEBUFFER_TEX_CUBEMAP_NEGATIVE_X/Y/Z`
- `GRL_RLGL_FRAMEBUFFER_TEX_TEXTURE2D`
- `GRL_RLGL_FRAMEBUFFER_TEX_RENDERBUFFER`

## Low-Level Shader Operations

### Shader Compilation

```c
/* Compile individual shaders */
guint vs_id = grl_rlgl_compile_shader (vs_code, GRL_RLGL_SHADER_VERTEX);
guint fs_id = grl_rlgl_compile_shader (fs_code, GRL_RLGL_SHADER_FRAGMENT);
guint cs_id = grl_rlgl_compile_shader (cs_code, GRL_RLGL_SHADER_COMPUTE);

/* Link shader program */
guint program_id = grl_rlgl_load_shader_program (vs_id, fs_id);

/* Or compile and link in one step */
guint program_id = grl_rlgl_load_shader_code (vs_code, fs_code);

/* Delete shader program */
grl_rlgl_unload_shader_program (program_id);
```

### Shader Usage

```c
/* Bind/unbind shader */
grl_rlgl_enable_shader (program_id);
grl_rlgl_disable_shader ();
```

### Uniforms

```c
/* Get uniform location */
gint loc = grl_rlgl_get_location_uniform (program_id, "uniformName");

/* Set uniform value */
grl_rlgl_set_uniform (loc, value_ptr, uniform_type, count);

/* Set matrix uniform */
g_autoptr(GrlMatrix) mat = grl_matrix_new ();
grl_rlgl_set_uniform_matrix (loc, mat);

/* Set sampler uniform */
grl_rlgl_set_uniform_sampler (loc, texture_id);
```

### Attributes

```c
/* Get attribute location */
gint attrib_loc = grl_rlgl_get_location_attrib (program_id, "attribName");
```

## Primitive Shapes (for Default Batching)

```c
/* Load internal rendering shapes */
grl_rlgl_load_draw_cube ();
grl_rlgl_load_draw_quad ();
```

## Enumerations

### GrlRlglDrawMode

| Value | Description |
|-------|-------------|
| `GRL_RLGL_LINES` | Draw lines |
| `GRL_RLGL_TRIANGLES` | Draw triangles |
| `GRL_RLGL_QUADS` | Draw quads |

### GrlRlglMatrixMode

| Value | Description |
|-------|-------------|
| `GRL_RLGL_MODELVIEW` | Model-view matrix mode |
| `GRL_RLGL_PROJECTION` | Projection matrix mode |
| `GRL_RLGL_TEXTURE` | Texture matrix mode |

### GrlRlglCullMode

| Value | Description |
|-------|-------------|
| `GRL_RLGL_CULL_FRONT` | Cull front faces |
| `GRL_RLGL_CULL_BACK` | Cull back faces |

### GrlRlglGlVersion

| Value | Description |
|-------|-------------|
| `GRL_RLGL_OPENGL_11` | OpenGL 1.1 |
| `GRL_RLGL_OPENGL_21` | OpenGL 2.1 |
| `GRL_RLGL_OPENGL_33` | OpenGL 3.3 |
| `GRL_RLGL_OPENGL_43` | OpenGL 4.3 |
| `GRL_RLGL_OPENGL_ES_20` | OpenGL ES 2.0 |
| `GRL_RLGL_OPENGL_ES_30` | OpenGL ES 3.0 |

### GrlRlglShaderType

| Value | Description |
|-------|-------------|
| `GRL_RLGL_SHADER_FRAGMENT` | Fragment shader |
| `GRL_RLGL_SHADER_VERTEX` | Vertex shader |
| `GRL_RLGL_SHADER_COMPUTE` | Compute shader |

## Example: Custom Rendering with RLGL

```c
g_autoptr(GrlWindow) window = grl_window_new (800, 600, "RLGL Example");
g_autoptr(GrlColor) bg = grl_color_new (40, 40, 60, 255);

while (!grl_window_should_close (window))
{
    grl_window_begin_drawing (window);
    grl_draw_clear_background (bg);

    /* Set up custom projection */
    grl_rlgl_matrix_mode (GRL_RLGL_PROJECTION);
    grl_rlgl_push_matrix ();
    grl_rlgl_load_identity ();
    grl_rlgl_ortho (0, 800, 600, 0, -1.0, 1.0);

    grl_rlgl_matrix_mode (GRL_RLGL_MODELVIEW);
    grl_rlgl_push_matrix ();
    grl_rlgl_load_identity ();

    /* Apply transform */
    grl_rlgl_translatef (400.0f, 300.0f, 0.0f);
    grl_rlgl_rotatef (grl_window_get_time (window) * 30.0f, 0.0f, 0.0f, 1.0f);

    /* Draw colored triangle */
    grl_rlgl_begin (GRL_RLGL_TRIANGLES);
        grl_rlgl_color4ub (255, 0, 0, 255);
        grl_rlgl_vertex2f (-50.0f, 50.0f);

        grl_rlgl_color4ub (0, 255, 0, 255);
        grl_rlgl_vertex2f (50.0f, 50.0f);

        grl_rlgl_color4ub (0, 0, 255, 255);
        grl_rlgl_vertex2f (0.0f, -50.0f);
    grl_rlgl_end ();

    /* Restore matrices */
    grl_rlgl_pop_matrix ();
    grl_rlgl_matrix_mode (GRL_RLGL_PROJECTION);
    grl_rlgl_pop_matrix ();

    grl_window_end_drawing (window);
}
```
