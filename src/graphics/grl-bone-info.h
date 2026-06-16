/* grl-bone-info.h
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Skeleton bone metadata (name + parent index).
 */

#pragma once

#if !defined(GRAYLIB_INSIDE) && !defined(GRAYLIB_COMPILATION)
#error "Only <graylib.h> can be included directly."
#endif

#include <glib-object.h>
#include "../grl-version.h"

G_BEGIN_DECLS

/* Forward declaration for this file */
typedef struct _GrlBoneInfo GrlBoneInfo;

#define GRL_TYPE_BONE_INFO (grl_bone_info_get_type ())

/**
 * GrlBoneInfo:
 *
 * Metadata describing a single bone within a model's animation skeleton:
 * the bone name and the index of its parent bone (-1 for a root bone).
 *
 * This is a GBoxed value type backing raylib 6.0's `BoneInfo`. Bone data
 * lives on the model's skeleton; use grl_model_get_bone() to obtain it.
 */
struct _GrlBoneInfo
{
    gchar name[32];
    gint  parent;
};

GRL_AVAILABLE_IN_ALL
GType           grl_bone_info_get_type   (void) G_GNUC_CONST;

GRL_AVAILABLE_IN_ALL
GrlBoneInfo *   grl_bone_info_new        (const gchar       *name,
                                          gint               parent);

GRL_AVAILABLE_IN_ALL
GrlBoneInfo *   grl_bone_info_copy       (const GrlBoneInfo *self);

GRL_AVAILABLE_IN_ALL
void            grl_bone_info_free       (GrlBoneInfo       *self);

/*
 * Accessors
 */

GRL_AVAILABLE_IN_ALL
const gchar *   grl_bone_info_get_name   (const GrlBoneInfo *self);

GRL_AVAILABLE_IN_ALL
gint            grl_bone_info_get_parent (const GrlBoneInfo *self);

G_DEFINE_AUTOPTR_CLEANUP_FUNC (GrlBoneInfo, grl_bone_info_free)

G_END_DECLS
