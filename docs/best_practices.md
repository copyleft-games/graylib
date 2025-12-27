# Best Practices

Common pitfalls and their solutions when working with graylib.

## 3D Rendering

### Always Clear the Background

**Problem:** Blocks/objects disappear, change colors based on camera angle, or show dithered/broken rendering.

**Cause:** The depth buffer is not being cleared each frame. Previous frame's depth values persist, causing incorrect depth comparisons.

**Solution:** Always call `grl_draw_clear_background()` at the start of your render loop. This function clears both the color buffer AND the depth buffer.

```c
static void
render_frame (Game *game)
{
    g_autoptr(GrlColor) bg_color = grl_color_new (135, 206, 235, 255);

    /* Clear color AND depth buffer first! */
    grl_draw_clear_background (bg_color);

    grl_draw_begin_mode_3d (game->camera);
    /* ... draw 3D content ... */
    grl_draw_end_mode_3d ();

    /* ... draw 2D UI ... */
}
```

### Avoid Z-Fighting Between Adjacent Surfaces

**Problem:** Flickering or dithering artifacts where two surfaces meet or overlap.

**Cause:** Z-fighting occurs when two surfaces occupy the exact same depth position. The depth buffer cannot reliably determine which surface is in front.

**Solution:** Offset adjacent surfaces slightly. For voxel games, use cubes that are slightly smaller than the grid spacing:

```c
/* Grid cells are 1.0 apart, but cubes are 0.98 to prevent z-fighting */
grl_draw_cube (pos, 0.98f, 0.98f, 0.98f, color);
```

This creates a tiny gap (0.01 on each side) that prevents faces of adjacent blocks from occupying the same position.

## Color Operations

### Understanding `grl_color_brightness()`

**Problem:** Colors appear washed out / too bright when trying to darken them.

**Cause:** Misunderstanding the brightness factor parameter:
- **Positive factor** = interpolate toward WHITE (brightens)
- **Negative factor** = scale down (darkens)

```c
/* WRONG: This makes the color 80% toward white (too bright!) */
lit_color = grl_color_brightness (base_color, 0.8f);

/* CORRECT: This gives 80% of original brightness */
lit_color = grl_color_brightness (base_color, 0.8f - 1.0f);  /* factor = -0.2 */
```

**Factor interpretation:**
| Factor | Result |
|--------|--------|
| `1.0` | 100% white |
| `0.5` | 50% toward white |
| `0.0` | Original color |
| `-0.2` | 80% brightness |
| `-0.5` | 50% brightness |
| `-1.0` | Black |

**Solution:** When you want X% of original brightness, use `(X - 1.0f)` as the factor:

```c
gfloat brightness = 0.6f;  /* Want 60% brightness */
lit_color = grl_color_brightness (base_color, brightness - 1.0f);
```

## General Tips

### Triangle Winding Order

OpenGL expects counter-clockwise vertex order for front-facing triangles. If you're drawing custom geometry with `grl_draw_triangle_3d()`, ensure vertices are specified in counter-clockwise order when viewed from the front:

```c
/*
 * For a face viewed from the front:
 *
 *   v4 --- v3
 *   |      |
 *   v1 --- v2
 *
 * Counter-clockwise triangles:
 *   Triangle 1: v1 -> v2 -> v3
 *   Triangle 2: v1 -> v3 -> v4
 */
grl_draw_triangle_3d (v1, v2, v3, color);
grl_draw_triangle_3d (v1, v3, v4, color);
```

### Prefer Built-in Primitives

When possible, use graylib's built-in drawing functions (`grl_draw_cube()`, `grl_draw_sphere()`, etc.) rather than rolling your own triangle-based rendering. The built-in functions:

- Handle winding order correctly
- Are optimized for raylib's renderer
- Reduce code complexity and potential for bugs

## Memory Management

### GrlModel and GrlMesh Ownership

**Problem:** Double-free crash or use-after-free when using `grl_model_new_from_mesh()`.

**Cause:** raylib's `LoadModelFromMesh()` performs a shallow copy - the model's internal mesh shares pointers with the original `GrlMesh`. If the mesh is freed while the model still exists, the model has dangling pointers.

**Solution:** This is handled automatically. When you call `grl_model_new_from_mesh()`, the model takes a reference to the mesh and keeps it alive. You can safely let the mesh go out of scope:

```c
static GrlModel *
create_cube_model (void)
{
    g_autoptr(GrlMesh) mesh = grl_mesh_new_cube (1.0f, 1.0f, 1.0f);
    grl_mesh_upload (mesh, FALSE);

    /* Model refs the mesh internally - safe to return even though
     * mesh will be unreffed when this function returns */
    return grl_model_new_from_mesh (mesh);
}
```

**Note:** This only applies to `grl_model_new_from_mesh()`. Models loaded from files with `grl_model_new_from_file()` own their mesh data directly and don't have shared ownership concerns.
