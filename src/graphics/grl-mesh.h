/* grl-mesh.h
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * 3D mesh data containing vertex attributes.
 */

#pragma once

#if !defined(GRAYLIB_INSIDE) && !defined(GRAYLIB_COMPILATION)
#error "Only <graylib.h> can be included directly."
#endif

#include <glib-object.h>
#include "../grl-version.h"
#include "../math/grl-bounding-box.h"

G_BEGIN_DECLS

#define GRL_TYPE_MESH (grl_mesh_get_type())

GRL_AVAILABLE_IN_ALL
G_DECLARE_DERIVABLE_TYPE (GrlMesh, grl_mesh, GRL, MESH, GObject)

struct _GrlMeshClass
{
    GObjectClass parent_class;

    gpointer _reserved[8];
};

/**
 * grl_mesh_new_cube:
 * @width: Cube width
 * @height: Cube height
 * @length: Cube length
 *
 * Generates a cube mesh.
 *
 * Returns: (transfer full): A new #GrlMesh
 */
GRL_AVAILABLE_IN_ALL
GrlMesh *           grl_mesh_new_cube               (gfloat              width,
                                                     gfloat              height,
                                                     gfloat              length);

/**
 * grl_mesh_new_sphere:
 * @radius: Sphere radius
 * @rings: Number of horizontal rings
 * @slices: Number of vertical slices
 *
 * Generates a sphere mesh.
 *
 * Returns: (transfer full): A new #GrlMesh
 */
GRL_AVAILABLE_IN_ALL
GrlMesh *           grl_mesh_new_sphere             (gfloat              radius,
                                                     gint                rings,
                                                     gint                slices);

/**
 * grl_mesh_new_hemisphere:
 * @radius: Hemisphere radius
 * @rings: Number of horizontal rings
 * @slices: Number of vertical slices
 *
 * Generates a hemisphere mesh.
 *
 * Returns: (transfer full): A new #GrlMesh
 */
GRL_AVAILABLE_IN_ALL
GrlMesh *           grl_mesh_new_hemisphere         (gfloat              radius,
                                                     gint                rings,
                                                     gint                slices);

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
GRL_AVAILABLE_IN_ALL
GrlMesh *           grl_mesh_new_cylinder           (gfloat              radius,
                                                     gfloat              height,
                                                     gint                slices);

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
GRL_AVAILABLE_IN_ALL
GrlMesh *           grl_mesh_new_cone               (gfloat              radius,
                                                     gfloat              height,
                                                     gint                slices);

/**
 * grl_mesh_new_torus:
 * @radius: Torus radius
 * @size: Tube size
 * @rad_seg: Radial segments
 * @sides: Number of sides
 *
 * Generates a torus mesh.
 *
 * Returns: (transfer full): A new #GrlMesh
 */
GRL_AVAILABLE_IN_ALL
GrlMesh *           grl_mesh_new_torus              (gfloat              radius,
                                                     gfloat              size,
                                                     gint                rad_seg,
                                                     gint                sides);

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
GRL_AVAILABLE_IN_ALL
GrlMesh *           grl_mesh_new_knot               (gfloat              radius,
                                                     gfloat              size,
                                                     gint                rad_seg,
                                                     gint                sides);

/**
 * grl_mesh_new_plane:
 * @width: Plane width
 * @length: Plane length
 * @res_x: Subdivisions in X
 * @res_z: Subdivisions in Z
 *
 * Generates a flat plane mesh.
 *
 * Returns: (transfer full): A new #GrlMesh
 */
GRL_AVAILABLE_IN_ALL
GrlMesh *           grl_mesh_new_plane              (gfloat              width,
                                                     gfloat              length,
                                                     gint                res_x,
                                                     gint                res_z);

/**
 * grl_mesh_new_heightmap:
 * @heightmap_path: (type filename): Path to heightmap image
 * @size: Size of the mesh
 * @error: (nullable): Return location for error
 *
 * Generates a mesh from a heightmap image.
 *
 * Returns: (transfer full) (nullable): A new #GrlMesh, or %NULL on error
 */
GRL_AVAILABLE_IN_ALL
GrlMesh *           grl_mesh_new_heightmap          (const gchar        *heightmap_path,
                                                     const GrlVector3   *size,
                                                     GError            **error);

/**
 * grl_mesh_new_polygon:
 * @sides: Number of sides
 * @radius: Polygon radius
 *
 * Generates a regular polygon mesh.
 *
 * Returns: (transfer full): A new #GrlMesh
 */
GRL_AVAILABLE_IN_ALL
GrlMesh *           grl_mesh_new_polygon            (gint                sides,
                                                     gfloat              radius);

/**
 * grl_mesh_new_custom:
 * @vertices: (array length=n_vertices) (element-type gfloat): Vertex positions (x,y,z per vertex)
 * @n_vertices: Number of vertices
 * @normals: (array length=n_vertices) (element-type gfloat) (nullable): Vertex normals (x,y,z per vertex), or %NULL to compute
 * @indices: (array length=n_indices) (element-type guint16): Triangle indices
 * @n_indices: Number of indices (must be multiple of 3)
 *
 * Creates a custom mesh from vertex and index data.
 * The mesh copies the provided arrays.
 *
 * If @normals is %NULL, normals will be computed from the triangle geometry.
 *
 * Returns: (transfer full) (nullable): A new #GrlMesh, or %NULL on error
 */
GRL_AVAILABLE_IN_ALL
GrlMesh *           grl_mesh_new_custom             (const gfloat       *vertices,
                                                     guint               n_vertices,
                                                     const gfloat       *normals,
                                                     const guint16      *indices,
                                                     guint               n_indices);

/**
 * grl_mesh_get_vertex_count:
 * @self: A #GrlMesh
 *
 * Gets the number of vertices in the mesh.
 *
 * Returns: The vertex count
 */
GRL_AVAILABLE_IN_ALL
gint                grl_mesh_get_vertex_count       (GrlMesh            *self);

/**
 * grl_mesh_get_triangle_count:
 * @self: A #GrlMesh
 *
 * Gets the number of triangles in the mesh.
 *
 * Returns: The triangle count
 */
GRL_AVAILABLE_IN_ALL
gint                grl_mesh_get_triangle_count     (GrlMesh            *self);

/**
 * grl_mesh_get_bone_count:
 * @self: A #GrlMesh
 *
 * Gets the number of bones influencing this mesh for skinning.
 *
 * Returns: The bone count (0 if the mesh has no skin data)
 */
GRL_AVAILABLE_IN_ALL
gint                grl_mesh_get_bone_count         (GrlMesh            *self);

/**
 * grl_mesh_get_bounding_box:
 * @self: A #GrlMesh
 *
 * Calculates the bounding box of the mesh.
 *
 * Returns: (transfer full): The mesh bounding box
 */
GRL_AVAILABLE_IN_ALL
GrlBoundingBox *    grl_mesh_get_bounding_box       (GrlMesh            *self);

/**
 * grl_mesh_upload:
 * @self: A #GrlMesh
 * @dynamic: Whether to use dynamic buffers
 *
 * Uploads mesh data to GPU memory (VRAM).
 * Call this before drawing with shaders.
 */
GRL_AVAILABLE_IN_ALL
void                grl_mesh_upload                 (GrlMesh            *self,
                                                     gboolean            dynamic);

/**
 * grl_mesh_is_valid:
 * @self: A #GrlMesh
 *
 * Checks if the mesh has valid data.
 *
 * Returns: %TRUE if valid
 */
GRL_AVAILABLE_IN_ALL
gboolean            grl_mesh_is_valid               (GrlMesh            *self);

/**
 * grl_mesh_get_handle:
 * @self: A #GrlMesh
 *
 * Gets the internal raylib Mesh pointer.
 * This is for internal use and advanced users.
 *
 * Returns: (transfer none): Internal mesh pointer
 */
GRL_AVAILABLE_IN_ALL
gpointer            grl_mesh_get_handle             (GrlMesh            *self);

G_END_DECLS
