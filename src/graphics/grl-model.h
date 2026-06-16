/* grl-model.h
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * 3D model with meshes and materials.
 */

#pragma once

#if !defined(GRAYLIB_INSIDE) && !defined(GRAYLIB_COMPILATION)
#error "Only <graylib.h> can be included directly."
#endif

#include <glib-object.h>
#include "../grl-version.h"
#include "grl-mesh.h"
#include "grl-material.h"
#include "grl-bone-info.h"
#include "grl-transform.h"
#include "../math/grl-matrix.h"
#include "../math/grl-bounding-box.h"
#include "../math/grl-vector3.h"
#include "../math/grl-color.h"

G_BEGIN_DECLS

#define GRL_TYPE_MODEL (grl_model_get_type())

GRL_AVAILABLE_IN_ALL
G_DECLARE_DERIVABLE_TYPE (GrlModel, grl_model, GRL, MODEL, GObject)

struct _GrlModelClass
{
    GObjectClass parent_class;

    gpointer _reserved[8];
};

/**
 * grl_model_new_from_file:
 * @filename: (type filename): Path to model file
 * @error: (nullable): Return location for error
 *
 * Loads a 3D model from a file.
 * Supported formats include OBJ, IQM, GLTF, and more.
 *
 * Returns: (transfer full) (nullable): A new #GrlModel, or %NULL on error
 */
GRL_AVAILABLE_IN_ALL
GrlModel *          grl_model_new_from_file         (const gchar        *filename,
                                                     GError            **error);

/**
 * grl_model_new_from_mesh:
 * @mesh: A #GrlMesh to use
 *
 * Creates a model from a single mesh.
 * The model takes ownership of a reference to the mesh and keeps it alive
 * for the lifetime of the model.
 *
 * Returns: (transfer full): A new #GrlModel
 */
GRL_AVAILABLE_IN_ALL
GrlModel *          grl_model_new_from_mesh         (GrlMesh            *mesh);

/**
 * grl_model_is_valid:
 * @self: A #GrlModel
 *
 * Checks if the model is valid.
 *
 * Returns: %TRUE if valid
 */
GRL_AVAILABLE_IN_ALL
gboolean            grl_model_is_valid              (GrlModel           *self);

/**
 * grl_model_get_mesh_count:
 * @self: A #GrlModel
 *
 * Gets the number of meshes in the model.
 *
 * Returns: The mesh count
 */
GRL_AVAILABLE_IN_ALL
gint                grl_model_get_mesh_count        (GrlModel           *self);

/**
 * grl_model_get_material_count:
 * @self: A #GrlModel
 *
 * Gets the number of materials in the model.
 *
 * Returns: The material count
 */
GRL_AVAILABLE_IN_ALL
gint                grl_model_get_material_count    (GrlModel           *self);

/**
 * grl_model_get_bone_count:
 * @self: A #GrlModel
 *
 * Gets the number of bones in the model's skeleton.
 *
 * Returns: The bone count
 */
GRL_AVAILABLE_IN_ALL
gint                grl_model_get_bone_count        (GrlModel           *self);

/**
 * grl_model_get_bone:
 * @self: A #GrlModel
 * @index: Bone index in range [0, grl_model_get_bone_count())
 *
 * Gets metadata for a bone in the model's animation skeleton.
 *
 * Returns: (transfer full) (nullable): A new #GrlBoneInfo, or %NULL if the
 *          model has no skeleton or @index is out of range
 */
GRL_AVAILABLE_IN_ALL
GrlBoneInfo *       grl_model_get_bone              (GrlModel           *self,
                                                     gint                index);

/**
 * grl_model_get_bind_pose_transform:
 * @self: A #GrlModel
 * @bone: Bone index in range [0, grl_model_get_bone_count())
 *
 * Gets the bind-pose (rest) transform of a skeleton bone.
 *
 * Returns: (transfer full) (nullable): A new #GrlTransform, or %NULL if the
 *          model has no skeleton or @bone is out of range
 */
GRL_AVAILABLE_IN_ALL
GrlTransform *      grl_model_get_bind_pose_transform (GrlModel         *self,
                                                       gint              bone);

/**
 * grl_model_get_bounding_box:
 * @self: A #GrlModel
 *
 * Gets the bounding box of the model.
 *
 * Returns: (transfer full): The model's bounding box
 */
GRL_AVAILABLE_IN_ALL
GrlBoundingBox *    grl_model_get_bounding_box      (GrlModel           *self);

/**
 * grl_model_get_transform:
 * @self: A #GrlModel
 *
 * Gets the model's transformation matrix.
 *
 * Returns: (transfer full): The transform matrix
 */
GRL_AVAILABLE_IN_ALL
GrlMatrix *         grl_model_get_transform         (GrlModel           *self);

/**
 * grl_model_set_transform:
 * @self: A #GrlModel
 * @transform: The transformation matrix
 *
 * Sets the model's transformation matrix.
 */
GRL_AVAILABLE_IN_ALL
void                grl_model_set_transform         (GrlModel           *self,
                                                     const GrlMatrix    *transform);

/**
 * grl_model_set_material:
 * @self: A #GrlModel
 * @mesh_index: Index of the mesh
 * @material: The material to set
 *
 * Sets the material for a specific mesh in the model.
 */
GRL_AVAILABLE_IN_ALL
void                grl_model_set_material          (GrlModel           *self,
                                                     gint                mesh_index,
                                                     GrlMaterial        *material);

/**
 * grl_model_set_texture:
 * @self: A #GrlModel
 * @material_index: Index of the material
 * @map_type: Which texture map to set
 * @texture: The texture to use
 *
 * Sets a texture for a material in the model.
 */
GRL_AVAILABLE_IN_ALL
void                grl_model_set_texture           (GrlModel           *self,
                                                     gint                material_index,
                                                     GrlMaterialMapType  map_type,
                                                     GrlTexture         *texture);

/**
 * grl_model_draw:
 * @self: A #GrlModel
 * @position: World position to draw at
 * @scale: Scale factor
 * @tint: Color tint
 *
 * Draws the model at the specified position.
 */
GRL_AVAILABLE_IN_ALL
void                grl_model_draw                  (GrlModel           *self,
                                                     const GrlVector3   *position,
                                                     gfloat              scale,
                                                     GrlColor           *tint);

/**
 * grl_model_draw_ex:
 * @self: A #GrlModel
 * @position: World position
 * @rotation_axis: Axis to rotate around
 * @rotation_angle: Rotation angle in degrees
 * @scale: Scale for each axis
 * @tint: Color tint
 *
 * Draws the model with extended parameters.
 */
GRL_AVAILABLE_IN_ALL
void                grl_model_draw_ex               (GrlModel           *self,
                                                     const GrlVector3   *position,
                                                     const GrlVector3   *rotation_axis,
                                                     gfloat              rotation_angle,
                                                     const GrlVector3   *scale,
                                                     GrlColor           *tint);

/**
 * grl_model_draw_wires:
 * @self: A #GrlModel
 * @position: World position
 * @scale: Scale factor
 * @tint: Wire color
 *
 * Draws the model as wireframe.
 */
GRL_AVAILABLE_IN_ALL
void                grl_model_draw_wires            (GrlModel           *self,
                                                     const GrlVector3   *position,
                                                     gfloat              scale,
                                                     GrlColor           *tint);

/**
 * grl_model_draw_wires_ex:
 * @self: A #GrlModel
 * @position: World position
 * @rotation_axis: Axis to rotate around
 * @rotation_angle: Rotation angle in degrees
 * @scale: Scale for each axis
 * @tint: Wire color
 *
 * Draws the model as wireframe with extended parameters.
 */
GRL_AVAILABLE_IN_ALL
void                grl_model_draw_wires_ex         (GrlModel           *self,
                                                     const GrlVector3   *position,
                                                     const GrlVector3   *rotation_axis,
                                                     gfloat              rotation_angle,
                                                     const GrlVector3   *scale,
                                                     GrlColor           *tint);

/**
 * grl_model_get_handle:
 * @self: A #GrlModel
 *
 * Gets the internal raylib Model pointer.
 * This is for internal use and advanced users.
 *
 * Returns: (transfer none): Internal model pointer
 */
GRL_AVAILABLE_IN_ALL
gpointer            grl_model_get_handle            (GrlModel           *self);

G_END_DECLS
