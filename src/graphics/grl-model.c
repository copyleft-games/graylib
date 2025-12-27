/* grl-model.c
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */

#include "grl-model.h"
#include <gio/gio.h>
#include <raylib.h>

/**
 * SECTION:grl-model
 * @Title: GrlModel
 * @Short_description: 3D model with meshes and materials
 *
 * #GrlModel represents a 3D model consisting of one or more meshes,
 * materials, and optionally a skeleton for animation.
 *
 * # Loading Models
 *
 * Models can be loaded from various file formats:
 * |[<!-- language="C" -->
 * g_autoptr(GError) error = NULL;
 * g_autoptr(GrlModel) model = grl_model_new_from_file ("model.obj", &error);
 * if (model == NULL)
 *     g_printerr ("Failed to load: %s\n", error->message);
 * ]|
 *
 * # Drawing Models
 *
 * Models are drawn within a 3D camera context:
 * |[<!-- language="C" -->
 * g_autoptr(GrlVector3) pos = grl_vector3_new (0.0f, 0.0f, 0.0f);
 * g_autoptr(GrlColor) white = grl_color_new (255, 255, 255, 255);
 * grl_model_draw (model, pos, 1.0f, white);
 * ]|
 */

typedef struct
{
    Model        model;
    gboolean     valid;
    GrlMaterial **materials;
    gint         material_count;
    GrlMesh     *source_mesh;   /* Holds ref to mesh if created via new_from_mesh */
} GrlModelPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (GrlModel, grl_model, G_TYPE_OBJECT)

enum
{
    PROP_0,
    PROP_VALID,
    PROP_MESH_COUNT,
    PROP_MATERIAL_COUNT,
    PROP_BONE_COUNT,
    N_PROPS
};

static GParamSpec *properties[N_PROPS];

static void
grl_model_dispose (GObject *object)
{
    GrlModel *self = GRL_MODEL (object);
    GrlModelPrivate *priv = grl_model_get_instance_private (self);
    gint i;

    /* If model was created from a GrlMesh, null out mesh pointers before
     * releasing reference. This prevents UnloadModel() from double-freeing
     * the mesh data that source_mesh owns. raylib's UnloadMesh() safely
     * handles zeroed data: rlUnloadVertexArray(0) is a no-op, it checks
     * vboId != NULL before iterating, and RL_FREE(NULL) is safe.
     */
    if (priv->source_mesh != NULL && priv->valid)
    {
        for (i = 0; i < priv->model.meshCount; i++)
        {
            memset (&priv->model.meshes[i], 0, sizeof (Mesh));
        }
        g_clear_object (&priv->source_mesh);
    }

    /* Release material references */
    if (priv->materials != NULL)
    {
        for (i = 0; i < priv->material_count; i++)
            g_clear_object (&priv->materials[i]);

        g_free (priv->materials);
        priv->materials = NULL;
    }

    G_OBJECT_CLASS (grl_model_parent_class)->dispose (object);
}

static void
grl_model_finalize (GObject *object)
{
    GrlModel *self = GRL_MODEL (object);
    GrlModelPrivate *priv = grl_model_get_instance_private (self);

    if (priv->valid)
    {
        UnloadModel (priv->model);
        priv->valid = FALSE;
    }

    G_OBJECT_CLASS (grl_model_parent_class)->finalize (object);
}

static void
grl_model_get_property (GObject    *object,
                        guint       prop_id,
                        GValue     *value,
                        GParamSpec *pspec)
{
    GrlModel *self = GRL_MODEL (object);
    GrlModelPrivate *priv = grl_model_get_instance_private (self);

    switch (prop_id)
    {
    case PROP_VALID:
        g_value_set_boolean (value, priv->valid);
        break;

    case PROP_MESH_COUNT:
        g_value_set_int (value, priv->valid ? priv->model.meshCount : 0);
        break;

    case PROP_MATERIAL_COUNT:
        g_value_set_int (value, priv->valid ? priv->model.materialCount : 0);
        break;

    case PROP_BONE_COUNT:
        g_value_set_int (value, priv->valid ? priv->model.boneCount : 0);
        break;

    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
grl_model_class_init (GrlModelClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    object_class->dispose = grl_model_dispose;
    object_class->finalize = grl_model_finalize;
    object_class->get_property = grl_model_get_property;

    /**
     * GrlModel:valid:
     *
     * Whether the model is valid.
     */
    properties[PROP_VALID] =
        g_param_spec_boolean ("valid",
                              "Valid",
                              "Whether model is valid",
                              FALSE,
                              G_PARAM_READABLE |
                              G_PARAM_STATIC_STRINGS);

    /**
     * GrlModel:mesh-count:
     *
     * Number of meshes in the model.
     */
    properties[PROP_MESH_COUNT] =
        g_param_spec_int ("mesh-count",
                          "Mesh Count",
                          "Number of meshes",
                          0, G_MAXINT, 0,
                          G_PARAM_READABLE |
                          G_PARAM_STATIC_STRINGS);

    /**
     * GrlModel:material-count:
     *
     * Number of materials in the model.
     */
    properties[PROP_MATERIAL_COUNT] =
        g_param_spec_int ("material-count",
                          "Material Count",
                          "Number of materials",
                          0, G_MAXINT, 0,
                          G_PARAM_READABLE |
                          G_PARAM_STATIC_STRINGS);

    /**
     * GrlModel:bone-count:
     *
     * Number of bones in the model skeleton.
     */
    properties[PROP_BONE_COUNT] =
        g_param_spec_int ("bone-count",
                          "Bone Count",
                          "Number of bones",
                          0, G_MAXINT, 0,
                          G_PARAM_READABLE |
                          G_PARAM_STATIC_STRINGS);

    g_object_class_install_properties (object_class, N_PROPS, properties);
}

static void
grl_model_init (GrlModel *self)
{
    GrlModelPrivate *priv = grl_model_get_instance_private (self);

    priv->valid = FALSE;
    priv->materials = NULL;
    priv->material_count = 0;
    priv->source_mesh = NULL;
    memset (&priv->model, 0, sizeof (Model));
}

/**
 * grl_model_new_from_file:
 * @filename: (type filename): Path to model file
 * @error: (nullable): Return location for error
 *
 * Loads a 3D model from a file.
 * Supported formats include OBJ, IQM, GLTF, GLB, and more.
 *
 * Returns: (transfer full) (nullable): A new #GrlModel, or %NULL on error
 */
GrlModel *
grl_model_new_from_file (const gchar  *filename,
                         GError      **error)
{
    GrlModel *self;
    GrlModelPrivate *priv;
    Model model;

    g_return_val_if_fail (filename != NULL, NULL);

    model = LoadModel (filename);

    if (model.meshCount == 0)
    {
        g_set_error (error,
                     G_IO_ERROR,
                     G_IO_ERROR_FAILED,
                     "Failed to load model: %s",
                     filename);
        return NULL;
    }

    self = g_object_new (GRL_TYPE_MODEL, NULL);
    priv = grl_model_get_instance_private (self);

    priv->model = model;
    priv->valid = TRUE;

    return self;
}

/**
 * grl_model_new_from_mesh:
 * @mesh: A #GrlMesh to use
 *
 * Creates a model from a single mesh.
 * The model takes ownership of a reference to the mesh and keeps it alive
 * for the lifetime of the model. The underlying mesh data is shared, not
 * copied.
 *
 * Returns: (transfer full): A new #GrlModel
 */
GrlModel *
grl_model_new_from_mesh (GrlMesh *mesh)
{
    GrlModel *self;
    GrlModelPrivate *priv;
    Mesh *rl_mesh;

    g_return_val_if_fail (GRL_IS_MESH (mesh), NULL);

    self = g_object_new (GRL_TYPE_MODEL, NULL);
    priv = grl_model_get_instance_private (self);

    rl_mesh = (Mesh *)grl_mesh_get_handle (mesh);
    priv->model = LoadModelFromMesh (*rl_mesh);
    priv->valid = (priv->model.meshCount > 0);

    /* Keep reference to source mesh - raylib does a shallow copy of the Mesh
     * struct, so the model's meshes array shares pointers with the original.
     * We must keep the mesh alive to prevent use-after-free.
     */
    priv->source_mesh = g_object_ref (mesh);

    return self;
}

/**
 * grl_model_is_valid:
 * @self: A #GrlModel
 *
 * Checks if the model is valid and ready for use.
 *
 * Returns: %TRUE if valid
 */
gboolean
grl_model_is_valid (GrlModel *self)
{
    GrlModelPrivate *priv;

    g_return_val_if_fail (GRL_IS_MODEL (self), FALSE);

    priv = grl_model_get_instance_private (self);

    return priv->valid;
}

/**
 * grl_model_get_mesh_count:
 * @self: A #GrlModel
 *
 * Gets the number of meshes in the model.
 *
 * Returns: The mesh count
 */
gint
grl_model_get_mesh_count (GrlModel *self)
{
    GrlModelPrivate *priv;

    g_return_val_if_fail (GRL_IS_MODEL (self), 0);

    priv = grl_model_get_instance_private (self);

    return priv->valid ? priv->model.meshCount : 0;
}

/**
 * grl_model_get_material_count:
 * @self: A #GrlModel
 *
 * Gets the number of materials in the model.
 *
 * Returns: The material count
 */
gint
grl_model_get_material_count (GrlModel *self)
{
    GrlModelPrivate *priv;

    g_return_val_if_fail (GRL_IS_MODEL (self), 0);

    priv = grl_model_get_instance_private (self);

    return priv->valid ? priv->model.materialCount : 0;
}

/**
 * grl_model_get_bone_count:
 * @self: A #GrlModel
 *
 * Gets the number of bones in the model's skeleton.
 *
 * Returns: The bone count
 */
gint
grl_model_get_bone_count (GrlModel *self)
{
    GrlModelPrivate *priv;

    g_return_val_if_fail (GRL_IS_MODEL (self), 0);

    priv = grl_model_get_instance_private (self);

    return priv->valid ? priv->model.boneCount : 0;
}

/**
 * grl_model_get_bounding_box:
 * @self: A #GrlModel
 *
 * Calculates the bounding box that encloses all meshes in the model.
 *
 * Returns: (transfer full): The model's bounding box
 */
GrlBoundingBox *
grl_model_get_bounding_box (GrlModel *self)
{
    GrlModelPrivate *priv;
    BoundingBox bb;

    g_return_val_if_fail (GRL_IS_MODEL (self), NULL);

    priv = grl_model_get_instance_private (self);

    if (!priv->valid)
        return grl_bounding_box_new_xyz (0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);

    bb = GetModelBoundingBox (priv->model);

    return grl_bounding_box_new_xyz (bb.min.x, bb.min.y, bb.min.z,
                                     bb.max.x, bb.max.y, bb.max.z);
}

/**
 * grl_model_get_transform:
 * @self: A #GrlModel
 *
 * Gets the model's transformation matrix.
 *
 * Returns: (transfer full): The transform matrix
 */
GrlMatrix *
grl_model_get_transform (GrlModel *self)
{
    GrlModelPrivate *priv;
    GrlMatrix *matrix;
    Matrix m;

    g_return_val_if_fail (GRL_IS_MODEL (self), NULL);

    priv = grl_model_get_instance_private (self);

    if (!priv->valid)
        return grl_matrix_new_identity ();

    m = priv->model.transform;
    matrix = grl_matrix_new ();

    matrix->m0 = m.m0;   matrix->m4 = m.m4;   matrix->m8 = m.m8;   matrix->m12 = m.m12;
    matrix->m1 = m.m1;   matrix->m5 = m.m5;   matrix->m9 = m.m9;   matrix->m13 = m.m13;
    matrix->m2 = m.m2;   matrix->m6 = m.m6;   matrix->m10 = m.m10; matrix->m14 = m.m14;
    matrix->m3 = m.m3;   matrix->m7 = m.m7;   matrix->m11 = m.m11; matrix->m15 = m.m15;

    return matrix;
}

/**
 * grl_model_set_transform:
 * @self: A #GrlModel
 * @transform: The transformation matrix
 *
 * Sets the model's transformation matrix.
 */
void
grl_model_set_transform (GrlModel        *self,
                         const GrlMatrix *transform)
{
    GrlModelPrivate *priv;

    g_return_if_fail (GRL_IS_MODEL (self));
    g_return_if_fail (transform != NULL);

    priv = grl_model_get_instance_private (self);

    if (!priv->valid)
        return;

    priv->model.transform.m0 = transform->m0;
    priv->model.transform.m1 = transform->m1;
    priv->model.transform.m2 = transform->m2;
    priv->model.transform.m3 = transform->m3;
    priv->model.transform.m4 = transform->m4;
    priv->model.transform.m5 = transform->m5;
    priv->model.transform.m6 = transform->m6;
    priv->model.transform.m7 = transform->m7;
    priv->model.transform.m8 = transform->m8;
    priv->model.transform.m9 = transform->m9;
    priv->model.transform.m10 = transform->m10;
    priv->model.transform.m11 = transform->m11;
    priv->model.transform.m12 = transform->m12;
    priv->model.transform.m13 = transform->m13;
    priv->model.transform.m14 = transform->m14;
    priv->model.transform.m15 = transform->m15;
}

/**
 * grl_model_set_material:
 * @self: A #GrlModel
 * @mesh_index: Index of the mesh
 * @material: The material to set
 *
 * Sets the material for a specific mesh in the model.
 */
void
grl_model_set_material (GrlModel    *self,
                        gint         mesh_index,
                        GrlMaterial *material)
{
    GrlModelPrivate *priv;

    g_return_if_fail (GRL_IS_MODEL (self));
    g_return_if_fail (GRL_IS_MATERIAL (material));

    priv = grl_model_get_instance_private (self);

    if (!priv->valid || mesh_index < 0 || mesh_index >= priv->model.meshCount)
        return;

    /* Copy material data to model */
    priv->model.materials[priv->model.meshMaterial[mesh_index]] =
        *(Material *)grl_material_get_handle (material);
}

/**
 * grl_model_set_texture:
 * @self: A #GrlModel
 * @material_index: Index of the material
 * @map_type: Which texture map to set
 * @texture: The texture to use
 *
 * Sets a texture for a material in the model.
 */
void
grl_model_set_texture (GrlModel           *self,
                       gint                material_index,
                       GrlMaterialMapType  map_type,
                       GrlTexture         *texture)
{
    GrlModelPrivate *priv;

    g_return_if_fail (GRL_IS_MODEL (self));
    g_return_if_fail (GRL_IS_TEXTURE (texture));

    priv = grl_model_get_instance_private (self);

    if (!priv->valid || material_index < 0 ||
        material_index >= priv->model.materialCount)
        return;

    SetMaterialTexture (&priv->model.materials[material_index],
                        map_type,
                        *(Texture2D *)grl_texture_get_handle (texture));
}

/**
 * grl_model_draw:
 * @self: A #GrlModel
 * @position: World position to draw at
 * @scale: Uniform scale factor
 * @tint: Color tint
 *
 * Draws the model at the specified position with uniform scaling.
 */
void
grl_model_draw (GrlModel         *self,
                const GrlVector3 *position,
                gfloat            scale,
                GrlColor         *tint)
{
    GrlModelPrivate *priv;
    Vector3 pos;
    Color color;

    g_return_if_fail (GRL_IS_MODEL (self));
    g_return_if_fail (position != NULL);
    g_return_if_fail (tint != NULL);

    priv = grl_model_get_instance_private (self);

    if (!priv->valid)
        return;

    pos.x = position->x;
    pos.y = position->y;
    pos.z = position->z;

    color.r = grl_color_get_r (tint);
    color.g = grl_color_get_g (tint);
    color.b = grl_color_get_b (tint);
    color.a = grl_color_get_a (tint);

    DrawModel (priv->model, pos, scale, color);
}

/**
 * grl_model_draw_ex:
 * @self: A #GrlModel
 * @position: World position
 * @rotation_axis: Axis to rotate around
 * @rotation_angle: Rotation angle in degrees
 * @scale: Scale for each axis
 * @tint: Color tint
 *
 * Draws the model with extended parameters for rotation and non-uniform scale.
 */
void
grl_model_draw_ex (GrlModel         *self,
                   const GrlVector3 *position,
                   const GrlVector3 *rotation_axis,
                   gfloat            rotation_angle,
                   const GrlVector3 *scale,
                   GrlColor         *tint)
{
    GrlModelPrivate *priv;
    Vector3 pos, axis, scl;
    Color color;

    g_return_if_fail (GRL_IS_MODEL (self));
    g_return_if_fail (position != NULL);
    g_return_if_fail (rotation_axis != NULL);
    g_return_if_fail (scale != NULL);
    g_return_if_fail (tint != NULL);

    priv = grl_model_get_instance_private (self);

    if (!priv->valid)
        return;

    pos.x = position->x;
    pos.y = position->y;
    pos.z = position->z;

    axis.x = rotation_axis->x;
    axis.y = rotation_axis->y;
    axis.z = rotation_axis->z;

    scl.x = scale->x;
    scl.y = scale->y;
    scl.z = scale->z;

    color.r = grl_color_get_r (tint);
    color.g = grl_color_get_g (tint);
    color.b = grl_color_get_b (tint);
    color.a = grl_color_get_a (tint);

    DrawModelEx (priv->model, pos, axis, rotation_angle, scl, color);
}

/**
 * grl_model_draw_wires:
 * @self: A #GrlModel
 * @position: World position
 * @scale: Uniform scale factor
 * @tint: Wire color
 *
 * Draws the model as wireframe with uniform scaling.
 */
void
grl_model_draw_wires (GrlModel         *self,
                      const GrlVector3 *position,
                      gfloat            scale,
                      GrlColor         *tint)
{
    GrlModelPrivate *priv;
    Vector3 pos;
    Color color;

    g_return_if_fail (GRL_IS_MODEL (self));
    g_return_if_fail (position != NULL);
    g_return_if_fail (tint != NULL);

    priv = grl_model_get_instance_private (self);

    if (!priv->valid)
        return;

    pos.x = position->x;
    pos.y = position->y;
    pos.z = position->z;

    color.r = grl_color_get_r (tint);
    color.g = grl_color_get_g (tint);
    color.b = grl_color_get_b (tint);
    color.a = grl_color_get_a (tint);

    DrawModelWires (priv->model, pos, scale, color);
}

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
void
grl_model_draw_wires_ex (GrlModel         *self,
                         const GrlVector3 *position,
                         const GrlVector3 *rotation_axis,
                         gfloat            rotation_angle,
                         const GrlVector3 *scale,
                         GrlColor         *tint)
{
    GrlModelPrivate *priv;
    Vector3 pos, axis, scl;
    Color color;

    g_return_if_fail (GRL_IS_MODEL (self));
    g_return_if_fail (position != NULL);
    g_return_if_fail (rotation_axis != NULL);
    g_return_if_fail (scale != NULL);
    g_return_if_fail (tint != NULL);

    priv = grl_model_get_instance_private (self);

    if (!priv->valid)
        return;

    pos.x = position->x;
    pos.y = position->y;
    pos.z = position->z;

    axis.x = rotation_axis->x;
    axis.y = rotation_axis->y;
    axis.z = rotation_axis->z;

    scl.x = scale->x;
    scl.y = scale->y;
    scl.z = scale->z;

    color.r = grl_color_get_r (tint);
    color.g = grl_color_get_g (tint);
    color.b = grl_color_get_b (tint);
    color.a = grl_color_get_a (tint);

    DrawModelWiresEx (priv->model, pos, axis, rotation_angle, scl, color);
}

/**
 * grl_model_get_handle:
 * @self: A #GrlModel
 *
 * Gets the internal raylib Model pointer.
 * This is for internal use and advanced users only.
 *
 * Returns: (transfer none): Internal model pointer
 */
gpointer
grl_model_get_handle (GrlModel *self)
{
    GrlModelPrivate *priv;

    g_return_val_if_fail (GRL_IS_MODEL (self), NULL);

    priv = grl_model_get_instance_private (self);

    return &priv->model;
}
