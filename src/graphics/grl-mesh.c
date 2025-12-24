/* grl-mesh.c
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#include "grl-mesh.h"
#include <gio/gio.h>
#include <raylib.h>

/**
 * SECTION:grl-mesh
 * @Title: GrlMesh
 * @Short_description: 3D mesh data
 *
 * #GrlMesh represents 3D mesh vertex data including positions, normals,
 * texture coordinates, colors, and indices. Meshes can be generated
 * procedurally or loaded from models.
 *
 * # Generated Meshes
 *
 * Several functions create common geometric shapes:
 * - grl_mesh_new_cube() - Box shape
 * - grl_mesh_new_sphere() - Sphere
 * - grl_mesh_new_cylinder() - Cylinder
 * - grl_mesh_new_plane() - Flat plane
 *
 * # GPU Upload
 *
 * Before drawing with shaders, mesh data must be uploaded to GPU memory
 * using grl_mesh_upload().
 */

typedef struct
{
    Mesh     mesh;
    gboolean valid;
    gboolean uploaded;
} GrlMeshPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (GrlMesh, grl_mesh, G_TYPE_OBJECT)

enum
{
    PROP_0,
    PROP_VALID,
    PROP_VERTEX_COUNT,
    PROP_TRIANGLE_COUNT,
    N_PROPS
};

static GParamSpec *properties[N_PROPS];

static void
grl_mesh_finalize (GObject *object)
{
    GrlMesh *self = GRL_MESH (object);
    GrlMeshPrivate *priv = grl_mesh_get_instance_private (self);

    if (priv->valid)
    {
        UnloadMesh (priv->mesh);
        priv->valid = FALSE;
    }

    G_OBJECT_CLASS (grl_mesh_parent_class)->finalize (object);
}

static void
grl_mesh_get_property (GObject    *object,
                       guint       prop_id,
                       GValue     *value,
                       GParamSpec *pspec)
{
    GrlMesh *self = GRL_MESH (object);
    GrlMeshPrivate *priv = grl_mesh_get_instance_private (self);

    switch (prop_id)
    {
    case PROP_VALID:
        g_value_set_boolean (value, priv->valid);
        break;

    case PROP_VERTEX_COUNT:
        g_value_set_int (value, priv->valid ? priv->mesh.vertexCount : 0);
        break;

    case PROP_TRIANGLE_COUNT:
        g_value_set_int (value, priv->valid ? priv->mesh.triangleCount : 0);
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
grl_mesh_class_init (GrlMeshClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->finalize = grl_mesh_finalize;
    object_class->get_property = grl_mesh_get_property;

    /**
     * GrlMesh:valid:
     *
     * Whether the mesh contains valid data.
     */
    properties[PROP_VALID] =
        g_param_spec_boolean ("valid",
                              "Valid",
                              "Whether mesh is valid",
                              FALSE,
                              G_PARAM_READABLE |
                              G_PARAM_STATIC_STRINGS);

    /**
     * GrlMesh:vertex-count:
     *
     * Number of vertices in the mesh.
     */
    properties[PROP_VERTEX_COUNT] =
        g_param_spec_int ("vertex-count",
                          "Vertex Count",
                          "Number of vertices",
                          0, G_MAXINT, 0,
                          G_PARAM_READABLE |
                          G_PARAM_STATIC_STRINGS);

    /**
     * GrlMesh:triangle-count:
     *
     * Number of triangles in the mesh.
     */
    properties[PROP_TRIANGLE_COUNT] =
        g_param_spec_int ("triangle-count",
                          "Triangle Count",
                          "Number of triangles",
                          0, G_MAXINT, 0,
                          G_PARAM_READABLE |
                          G_PARAM_STATIC_STRINGS);

    g_object_class_install_properties (object_class, N_PROPS, properties);
}

static void
grl_mesh_init (GrlMesh *self)
{
    GrlMeshPrivate *priv = grl_mesh_get_instance_private (self);

    priv->valid = FALSE;
    priv->uploaded = FALSE;
    memset (&priv->mesh, 0, sizeof (Mesh));
}

/*
 * Helper to create GrlMesh from raylib Mesh
 */
static GrlMesh *
grl_mesh_from_raylib (Mesh mesh)
{
    GrlMesh *self;
    GrlMeshPrivate *priv;

    self = g_object_new (GRL_TYPE_MESH, NULL);
    priv = grl_mesh_get_instance_private (self);

    priv->mesh = mesh;
    priv->valid = (mesh.vertexCount > 0);

    return self;
}

/**
 * grl_mesh_new_cube:
 * @width: Cube width
 * @height: Cube height
 * @length: Cube length
 *
 * Generates a cube mesh with the given dimensions.
 *
 * Returns: (transfer full): A new #GrlMesh
 */
GrlMesh *
grl_mesh_new_cube (gfloat width,
                   gfloat height,
                   gfloat length)
{
    return grl_mesh_from_raylib (GenMeshCube (width, height, length));
}

/**
 * grl_mesh_new_sphere:
 * @radius: Sphere radius
 * @rings: Number of horizontal rings
 * @slices: Number of vertical slices
 *
 * Generates a sphere mesh with the given parameters.
 *
 * Returns: (transfer full): A new #GrlMesh
 */
GrlMesh *
grl_mesh_new_sphere (gfloat radius,
                     gint   rings,
                     gint   slices)
{
    return grl_mesh_from_raylib (GenMeshSphere (radius, rings, slices));
}

/**
 * grl_mesh_new_hemisphere:
 * @radius: Hemisphere radius
 * @rings: Number of horizontal rings
 * @slices: Number of vertical slices
 *
 * Generates a hemisphere (half-sphere) mesh.
 *
 * Returns: (transfer full): A new #GrlMesh
 */
GrlMesh *
grl_mesh_new_hemisphere (gfloat radius,
                         gint   rings,
                         gint   slices)
{
    return grl_mesh_from_raylib (GenMeshHemiSphere (radius, rings, slices));
}

/**
 * grl_mesh_new_cylinder:
 * @radius: Cylinder radius
 * @height: Cylinder height
 * @slices: Number of slices
 *
 * Generates a cylinder mesh.
 *
 * Returns: (transfer full): A new #GrlMesh
 */
GrlMesh *
grl_mesh_new_cylinder (gfloat radius,
                       gfloat height,
                       gint   slices)
{
    return grl_mesh_from_raylib (GenMeshCylinder (radius, height, slices));
}

/**
 * grl_mesh_new_cone:
 * @radius: Base radius
 * @height: Cone height
 * @slices: Number of slices
 *
 * Generates a cone mesh.
 *
 * Returns: (transfer full): A new #GrlMesh
 */
GrlMesh *
grl_mesh_new_cone (gfloat radius,
                   gfloat height,
                   gint   slices)
{
    return grl_mesh_from_raylib (GenMeshCone (radius, height, slices));
}

/**
 * grl_mesh_new_torus:
 * @radius: Torus radius
 * @size: Tube size
 * @rad_seg: Radial segments
 * @sides: Number of sides
 *
 * Generates a torus (donut) mesh.
 *
 * Returns: (transfer full): A new #GrlMesh
 */
GrlMesh *
grl_mesh_new_torus (gfloat radius,
                    gfloat size,
                    gint   rad_seg,
                    gint   sides)
{
    return grl_mesh_from_raylib (GenMeshTorus (radius, size, rad_seg, sides));
}

/**
 * grl_mesh_new_knot:
 * @radius: Knot radius
 * @size: Tube size
 * @rad_seg: Radial segments
 * @sides: Number of sides
 *
 * Generates a trefoil knot mesh.
 *
 * Returns: (transfer full): A new #GrlMesh
 */
GrlMesh *
grl_mesh_new_knot (gfloat radius,
                   gfloat size,
                   gint   rad_seg,
                   gint   sides)
{
    return grl_mesh_from_raylib (GenMeshKnot (radius, size, rad_seg, sides));
}

/**
 * grl_mesh_new_plane:
 * @width: Plane width
 * @length: Plane length
 * @res_x: Subdivisions in X
 * @res_z: Subdivisions in Z
 *
 * Generates a flat plane mesh on the XZ plane.
 *
 * Returns: (transfer full): A new #GrlMesh
 */
GrlMesh *
grl_mesh_new_plane (gfloat width,
                    gfloat length,
                    gint   res_x,
                    gint   res_z)
{
    return grl_mesh_from_raylib (GenMeshPlane (width, length, res_x, res_z));
}

/**
 * grl_mesh_new_heightmap:
 * @heightmap_path: (type filename): Path to heightmap image
 * @size: Size of the resulting mesh
 * @error: (nullable): Return location for error
 *
 * Generates a mesh from a heightmap image.
 * The image's grayscale values determine vertex heights.
 *
 * Returns: (transfer full) (nullable): A new #GrlMesh, or %NULL on error
 */
GrlMesh *
grl_mesh_new_heightmap (const gchar      *heightmap_path,
                        const GrlVector3 *size,
                        GError          **error)
{
    Image heightmap;
    Mesh mesh;
    Vector3 rl_size;

    g_return_val_if_fail (heightmap_path != NULL, NULL);
    g_return_val_if_fail (size != NULL, NULL);

    heightmap = LoadImage (heightmap_path);

    if (heightmap.data == NULL)
    {
        g_set_error (error,
                     G_IO_ERROR,
                     G_IO_ERROR_FAILED,
                     "Failed to load heightmap: %s",
                     heightmap_path);
        return NULL;
    }

    rl_size.x = size->x;
    rl_size.y = size->y;
    rl_size.z = size->z;

    mesh = GenMeshHeightmap (heightmap, rl_size);
    UnloadImage (heightmap);

    return grl_mesh_from_raylib (mesh);
}

/**
 * grl_mesh_new_polygon:
 * @sides: Number of sides
 * @radius: Polygon radius
 *
 * Generates a regular polygon mesh (flat, on XZ plane).
 *
 * Returns: (transfer full): A new #GrlMesh
 */
GrlMesh *
grl_mesh_new_polygon (gint   sides,
                      gfloat radius)
{
    return grl_mesh_from_raylib (GenMeshPoly (sides, radius));
}

/**
 * grl_mesh_get_vertex_count:
 * @self: A #GrlMesh
 *
 * Gets the number of vertices in the mesh.
 *
 * Returns: The vertex count
 */
gint
grl_mesh_get_vertex_count (GrlMesh *self)
{
    GrlMeshPrivate *priv;

    g_return_val_if_fail (GRL_IS_MESH (self), 0);

    priv = grl_mesh_get_instance_private (self);

    return priv->valid ? priv->mesh.vertexCount : 0;
}

/**
 * grl_mesh_get_triangle_count:
 * @self: A #GrlMesh
 *
 * Gets the number of triangles in the mesh.
 *
 * Returns: The triangle count
 */
gint
grl_mesh_get_triangle_count (GrlMesh *self)
{
    GrlMeshPrivate *priv;

    g_return_val_if_fail (GRL_IS_MESH (self), 0);

    priv = grl_mesh_get_instance_private (self);

    return priv->valid ? priv->mesh.triangleCount : 0;
}

/**
 * grl_mesh_get_bounding_box:
 * @self: A #GrlMesh
 *
 * Calculates the axis-aligned bounding box that encloses the mesh.
 *
 * Returns: (transfer full): The mesh bounding box
 */
GrlBoundingBox *
grl_mesh_get_bounding_box (GrlMesh *self)
{
    GrlMeshPrivate *priv;
    BoundingBox bb;

    g_return_val_if_fail (GRL_IS_MESH (self), NULL);

    priv = grl_mesh_get_instance_private (self);

    if (!priv->valid)
        return grl_bounding_box_new_xyz (0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);

    bb = GetMeshBoundingBox (priv->mesh);

    return grl_bounding_box_new_xyz (bb.min.x, bb.min.y, bb.min.z,
                                     bb.max.x, bb.max.y, bb.max.z);
}

/**
 * grl_mesh_upload:
 * @self: A #GrlMesh
 * @dynamic: Whether to use dynamic buffers
 *
 * Uploads mesh data to GPU memory (VRAM).
 * This must be called before drawing with shaders.
 *
 * Set @dynamic to %TRUE if you plan to update vertex data frequently.
 */
void
grl_mesh_upload (GrlMesh  *self,
                 gboolean  dynamic)
{
    GrlMeshPrivate *priv;

    g_return_if_fail (GRL_IS_MESH (self));

    priv = grl_mesh_get_instance_private (self);

    if (!priv->valid || priv->uploaded)
        return;

    UploadMesh (&priv->mesh, dynamic);
    priv->uploaded = TRUE;
}

/**
 * grl_mesh_is_valid:
 * @self: A #GrlMesh
 *
 * Checks if the mesh contains valid vertex data.
 *
 * Returns: %TRUE if valid
 */
gboolean
grl_mesh_is_valid (GrlMesh *self)
{
    GrlMeshPrivate *priv;

    g_return_val_if_fail (GRL_IS_MESH (self), FALSE);

    priv = grl_mesh_get_instance_private (self);

    return priv->valid;
}

/**
 * grl_mesh_get_handle:
 * @self: A #GrlMesh
 *
 * Gets the internal raylib Mesh pointer.
 * This is for internal use and advanced users only.
 *
 * Returns: (transfer none): Internal mesh pointer
 */
gpointer
grl_mesh_get_handle (GrlMesh *self)
{
    GrlMeshPrivate *priv;

    g_return_val_if_fail (GRL_IS_MESH (self), NULL);

    priv = grl_mesh_get_instance_private (self);

    return &priv->mesh;
}
