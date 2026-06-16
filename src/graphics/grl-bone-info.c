/* grl-bone-info.c
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Implementation of the bone-metadata value type.
 */

#include "config.h"
#include "grl-bone-info.h"
#include <string.h>

/**
 * SECTION:grl-bone-info
 * @Title: GrlBoneInfo
 * @Short_description: Skeleton bone metadata
 *
 * #GrlBoneInfo describes one bone of a model's animation skeleton: its
 * name and the index of its parent bone (a parent of -1 marks a root bone).
 * It is a GBoxed value type wrapping raylib's `BoneInfo`.
 */

G_DEFINE_BOXED_TYPE (GrlBoneInfo, grl_bone_info,
                     grl_bone_info_copy, grl_bone_info_free)

/**
 * grl_bone_info_new:
 * @name: (nullable): The bone name (truncated to 31 characters)
 * @parent: Index of the parent bone, or -1 for a root bone
 *
 * Creates a new #GrlBoneInfo.
 *
 * Returns: (transfer full): A newly allocated #GrlBoneInfo
 */
GrlBoneInfo *
grl_bone_info_new (const gchar *name,
                   gint         parent)
{
    GrlBoneInfo *self;

    self = g_new0 (GrlBoneInfo, 1);

    if (name != NULL)
    {
        g_strlcpy (self->name, name, sizeof (self->name));
    }

    self->parent = parent;

    return self;
}

/**
 * grl_bone_info_copy:
 * @self: A #GrlBoneInfo
 *
 * Creates a deep copy of @self.
 *
 * Returns: (transfer full): A newly allocated #GrlBoneInfo
 */
GrlBoneInfo *
grl_bone_info_copy (const GrlBoneInfo *self)
{
    GrlBoneInfo *copy;

    g_return_val_if_fail (self != NULL, NULL);

    copy = g_new0 (GrlBoneInfo, 1);
    memcpy (copy, self, sizeof (GrlBoneInfo));

    return copy;
}

/**
 * grl_bone_info_free:
 * @self: A #GrlBoneInfo
 *
 * Frees a bone-info value allocated with grl_bone_info_new() or
 * grl_bone_info_copy().
 */
void
grl_bone_info_free (GrlBoneInfo *self)
{
    g_return_if_fail (self != NULL);

    g_free (self);
}

/**
 * grl_bone_info_get_name:
 * @self: A #GrlBoneInfo
 *
 * Gets the bone name.
 *
 * Returns: (transfer none): The bone name
 */
const gchar *
grl_bone_info_get_name (const GrlBoneInfo *self)
{
    g_return_val_if_fail (self != NULL, "");

    return self->name;
}

/**
 * grl_bone_info_get_parent:
 * @self: A #GrlBoneInfo
 *
 * Gets the index of this bone's parent within the skeleton.
 *
 * Returns: The parent bone index, or -1 for a root bone
 */
gint
grl_bone_info_get_parent (const GrlBoneInfo *self)
{
    g_return_val_if_fail (self != NULL, -1);

    return self->parent;
}
