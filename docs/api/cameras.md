# Cameras

Graylib provides 2D and 3D camera systems for controlling the view into your game world.

## GrlCamera2D

A 2D camera for scrolling, zooming, and rotating 2D scenes.

### Constructors

```c
GrlCamera2D * grl_camera2d_new (void);
```

### Properties

| Property | Type | Access | Default | Description |
|----------|------|--------|---------|-------------|
| `offset` | `GrlVector2` | Read/Write | (0, 0) | Camera offset from target (screen space) |
| `target` | `GrlVector2` | Read/Write | (0, 0) | Point the camera is looking at (world space) |
| `rotation` | `gfloat` | Read/Write | 0.0 | Camera rotation in degrees |
| `zoom` | `gfloat` | Read/Write | 1.0 | Camera zoom (scale factor) |

### Methods

```c
/* Property accessors */
void         grl_camera2d_set_offset   (GrlCamera2D      *camera,
                                        const GrlVector2 *offset);
GrlVector2 * grl_camera2d_get_offset   (GrlCamera2D *camera);

void         grl_camera2d_set_target   (GrlCamera2D      *camera,
                                        const GrlVector2 *target);
GrlVector2 * grl_camera2d_get_target   (GrlCamera2D *camera);

void         grl_camera2d_set_rotation (GrlCamera2D *camera,
                                        gfloat       rotation);
gfloat       grl_camera2d_get_rotation (GrlCamera2D *camera);

void         grl_camera2d_set_zoom     (GrlCamera2D *camera,
                                        gfloat       zoom);
gfloat       grl_camera2d_get_zoom     (GrlCamera2D *camera);

/* Camera mode */
void         grl_camera2d_begin        (GrlCamera2D *camera);
void         grl_camera2d_end          (GrlCamera2D *camera);

/* Coordinate conversion */
GrlVector2 * grl_camera2d_get_world_to_screen (GrlCamera2D      *camera,
                                               const GrlVector2 *world_pos);
GrlVector2 * grl_camera2d_get_screen_to_world (GrlCamera2D      *camera,
                                               const GrlVector2 *screen_pos);
```

### Usage

The camera transform is applied between `begin()` and `end()` calls:

```c
grl_camera2d_begin (camera);
/* All drawing here uses camera coordinates */
grl_draw_circle_v (world_position, 50.0f, red);
grl_camera2d_end (camera);

/* Drawing here uses screen coordinates */
grl_draw_text ("HUD", 10, 10, 20, white);
```

### Example: Following a Player

```c
g_autoptr(GrlCamera2D) camera = grl_camera2d_new ();
g_autoptr(GrlVector2) offset = grl_vector2_new (400.0f, 300.0f);  /* Screen center */

/* Center camera on screen */
grl_camera2d_set_offset (camera, offset);

/* In update loop - follow player */
grl_camera2d_set_target (camera, player_position);

/* Optional: smooth zoom with mouse wheel */
gfloat zoom = grl_camera2d_get_zoom (camera);
zoom += grl_input_get_mouse_wheel_move () * 0.1f;
if (zoom < 0.1f) zoom = 0.1f;
if (zoom > 3.0f) zoom = 3.0f;
grl_camera2d_set_zoom (camera, zoom);
```

### Example: Screen to World Picking

```c
/* Get mouse position in world coordinates */
g_autoptr(GrlVector2) mouse_screen = grl_vector2_new (
    grl_input_get_mouse_x (),
    grl_input_get_mouse_y ()
);
g_autoptr(GrlVector2) mouse_world = grl_camera2d_get_screen_to_world (
    camera, mouse_screen
);

g_print ("Mouse world position: %.1f, %.1f\n", mouse_world->x, mouse_world->y);
```

## GrlCamera3D

A 3D camera for perspective or orthographic rendering.

### Constructors

```c
GrlCamera3D * grl_camera3d_new (void);
```

### Properties

| Property | Type | Access | Default | Description |
|----------|------|--------|---------|-------------|
| `position` | `GrlVector3` | Read/Write | (0, 10, 10) | Camera position in world space |
| `target` | `GrlVector3` | Read/Write | (0, 0, 0) | Point the camera looks at |
| `up` | `GrlVector3` | Read/Write | (0, 1, 0) | Up direction vector |
| `fovy` | `gfloat` | Read/Write | 45.0 | Field of view (Y axis, in degrees) |
| `projection` | `GrlCameraProjection` | Read/Write | PERSPECTIVE | Projection mode |

### Camera Projection Modes

```c
typedef enum
{
    GRL_CAMERA_PROJECTION_PERSPECTIVE,   /* Perspective projection (3D look) */
    GRL_CAMERA_PROJECTION_ORTHOGRAPHIC   /* Orthographic projection (2D look) */
} GrlCameraProjection;
```

### Methods

```c
/* Property accessors */
void         grl_camera3d_set_position   (GrlCamera3D      *camera,
                                          const GrlVector3 *position);
GrlVector3 * grl_camera3d_get_position   (GrlCamera3D *camera);

void         grl_camera3d_set_target     (GrlCamera3D      *camera,
                                          const GrlVector3 *target);
GrlVector3 * grl_camera3d_get_target     (GrlCamera3D *camera);

void         grl_camera3d_set_up         (GrlCamera3D      *camera,
                                          const GrlVector3 *up);
GrlVector3 * grl_camera3d_get_up         (GrlCamera3D *camera);

void         grl_camera3d_set_fovy       (GrlCamera3D *camera,
                                          gfloat       fovy);
gfloat       grl_camera3d_get_fovy       (GrlCamera3D *camera);

void                  grl_camera3d_set_projection (GrlCamera3D         *camera,
                                                   GrlCameraProjection  projection);
GrlCameraProjection   grl_camera3d_get_projection (GrlCamera3D *camera);

/* Camera mode */
void         grl_camera3d_begin          (GrlCamera3D *camera);
void         grl_camera3d_end            (GrlCamera3D *camera);
```

### Usage

The 3D camera transform is applied between `begin()` and `end()` calls:

```c
grl_camera3d_begin (camera);
/* All 3D drawing here uses camera view */
grl_model_draw (model, position, 1.0f, white);
grl_draw_grid (10, 1.0f);
grl_camera3d_end (camera);

/* 2D drawing (HUD, UI) */
grl_draw_text ("Score: 100", 10, 10, 20, white);
```

### Example: Orbiting Camera

```c
g_autoptr(GrlCamera3D) camera = grl_camera3d_new ();
g_autoptr(GrlVector3) target = grl_vector3_new (0.0f, 0.0f, 0.0f);
gfloat angle = 0.0f;
gfloat distance = 10.0f;
gfloat height = 5.0f;

grl_camera3d_set_target (camera, target);
grl_camera3d_set_fovy (camera, 45.0f);

/* In update loop */
angle += 0.5f;  /* Rotate around target */

g_autoptr(GrlVector3) pos = grl_vector3_new (
    cosf (angle * DEG2RAD) * distance,
    height,
    sinf (angle * DEG2RAD) * distance
);
grl_camera3d_set_position (camera, pos);
```

### Example: First-Person Camera

```c
g_autoptr(GrlCamera3D) camera = grl_camera3d_new ();
g_autoptr(GrlVector3) position = grl_vector3_new (0.0f, 1.8f, 0.0f);  /* Eye height */
g_autoptr(GrlVector3) target = grl_vector3_new (0.0f, 1.8f, -1.0f);   /* Looking forward */

grl_camera3d_set_position (camera, position);
grl_camera3d_set_target (camera, target);
grl_camera3d_set_fovy (camera, 60.0f);  /* Wider FOV for first-person */

/* In update loop - handle mouse look */
gfloat yaw = 0.0f;
gfloat pitch = 0.0f;

if (grl_input_is_mouse_button_down (GRL_MOUSE_BUTTON_RIGHT))
{
    yaw -= grl_input_get_mouse_delta_x () * 0.1f;
    pitch -= grl_input_get_mouse_delta_y () * 0.1f;

    /* Clamp pitch to prevent flipping */
    if (pitch > 89.0f) pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;
}

/* Calculate new target from yaw/pitch */
g_autoptr(GrlVector3) new_target = grl_vector3_new (
    position->x + cosf (pitch * DEG2RAD) * sinf (yaw * DEG2RAD),
    position->y + sinf (pitch * DEG2RAD),
    position->z + cosf (pitch * DEG2RAD) * cosf (yaw * DEG2RAD)
);
grl_camera3d_set_target (camera, new_target);
```

### Example: Isometric Camera

```c
g_autoptr(GrlCamera3D) camera = grl_camera3d_new ();

/* Position camera at 45-degree angle */
g_autoptr(GrlVector3) position = grl_vector3_new (10.0f, 10.0f, 10.0f);
g_autoptr(GrlVector3) target = grl_vector3_new (0.0f, 0.0f, 0.0f);

grl_camera3d_set_position (camera, position);
grl_camera3d_set_target (camera, target);

/* Use orthographic projection for true isometric look */
grl_camera3d_set_projection (camera, GRL_CAMERA_PROJECTION_ORTHOGRAPHIC);
grl_camera3d_set_fovy (camera, 20.0f);  /* Controls zoom in ortho mode */
```

## Coordinate Systems

### 2D Coordinate System

- **Screen space**: Origin at top-left, Y increases downward
- **World space**: Defined by your game logic (typically origin at center or top-left)
- Use `get_screen_to_world()` and `get_world_to_screen()` to convert

### 3D Coordinate System

Graylib uses a right-handed coordinate system (same as raylib):

- **X axis**: Right
- **Y axis**: Up
- **Z axis**: Out of screen (toward viewer)

## Complete Example: 2D Camera Game

```c
#include <graylib.h>

int
main (int argc, char *argv[])
{
    g_autoptr(GrlWindow) window = NULL;
    g_autoptr(GrlCamera2D) camera = NULL;
    g_autoptr(GrlVector2) player_pos = NULL;
    g_autoptr(GrlVector2) offset = NULL;
    g_autoptr(GrlColor) bg = NULL;
    g_autoptr(GrlColor) red = NULL;
    g_autoptr(GrlColor) green = NULL;
    g_autoptr(GrlColor) white = NULL;
    gfloat speed = 200.0f;

    window = grl_window_new (800, 600, "2D Camera Demo");
    grl_window_set_target_fps (window, 60);

    /* Setup camera centered on screen */
    camera = grl_camera2d_new ();
    offset = grl_vector2_new (400.0f, 300.0f);
    grl_camera2d_set_offset (camera, offset);
    grl_camera2d_set_zoom (camera, 1.0f);

    /* Player starts at origin */
    player_pos = grl_vector2_new (0.0f, 0.0f);

    bg = grl_color_new (40, 40, 60, 255);
    red = grl_color_new_red ();
    green = grl_color_new_green ();
    white = grl_color_new_white ();

    while (!grl_window_should_close (window))
    {
        gfloat dt = grl_window_get_frame_time (window);

        /* Player movement */
        if (grl_input_is_key_down (GRL_KEY_W)) player_pos->y -= speed * dt;
        if (grl_input_is_key_down (GRL_KEY_S)) player_pos->y += speed * dt;
        if (grl_input_is_key_down (GRL_KEY_A)) player_pos->x -= speed * dt;
        if (grl_input_is_key_down (GRL_KEY_D)) player_pos->x += speed * dt;

        /* Camera follows player */
        grl_camera2d_set_target (camera, player_pos);

        /* Zoom with mouse wheel */
        gfloat zoom = grl_camera2d_get_zoom (camera);
        zoom += grl_input_get_mouse_wheel_move () * 0.1f;
        if (zoom < 0.25f) zoom = 0.25f;
        if (zoom > 4.0f) zoom = 4.0f;
        grl_camera2d_set_zoom (camera, zoom);

        grl_window_begin_drawing (window);
        grl_window_clear_background (window, bg);

        /* World space drawing */
        grl_camera2d_begin (camera);

        /* Draw grid */
        for (gint x = -500; x <= 500; x += 100)
        {
            for (gint y = -500; y <= 500; y += 100)
            {
                g_autoptr(GrlRectangle) rect = grl_rectangle_new (x, y, 90, 90);
                grl_draw_rectangle_rec (rect, green);
            }
        }

        /* Draw player */
        grl_draw_circle_v (player_pos, 25.0f, red);

        grl_camera2d_end (camera);

        /* HUD (screen space) */
        grl_draw_text ("WASD to move, Mouse wheel to zoom", 10, 10, 20, white);

        grl_window_end_drawing (window);
    }

    return 0;
}
```
