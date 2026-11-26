/* grl-native-window.c
 *
 * Copyright 2026 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * raylib's Android platform sizes everything once, when the window first
 * appears (InitWindow()), and ignores every later change of the native
 * window: its EGL buffers keep the first size and the compositor scales
 * them, so a foldable opened (or a phone put into split screen) shows the
 * old picture stretched.  raylib has no setter for that state on Android
 * (SetWindowSize() is a no-op there), and raylib is never modified, so
 * this file reaches the state the way the platform files do: through
 * raylib's global CORE, with a mirror of its CoreData layout.
 *
 * The mirror is checked before every use (grl_native_window_can_resize()):
 * sentinel values written through it must read back through raylib's own
 * getters, from the first section of the struct to the last.  A raylib
 * whose layout moved fails the check, and nothing is written.
 * tests/test-native-window.c runs the check, and
 * tests/test-native-window-layout.sh compares the mirror with the pinned
 * raylib's rcore.c, so a raylib bump that changes the struct fails
 * `make test` before it can fail on a device.
 */

#include "config.h"
#include "grl-native-window.h"
#include "grl-native-window-private.h"

#include <raylib.h>
#include <rlgl.h>

/* raymath's, compiled into raylib (graylib's math does the same) */
extern Matrix MatrixIdentity (void);

/* ------------------------------------------------------------------------
 * The mirror of raylib 6.2's CoreData (src/rcore.c), with the array sizes
 * raylib builds with (rcore.c's defaults; graylib's raylib config does
 * not change them).  Field names and order are raylib's.
 * ------------------------------------------------------------------------ */

#define GRL_CORE_MAX_KEYBOARD_KEYS        (512)
#define GRL_CORE_MAX_MOUSE_BUTTONS        (8)
#define GRL_CORE_MAX_GAMEPADS             (4)
#define GRL_CORE_MAX_GAMEPAD_NAME_LENGTH  (128)
#define GRL_CORE_MAX_GAMEPAD_AXES         (8)
#define GRL_CORE_MAX_GAMEPAD_BUTTONS      (32)
#define GRL_CORE_MAX_TOUCH_POINTS         (8)
#define GRL_CORE_MAX_KEY_PRESSED_QUEUE    (16)
#define GRL_CORE_MAX_CHAR_PRESSED_QUEUE   (16)

typedef struct { int x; int y; } GrlCorePoint;
typedef struct { unsigned int width; unsigned int height; } GrlCoreSize;

typedef struct {
    struct {
        const char *title;
        unsigned int flags;
        bool ready;
        bool shouldClose;
        bool resizedLastFrame;
        bool eventWaiting;
        bool usingFbo;

        GrlCoreSize display;
        GrlCoreSize screen;
        GrlCorePoint position;
        GrlCoreSize previousScreen;
        GrlCorePoint previousPosition;
        GrlCoreSize render;
        GrlCorePoint renderOffset;
        GrlCoreSize currentFbo;
        GrlCoreSize screenMin;
        GrlCoreSize screenMax;
        Matrix screenScale;

        char **dropFilepaths;
        unsigned int dropFileCount;
    } Window;
    struct {
        const char *basePath;
    } Storage;
    struct {
        struct {
            int exitKey;
            char currentKeyState[GRL_CORE_MAX_KEYBOARD_KEYS];
            char previousKeyState[GRL_CORE_MAX_KEYBOARD_KEYS];
            char keyRepeatInFrame[GRL_CORE_MAX_KEYBOARD_KEYS];
            int keyPressedQueue[GRL_CORE_MAX_KEY_PRESSED_QUEUE];
            int keyPressedQueueCount;
            int charPressedQueue[GRL_CORE_MAX_CHAR_PRESSED_QUEUE];
            int charPressedQueueCount;
        } Keyboard;
        struct {
            Vector2 offset;
            Vector2 scale;
            Vector2 currentPosition;
            Vector2 previousPosition;
            Vector2 lockedPosition;
            int cursor;
            bool cursorHidden;
            bool cursorLocked;
            bool cursorOnScreen;
            char currentButtonState[GRL_CORE_MAX_MOUSE_BUTTONS];
            char previousButtonState[GRL_CORE_MAX_MOUSE_BUTTONS];
            Vector2 currentWheelMove;
            Vector2 previousWheelMove;
        } Mouse;
        struct {
            int pointCount;
            int pointId[GRL_CORE_MAX_TOUCH_POINTS];
            Vector2 position[GRL_CORE_MAX_TOUCH_POINTS];
            Vector2 previousPosition[GRL_CORE_MAX_TOUCH_POINTS];
            char currentTouchState[GRL_CORE_MAX_TOUCH_POINTS];
            char previousTouchState[GRL_CORE_MAX_TOUCH_POINTS];
        } Touch;
        struct {
            int lastButtonPressed;
            int axisCount[GRL_CORE_MAX_GAMEPADS];
            bool ready[GRL_CORE_MAX_GAMEPADS];
            char name[GRL_CORE_MAX_GAMEPADS][GRL_CORE_MAX_GAMEPAD_NAME_LENGTH];
            char currentButtonState[GRL_CORE_MAX_GAMEPADS][GRL_CORE_MAX_GAMEPAD_BUTTONS];
            char previousButtonState[GRL_CORE_MAX_GAMEPADS][GRL_CORE_MAX_GAMEPAD_BUTTONS];
            float axisState[GRL_CORE_MAX_GAMEPADS][GRL_CORE_MAX_GAMEPAD_AXES];
        } Gamepad;
    } Input;
    struct {
        double current;
        double previous;
        double update;
        double draw;
        double frame;
        double target;
        unsigned long long base;
        unsigned int frameCounter;
    } Time;
} GrlCoreData;

/* raylib's global state (rcore.c), with external linkage for the
 * platform files */
extern GrlCoreData CORE;

/* a resize set raylib's resized flag, for grl_native_window_end_frame_private() to clear */
static gboolean resize_pending = FALSE;

/* ------------------------------------------------------------------------
 * The layout check
 * ------------------------------------------------------------------------ */

/* probe_layout: a sentinel through each probed field, read back through
 * raylib's own getter (or written by a raylib setter and read through the
 * mirror), then every old value restored.  The fields span the struct:
 * Window (the sizes, the flags), Input.Mouse (after the keyboard arrays),
 * Input.Gamepad (after the touch arrays) and Time, the last section. */
static gboolean
probe_layout (void)
{
    GrlCoreSize screen;
    GrlCoreSize render;
    bool resized;
    bool using_fbo;
    Vector2 scale;
    int last_button;
    double frame;
    unsigned char raw_resized;
    gboolean ok;

    screen = CORE.Window.screen;
    render = CORE.Window.render;
    resized = CORE.Window.resizedLastFrame;
    using_fbo = CORE.Window.usingFbo;
    scale = CORE.Input.Mouse.scale;
    last_button = CORE.Input.Gamepad.lastButtonPressed;
    frame = CORE.Time.frame;

    CORE.Window.screen.width = 6197u;
    CORE.Window.screen.height = 3511u;
    CORE.Window.render.width = 4093u;
    CORE.Window.render.height = 2861u;
    CORE.Window.resizedLastFrame = true;
    CORE.Window.usingFbo = false;
    CORE.Input.Gamepad.lastButtonPressed = 13;
    CORE.Time.frame = 0.0625;
    SetMouseScale (0.5f, 0.125f);

    raw_resized = IsWindowResized ();
    ok = GetScreenWidth () == 6197 && GetScreenHeight () == 3511 &&
         GetRenderWidth () == 4093 && GetRenderHeight () == 2861 &&
         raw_resized != 0 &&
         CORE.Input.Mouse.scale.x == 0.5f && CORE.Input.Mouse.scale.y == 0.125f &&
         GetGamepadButtonPressed () == 13 &&
         GetFrameTime () == 0.0625f;

    CORE.Window.screen = screen;
    CORE.Window.render = render;
    CORE.Window.resizedLastFrame = resized;
    CORE.Window.usingFbo = using_fbo;
    CORE.Input.Mouse.scale = scale;
    CORE.Input.Gamepad.lastButtonPressed = last_button;
    CORE.Time.frame = frame;
    return ok;
}

/**
 * grl_native_window_can_resize:
 *
 * Whether grl_native_window_resize() can update raylib on this build:
 * graylib's mirror of raylib's internal state matches the raylib it was
 * linked with.  Needs no window (the check writes sentinel values and
 * reads them back through raylib's getters, then restores them).
 *
 * Returns: %TRUE when the mirror matches
 */
gboolean
grl_native_window_can_resize (void)
{
    static gint cached = -1;

    if (cached < 0)
        cached = probe_layout () ? 1 : 0;
    return cached == 1;
}

/**
 * grl_native_window_resize:
 * @width: the native window's new width, in pixels
 * @height: its new height, in pixels
 *
 * Makes raylib draw to a native window that is now @width x @height, on
 * platforms where raylib does not follow the native window itself
 * (Android: unfolding a foldable, split screen, a freeform window).  The
 * screen, render, display and framebuffer sizes raylib reports become
 * @width x @height with no offset or scaling, the default viewport and
 * 2D projection are reset to it, and IsWindowResized()
 * (grl_window_is_resized()) is %TRUE for the next frame, as on the
 * desktop.  The caller has already resized the native buffers (on
 * Android, ANativeWindow_setBuffersGeometry() with the window's base
 * size); libregnum's Android platform layer does both.
 *
 * Call it between frames, with the window's context current and no
 * render texture active.  On the desktop raylib follows resizes itself;
 * there it is not needed.
 *
 * Returns: %TRUE when raylib was updated; %FALSE (nothing changed) when
 *   the window is not ready, a size is not positive, or
 *   grl_native_window_can_resize() is %FALSE
 */
gboolean
grl_native_window_resize (gint width,
                          gint height)
{
    unsigned char ready;

    ready = IsWindowReady ();
    if (!ready || width <= 0 || height <= 0)
        return FALSE;
    if (!grl_native_window_can_resize ())
    {
        g_warning ("graylib: this raylib's internal layout is not the one graylib "
                   "mirrors; the %dx%d window size change is not applied", width, height);
        return FALSE;
    }

    /* the sizes raylib keeps: the native window is the display, the
     * screen the game sees and the framebuffer, one to one */
    CORE.Window.display.width = (unsigned int) width;
    CORE.Window.display.height = (unsigned int) height;
    CORE.Window.screen.width = (unsigned int) width;
    CORE.Window.screen.height = (unsigned int) height;
    CORE.Window.currentFbo.width = (unsigned int) width;
    CORE.Window.currentFbo.height = (unsigned int) height;
    CORE.Window.renderOffset.x = 0;
    CORE.Window.renderOffset.y = 0;
    CORE.Window.screenScale = MatrixIdentity ();

    /* the render size, rlgl's framebuffer size, and the default viewport
     * and top-left-origin 2D projection: what raylib's (static)
     * SetupViewport() does at startup */
    CORE.Window.render.width = (unsigned int) width;
    CORE.Window.render.height = (unsigned int) height;
    rlSetFramebufferWidth (width);
    rlSetFramebufferHeight (height);
    rlViewport (0, 0, width, height);
    rlMatrixMode (RL_PROJECTION);
    rlLoadIdentity ();
    rlOrtho (0.0, (double) width, (double) height, 0.0, 0.0, 1.0);
    rlMatrixMode (RL_MODELVIEW);
    rlLoadIdentity ();

    CORE.Window.resizedLastFrame = true;
    resize_pending = TRUE;
    return TRUE;
}

/* grl_native_window_end_frame_private: documented in the private header.
 * A resize during one EndDrawing()'s event poll is seen through the next
 * frame and cleared before that frame's EndDrawing() (a resize between
 * frames: the same).  On the desktop raylib's own poll clears it. */
void
grl_native_window_end_frame_private (void)
{
    if (!resize_pending)
        return;
    resize_pending = FALSE;
#ifdef GRL_PLATFORM_ANDROID
    CORE.Window.resizedLastFrame = false;
#endif
}
