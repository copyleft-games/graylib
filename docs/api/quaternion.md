# Quaternions

This document covers the GrlQuaternion type for representing 3D rotations and orientations.

## Overview

Quaternions are a mathematical representation of 3D rotations that avoid gimbal lock and provide smooth interpolation. They are particularly useful for camera control, character animation, and any situation requiring smooth rotation transitions.

## GrlQuaternion

A quaternion consists of four components: x, y, z (imaginary/vector part) and w (real/scalar part).

### Structure

```c
struct _GrlQuaternion
{
    gfloat x;  /* Imaginary i component */
    gfloat y;  /* Imaginary j component */
    gfloat z;  /* Imaginary k component */
    gfloat w;  /* Real/scalar component */
};
```

## Creating Quaternions

### Identity Quaternion

The identity quaternion represents no rotation:

```c
g_autoptr(GrlQuaternion) identity = grl_quaternion_new_identity ();
/* Creates (0, 0, 0, 1) */
```

### From Components

```c
g_autoptr(GrlQuaternion) q = grl_quaternion_new (0.0f, 0.0f, 0.0f, 1.0f);
```

### From Axis-Angle

Create a rotation around an axis:

```c
g_autoptr(GrlVector3) axis = grl_vector3_new (0.0f, 1.0f, 0.0f);  /* Y axis */
gfloat angle = G_PI / 4.0f;  /* 45 degrees in radians */

g_autoptr(GrlQuaternion) rotation = grl_quaternion_new_from_axis_angle (axis, angle);
```

### From Euler Angles

Create from pitch, yaw, and roll:

```c
gfloat pitch = 0.0f;           /* Rotation around X */
gfloat yaw = G_PI / 2.0f;      /* Rotation around Y (90 degrees) */
gfloat roll = 0.0f;            /* Rotation around Z */

g_autoptr(GrlQuaternion) rotation = grl_quaternion_new_from_euler (pitch, yaw, roll);
```

### From Rotation Matrix

```c
g_autoptr(GrlMatrix) rotation_matrix = grl_matrix_new_rotate_y (G_PI / 4.0f);
g_autoptr(GrlQuaternion) q = grl_quaternion_new_from_matrix (rotation_matrix);
```

### From Two Vectors

Create a quaternion that rotates one vector to another:

```c
g_autoptr(GrlVector3) from = grl_vector3_new (1.0f, 0.0f, 0.0f);  /* X axis */
g_autoptr(GrlVector3) to = grl_vector3_new (0.0f, 1.0f, 0.0f);    /* Y axis */

g_autoptr(GrlQuaternion) rotation = grl_quaternion_new_from_vectors (from, to);
/* Rotation that turns X into Y (90 degrees around Z) */
```

## Arithmetic Operations

### Addition and Subtraction

```c
g_autoptr(GrlQuaternion) sum = grl_quaternion_add (q1, q2);
g_autoptr(GrlQuaternion) diff = grl_quaternion_subtract (q1, q2);

/* Add/subtract a scalar to all components */
g_autoptr(GrlQuaternion) shifted = grl_quaternion_add_value (q, 0.5f);
```

### Multiplication (Combining Rotations)

Quaternion multiplication combines rotations. **Order matters!**

```c
/* Apply rotation2 AFTER rotation1 */
g_autoptr(GrlQuaternion) combined = grl_quaternion_multiply (rotation1, rotation2);
```

### Scaling

```c
g_autoptr(GrlQuaternion) scaled = grl_quaternion_scale (q, 2.0f);
```

### Division

```c
g_autoptr(GrlQuaternion) quotient = grl_quaternion_divide (q1, q2);
```

### Negation

```c
g_autoptr(GrlQuaternion) negated = grl_quaternion_negate (q);
```

## Quaternion Operations

### Length (Magnitude)

```c
gfloat len = grl_quaternion_length (q);
```

### Normalization

Unit quaternions are required for proper rotation representation:

```c
g_autoptr(GrlQuaternion) unit = grl_quaternion_normalize (q);
```

### Inverse

The inverse quaternion represents the opposite rotation:

```c
g_autoptr(GrlQuaternion) inverse = grl_quaternion_invert (q);

/* Multiplying by inverse gives identity */
g_autoptr(GrlQuaternion) identity = grl_quaternion_multiply (q, inverse);
```

## Interpolation

### Linear Interpolation (LERP)

Simple but doesn't preserve unit length:

```c
g_autoptr(GrlQuaternion) mid = grl_quaternion_lerp (start, end, 0.5f);
```

### Normalized LERP (NLERP)

Fast rotation interpolation:

```c
/* Interpolate 30% from start toward end */
g_autoptr(GrlQuaternion) result = grl_quaternion_nlerp (start, end, 0.3f);
```

### Spherical LERP (SLERP)

Constant angular velocity interpolation (best for animation):

```c
g_autoptr(GrlQuaternion) result = grl_quaternion_slerp (start, end, 0.5f);
```

### Cubic Hermite Spline

Smooth curve interpolation through control points:

```c
g_autoptr(GrlQuaternion) result = grl_quaternion_cubic_hermite_spline (
    q1, out_tangent1,
    q2, in_tangent2,
    t
);
```

## Conversions

### To Rotation Matrix

```c
g_autoptr(GrlMatrix) matrix = grl_quaternion_to_matrix (q);
```

### To Euler Angles

```c
gfloat pitch, yaw, roll;
grl_quaternion_to_euler (q, &pitch, &yaw, &roll);

g_print ("Pitch: %.2f, Yaw: %.2f, Roll: %.2f (radians)\n",
         pitch, yaw, roll);
```

### To Axis-Angle

```c
GrlVector3 axis;
gfloat angle;
grl_quaternion_to_axis_angle (q, &axis, &angle);

g_print ("Axis: (%.2f, %.2f, %.2f), Angle: %.2f radians\n",
         axis.x, axis.y, axis.z, angle);
```

## Transformation

Apply a matrix transformation to a quaternion:

```c
g_autoptr(GrlMatrix) transform = grl_matrix_new_identity ();
g_autoptr(GrlQuaternion) transformed = grl_quaternion_transform (q, transform);
```

## Comparison

Check if two quaternions are approximately equal:

```c
if (grl_quaternion_equal (q1, q2))
{
    g_print ("Quaternions represent the same rotation\n");
}
```

## NLERP vs SLERP

| Feature | NLERP | SLERP |
|---------|-------|-------|
| Speed | Faster | Slower |
| Accuracy | Good for small angles | Perfect for all angles |
| Angular velocity | Variable | Constant |
| Use case | Real-time games | Cinematics, animations |

**Recommendation:** Use NLERP for real-time interpolation (camera smoothing, input response). Use SLERP for pre-recorded animations where constant speed matters.

## Complete Example

```c
#include <graylib.h>

int
main (int argc, char *argv[])
{
    g_autoptr(GrlWindow) window = NULL;
    g_autoptr(GrlCamera3D) camera = NULL;
    g_autoptr(GrlColor) bg = NULL;
    g_autoptr(GrlQuaternion) start_rot = NULL;
    g_autoptr(GrlQuaternion) end_rot = NULL;
    gfloat t = 0.0f;
    gfloat direction = 1.0f;

    window = grl_window_new (800, 600, "Quaternion Demo");
    grl_window_set_target_fps (window, 60);

    camera = grl_camera3d_new_default ();
    bg = grl_color_new (40, 40, 60, 255);

    /* Create two rotations to interpolate between */
    start_rot = grl_quaternion_new_from_euler (0.0f, 0.0f, 0.0f);
    end_rot = grl_quaternion_new_from_euler (0.0f, G_PI, G_PI / 4.0f);

    while (!grl_window_should_close (window))
    {
        gfloat delta = grl_window_get_frame_time (window);
        g_autoptr(GrlQuaternion) current_rot = NULL;
        g_autoptr(GrlMatrix) rotation_matrix = NULL;
        g_autoptr(GrlVector3) position = NULL;

        /* Animate interpolation parameter */
        t += delta * direction * 0.5f;
        if (t >= 1.0f) { t = 1.0f; direction = -1.0f; }
        if (t <= 0.0f) { t = 0.0f; direction = 1.0f; }

        /* Interpolate rotation using SLERP */
        current_rot = grl_quaternion_slerp (start_rot, end_rot, t);
        rotation_matrix = grl_quaternion_to_matrix (current_rot);

        grl_window_begin_drawing (window);
        grl_window_clear_background (window, bg);

        grl_camera3d_begin_mode (camera);

        /* Draw a rotating cube using the quaternion rotation */
        position = grl_vector3_new (0.0f, 0.0f, 0.0f);
        /* Apply rotation_matrix to cube here... */

        grl_camera3d_end_mode (camera);

        grl_draw_fps (10, 10);
        grl_window_end_drawing (window);
    }

    return 0;
}
```

## Function Reference

### Constructors

| Function | Description |
|----------|-------------|
| `grl_quaternion_new()` | Create from components |
| `grl_quaternion_new_identity()` | Create identity (no rotation) |
| `grl_quaternion_new_from_axis_angle()` | Create from axis and angle |
| `grl_quaternion_new_from_euler()` | Create from pitch/yaw/roll |
| `grl_quaternion_new_from_matrix()` | Create from rotation matrix |
| `grl_quaternion_new_from_vectors()` | Create rotation between vectors |
| `grl_quaternion_copy()` | Copy a quaternion |
| `grl_quaternion_free()` | Free a quaternion |

### Arithmetic

| Function | Description |
|----------|-------------|
| `grl_quaternion_add()` | Add two quaternions |
| `grl_quaternion_add_value()` | Add scalar to all components |
| `grl_quaternion_subtract()` | Subtract quaternions |
| `grl_quaternion_subtract_value()` | Subtract scalar from components |
| `grl_quaternion_multiply()` | Multiply (combine rotations) |
| `grl_quaternion_scale()` | Scale by scalar |
| `grl_quaternion_divide()` | Divide quaternions |
| `grl_quaternion_negate()` | Negate all components |

### Operations

| Function | Description |
|----------|-------------|
| `grl_quaternion_length()` | Get magnitude |
| `grl_quaternion_normalize()` | Normalize to unit length |
| `grl_quaternion_invert()` | Get inverse rotation |

### Interpolation

| Function | Description |
|----------|-------------|
| `grl_quaternion_lerp()` | Linear interpolation |
| `grl_quaternion_nlerp()` | Normalized linear interpolation |
| `grl_quaternion_slerp()` | Spherical linear interpolation |
| `grl_quaternion_cubic_hermite_spline()` | Cubic spline interpolation |

### Conversions

| Function | Description |
|----------|-------------|
| `grl_quaternion_to_matrix()` | Convert to rotation matrix |
| `grl_quaternion_to_euler()` | Convert to Euler angles |
| `grl_quaternion_to_axis_angle()` | Convert to axis-angle |
| `grl_quaternion_transform()` | Transform by matrix |

### Comparison

| Function | Description |
|----------|-------------|
| `grl_quaternion_equal()` | Check approximate equality |
