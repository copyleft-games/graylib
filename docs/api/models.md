# 3D Models

Graylib provides a comprehensive 3D model system including procedural mesh generation, PBR materials, model loading, and skeletal animation.

## GrlMesh

A mesh containing vertex data for 3D geometry. Meshes can be generated procedurally or extracted from models.

### Mesh Generation

Graylib provides functions to generate common 3D shapes:

| Function | Description |
|----------|-------------|
| `grl_mesh_gen_cube(width, height, length)` | Generate a cube mesh |
| `grl_mesh_gen_sphere(radius, rings, slices)` | Generate a UV sphere |
| `grl_mesh_gen_hemisphere(radius, rings, slices)` | Generate a hemisphere |
| `grl_mesh_gen_cylinder(radius, height, slices)` | Generate a cylinder |
| `grl_mesh_gen_cone(radius, height, slices)` | Generate a cone |
| `grl_mesh_gen_torus(radius, size, rad_seg, sides)` | Generate a torus |
| `grl_mesh_gen_knot(radius, size, rad_seg, sides)` | Generate a trefoil knot |
| `grl_mesh_gen_plane(width, length, res_x, res_z)` | Generate a subdivided plane |
| `grl_mesh_gen_heightmap(heightmap, size)` | Generate terrain from heightmap |
| `grl_mesh_gen_polygon(sides, radius)` | Generate a regular polygon |

### Properties

| Property | Type | Description |
|----------|------|-------------|
| `vertex-count` | `gint` | Number of vertices in the mesh |
| `triangle-count` | `gint` | Number of triangles in the mesh |

### Methods

```c
gint             grl_mesh_get_vertex_count   (GrlMesh *mesh);
gint             grl_mesh_get_triangle_count (GrlMesh *mesh);
GrlBoundingBox * grl_mesh_get_bounding_box   (GrlMesh *mesh);
gboolean         grl_mesh_is_valid           (GrlMesh *mesh);
void             grl_mesh_upload             (GrlMesh *mesh,
                                              gboolean dynamic);
```

| Method | Description |
|--------|-------------|
| `get_vertex_count()` | Get number of vertices |
| `get_triangle_count()` | Get number of triangles |
| `get_bounding_box()` | Calculate and return the mesh's bounding box |
| `is_valid()` | Check if mesh data is valid |
| `upload(dynamic)` | Upload mesh data to GPU (set `dynamic` for frequently updated meshes) |

### Example: Procedural Mesh

```c
g_autoptr(GrlMesh) sphere = grl_mesh_gen_sphere (1.0f, 16, 32);
g_autoptr(GrlMesh) cube = grl_mesh_gen_cube (2.0f, 2.0f, 2.0f);
g_autoptr(GrlMesh) plane = grl_mesh_gen_plane (10.0f, 10.0f, 10, 10);

/* Upload to GPU for rendering */
grl_mesh_upload (sphere, FALSE);
```

## GrlMaterial

Materials define the visual appearance of 3D surfaces using texture maps and shader parameters. Graylib supports PBR (Physically Based Rendering) workflows.

### Material Map Types

```c
typedef enum
{
    GRL_MATERIAL_MAP_ALBEDO,      /* Base color / diffuse */
    GRL_MATERIAL_MAP_METALNESS,   /* Metallic factor */
    GRL_MATERIAL_MAP_NORMAL,      /* Normal map for surface detail */
    GRL_MATERIAL_MAP_ROUGHNESS,   /* Surface roughness */
    GRL_MATERIAL_MAP_OCCLUSION,   /* Ambient occlusion */
    GRL_MATERIAL_MAP_EMISSION,    /* Emissive / self-illumination */
    GRL_MATERIAL_MAP_HEIGHT,      /* Height / displacement map */
    GRL_MATERIAL_MAP_CUBEMAP,     /* Environment cubemap */
    GRL_MATERIAL_MAP_IRRADIANCE,  /* Irradiance cubemap (diffuse IBL) */
    GRL_MATERIAL_MAP_PREFILTER,   /* Prefiltered cubemap (specular IBL) */
    GRL_MATERIAL_MAP_BRDF         /* BRDF lookup texture */
} GrlMaterialMapType;
```

### Methods

```c
/* Shader management */
void        grl_material_set_shader      (GrlMaterial *material,
                                          GrlShader   *shader);
GrlShader * grl_material_get_shader      (GrlMaterial *material);

/* Texture maps */
void        grl_material_set_map_texture (GrlMaterial        *material,
                                          GrlMaterialMapType  map_type,
                                          GrlTexture         *texture);
GrlTexture *grl_material_get_map_texture (GrlMaterial        *material,
                                          GrlMaterialMapType  map_type);

/* Map colors (tint) */
void        grl_material_set_map_color   (GrlMaterial        *material,
                                          GrlMaterialMapType  map_type,
                                          const GrlColor     *color);
GrlColor *  grl_material_get_map_color   (GrlMaterial        *material,
                                          GrlMaterialMapType  map_type);

/* Map values (scalar parameters) */
void        grl_material_set_map_value   (GrlMaterial        *material,
                                          GrlMaterialMapType  map_type,
                                          gfloat              value);
gfloat      grl_material_get_map_value   (GrlMaterial        *material,
                                          GrlMaterialMapType  map_type);
```

### Example: PBR Material Setup

```c
g_autoptr(GrlMaterial) material = grl_material_new ();
g_autoptr(GrlTexture) albedo = grl_texture_new_from_file ("albedo.png", NULL);
g_autoptr(GrlTexture) normal = grl_texture_new_from_file ("normal.png", NULL);
g_autoptr(GrlTexture) roughness = grl_texture_new_from_file ("roughness.png", NULL);

/* Set texture maps */
grl_material_set_map_texture (material, GRL_MATERIAL_MAP_ALBEDO, albedo);
grl_material_set_map_texture (material, GRL_MATERIAL_MAP_NORMAL, normal);
grl_material_set_map_texture (material, GRL_MATERIAL_MAP_ROUGHNESS, roughness);

/* Set metalness as a constant value (no texture) */
grl_material_set_map_value (material, GRL_MATERIAL_MAP_METALNESS, 0.0f);
```

## GrlModel

A 3D model containing meshes, materials, and optionally bone data for animation.

### Constructors

```c
GrlModel * grl_model_new_from_file (const gchar  *path,
                                    GError      **error);
GrlModel * grl_model_new_from_mesh (GrlMesh *mesh);
```

| Constructor | Description |
|-------------|-------------|
| `new_from_file(path, error)` | Load model from file (OBJ, IQM, GLTF, GLB) |
| `new_from_mesh(mesh)` | Create model from a single mesh. The model keeps the mesh alive. |

### Mesh Ownership in `grl_model_new_from_mesh()`

When creating a model from a mesh, the model takes a reference to the mesh and keeps it alive for the lifetime of the model. This is necessary because raylib performs a shallow copy of mesh data - the model's internal mesh shares vertex and GPU buffer pointers with the original `GrlMesh`.

You can safely unref the `GrlMesh` after creating the model:

```c
g_autoptr(GrlMesh) mesh = grl_mesh_new_cube (1.0f, 1.0f, 1.0f);
g_autoptr(GrlModel) model = grl_model_new_from_mesh (mesh);
/* mesh can go out of scope - model keeps it alive internally */
```

**Note:** Models loaded from files (`grl_model_new_from_file()`) do not have this behavior - they own their mesh data directly.

### Properties

| Property | Type | Access | Description |
|----------|------|--------|-------------|
| `mesh-count` | `gint` | Read | Number of meshes in the model |
| `material-count` | `gint` | Read | Number of materials |
| `bone-count` | `gint` | Read | Number of bones (0 if not animated) |

### Methods

```c
/* Property getters */
gint             grl_model_get_mesh_count     (GrlModel *model);
gint             grl_model_get_material_count (GrlModel *model);
gint             grl_model_get_bone_count     (GrlModel *model);

/* Bounding box */
GrlBoundingBox * grl_model_get_bounding_box   (GrlModel *model);

/* Transform */
void             grl_model_set_transform      (GrlModel         *model,
                                               const GrlMatrix  *transform);
GrlMatrix *      grl_model_get_transform      (GrlModel *model);

/* Material access */
GrlMaterial *    grl_model_get_material       (GrlModel *model,
                                               gint      index);
void             grl_model_set_material       (GrlModel    *model,
                                               gint         index,
                                               GrlMaterial *material);

/* Drawing */
void             grl_model_draw               (GrlModel         *model,
                                               const GrlVector3 *position,
                                               gfloat            scale,
                                               const GrlColor   *tint);
void             grl_model_draw_ex            (GrlModel         *model,
                                               const GrlVector3 *position,
                                               const GrlVector3 *rotation_axis,
                                               gfloat            rotation_angle,
                                               const GrlVector3 *scale,
                                               const GrlColor   *tint);
void             grl_model_draw_wires         (GrlModel         *model,
                                               const GrlVector3 *position,
                                               gfloat            scale,
                                               const GrlColor   *tint);
void             grl_model_draw_wires_ex      (GrlModel         *model,
                                               const GrlVector3 *position,
                                               const GrlVector3 *rotation_axis,
                                               gfloat            rotation_angle,
                                               const GrlVector3 *scale,
                                               const GrlColor   *tint);
```

### Example: Loading and Drawing a Model

```c
g_autoptr(GError) error = NULL;
g_autoptr(GrlModel) model = grl_model_new_from_file ("assets/character.glb", &error);

if (model == NULL)
{
    g_printerr ("Failed to load model: %s\n", error->message);
    return;
}

g_autoptr(GrlVector3) position = grl_vector3_new (0.0f, 0.0f, 0.0f);
g_autoptr(GrlColor) white = grl_color_new_white ();

/* In render loop */
grl_model_draw (model, position, 1.0f, white);
```

### Example: Model with Custom Material

```c
g_autoptr(GrlModel) model = grl_model_new_from_file ("model.obj", NULL);
g_autoptr(GrlMaterial) custom_mat = grl_material_new ();
g_autoptr(GrlTexture) texture = grl_texture_new_from_file ("custom.png", NULL);

grl_material_set_map_texture (custom_mat, GRL_MATERIAL_MAP_ALBEDO, texture);

/* Replace the first material */
grl_model_set_material (model, 0, custom_mat);
```

## GrlModelAnimation

Skeletal animation data that can be applied to models with matching bone structures.

### Loading Animations

```c
GrlModelAnimation ** grl_model_animation_load (const gchar *path,
                                               gint        *count,
                                               GError     **error);
```

Returns a NULL-terminated array of animations loaded from the file. The `count` parameter receives the number of animations loaded.

### Properties

| Property | Type | Access | Description |
|----------|------|--------|-------------|
| `name` | `gchar *` | Read | Animation name |
| `frame-count` | `gint` | Read | Total number of frames |
| `bone-count` | `gint` | Read | Number of bones in the animation |

### Methods

```c
const gchar * grl_model_animation_get_name        (GrlModelAnimation *anim);
gint          grl_model_animation_get_frame_count (GrlModelAnimation *anim);
gint          grl_model_animation_get_bone_count  (GrlModelAnimation *anim);
gboolean      grl_model_animation_is_valid        (GrlModelAnimation *anim);
void          grl_model_animation_update          (GrlModelAnimation *anim,
                                                   GrlModel          *model,
                                                   gint               frame);
```

| Method | Description |
|--------|-------------|
| `get_name()` | Get the animation name |
| `get_frame_count()` | Get total number of frames |
| `get_bone_count()` | Get number of bones |
| `is_valid()` | Check if animation data is valid |
| `update(model, frame)` | Apply animation frame to model |

### Example: Animated Character

```c
g_autoptr(GError) error = NULL;
gint anim_count = 0;

/* Load model and animations */
g_autoptr(GrlModel) character = grl_model_new_from_file ("character.glb", &error);
GrlModelAnimation **animations = grl_model_animation_load ("character.glb",
                                                            &anim_count,
                                                            &error);

if (character == NULL || animations == NULL)
{
    g_printerr ("Failed to load: %s\n", error->message);
    return;
}

g_print ("Loaded %d animations\n", anim_count);

/* Animation state */
GrlModelAnimation *current_anim = animations[0];  /* First animation */
gint current_frame = 0;
gint frame_count = grl_model_animation_get_frame_count (current_anim);

/* In update loop */
current_frame = (current_frame + 1) % frame_count;
grl_model_animation_update (current_anim, character, current_frame);

/* In render loop */
g_autoptr(GrlVector3) pos = grl_vector3_new (0.0f, 0.0f, 0.0f);
g_autoptr(GrlColor) white = grl_color_new_white ();
grl_model_draw (character, pos, 1.0f, white);

/* Cleanup animations when done */
for (gint i = 0; i < anim_count; i++)
{
    g_object_unref (animations[i]);
}
g_free (animations);
```

## Supported File Formats

| Format | Extension | Features |
|--------|-----------|----------|
| Wavefront OBJ | `.obj` | Static meshes, materials (MTL) |
| Inter-Quake Model | `.iqm` | Skeletal animation, materials |
| glTF 2.0 | `.gltf`, `.glb` | PBR materials, skeletal animation, embedded textures |

## Complete Example: 3D Model Viewer

```c
#include <graylib.h>

int
main (int argc, char *argv[])
{
    g_autoptr(GrlWindow) window = NULL;
    g_autoptr(GrlCamera3D) camera = NULL;
    g_autoptr(GrlModel) model = NULL;
    g_autoptr(GrlVector3) position = NULL;
    g_autoptr(GrlVector3) cam_pos = NULL;
    g_autoptr(GrlVector3) cam_target = NULL;
    g_autoptr(GrlColor) white = NULL;
    g_autoptr(GrlColor) gray = NULL;
    g_autoptr(GError) error = NULL;
    gfloat rotation = 0.0f;

    window = grl_window_new (800, 600, "Model Viewer");
    grl_window_set_target_fps (window, 60);

    /* Setup camera */
    cam_pos = grl_vector3_new (5.0f, 5.0f, 5.0f);
    cam_target = grl_vector3_new (0.0f, 1.0f, 0.0f);
    camera = grl_camera3d_new ();
    grl_camera3d_set_position (camera, cam_pos);
    grl_camera3d_set_target (camera, cam_target);
    grl_camera3d_set_fovy (camera, 45.0f);

    /* Load model */
    model = grl_model_new_from_file ("assets/model.glb", &error);
    if (model == NULL)
    {
        g_printerr ("Error: %s\n", error->message);
        return 1;
    }

    position = grl_vector3_new (0.0f, 0.0f, 0.0f);
    white = grl_color_new_white ();
    gray = grl_color_new (50, 50, 50, 255);

    while (!grl_window_should_close (window))
    {
        /* Rotate model */
        rotation += 0.5f;

        grl_window_begin_drawing (window);
        grl_window_clear_background (window, gray);

        grl_camera3d_begin (camera);

        /* Draw rotating model */
        g_autoptr(GrlVector3) axis = grl_vector3_new (0.0f, 1.0f, 0.0f);
        g_autoptr(GrlVector3) scale = grl_vector3_new (1.0f, 1.0f, 1.0f);
        grl_model_draw_ex (model, position, axis, rotation, scale, white);

        /* Draw ground grid */
        grl_draw_grid (10, 1.0f);

        grl_camera3d_end (camera);

        grl_window_end_drawing (window);
    }

    return 0;
}
```
