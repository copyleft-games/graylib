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
 * GrlQuaternion:
 *
 * A quaternion for representing 3D rotations and orientations.
 *
 * This is a GBoxed type.
 */
typedef struct _GrlQuaternion GrlQuaternion;

/**
 * GrlBoundingBox:
 *
 * A 3D axis-aligned bounding box defined by min and max corners.
 *
 * This is a GBoxed type.
 */
typedef struct _GrlBoundingBox GrlBoundingBox;

/**
 * GrlPngChunk:
 *
 * A PNG chunk containing metadata or image data.
 *
 * This is a GBoxed type.
 */
typedef struct _GrlPngChunk GrlPngChunk;

/**
 * GrlPngPalette:
 *
 * A color palette for indexed PNG images.
 *
 * This is a GBoxed type.
 */
typedef struct _GrlPngPalette GrlPngPalette;

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
 * GrlGifWriter:
 *
 * A streaming animated GIF writer.
 *
 * This is a final GObject type.
 */
typedef struct _GrlGifWriter GrlGifWriter;
/* Note: GrlGifWriterClass is defined by G_DECLARE_FINAL_TYPE in grl-gif-writer.h */

/**
 * GrlPath:
 *
 * A retained 2D vector path for building, transforming, filling, stroking, and
 * combining shapes. Paths are composed of one or more subpaths; each subpath
 * is a sequence of move-to, line-to, cubic, and quadratic bezier verbs that
 * are lazily flattened to polylines for rasterization.
 *
 * This is a final GObject type.
 */
typedef struct _GrlPath GrlPath;
/* Note: GrlPathClass is defined by G_DECLARE_FINAL_TYPE in grl-path.h */

/**
 * GrlImageAccumulator:
 *
 * A float-precision RGBA accumulation buffer for frame averaging.
 *
 * This is a final GObject type.
 */
typedef struct _GrlImageAccumulator GrlImageAccumulator;
/* Note: GrlImageAccumulatorClass is defined by G_DECLARE_FINAL_TYPE in grl-image-accumulator.h */

/**
 * GrlImageFont:
 *
 * A headless TTF/OTF font rasteriser backed by stb_truetype.
 *
 * This is a final GObject type.
 */
typedef struct _GrlImageFont GrlImageFont;
/* Note: GrlImageFontClass is defined by G_DECLARE_FINAL_TYPE in grl-image-font.h */

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
 * GrlWave:
 *
 * Raw audio wave data for manipulation and processing.
 *
 * This is a GBoxed type.
 */
typedef struct _GrlWave GrlWave;

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

/**
 * GrlAudioStream:
 *
 * Real-time audio stream for procedural audio generation.
 *
 * This is a final GObject type.
 */
typedef struct _GrlAudioStream GrlAudioStream;

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
 * UI Module
 */

/**
 * GrlUiControl:
 *
 * Base class for all UI controls.
 *
 * This is a derivable GObject type.
 */
typedef struct _GrlUiControl GrlUiControl;
typedef struct _GrlUiControlClass GrlUiControlClass;

/**
 * GrlUiLabel:
 *
 * A label control for displaying text.
 *
 * This is a final GObject type.
 */
typedef struct _GrlUiLabel GrlUiLabel;

/**
 * GrlUiButton:
 *
 * A button control that emits a clicked signal.
 *
 * This is a final GObject type.
 */
typedef struct _GrlUiButton GrlUiButton;

/**
 * GrlUiCheckBox:
 *
 * A checkbox control for boolean options.
 *
 * This is a final GObject type.
 */
typedef struct _GrlUiCheckBox GrlUiCheckBox;

/**
 * GrlUiSlider:
 *
 * A slider control for selecting values in a range.
 *
 * This is a final GObject type.
 */
typedef struct _GrlUiSlider GrlUiSlider;

/**
 * GrlUiTextBox:
 *
 * A textbox control for text input.
 *
 * This is a final GObject type.
 */
typedef struct _GrlUiTextBox GrlUiTextBox;

/**
 * GrlUiToggle:
 *
 * A toggle button control for on/off states.
 *
 * This is a final GObject type.
 */
typedef struct _GrlUiToggle GrlUiToggle;

/**
 * GrlUiProgressBar:
 *
 * A progress bar control for displaying progress.
 *
 * This is a final GObject type.
 */
typedef struct _GrlUiProgressBar GrlUiProgressBar;

/**
 * GrlUiSpinner:
 *
 * A spinner control for integer input with +/- buttons.
 *
 * This is a final GObject type.
 */
typedef struct _GrlUiSpinner GrlUiSpinner;

/**
 * GrlUiValueBox:
 *
 * A value box control for integer input.
 *
 * This is a final GObject type.
 */
typedef struct _GrlUiValueBox GrlUiValueBox;

/**
 * GrlUiComboBox:
 *
 * A combo box control for selecting from options.
 *
 * This is a final GObject type.
 */
typedef struct _GrlUiComboBox GrlUiComboBox;

/**
 * GrlUiDropdownBox:
 *
 * A dropdown box control for selecting from a dropdown list.
 *
 * This is a final GObject type.
 */
typedef struct _GrlUiDropdownBox GrlUiDropdownBox;

/**
 * GrlUiToggleGroup:
 *
 * A toggle group control for selecting one option from a group.
 *
 * This is a final GObject type.
 */
typedef struct _GrlUiToggleGroup GrlUiToggleGroup;

/**
 * GrlUiListView:
 *
 * A list view control for displaying and selecting from a scrollable list.
 *
 * This is a final GObject type.
 */
typedef struct _GrlUiListView GrlUiListView;

/**
 * GrlUiColorPicker:
 *
 * A color picker control for selecting colors with interactive UI.
 *
 * This is a final GObject type.
 */
typedef struct _GrlUiColorPicker GrlUiColorPicker;

/**
 * GrlUiPanel:
 *
 * A panel control for grouping controls with optional header.
 *
 * This is a final GObject type.
 */
typedef struct _GrlUiPanel GrlUiPanel;

/**
 * GrlUiGroupBox:
 *
 * A group box control for visually grouping related controls.
 *
 * This is a final GObject type.
 */
typedef struct _GrlUiGroupBox GrlUiGroupBox;

/**
 * GrlUiWindowBox:
 *
 * A closeable window container with title bar.
 *
 * This is a final GObject type.
 */
typedef struct _GrlUiWindowBox GrlUiWindowBox;

/*
 * Resources Module
 */

/**
 * GrlResourceChunkInfo:
 *
 * Information about a resource chunk in an rres file.
 *
 * This is a GBoxed type.
 */
typedef struct _GrlResourceChunkInfo GrlResourceChunkInfo;

/**
 * GrlResourcePack:
 *
 * A resource pack for loading rres files.
 *
 * This is a final GObject type.
 */
typedef struct _GrlResourcePack GrlResourcePack;

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
