/* voxelcraft3d.c
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Minecraft-style voxel game demonstrating chunk-based world,
 * first-person controls, block placement/destruction, and inventory.
 */

#include <graylib.h>
#include <stdlib.h>
#include <math.h>

/* Window dimensions */
#define WINDOW_WIDTH      1280
#define WINDOW_HEIGHT     900

/* World settings */
#define CHUNK_SIZE        16
#define CHUNK_COUNT_X     3
#define CHUNK_COUNT_Z     3
#define WORLD_HEIGHT      CHUNK_SIZE

/* Player settings */
#define PLAYER_HEIGHT     1.7f
#define PLAYER_EYE_OFFSET 1.6f
#define PLAYER_RADIUS     0.3f
#define MOVE_SPEED        5.0f
#define SPRINT_MULTIPLIER 1.5f
#define MOUSE_SENSITIVITY 0.003f
#define GRAVITY           20.0f
#define JUMP_VELOCITY     8.0f
#define REACH_DISTANCE    5.0f

/* Inventory settings */
#define INVENTORY_SLOTS   6

/* Block types available in the world */
typedef enum
{
    BLOCK_AIR = 0,
    BLOCK_GRASS,
    BLOCK_DIRT,
    BLOCK_STONE,
    BLOCK_WOOD,
    BLOCK_BRICK,
    BLOCK_COUNT
} BlockType;

/* A chunk is a 16x16x16 volume of blocks */
typedef struct
{
    BlockType blocks[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE];  /* [y][z][x] */
    gint      origin_x;
    gint      origin_z;
} Chunk;

/* The voxel world composed of chunks */
typedef struct
{
    Chunk chunks[CHUNK_COUNT_Z][CHUNK_COUNT_X];
} VoxelWorld;

/* Player state */
typedef struct
{
    gfloat   pos_x;
    gfloat   pos_y;
    gfloat   pos_z;
    gfloat   vel_y;
    gfloat   yaw;
    gfloat   pitch;
    gboolean on_ground;
} Player;

/* Simple inventory system */
typedef struct
{
    BlockType slots[INVENTORY_SLOTS];
    gint      selected;
} Inventory;

/* Result of block selection raycast */
typedef struct
{
    gboolean hit;
    gint     block_x;
    gint     block_y;
    gint     block_z;
    gint     face_x;
    gint     face_y;
    gint     face_z;
} BlockHit;

/* Main game state */
typedef struct
{
    VoxelWorld world;
    Player     player;
    Inventory  inventory;
    BlockHit   target_block;
    gboolean   cursor_locked;
    gboolean   show_debug;
} VoxelGame;

/* Forward declarations */
static gboolean world_is_solid (VoxelWorld *world, gint x, gint y, gint z);

/*
 * Simple hash function for deterministic terrain height.
 */
static gint
terrain_height_at (gint world_x,
                   gint world_z)
{
    gint hash;
    gint base_height;
    gint variation;

    /* Simple hash-based pseudo-random height */
    hash = world_x * 374761393 + world_z * 668265263;
    hash = (hash ^ (hash >> 13)) * 1274126177;
    hash = hash ^ (hash >> 16);

    base_height = 6;
    variation = (hash & 0x7) - 3;  /* -3 to +4 */

    return base_height + variation;
}

/*
 * Generate a single chunk with terrain.
 */
static void
chunk_generate (Chunk *chunk,
                gint   chunk_x,
                gint   chunk_z)
{
    gint x, y, z;
    gint world_x, world_z;
    gint height;

    chunk->origin_x = chunk_x * CHUNK_SIZE;
    chunk->origin_z = chunk_z * CHUNK_SIZE;

    for (z = 0; z < CHUNK_SIZE; z++)
    {
        for (x = 0; x < CHUNK_SIZE; x++)
        {
            world_x = chunk->origin_x + x;
            world_z = chunk->origin_z + z;
            height = terrain_height_at (world_x, world_z);

            /* Clamp height to chunk bounds */
            if (height >= CHUNK_SIZE)
                height = CHUNK_SIZE - 1;
            if (height < 1)
                height = 1;

            for (y = 0; y < CHUNK_SIZE; y++)
            {
                if (y < height - 3)
                    chunk->blocks[y][z][x] = BLOCK_STONE;
                else if (y < height)
                    chunk->blocks[y][z][x] = BLOCK_DIRT;
                else if (y == height)
                    chunk->blocks[y][z][x] = BLOCK_GRASS;
                else
                    chunk->blocks[y][z][x] = BLOCK_AIR;
            }
        }
    }
}

/*
 * Initialize all chunks in the world.
 */
static void
world_init (VoxelWorld *world)
{
    gint cx, cz;

    for (cz = 0; cz < CHUNK_COUNT_Z; cz++)
    {
        for (cx = 0; cx < CHUNK_COUNT_X; cx++)
        {
            chunk_generate (&world->chunks[cz][cx], cx, cz);
        }
    }
}

/*
 * Get block at world coordinates.
 * Returns BLOCK_AIR if out of bounds.
 */
static BlockType
world_get_block (VoxelWorld *world,
                 gint        x,
                 gint        y,
                 gint        z)
{
    gint chunk_x, chunk_z;
    gint local_x, local_z;
    Chunk *chunk;

    /* Check vertical bounds */
    if (y < 0 || y >= CHUNK_SIZE)
        return BLOCK_AIR;

    /* Check horizontal bounds */
    if (x < 0 || x >= CHUNK_COUNT_X * CHUNK_SIZE)
        return BLOCK_AIR;
    if (z < 0 || z >= CHUNK_COUNT_Z * CHUNK_SIZE)
        return BLOCK_AIR;

    chunk_x = x / CHUNK_SIZE;
    chunk_z = z / CHUNK_SIZE;
    local_x = x % CHUNK_SIZE;
    local_z = z % CHUNK_SIZE;

    chunk = &world->chunks[chunk_z][chunk_x];
    return chunk->blocks[y][local_z][local_x];
}

/*
 * Set block at world coordinates.
 */
static void
world_set_block (VoxelWorld *world,
                 gint        x,
                 gint        y,
                 gint        z,
                 BlockType   type)
{
    gint chunk_x, chunk_z;
    gint local_x, local_z;
    Chunk *chunk;

    /* Check vertical bounds */
    if (y < 0 || y >= CHUNK_SIZE)
        return;

    /* Check horizontal bounds */
    if (x < 0 || x >= CHUNK_COUNT_X * CHUNK_SIZE)
        return;
    if (z < 0 || z >= CHUNK_COUNT_Z * CHUNK_SIZE)
        return;

    chunk_x = x / CHUNK_SIZE;
    chunk_z = z / CHUNK_SIZE;
    local_x = x % CHUNK_SIZE;
    local_z = z % CHUNK_SIZE;

    chunk = &world->chunks[chunk_z][chunk_x];
    chunk->blocks[y][local_z][local_x] = type;
}

/*
 * Check if a block position is solid (not air).
 */
static gboolean
world_is_solid (VoxelWorld *world,
                gint        x,
                gint        y,
                gint        z)
{
    return world_get_block (world, x, y, z) != BLOCK_AIR;
}

/*
 * Initialize inventory with default block types.
 */
static void
inventory_init (Inventory *inv)
{
    inv->slots[0] = BLOCK_GRASS;
    inv->slots[1] = BLOCK_DIRT;
    inv->slots[2] = BLOCK_STONE;
    inv->slots[3] = BLOCK_WOOD;
    inv->slots[4] = BLOCK_BRICK;
    inv->slots[5] = BLOCK_GRASS;  /* Duplicate for variety */
    inv->selected = 0;
}

/*
 * Initialize player at spawn position.
 */
static void
player_init (Player     *player,
             VoxelWorld *world)
{
    gint spawn_x, spawn_z;
    gint spawn_height;

    /* Spawn in center of world */
    spawn_x = (CHUNK_COUNT_X * CHUNK_SIZE) / 2;
    spawn_z = (CHUNK_COUNT_Z * CHUNK_SIZE) / 2;

    /* Find ground level */
    spawn_height = terrain_height_at (spawn_x, spawn_z) + 2;

    player->pos_x = (gfloat)spawn_x + 0.5f;
    player->pos_y = (gfloat)spawn_height;
    player->pos_z = (gfloat)spawn_z + 0.5f;
    player->vel_y = 0.0f;
    player->yaw = 0.0f;
    player->pitch = 0.0f;
    player->on_ground = FALSE;
}

/*
 * Initialize the entire game state.
 */
static void
game_init (VoxelGame *game)
{
    world_init (&game->world);
    player_init (&game->player, &game->world);
    inventory_init (&game->inventory);

    game->target_block.hit = FALSE;
    game->cursor_locked = TRUE;
    game->show_debug = FALSE;
}

/*
 * Update camera angles from mouse delta.
 */
static void
player_update_camera (VoxelGame *game)
{
    g_autoptr(GrlVector2) delta = NULL;

    if (!game->cursor_locked)
        return;

    delta = grl_input_get_mouse_delta ();
    if (delta == NULL)
        return;

    game->player.yaw -= delta->x * MOUSE_SENSITIVITY;
    game->player.pitch -= delta->y * MOUSE_SENSITIVITY;

    /* Clamp pitch to prevent camera flip */
    if (game->player.pitch > G_PI / 2.0f - 0.1f)
        game->player.pitch = G_PI / 2.0f - 0.1f;
    if (game->player.pitch < -G_PI / 2.0f + 0.1f)
        game->player.pitch = -G_PI / 2.0f + 0.1f;
}

/*
 * Check collision of player against world blocks.
 * Returns TRUE if position would collide.
 */
static gboolean
player_check_collision (VoxelGame *game,
                        gfloat     x,
                        gfloat     y,
                        gfloat     z)
{
    gint bx, by, bz;
    gint min_x, max_x, min_z, max_z;
    gint min_y, max_y;

    /* Player bounding box */
    min_x = (gint)floorf (x - PLAYER_RADIUS);
    max_x = (gint)floorf (x + PLAYER_RADIUS);
    min_z = (gint)floorf (z - PLAYER_RADIUS);
    max_z = (gint)floorf (z + PLAYER_RADIUS);
    min_y = (gint)floorf (y);
    max_y = (gint)floorf (y + PLAYER_HEIGHT);

    for (by = min_y; by <= max_y; by++)
    {
        for (bz = min_z; bz <= max_z; bz++)
        {
            for (bx = min_x; bx <= max_x; bx++)
            {
                if (world_is_solid (&game->world, bx, by, bz))
                    return TRUE;
            }
        }
    }

    return FALSE;
}

/*
 * Update player movement from keyboard input.
 */
static void
player_update_movement (VoxelGame *game,
                        gfloat     dt)
{
    gfloat forward_x, forward_z;
    gfloat right_x, right_z;
    gfloat speed;
    gfloat move_x, move_z;
    gfloat new_x, new_z;
    gfloat len;

    if (!game->cursor_locked)
        return;

    /* Calculate direction vectors from yaw (horizontal only) */
    forward_x = sinf (game->player.yaw);
    forward_z = cosf (game->player.yaw);
    right_x = -cosf (game->player.yaw);
    right_z = sinf (game->player.yaw);

    /* Check sprint */
    speed = MOVE_SPEED;
    if (grl_input_is_key_down (GRL_KEY_LEFT_SHIFT))
        speed *= SPRINT_MULTIPLIER;

    move_x = 0.0f;
    move_z = 0.0f;

    /* WASD movement */
    if (grl_input_is_key_down (GRL_KEY_W))
    {
        move_x += forward_x;
        move_z += forward_z;
    }
    if (grl_input_is_key_down (GRL_KEY_S))
    {
        move_x -= forward_x;
        move_z -= forward_z;
    }
    if (grl_input_is_key_down (GRL_KEY_A))
    {
        move_x -= right_x;
        move_z -= right_z;
    }
    if (grl_input_is_key_down (GRL_KEY_D))
    {
        move_x += right_x;
        move_z += right_z;
    }

    /* Normalize diagonal movement */
    if (move_x != 0.0f || move_z != 0.0f)
    {
        len = sqrtf (move_x * move_x + move_z * move_z);
        move_x /= len;
        move_z /= len;

        /* Try X movement */
        new_x = game->player.pos_x + move_x * speed * dt;
        if (!player_check_collision (game, new_x, game->player.pos_y, game->player.pos_z))
            game->player.pos_x = new_x;

        /* Try Z movement */
        new_z = game->player.pos_z + move_z * speed * dt;
        if (!player_check_collision (game, game->player.pos_x, game->player.pos_y, new_z))
            game->player.pos_z = new_z;
    }
}

/*
 * Apply gravity and ground collision.
 */
static void
player_update_physics (VoxelGame *game,
                       gfloat     dt)
{
    gfloat new_y;

    /* Jump (Space or Enter) */
    if (game->cursor_locked &&
        game->player.on_ground &&
        (grl_input_is_key_pressed (GRL_KEY_SPACE) ||
         grl_input_is_key_pressed (GRL_KEY_ENTER)))
    {
        game->player.vel_y = JUMP_VELOCITY;
        game->player.on_ground = FALSE;
    }

    /* Apply gravity */
    game->player.vel_y -= GRAVITY * dt;

    /* Apply vertical velocity */
    new_y = game->player.pos_y + game->player.vel_y * dt;

    /* Check collision */
    if (player_check_collision (game, game->player.pos_x, new_y, game->player.pos_z))
    {
        if (game->player.vel_y < 0)
        {
            /* Hit ground */
            game->player.on_ground = TRUE;
        }
        game->player.vel_y = 0.0f;
    }
    else
    {
        game->player.pos_y = new_y;
        game->player.on_ground = FALSE;
    }

    /* Prevent falling through world */
    if (game->player.pos_y < 1.0f)
    {
        game->player.pos_y = 1.0f;
        game->player.vel_y = 0.0f;
        game->player.on_ground = TRUE;
    }
}

/*
 * DDA raycasting algorithm for voxel block selection.
 */
static BlockHit
raycast_block (VoxelGame *game)
{
    BlockHit result;
    gfloat   ray_x, ray_y, ray_z;
    gfloat   dir_x, dir_y, dir_z;
    gint     step_x, step_y, step_z;
    gint     block_x, block_y, block_z;
    gfloat   t_max_x, t_max_y, t_max_z;
    gfloat   t_delta_x, t_delta_y, t_delta_z;
    gint     last_step_x, last_step_y, last_step_z;
    gfloat   t;
    gint     iterations;

    result.hit = FALSE;

    /* Calculate ray origin (player eye position) */
    ray_x = game->player.pos_x;
    ray_y = game->player.pos_y + PLAYER_EYE_OFFSET;
    ray_z = game->player.pos_z;

    /* Calculate ray direction from yaw and pitch */
    dir_x = cosf (game->player.pitch) * sinf (game->player.yaw);
    dir_y = sinf (game->player.pitch);
    dir_z = cosf (game->player.pitch) * cosf (game->player.yaw);

    /* Starting block */
    block_x = (gint)floorf (ray_x);
    block_y = (gint)floorf (ray_y);
    block_z = (gint)floorf (ray_z);

    /* Calculate step direction and initial t values for X */
    if (dir_x > 0.0001f)
    {
        step_x = 1;
        t_max_x = ((gfloat)(block_x + 1) - ray_x) / dir_x;
        t_delta_x = 1.0f / dir_x;
    }
    else if (dir_x < -0.0001f)
    {
        step_x = -1;
        t_max_x = (ray_x - (gfloat)block_x) / (-dir_x);
        t_delta_x = 1.0f / (-dir_x);
    }
    else
    {
        step_x = 0;
        t_max_x = 1000000.0f;
        t_delta_x = 1000000.0f;
    }

    /* Calculate step direction and initial t values for Y */
    if (dir_y > 0.0001f)
    {
        step_y = 1;
        t_max_y = ((gfloat)(block_y + 1) - ray_y) / dir_y;
        t_delta_y = 1.0f / dir_y;
    }
    else if (dir_y < -0.0001f)
    {
        step_y = -1;
        t_max_y = (ray_y - (gfloat)block_y) / (-dir_y);
        t_delta_y = 1.0f / (-dir_y);
    }
    else
    {
        step_y = 0;
        t_max_y = 1000000.0f;
        t_delta_y = 1000000.0f;
    }

    /* Calculate step direction and initial t values for Z */
    if (dir_z > 0.0001f)
    {
        step_z = 1;
        t_max_z = ((gfloat)(block_z + 1) - ray_z) / dir_z;
        t_delta_z = 1.0f / dir_z;
    }
    else if (dir_z < -0.0001f)
    {
        step_z = -1;
        t_max_z = (ray_z - (gfloat)block_z) / (-dir_z);
        t_delta_z = 1.0f / (-dir_z);
    }
    else
    {
        step_z = 0;
        t_max_z = 1000000.0f;
        t_delta_z = 1000000.0f;
    }

    last_step_x = 0;
    last_step_y = 0;
    last_step_z = 0;
    t = 0.0f;

    /* Step through the grid */
    for (iterations = 0; iterations < 100; iterations++)
    {
        /* Check current block */
        if (world_is_solid (&game->world, block_x, block_y, block_z))
        {
            result.hit = TRUE;
            result.block_x = block_x;
            result.block_y = block_y;
            result.block_z = block_z;
            result.face_x = block_x - last_step_x;
            result.face_y = block_y - last_step_y;
            result.face_z = block_z - last_step_z;
            return result;
        }

        /* Step to next cell (choose axis with smallest t_max) */
        if (t_max_x < t_max_y && t_max_x < t_max_z)
        {
            t = t_max_x;
            if (t > REACH_DISTANCE)
                break;
            block_x += step_x;
            t_max_x += t_delta_x;
            last_step_x = step_x;
            last_step_y = 0;
            last_step_z = 0;
        }
        else if (t_max_y < t_max_z)
        {
            t = t_max_y;
            if (t > REACH_DISTANCE)
                break;
            block_y += step_y;
            t_max_y += t_delta_y;
            last_step_x = 0;
            last_step_y = step_y;
            last_step_z = 0;
        }
        else
        {
            t = t_max_z;
            if (t > REACH_DISTANCE)
                break;
            block_z += step_z;
            t_max_z += t_delta_z;
            last_step_x = 0;
            last_step_y = 0;
            last_step_z = step_z;
        }
    }

    return result;
}

/*
 * Destroy the currently targeted block.
 */
static void
destroy_block (VoxelGame *game)
{
    if (!game->target_block.hit)
        return;

    world_set_block (&game->world,
                     game->target_block.block_x,
                     game->target_block.block_y,
                     game->target_block.block_z,
                     BLOCK_AIR);
}

/*
 * Place a block adjacent to the targeted block.
 */
static void
place_block (VoxelGame *game)
{
    gint place_x, place_y, place_z;
    BlockType block_type;

    if (!game->target_block.hit)
        return;

    place_x = game->target_block.face_x;
    place_y = game->target_block.face_y;
    place_z = game->target_block.face_z;

    /* Don't place if it would intersect the player */
    if (player_check_collision (game, game->player.pos_x, game->player.pos_y, game->player.pos_z))
    {
        /* Already colliding, allow placement */
    }

    /* Check if placement position would collide with player */
    if (place_x == (gint)floorf (game->player.pos_x) &&
        place_z == (gint)floorf (game->player.pos_z) &&
        place_y >= (gint)floorf (game->player.pos_y) &&
        place_y <= (gint)floorf (game->player.pos_y + PLAYER_HEIGHT))
    {
        return;  /* Don't place inside player */
    }

    block_type = game->inventory.slots[game->inventory.selected];
    world_set_block (&game->world, place_x, place_y, place_z, block_type);
}

/*
 * Handle mouse input for block interaction.
 */
static void
handle_block_interaction (VoxelGame *game)
{
    if (!game->cursor_locked)
        return;

    /* Left click: destroy */
    if (grl_input_is_mouse_button_pressed (GRL_MOUSE_BUTTON_LEFT))
    {
        destroy_block (game);
    }

    /* Right click: place */
    if (grl_input_is_mouse_button_pressed (GRL_MOUSE_BUTTON_RIGHT))
    {
        place_block (game);
    }
}

/*
 * Handle number key input for slot selection.
 */
static void
inventory_handle_input (VoxelGame *game)
{
    if (grl_input_is_key_pressed (GRL_KEY_ONE))
        game->inventory.selected = 0;
    if (grl_input_is_key_pressed (GRL_KEY_TWO))
        game->inventory.selected = 1;
    if (grl_input_is_key_pressed (GRL_KEY_THREE))
        game->inventory.selected = 2;
    if (grl_input_is_key_pressed (GRL_KEY_FOUR))
        game->inventory.selected = 3;
    if (grl_input_is_key_pressed (GRL_KEY_FIVE))
        game->inventory.selected = 4;
    if (grl_input_is_key_pressed (GRL_KEY_SIX))
        game->inventory.selected = 5;

    /* Mouse wheel scrolling */
    {
        gfloat wheel;

        wheel = grl_input_get_mouse_wheel_move ();
        if (wheel > 0.0f)
        {
            game->inventory.selected--;
            if (game->inventory.selected < 0)
                game->inventory.selected = INVENTORY_SLOTS - 1;
        }
        else if (wheel < 0.0f)
        {
            game->inventory.selected++;
            if (game->inventory.selected >= INVENTORY_SLOTS)
                game->inventory.selected = 0;
        }
    }
}

/*
 * Get the color for a given block type.
 */
static GrlColor *
block_get_color (BlockType type)
{
    switch (type)
    {
        case BLOCK_GRASS:
            return grl_color_new (95, 159, 53, 255);
        case BLOCK_DIRT:
            return grl_color_new (134, 96, 67, 255);
        case BLOCK_STONE:
            return grl_color_new (128, 128, 128, 255);
        case BLOCK_WOOD:
            return grl_color_new (133, 94, 66, 255);
        case BLOCK_BRICK:
            return grl_color_new (156, 74, 67, 255);
        default:
            return grl_color_new (255, 0, 255, 255);
    }
}

/*
 * Get block name for display.
 */
static const gchar *
block_get_name (BlockType type)
{
    switch (type)
    {
        case BLOCK_GRASS:
            return "Grass";
        case BLOCK_DIRT:
            return "Dirt";
        case BLOCK_STONE:
            return "Stone";
        case BLOCK_WOOD:
            return "Wood";
        case BLOCK_BRICK:
            return "Brick";
        default:
            return "???";
    }
}

/*
 * Check if a block has any exposed faces.
 */
static gboolean
has_exposed_face (VoxelGame *game,
                  gint       x,
                  gint       y,
                  gint       z)
{
    return !world_is_solid (&game->world, x - 1, y, z) ||
           !world_is_solid (&game->world, x + 1, y, z) ||
           !world_is_solid (&game->world, x, y - 1, z) ||
           !world_is_solid (&game->world, x, y + 1, z) ||
           !world_is_solid (&game->world, x, y, z - 1) ||
           !world_is_solid (&game->world, x, y, z + 1);
}

/*
 * Draw the entire world (all visible blocks).
 */
static void
draw_world (VoxelGame   *game,
            GrlCamera3D *camera)
{
    gint cx, cz;
    gint x, y, z;
    gint world_x, world_z;
    BlockType block;
    Chunk *chunk;
    gfloat wx, wy, wz;

    grl_camera3d_begin (camera);

    for (cz = 0; cz < CHUNK_COUNT_Z; cz++)
    {
        for (cx = 0; cx < CHUNK_COUNT_X; cx++)
        {
            chunk = &game->world.chunks[cz][cx];

            for (y = 0; y < CHUNK_SIZE; y++)
            {
                for (z = 0; z < CHUNK_SIZE; z++)
                {
                    for (x = 0; x < CHUNK_SIZE; x++)
                    {
                        g_autoptr(GrlVector3) pos = NULL;
                        g_autoptr(GrlColor) color = NULL;

                        block = chunk->blocks[y][z][x];
                        if (block == BLOCK_AIR)
                            continue;

                        world_x = chunk->origin_x + x;
                        world_z = chunk->origin_z + z;

                        /* Check if block has any exposed faces */
                        if (!has_exposed_face (game, world_x, y, world_z))
                            continue;

                        wx = (gfloat)world_x + 0.5f;
                        wy = (gfloat)y + 0.5f;
                        wz = (gfloat)world_z + 0.5f;

                        pos = grl_vector3_new (wx, wy, wz);
                        color = block_get_color (block);
                        grl_draw_cube (pos, 1.0f, 1.0f, 1.0f, color);
                    }
                }
            }
        }
    }

    /* Draw block highlight */
    if (game->target_block.hit)
    {
        g_autoptr(GrlVector3) highlight_pos = NULL;
        g_autoptr(GrlColor) highlight_color = NULL;

        highlight_pos = grl_vector3_new (
            (gfloat)game->target_block.block_x + 0.5f,
            (gfloat)game->target_block.block_y + 0.5f,
            (gfloat)game->target_block.block_z + 0.5f
        );
        highlight_color = grl_color_new (255, 255, 255, 255);
        grl_draw_cube_wires (highlight_pos, 1.02f, 1.02f, 1.02f, highlight_color);
    }

    grl_camera3d_end (camera);
}

/*
 * Draw crosshair in center of screen.
 */
static void
draw_crosshair (void)
{
    g_autoptr(GrlColor) color = grl_color_new (255, 255, 255, 200);
    gint cx, cy;

    cx = WINDOW_WIDTH / 2;
    cy = WINDOW_HEIGHT / 2;

    grl_draw_line (cx - 10, cy, cx + 10, cy, color);
    grl_draw_line (cx, cy - 10, cx, cy + 10, color);
}

/*
 * Draw inventory hotbar at bottom of screen.
 */
static void
draw_hotbar (VoxelGame *game)
{
    g_autoptr(GrlColor) bg_color = NULL;
    g_autoptr(GrlColor) selected_color = NULL;
    g_autoptr(GrlColor) text_color = NULL;
    gint slot_size;
    gint hotbar_width;
    gint start_x, start_y;
    gint i;
    gchar text[32];

    slot_size = 50;
    hotbar_width = INVENTORY_SLOTS * slot_size + (INVENTORY_SLOTS - 1) * 5;
    start_x = (WINDOW_WIDTH - hotbar_width) / 2;
    start_y = WINDOW_HEIGHT - slot_size - 20;

    bg_color = grl_color_new (60, 60, 60, 200);
    selected_color = grl_color_new (255, 255, 255, 255);
    text_color = grl_color_new (255, 255, 255, 255);

    for (i = 0; i < INVENTORY_SLOTS; i++)
    {
        g_autoptr(GrlColor) block_color = NULL;
        gint slot_x;

        slot_x = start_x + i * (slot_size + 5);

        /* Draw slot background */
        grl_draw_rectangle (slot_x, start_y, slot_size, slot_size, bg_color);

        /* Draw selection border */
        if (i == game->inventory.selected)
        {
            grl_draw_rectangle_lines (slot_x, start_y, slot_size, slot_size, selected_color);
        }

        /* Draw block preview */
        block_color = block_get_color (game->inventory.slots[i]);
        grl_draw_rectangle (slot_x + 5, start_y + 5, slot_size - 10, slot_size - 10, block_color);

        /* Draw slot number */
        g_snprintf (text, sizeof (text), "%d", i + 1);
        grl_draw_text (text, slot_x + 3, start_y + 2, 12, text_color);
    }

    /* Draw selected block name */
    g_snprintf (text, sizeof (text), "%s",
                block_get_name (game->inventory.slots[game->inventory.selected]));
    grl_draw_text (text, start_x, start_y - 25, 20, text_color);
}

/*
 * Draw debug information.
 */
static void
draw_debug (VoxelGame *game)
{
    g_autoptr(GrlColor) text_color = NULL;
    gchar text[128];
    gint y;

    if (!game->show_debug)
        return;

    text_color = grl_color_new (255, 255, 255, 255);
    y = 10;

    g_snprintf (text, sizeof (text), "FPS: %d", grl_window_get_fps (NULL));
    grl_draw_text (text, 10, y, 16, text_color);
    y += 20;

    g_snprintf (text, sizeof (text), "Pos: %.1f, %.1f, %.1f",
                game->player.pos_x, game->player.pos_y, game->player.pos_z);
    grl_draw_text (text, 10, y, 16, text_color);
    y += 20;

    g_snprintf (text, sizeof (text), "Yaw: %.2f  Pitch: %.2f",
                game->player.yaw, game->player.pitch);
    grl_draw_text (text, 10, y, 16, text_color);
    y += 20;

    if (game->target_block.hit)
    {
        g_snprintf (text, sizeof (text), "Target: %d, %d, %d",
                    game->target_block.block_x,
                    game->target_block.block_y,
                    game->target_block.block_z);
        grl_draw_text (text, 10, y, 16, text_color);
        y += 20;
    }

    g_snprintf (text, sizeof (text), "On Ground: %s",
                game->player.on_ground ? "Yes" : "No");
    grl_draw_text (text, 10, y, 16, text_color);
}

/*
 * Draw controls help.
 */
static void
draw_controls (VoxelGame *game)
{
    g_autoptr(GrlColor) gray = NULL;
    g_autoptr(GrlColor) white = NULL;

    gray = grl_color_new (150, 150, 150, 255);
    white = grl_color_new (255, 255, 255, 255);

    grl_draw_text ("WASD: Move  Space/Enter: Jump  Mouse: Look  LMB: Break  RMB: Place  1-6: Select  F3: Debug",
                   10, WINDOW_HEIGHT - 20, 14, gray);

    if (!game->cursor_locked)
    {
        grl_draw_text ("CURSOR UNLOCKED - Click to lock",
                       WINDOW_WIDTH / 2 - 120, 50, 18, white);
    }
}

/*
 * Apply camera state to GrlCamera3D.
 */
static void
apply_camera_state (VoxelGame   *game,
                    GrlCamera3D *camera)
{
    gfloat target_x, target_y, target_z;
    gfloat eye_y;

    eye_y = game->player.pos_y + PLAYER_EYE_OFFSET;

    /* Calculate target from spherical coordinates */
    target_x = game->player.pos_x + cosf (game->player.pitch) * sinf (game->player.yaw);
    target_y = eye_y + sinf (game->player.pitch);
    target_z = game->player.pos_z + cosf (game->player.pitch) * cosf (game->player.yaw);

    grl_camera3d_set_position_xyz (camera, game->player.pos_x, eye_y, game->player.pos_z);
    grl_camera3d_set_target_xyz (camera, target_x, target_y, target_z);
}

int
main (int    argc,
      char **argv)
{
    g_autoptr(GrlApplication) app = NULL;
    g_autoptr(GrlWindow) window = NULL;
    g_autoptr(GrlCamera3D) camera = NULL;
    g_autoptr(GrlColor) bg_color = NULL;
    VoxelGame game;
    gfloat dt;

    app = grl_application_new ("com.example.voxelcraft3d");
    window = grl_window_new (WINDOW_WIDTH, WINDOW_HEIGHT, "Voxelcraft 3D");
    bg_color = grl_color_new (135, 200, 255, 255);  /* Sky blue */

    /* Setup first-person camera */
    camera = grl_camera3d_new ();
    grl_camera3d_set_fovy (camera, 70.0f);

    grl_window_set_target_fps (window, 60);

    game_init (&game);

    /* Lock cursor on start */
    grl_window_disable_cursor (window);

    while (!grl_window_should_close (window))
    {
        dt = grl_window_get_frame_time (window);

        grl_window_poll_input (window);

        /* Handle cursor lock toggle (ESC) */
        if (grl_input_is_key_pressed (GRL_KEY_ESCAPE))
        {
            game.cursor_locked = !game.cursor_locked;
            if (game.cursor_locked)
                grl_window_disable_cursor (window);
            else
                grl_window_enable_cursor (window);
        }

        /* Toggle debug display (F3) */
        if (grl_input_is_key_pressed (GRL_KEY_F3))
            game.show_debug = !game.show_debug;

        /* Click to re-lock cursor */
        if (!game.cursor_locked &&
            grl_input_is_mouse_button_pressed (GRL_MOUSE_BUTTON_LEFT))
        {
            game.cursor_locked = TRUE;
            grl_window_disable_cursor (window);
        }

        /* Update player */
        player_update_camera (&game);
        player_update_movement (&game, dt);
        player_update_physics (&game, dt);

        /* Update block targeting */
        game.target_block = raycast_block (&game);

        /* Handle inventory selection */
        inventory_handle_input (&game);

        /* Handle block interaction */
        handle_block_interaction (&game);

        /* Apply camera to GrlCamera3D */
        apply_camera_state (&game, camera);

        /* Draw */
        grl_window_begin_drawing (window);
        grl_draw_clear_background (bg_color);

        draw_world (&game, camera);

        draw_crosshair ();
        draw_hotbar (&game);
        draw_debug (&game);
        draw_controls (&game);

        grl_window_end_drawing (window);
    }

    return 0;
}
