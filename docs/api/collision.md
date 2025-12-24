# Collision Detection

Graylib provides collision detection functions for both 2D and 3D shapes. These are utility functions (not GObject types) that return boolean results or collision information.

## 2D Point Tests

Check if a point is inside various 2D shapes.

### grl_collision_point_rect

```c
gboolean grl_collision_point_rect (const GrlVector2   *point,
                                   const GrlRectangle *rect);
```

Checks if a point is inside a rectangle.

### grl_collision_point_circle

```c
gboolean grl_collision_point_circle (const GrlVector2 *point,
                                     const GrlVector2 *center,
                                     gfloat            radius);
```

Checks if a point is inside a circle.

### grl_collision_point_triangle

```c
gboolean grl_collision_point_triangle (const GrlVector2 *point,
                                       const GrlVector2 *p1,
                                       const GrlVector2 *p2,
                                       const GrlVector2 *p3);
```

Checks if a point is inside a triangle defined by three vertices.

### grl_collision_point_poly

```c
gboolean grl_collision_point_poly (const GrlVector2 *point,
                                   const GrlVector2 *points,
                                   gint              point_count);
```

Checks if a point is inside a polygon defined by an array of vertices.

### grl_collision_point_line

```c
gboolean grl_collision_point_line (const GrlVector2 *point,
                                   const GrlVector2 *p1,
                                   const GrlVector2 *p2,
                                   gint              threshold);
```

Checks if a point is on a line segment within a threshold distance.

## 2D Shape vs Shape

Check collisions between 2D shapes.

### grl_collision_rects

```c
gboolean grl_collision_rects (const GrlRectangle *rect1,
                              const GrlRectangle *rect2);
```

Checks if two rectangles overlap.

### grl_collision_circles

```c
gboolean grl_collision_circles (const GrlVector2 *center1,
                                gfloat            radius1,
                                const GrlVector2 *center2,
                                gfloat            radius2);
```

Checks if two circles overlap.

### grl_collision_circle_rect

```c
gboolean grl_collision_circle_rect (const GrlVector2   *center,
                                    gfloat              radius,
                                    const GrlRectangle *rect);
```

Checks if a circle and rectangle overlap.

### grl_collision_circle_line

```c
gboolean grl_collision_circle_line (const GrlVector2 *center,
                                    gfloat            radius,
                                    const GrlVector2 *p1,
                                    const GrlVector2 *p2);
```

Checks if a circle intersects a line segment.

### grl_collision_lines

```c
gboolean grl_collision_lines (const GrlVector2 *start1,
                              const GrlVector2 *end1,
                              const GrlVector2 *start2,
                              const GrlVector2 *end2,
                              GrlVector2       *collision_point);
```

Checks if two line segments intersect. Optionally returns the intersection point.

### grl_collision_get_rect_overlap

```c
GrlRectangle *grl_collision_get_rect_overlap (const GrlRectangle *rect1,
                                              const GrlRectangle *rect2);
```

Returns the overlapping area of two rectangles, or `NULL` if they don't overlap.

## 3D Point Tests

Check if a point is inside 3D shapes.

### grl_collision_point_sphere

```c
gboolean grl_collision_point_sphere (const GrlVector3 *point,
                                     const GrlVector3 *center,
                                     gfloat            radius);
```

Checks if a point is inside a sphere.

### grl_collision_point_box

```c
gboolean grl_collision_point_box (const GrlVector3     *point,
                                  const GrlBoundingBox *box);
```

Checks if a point is inside an axis-aligned bounding box.

## 3D Shape vs Shape

Check collisions between 3D shapes.

### grl_collision_spheres

```c
gboolean grl_collision_spheres (const GrlVector3 *center1,
                                gfloat            radius1,
                                const GrlVector3 *center2,
                                gfloat            radius2);
```

Checks if two spheres overlap.

### grl_collision_boxes

```c
gboolean grl_collision_boxes (const GrlBoundingBox *box1,
                              const GrlBoundingBox *box2);
```

Checks if two axis-aligned bounding boxes overlap.

### grl_collision_box_sphere

```c
gboolean grl_collision_box_sphere (const GrlBoundingBox *box,
                                   const GrlVector3     *center,
                                   gfloat                radius);
```

Checks if a bounding box and sphere overlap.

## Ray Collision

For 3D picking and raycasting, Graylib provides ray collision detection.

### GrlRay

A 3D ray defined by an origin and direction.

```c
struct _GrlRay
{
    GrlVector3 position;   /* Ray origin */
    GrlVector3 direction;  /* Ray direction (normalized) */
};
```

**Functions:**

| Function | Description |
|----------|-------------|
| `grl_ray_new(position, direction)` | Create a new ray |
| `grl_ray_copy(ray)` | Copy a ray |
| `grl_ray_free(ray)` | Free a ray |

### GrlRayCollision

Information about a ray collision result.

```c
struct _GrlRayCollision
{
    gboolean   hit;       /* Whether the ray hit */
    gfloat     distance;  /* Distance from ray origin to hit point */
    GrlVector3 point;     /* Hit point position */
    GrlVector3 normal;    /* Surface normal at hit point */
};
```

### grl_collision_ray_sphere

```c
GrlRayCollision *grl_collision_ray_sphere (const GrlRay     *ray,
                                           const GrlVector3 *center,
                                           gfloat            radius);
```

Tests a ray against a sphere.

### grl_collision_ray_box

```c
GrlRayCollision *grl_collision_ray_box (const GrlRay         *ray,
                                        const GrlBoundingBox *box);
```

Tests a ray against an axis-aligned bounding box.

### grl_collision_ray_triangle

```c
GrlRayCollision *grl_collision_ray_triangle (const GrlRay     *ray,
                                             const GrlVector3 *p1,
                                             const GrlVector3 *p2,
                                             const GrlVector3 *p3);
```

Tests a ray against a triangle.

### grl_collision_ray_quad

```c
GrlRayCollision *grl_collision_ray_quad (const GrlRay     *ray,
                                         const GrlVector3 *p1,
                                         const GrlVector3 *p2,
                                         const GrlVector3 *p3,
                                         const GrlVector3 *p4);
```

Tests a ray against a quad (four vertices).

### grl_collision_ray_plane

```c
GrlRayCollision *grl_collision_ray_plane (const GrlRay     *ray,
                                          const GrlVector3 *plane_normal,
                                          gfloat            plane_distance);
```

Tests a ray against an infinite plane. The plane is defined by `dot(normal, point) = distance`.

## Screen to World

### grl_collision_get_ray_from_screen

```c
GrlRay *grl_collision_get_ray_from_screen (const GrlVector2 *screen_pos,
                                           const GrlVector3 *camera_position,
                                           const GrlVector3 *camera_target,
                                           const GrlVector3 *camera_up,
                                           gfloat            fovy,
                                           gint              screen_width,
                                           gint              screen_height);
```

Creates a ray from a 2D screen position (e.g., mouse coordinates) into 3D world space. Useful for mouse picking in 3D scenes.

## Example: 2D Collision

```c
g_autoptr(GrlVector2) player_pos = grl_vector2_new (100.0f, 100.0f);
g_autoptr(GrlRectangle) wall = grl_rectangle_new (50.0f, 50.0f, 100.0f, 100.0f);

if (grl_collision_point_rect (player_pos, wall))
{
    g_print ("Player is inside the wall!\n");
}
```

## Example: 3D Ray Picking

```c
g_autoptr(GrlVector2) mouse = grl_vector2_new (mouse_x, mouse_y);
g_autoptr(GrlVector3) cam_pos = grl_camera3d_get_position (camera);
g_autoptr(GrlVector3) cam_target = grl_camera3d_get_target (camera);
g_autoptr(GrlVector3) cam_up = grl_camera3d_get_up (camera);
gfloat fovy = grl_camera3d_get_fovy (camera);

g_autoptr(GrlRay) ray = grl_collision_get_ray_from_screen (
    mouse, cam_pos, cam_target, cam_up, fovy, 800, 600);

g_autoptr(GrlRayCollision) hit = grl_collision_ray_sphere (
    ray, sphere_center, sphere_radius);

if (hit->hit)
{
    g_print ("Hit sphere at distance %.2f\n", hit->distance);
}
```
