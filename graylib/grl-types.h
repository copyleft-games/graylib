/* grl-types.h
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Forward declarations for all Graylib types.
 */

#pragma once

#if !defined(GRAYLIB_INSIDE) && !defined(GRAYLIB_COMPILATION)
#error "Only <graylib.h> can be included directly."
#endif

#include <glib.h>

G_BEGIN_DECLS

/*
 * =============================================================================
 * GBoxed Types (Value Types)
 * =============================================================================
 *
 * These are stack-allocatable, copyable value types registered with GLib's
 * type system as GBoxed types. They can be used in GObject properties and
 * signals, and are fully supported by GObject Introspection.
 */

/**
 * GrlVector2:
 *
 * A 2D vector for positions, velocities, and other 2-component values.
 *
 * This is a GBoxed type.
 */
typedef struct _GrlVector2 GrlVector2;

/**
 * GrlVector3:
 *
 * A 3D vector for positions, velocities, and other 3-component values.
 *
 * This is a GBoxed type.
 */
typedef struct _GrlVector3 GrlVector3;

/**
 * GrlVector4:
 *
 * A 4D vector for homogeneous coordinates, quaternions, and other
 * 4-component values.
 *
 * This is a GBoxed type.
 */
typedef struct _GrlVector4 GrlVector4;

/**
 * GrlColor:
 *
 * An RGBA color with 8-bit components.
 *
 * This is a GBoxed type.
 */
typedef struct _GrlColor GrlColor;

/**
 * GrlRectangle:
 *
 * A 2D axis-aligned rectangle defined by position and size.
 *
 * This is a GBoxed type.
 */
typedef struct _GrlRectangle GrlRectangle;

/**
 * GrlMatrix:
 *
 * A 4x4 transformation matrix in column-major order.
 *
 * This is a GBoxed type.
 */
typedef struct _GrlMatrix GrlMatrix;

/**
 * GrlBoundingBox:
 *
 * A 3D axis-aligned bounding box defined by min and max corners.
 *
 * This is a GBoxed type.
 */
typedef struct _GrlBoundingBox GrlBoundingBox;

/*
 * =============================================================================
 * GObject Types (Reference Types)
 * =============================================================================
 *
 * These are heap-allocated, reference-counted objects that inherit from
 * GObject. They support properties, signals, and can be subclassed.
 */

/*
 * Core Module
 */

/**
 * GrlApplication:
 *
 * The main application singleton that manages the raylib context.
 *
 * This is a derivable GObject type.
 */
typedef struct _GrlApplication GrlApplication;
typedef struct _GrlApplicationClass GrlApplicationClass;

/**
 * GrlWindow:
 *
 * A window for rendering graphics.
 *
 * This is a derivable GObject type.
 */
typedef struct _GrlWindow GrlWindow;
typedef struct _GrlWindowClass GrlWindowClass;

/**
 * GrlInput:
 *
 * Input manager for keyboard, mouse, and gamepad input.
 *
 * This is a derivable GObject type.
 */
typedef struct _GrlInput GrlInput;
typedef struct _GrlInputClass GrlInputClass;

/*
 * Graphics Module
 */

/**
 * GrlResource:
 *
 * Abstract base class for loadable resources.
 *
 * This is a derivable GObject type.
 */
typedef struct _GrlResource GrlResource;
typedef struct _GrlResourceClass GrlResourceClass;

/**
 * GrlImage:
 *
 * A CPU-side image (pixel data in RAM).
 *
 * This is a final GObject type.
 */
typedef struct _GrlImage GrlImage;
/* Note: GrlImageClass is defined by G_DECLARE_FINAL_TYPE in grl-image.h */

/**
 * GrlTexture:
 *
 * A GPU texture (pixel data in VRAM).
 *
 * This is a final GObject type.
 */
typedef struct _GrlTexture GrlTexture;
/* Note: GrlTextureClass is defined by G_DECLARE_FINAL_TYPE in grl-texture.h */

/**
 * GrlAnimatedTexture:
 *
 * A spritesheet animation texture.
 *
 * This is a final GObject type.
 */
typedef struct _GrlAnimatedTexture GrlAnimatedTexture;
/* Note: GrlAnimatedTextureClass is defined by G_DECLARE_FINAL_TYPE */

/**
 * GrlRenderTexture:
 *
 * A render target texture (framebuffer object).
 *
 * This is a derivable GObject type.
 */
typedef struct _GrlRenderTexture GrlRenderTexture;
typedef struct _GrlRenderTextureClass GrlRenderTextureClass;

/**
 * GrlShader:
 *
 * A compiled shader program.
 *
 * This is a derivable GObject type.
 */
typedef struct _GrlShader GrlShader;
typedef struct _GrlShaderClass GrlShaderClass;

/**
 * GrlFont:
 *
 * A font for text rendering.
 *
 * This is a final GObject type.
 */
typedef struct _GrlFont GrlFont;
/* Note: GrlFontClass is defined by G_DECLARE_FINAL_TYPE in grl-font.h */

/**
 * GrlCamera2D:
 *
 * A 2D camera for 2D games.
 *
 * This is a derivable GObject type.
 */
typedef struct _GrlCamera2D GrlCamera2D;
typedef struct _GrlCamera2DClass GrlCamera2DClass;

/**
 * GrlCamera3D:
 *
 * A 3D camera for 3D games.
 *
 * This is a derivable GObject type.
 */
typedef struct _GrlCamera3D GrlCamera3D;
typedef struct _GrlCamera3DClass GrlCamera3DClass;

/*
 * Audio Module
 */

/**
 * GrlAudioDevice:
 *
 * The audio device singleton.
 *
 * This is a derivable GObject type.
 */
typedef struct _GrlAudioDevice GrlAudioDevice;
typedef struct _GrlAudioDeviceClass GrlAudioDeviceClass;

/**
 * GrlSound:
 *
 * A sound effect (loaded entirely into memory).
 *
 * This is a final GObject type.
 */
typedef struct _GrlSound GrlSound;

/**
 * GrlMusic:
 *
 * Streaming music (loaded in chunks).
 *
 * This is a final GObject type.
 */
typedef struct _GrlMusic GrlMusic;

/*
 * Models Module
 */

/**
 * GrlMesh:
 *
 * A 3D mesh (vertex data).
 *
 * This is a derivable GObject type.
 */
typedef struct _GrlMesh GrlMesh;
typedef struct _GrlMeshClass GrlMeshClass;

/**
 * GrlMaterial:
 *
 * A material with textures and shader properties.
 *
 * This is a derivable GObject type.
 */
typedef struct _GrlMaterial GrlMaterial;
typedef struct _GrlMaterialClass GrlMaterialClass;

/**
 * GrlModel:
 *
 * A 3D model (meshes + materials).
 *
 * This is a derivable GObject type.
 */
typedef struct _GrlModel GrlModel;
typedef struct _GrlModelClass GrlModelClass;

/**
 * GrlModelAnimation:
 *
 * A skeletal animation for models.
 *
 * This is a derivable GObject type.
 */
typedef struct _GrlModelAnimation GrlModelAnimation;
typedef struct _GrlModelAnimationClass GrlModelAnimationClass;

/*
 * Scene Module
 */

/**
 * GrlEntity:
 *
 * Base class for game entities.
 *
 * This is a derivable GObject type.
 */
typedef struct _GrlEntity GrlEntity;
typedef struct _GrlEntityClass GrlEntityClass;

/**
 * GrlSprite:
 *
 * A 2D sprite entity.
 *
 * This is a derivable GObject type.
 */
typedef struct _GrlSprite GrlSprite;
typedef struct _GrlSpriteClass GrlSpriteClass;

/**
 * GrlScene:
 *
 * A container for entities and scene data.
 *
 * This is a derivable GObject type.
 */
typedef struct _GrlScene GrlScene;
typedef struct _GrlSceneClass GrlSceneClass;

/**
 * GrlSceneManager:
 *
 * A stack-based scene manager.
 *
 * This is a derivable GObject type.
 */
typedef struct _GrlSceneManager GrlSceneManager;
typedef struct _GrlSceneManagerClass GrlSceneManagerClass;

/*
 * =============================================================================
 * Interface Types
 * =============================================================================
 */

/**
 * GrlDrawable:
 *
 * Interface for objects that can be drawn.
 */
typedef struct _GrlDrawable GrlDrawable;
typedef struct _GrlDrawableInterface GrlDrawableInterface;

/**
 * GrlUpdatable:
 *
 * Interface for objects that update each frame.
 */
typedef struct _GrlUpdatable GrlUpdatable;
typedef struct _GrlUpdatableInterface GrlUpdatableInterface;

/**
 * GrlCollidable:
 *
 * Interface for objects that participate in collision detection.
 */
typedef struct _GrlCollidable GrlCollidable;
typedef struct _GrlCollidableInterface GrlCollidableInterface;

G_END_DECLS
