/* voxelcraft3d.c
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Feature-complete Minecraft-style voxel game demonstrating:
 * - 18 block types with properties (hardness, transparency, gravity)
 * - 5x5 chunk world (80x64x80 blocks) with caves, trees, ores, water
 * - Per-face lighting and distance fog
 * - Day/night cycle with sun/moon
 * - Health system with fall damage
 * - Block breaking progress based on hardness
 * - First-person controls with inventory
 */

#include <graylib.h>
#include <stdlib.h>
#include <math.h>

/* ============================================================================
 * CONSTANTS
 * ============================================================================ */

/* Window dimensions */
#define WINDOW_WIDTH      1280
#define WINDOW_HEIGHT     900

/* World settings */
#define CHUNK_SIZE_XZ     16
#define CHUNK_HEIGHT      64
#define CHUNK_COUNT_X     5
#define CHUNK_COUNT_Z     5
#define SEA_LEVEL         12
#define WORLD_SEED        12345

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

/* Health settings */
#define PLAYER_MAX_HEALTH      10
#define FALL_DAMAGE_THRESHOLD  4.0f
#define FALL_DAMAGE_PER_BLOCK  1

/* Inventory settings */
#define INVENTORY_SLOTS   9

/* Visual settings */
#define FOG_START         30.0f
#define FOG_END           50.0f
#define RENDER_DISTANCE   55.0f
#define DAY_LENGTH        600.0f

/* Face lighting multipliers */
#define FACE_LIGHT_TOP    1.0f
#define FACE_LIGHT_BOTTOM 0.5f
#define FACE_LIGHT_NORTH  0.8f
#define FACE_LIGHT_SOUTH  0.7f
#define FACE_LIGHT_EAST   0.75f
#define FACE_LIGHT_WEST   0.85f

/* Tree generation */
#define TREE_MIN_HEIGHT   5
#define TREE_MAX_HEIGHT   7
#define TREE_CHANCE       0.02f

/* ============================================================================
 * TYPES AND ENUMS
 * ============================================================================ */

/* Block types - 18 total */
typedef enum
{
    BLOCK_AIR = 0,
    /* Terrain */
    BLOCK_GRASS,
    BLOCK_DIRT,
    BLOCK_STONE,
    BLOCK_SAND,
    BLOCK_GRAVEL,
    BLOCK_SNOW,
    /* Wood & vegetation */
    BLOCK_WOOD,
    BLOCK_PLANKS,
    BLOCK_LEAVES,
    /* Building materials */
    BLOCK_COBBLESTONE,
    BLOCK_BRICK,
    BLOCK_GLASS,
    /* Ores */
    BLOCK_COAL_ORE,
    BLOCK_IRON_ORE,
    BLOCK_GOLD_ORE,
    BLOCK_DIAMOND_ORE,
    /* Special */
    BLOCK_WATER,
    BLOCK_BEDROCK,
    BLOCK_COUNT
} BlockType;

/* Block property flags */
typedef enum
{
    BLOCK_FLAG_NONE        = 0,
    BLOCK_FLAG_TRANSPARENT = (1 << 0),
    BLOCK_FLAG_LIQUID      = (1 << 1),
    BLOCK_FLAG_GRAVITY     = (1 << 2),
    BLOCK_FLAG_UNBREAKABLE = (1 << 3),
    BLOCK_FLAG_REPLACEABLE = (1 << 4)
} BlockFlags;

/* Game state */
typedef enum
{
    GAME_STATE_PLAYING,
    GAME_STATE_DEAD
} GameState;

/* Block properties */
typedef struct
{
    guint8     r, g, b;
    gfloat     hardness;
    BlockFlags flags;
    const gchar *name;
} BlockProperties;

/* Block properties table */
static const BlockProperties BLOCK_PROPS[BLOCK_COUNT] = {
    /* AIR */         {   0,   0,   0, 0.0f, BLOCK_FLAG_NONE,                                          "Air" },
    /* GRASS */       {  95, 159,  53, 0.6f, BLOCK_FLAG_NONE,                                          "Grass" },
    /* DIRT */        { 134,  96,  67, 0.5f, BLOCK_FLAG_NONE,                                          "Dirt" },
    /* STONE */       { 128, 128, 128, 1.5f, BLOCK_FLAG_NONE,                                          "Stone" },
    /* SAND */        { 219, 211, 160, 0.5f, BLOCK_FLAG_GRAVITY,                                       "Sand" },
    /* GRAVEL */      { 136, 126, 126, 0.6f, BLOCK_FLAG_GRAVITY,                                       "Gravel" },
    /* SNOW */        { 240, 240, 255, 0.2f, BLOCK_FLAG_NONE,                                          "Snow" },
    /* WOOD */        { 133,  94,  66, 2.0f, BLOCK_FLAG_NONE,                                          "Wood" },
    /* PLANKS */      { 180, 144,  90, 2.0f, BLOCK_FLAG_NONE,                                          "Planks" },
    /* LEAVES */      {  60, 140,  40, 0.2f, BLOCK_FLAG_TRANSPARENT,                                   "Leaves" },
    /* COBBLESTONE */ { 100, 100, 100, 2.0f, BLOCK_FLAG_NONE,                                          "Cobblestone" },
    /* BRICK */       { 156,  74,  67, 2.0f, BLOCK_FLAG_NONE,                                          "Brick" },
    /* GLASS */       { 200, 220, 255, 0.3f, BLOCK_FLAG_TRANSPARENT,                                   "Glass" },
    /* COAL_ORE */    {  70,  70,  70, 3.0f, BLOCK_FLAG_NONE,                                          "Coal Ore" },
    /* IRON_ORE */    { 170, 140, 120, 3.0f, BLOCK_FLAG_NONE,                                          "Iron Ore" },
    /* GOLD_ORE */    { 200, 180,  80, 3.0f, BLOCK_FLAG_NONE,                                          "Gold Ore" },
    /* DIAMOND_ORE */ { 100, 220, 230, 3.0f, BLOCK_FLAG_NONE,                                          "Diamond Ore" },
    /* WATER */       {  64, 164, 223, 0.0f, BLOCK_FLAG_TRANSPARENT | BLOCK_FLAG_LIQUID | BLOCK_FLAG_REPLACEABLE, "Water" },
    /* BEDROCK */     {  50,  50,  50, 0.0f, BLOCK_FLAG_UNBREAKABLE,                                   "Bedrock" }
};

/* Chunk structure */
typedef struct
{
    BlockType blocks[CHUNK_HEIGHT][CHUNK_SIZE_XZ][CHUNK_SIZE_XZ];
    gint      origin_x;
    gint      origin_z;
} Chunk;

/* World structure */
typedef struct
{
    Chunk chunks[CHUNK_COUNT_Z][CHUNK_COUNT_X];
} VoxelWorld;

/* Player structure */
typedef struct
{
    gfloat   pos_x, pos_y, pos_z;
    gfloat   vel_y;
    gfloat   yaw, pitch;
    gboolean on_ground;
    gboolean was_on_ground;
    gint     health;
    gfloat   fall_start_y;
} Player;

/* Inventory structure */
typedef struct
{
    BlockType slots[INVENTORY_SLOTS];
    gint      selected;
} Inventory;

/* Raycast hit result */
typedef struct
{
    gboolean hit;
    gint     block_x, block_y, block_z;
    gint     face_x, face_y, face_z;
} BlockHit;

/* Block breaking state */
typedef struct
{
    gboolean active;
    gint     target_x, target_y, target_z;
    gfloat   progress;
    gfloat   total_time;
} BreakingState;

/* Main game state */
typedef struct
{
    VoxelWorld     world;
    Player         player;
    Inventory      inventory;
    BlockHit       target_block;
    BreakingState  breaking;
    gfloat         world_time;
    gfloat         game_time;
    gboolean       cursor_locked;
    gboolean       show_debug;
    GameState      state;
} VoxelGame;

/* ============================================================================
 * FORWARD DECLARATIONS
 * ============================================================================ */

static gboolean world_is_solid (VoxelWorld *world, gint x, gint y, gint z);
static BlockType world_get_block (VoxelWorld *world, gint x, gint y, gint z);
static void world_set_block (VoxelWorld *world, gint x, gint y, gint z, BlockType type);

/* ============================================================================
 * NOISE FUNCTIONS
 * ============================================================================ */

static gfloat
noise2d (gint x, gint z, guint seed)
{
    gint n;

    n = x + z * 57 + seed * 131;
    n = (n << 13) ^ n;
    return 1.0f - (gfloat)((n * (n * n * 15731 + 789221) + 1376312589) & 0x7fffffff)
           / 1073741824.0f;
}

static gfloat
interpolated_noise (gfloat x, gfloat z, guint seed)
{
    gint   ix, iz;
    gfloat fx, fz;
    gfloat v1, v2, v3, v4;
    gfloat i1, i2;

    ix = (gint)floorf (x);
    iz = (gint)floorf (z);
    fx = x - (gfloat)ix;
    fz = z - (gfloat)iz;

    /* Smooth interpolation */
    fx = fx * fx * (3.0f - 2.0f * fx);
    fz = fz * fz * (3.0f - 2.0f * fz);

    v1 = noise2d (ix, iz, seed);
    v2 = noise2d (ix + 1, iz, seed);
    v3 = noise2d (ix, iz + 1, seed);
    v4 = noise2d (ix + 1, iz + 1, seed);

    i1 = v1 + fx * (v2 - v1);
    i2 = v3 + fx * (v4 - v3);

    return i1 + fz * (i2 - i1);
}

static gfloat
terrain_noise (gint x, gint z, guint seed)
{
    gfloat total, frequency, amplitude;
    gint   i;

    total = 0.0f;
    frequency = 0.015f;
    amplitude = 1.0f;

    for (i = 0; i < 4; i++)
    {
        total += interpolated_noise ((gfloat)x * frequency,
                                     (gfloat)z * frequency,
                                     seed + i) * amplitude;
        frequency *= 2.0f;
        amplitude *= 0.5f;
    }

    return total;
}

static gfloat
cave_noise_3d (gint x, gint y, gint z, guint seed)
{
    gfloat total;
    gint   n, i;

    total = 0.0f;

    for (i = 0; i < 2; i++)
    {
        n = x * 3 + y * 57 + z * 131 + seed + i * 1000;
        n = (n << 13) ^ n;
        total += (1.0f - (gfloat)((n * (n * n * 15731 + 789221) + 1376312589) & 0x7fffffff)
                 / 1073741824.0f) * (1.0f / (i + 1));
    }

    return total;
}

/* ============================================================================
 * WORLD GENERATION
 * ============================================================================ */

static gint
terrain_height_at (gint x, gint z)
{
    gfloat noise_val;
    gint   height;

    noise_val = terrain_noise (x, z, WORLD_SEED);
    height = 20 + (gint)(noise_val * 12.0f);

    if (height < 1)
        height = 1;
    if (height >= CHUNK_HEIGHT - 5)
        height = CHUNK_HEIGHT - 6;

    return height;
}

static void
place_tree (Chunk *chunk,
            gint   local_x,
            gint   ground_y,
            gint   local_z,
            guint  seed)
{
    gint trunk_height;
    gint y, lx, lz, ly;
    gint leaf_start;
    gint radius;

    trunk_height = TREE_MIN_HEIGHT + (seed % (TREE_MAX_HEIGHT - TREE_MIN_HEIGHT + 1));
    leaf_start = ground_y + trunk_height - 2;

    /* Place trunk */
    for (y = ground_y + 1; y <= ground_y + trunk_height && y < CHUNK_HEIGHT; y++)
    {
        chunk->blocks[y][local_z][local_x] = BLOCK_WOOD;
    }

    /* Place leaves */
    for (ly = leaf_start; ly <= ground_y + trunk_height + 1 && ly < CHUNK_HEIGHT; ly++)
    {
        radius = (ly == ground_y + trunk_height + 1) ? 1 : 2;

        for (lz = -radius; lz <= radius; lz++)
        {
            for (lx = -radius; lx <= radius; lx++)
            {
                gint tx, tz;

                if (lx * lx + lz * lz > radius * radius + 1)
                    continue;

                tx = local_x + lx;
                tz = local_z + lz;

                if (tx < 0 || tx >= CHUNK_SIZE_XZ ||
                    tz < 0 || tz >= CHUNK_SIZE_XZ)
                    continue;

                if (chunk->blocks[ly][tz][tx] == BLOCK_AIR)
                    chunk->blocks[ly][tz][tx] = BLOCK_LEAVES;
            }
        }
    }
}

static void
place_ore_vein (Chunk     *chunk,
                gint       start_x,
                gint       start_y,
                gint       start_z,
                BlockType  ore_type,
                gint       vein_size,
                guint      seed)
{
    gint i, x, y, z;
    gint dx, dy, dz;

    x = start_x;
    y = start_y;
    z = start_z;

    for (i = 0; i < vein_size; i++)
    {
        if (x >= 0 && x < CHUNK_SIZE_XZ &&
            z >= 0 && z < CHUNK_SIZE_XZ &&
            y >= 1 && y < CHUNK_HEIGHT &&
            chunk->blocks[y][z][x] == BLOCK_STONE)
        {
            chunk->blocks[y][z][x] = ore_type;
        }

        dx = ((seed + i * 3) % 3) - 1;
        dy = ((seed + i * 7) % 3) - 1;
        dz = ((seed + i * 11) % 3) - 1;

        x += dx;
        y += dy;
        z += dz;
    }
}

static void
chunk_generate (Chunk *chunk,
                gint   chunk_x,
                gint   chunk_z)
{
    gint x, y, z;
    gint world_x, world_z;
    gint height;
    guint ore_seed;

    chunk->origin_x = chunk_x * CHUNK_SIZE_XZ;
    chunk->origin_z = chunk_z * CHUNK_SIZE_XZ;

    /* Generate base terrain */
    for (z = 0; z < CHUNK_SIZE_XZ; z++)
    {
        for (x = 0; x < CHUNK_SIZE_XZ; x++)
        {
            world_x = chunk->origin_x + x;
            world_z = chunk->origin_z + z;
            height = terrain_height_at (world_x, world_z);

            for (y = 0; y < CHUNK_HEIGHT; y++)
            {
                if (y == 0)
                {
                    chunk->blocks[y][z][x] = BLOCK_BEDROCK;
                }
                else if (y < height - 4)
                {
                    chunk->blocks[y][z][x] = BLOCK_STONE;
                }
                else if (y < height)
                {
                    chunk->blocks[y][z][x] = BLOCK_DIRT;
                }
                else if (y == height)
                {
                    /* Beach sand near sea level */
                    if (height <= SEA_LEVEL + 2 && height >= SEA_LEVEL - 1)
                        chunk->blocks[y][z][x] = BLOCK_SAND;
                    else
                        chunk->blocks[y][z][x] = BLOCK_GRASS;
                }
                else if (y <= SEA_LEVEL)
                {
                    chunk->blocks[y][z][x] = BLOCK_WATER;
                }
                else
                {
                    chunk->blocks[y][z][x] = BLOCK_AIR;
                }
            }

            /* Also place sand below surface near water */
            if (height <= SEA_LEVEL + 2 && height >= SEA_LEVEL - 1)
            {
                for (y = height - 1; y >= height - 3 && y > 0; y--)
                {
                    if (chunk->blocks[y][z][x] == BLOCK_DIRT)
                        chunk->blocks[y][z][x] = BLOCK_SAND;
                }
            }
        }
    }

    /* Carve caves */
    for (y = 2; y < CHUNK_HEIGHT - 10; y++)
    {
        for (z = 0; z < CHUNK_SIZE_XZ; z++)
        {
            for (x = 0; x < CHUNK_SIZE_XZ; x++)
            {
                gfloat density;
                gfloat threshold;

                world_x = chunk->origin_x + x;
                world_z = chunk->origin_z + z;

                density = cave_noise_3d (world_x, y, world_z, WORLD_SEED + 1000);
                threshold = 0.55f - (gfloat)y * 0.003f;

                if (density > threshold &&
                    chunk->blocks[y][z][x] != BLOCK_BEDROCK &&
                    chunk->blocks[y][z][x] != BLOCK_WATER)
                {
                    chunk->blocks[y][z][x] = BLOCK_AIR;
                }
            }
        }
    }

    /* Place ores */
    ore_seed = WORLD_SEED + chunk_x * 1000 + chunk_z;
    for (y = 1; y < CHUNK_HEIGHT - 10; y++)
    {
        for (z = 0; z < CHUNK_SIZE_XZ; z++)
        {
            for (x = 0; x < CHUNK_SIZE_XZ; x++)
            {
                guint hash;
                gfloat chance;

                if (chunk->blocks[y][z][x] != BLOCK_STONE)
                    continue;

                hash = (x + chunk->origin_x) * 374761393 +
                       y * 668265263 +
                       (z + chunk->origin_z) * 1274126177 +
                       ore_seed;
                hash = (hash ^ (hash >> 13)) * 1274126177;
                chance = (gfloat)(hash & 0xFFFF) / 65536.0f;

                if (y <= 16 && chance < 0.002f)
                    place_ore_vein (chunk, x, y, z, BLOCK_DIAMOND_ORE, 4, hash);
                else if (y <= 28 && chance < 0.007f)
                    place_ore_vein (chunk, x, y, z, BLOCK_GOLD_ORE, 5, hash);
                else if (y <= 40 && chance < 0.015f)
                    place_ore_vein (chunk, x, y, z, BLOCK_IRON_ORE, 6, hash);
                else if (y <= 52 && chance < 0.025f)
                    place_ore_vein (chunk, x, y, z, BLOCK_COAL_ORE, 8, hash);
            }
        }
    }

    /* Place trees (only on grass, away from edges) */
    for (z = 2; z < CHUNK_SIZE_XZ - 2; z++)
    {
        for (x = 2; x < CHUNK_SIZE_XZ - 2; x++)
        {
            guint  hash;
            gfloat tree_chance;

            world_x = chunk->origin_x + x;
            world_z = chunk->origin_z + z;

            hash = world_x * 374761393 + world_z * 668265263 + WORLD_SEED * 999;
            hash = (hash ^ (hash >> 13)) * 1274126177;
            tree_chance = (gfloat)(hash & 0xFFFF) / 65536.0f;

            if (tree_chance < TREE_CHANCE)
            {
                /* Find ground level */
                for (y = CHUNK_HEIGHT - 1; y > SEA_LEVEL; y--)
                {
                    if (chunk->blocks[y][z][x] == BLOCK_GRASS)
                    {
                        place_tree (chunk, x, y, z, hash);
                        break;
                    }
                }
            }
        }
    }
}

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

/* ============================================================================
 * WORLD ACCESS
 * ============================================================================ */

static BlockType
world_get_block (VoxelWorld *world,
                 gint        x,
                 gint        y,
                 gint        z)
{
    gint   chunk_x, chunk_z;
    gint   local_x, local_z;
    Chunk *chunk;

    if (y < 0 || y >= CHUNK_HEIGHT)
        return BLOCK_AIR;
    if (x < 0 || x >= CHUNK_COUNT_X * CHUNK_SIZE_XZ)
        return BLOCK_AIR;
    if (z < 0 || z >= CHUNK_COUNT_Z * CHUNK_SIZE_XZ)
        return BLOCK_AIR;

    chunk_x = x / CHUNK_SIZE_XZ;
    chunk_z = z / CHUNK_SIZE_XZ;
    local_x = x % CHUNK_SIZE_XZ;
    local_z = z % CHUNK_SIZE_XZ;

    chunk = &world->chunks[chunk_z][chunk_x];
    return chunk->blocks[y][local_z][local_x];
}

static void
world_set_block (VoxelWorld *world,
                 gint        x,
                 gint        y,
                 gint        z,
                 BlockType   type)
{
    gint   chunk_x, chunk_z;
    gint   local_x, local_z;
    Chunk *chunk;

    if (y < 0 || y >= CHUNK_HEIGHT)
        return;
    if (x < 0 || x >= CHUNK_COUNT_X * CHUNK_SIZE_XZ)
        return;
    if (z < 0 || z >= CHUNK_COUNT_Z * CHUNK_SIZE_XZ)
        return;

    chunk_x = x / CHUNK_SIZE_XZ;
    chunk_z = z / CHUNK_SIZE_XZ;
    local_x = x % CHUNK_SIZE_XZ;
    local_z = z % CHUNK_SIZE_XZ;

    chunk = &world->chunks[chunk_z][chunk_x];
    chunk->blocks[y][local_z][local_x] = type;
}

static gboolean
world_is_solid (VoxelWorld *world,
                gint        x,
                gint        y,
                gint        z)
{
    BlockType  block;
    BlockFlags flags;

    block = world_get_block (world, x, y, z);
    if (block == BLOCK_AIR)
        return FALSE;

    flags = BLOCK_PROPS[block].flags;
    if (flags & BLOCK_FLAG_LIQUID)
        return FALSE;

    return TRUE;
}

static gboolean
block_is_transparent (BlockType block)
{
    if (block == BLOCK_AIR)
        return TRUE;
    return (BLOCK_PROPS[block].flags & BLOCK_FLAG_TRANSPARENT) != 0;
}

/* ============================================================================
 * GRAVITY BLOCK HANDLING
 * ============================================================================ */

static void
update_gravity_blocks (VoxelGame *game,
                       gint       x,
                       gint       start_y,
                       gint       z)
{
    gint      y;
    BlockType block;

    for (y = start_y; y < CHUNK_HEIGHT; y++)
    {
        block = world_get_block (&game->world, x, y, z);

        if (block == BLOCK_AIR)
            continue;

        if (!(BLOCK_PROPS[block].flags & BLOCK_FLAG_GRAVITY))
            continue;

        /* Check if block below is air or liquid */
        if (!world_is_solid (&game->world, x, y - 1, z))
        {
            /* Move block down */
            world_set_block (&game->world, x, y, z, BLOCK_AIR);

            /* Find landing position */
            gint land_y = y - 1;
            while (land_y > 0 && !world_is_solid (&game->world, x, land_y - 1, z))
            {
                land_y--;
            }

            world_set_block (&game->world, x, land_y, z, block);
        }
    }
}

/* ============================================================================
 * PLAYER FUNCTIONS
 * ============================================================================ */

static void
inventory_init (Inventory *inv)
{
    inv->slots[0] = BLOCK_GRASS;
    inv->slots[1] = BLOCK_DIRT;
    inv->slots[2] = BLOCK_STONE;
    inv->slots[3] = BLOCK_WOOD;
    inv->slots[4] = BLOCK_PLANKS;
    inv->slots[5] = BLOCK_COBBLESTONE;
    inv->slots[6] = BLOCK_BRICK;
    inv->slots[7] = BLOCK_GLASS;
    inv->slots[8] = BLOCK_SAND;
    inv->selected = 0;
}

static void
player_init (Player     *player,
             VoxelWorld *world)
{
    gint spawn_x, spawn_z;
    gint spawn_height;
    gint y;

    spawn_x = (CHUNK_COUNT_X * CHUNK_SIZE_XZ) / 2;
    spawn_z = (CHUNK_COUNT_Z * CHUNK_SIZE_XZ) / 2;

    /* Find safe spawn height */
    for (y = CHUNK_HEIGHT - 1; y > 0; y--)
    {
        BlockType block = world_get_block (world, spawn_x, y, spawn_z);
        if (block != BLOCK_AIR && block != BLOCK_WATER && block != BLOCK_LEAVES)
        {
            spawn_height = y + 2;
            break;
        }
    }

    player->pos_x = (gfloat)spawn_x + 0.5f;
    player->pos_y = (gfloat)spawn_height;
    player->pos_z = (gfloat)spawn_z + 0.5f;
    player->vel_y = 0.0f;
    player->yaw = 0.0f;
    player->pitch = 0.0f;
    player->on_ground = FALSE;
    player->was_on_ground = TRUE;
    player->health = PLAYER_MAX_HEALTH;
    player->fall_start_y = player->pos_y;
}

static void
game_init (VoxelGame *game)
{
    world_init (&game->world);
    player_init (&game->player, &game->world);
    inventory_init (&game->inventory);

    game->target_block.hit = FALSE;
    game->breaking.active = FALSE;
    game->world_time = 0.25f;
    game->game_time = 0.0f;
    game->cursor_locked = TRUE;
    game->show_debug = FALSE;
    game->state = GAME_STATE_PLAYING;
}

static void
player_update_camera (VoxelGame *game)
{
    g_autoptr(GrlVector2) delta = NULL;

    if (!game->cursor_locked || game->state != GAME_STATE_PLAYING)
        return;

    delta = grl_input_get_mouse_delta ();
    if (delta == NULL)
        return;

    game->player.yaw -= delta->x * MOUSE_SENSITIVITY;
    game->player.pitch -= delta->y * MOUSE_SENSITIVITY;

    if (game->player.pitch > G_PI / 2.0f - 0.1f)
        game->player.pitch = G_PI / 2.0f - 0.1f;
    if (game->player.pitch < -G_PI / 2.0f + 0.1f)
        game->player.pitch = -G_PI / 2.0f + 0.1f;
}

static gboolean
player_check_collision (VoxelGame *game,
                        gfloat     x,
                        gfloat     y,
                        gfloat     z)
{
    gint bx, by, bz;
    gint min_x, max_x, min_z, max_z;
    gint min_y, max_y;

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

    if (!game->cursor_locked || game->state != GAME_STATE_PLAYING)
        return;

    forward_x = sinf (game->player.yaw);
    forward_z = cosf (game->player.yaw);
    right_x = -cosf (game->player.yaw);
    right_z = sinf (game->player.yaw);

    speed = MOVE_SPEED;
    if (grl_input_is_key_down (GRL_KEY_LEFT_SHIFT))
        speed *= SPRINT_MULTIPLIER;

    move_x = 0.0f;
    move_z = 0.0f;

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

    if (move_x != 0.0f || move_z != 0.0f)
    {
        len = sqrtf (move_x * move_x + move_z * move_z);
        move_x /= len;
        move_z /= len;

        new_x = game->player.pos_x + move_x * speed * dt;
        if (!player_check_collision (game, new_x, game->player.pos_y, game->player.pos_z))
            game->player.pos_x = new_x;

        new_z = game->player.pos_z + move_z * speed * dt;
        if (!player_check_collision (game, game->player.pos_x, game->player.pos_y, new_z))
            game->player.pos_z = new_z;
    }
}

static void
player_take_damage (VoxelGame *game, gint damage);

static void
player_check_fall_damage (VoxelGame *game)
{
    Player *player = &game->player;
    gfloat  fall_distance;
    gint    damage;

    if (!player->on_ground && player->was_on_ground)
    {
        player->fall_start_y = player->pos_y;
    }

    if (player->on_ground && !player->was_on_ground)
    {
        fall_distance = player->fall_start_y - player->pos_y;

        if (fall_distance > FALL_DAMAGE_THRESHOLD)
        {
            damage = (gint)(fall_distance - FALL_DAMAGE_THRESHOLD) * FALL_DAMAGE_PER_BLOCK;
            player_take_damage (game, damage);
        }
    }

    player->was_on_ground = player->on_ground;
}

static void
player_take_damage (VoxelGame *game, gint damage)
{
    game->player.health -= damage;

    if (game->player.health <= 0)
    {
        game->player.health = 0;
        game->state = GAME_STATE_DEAD;
    }
}

static void
player_respawn (VoxelGame *game)
{
    player_init (&game->player, &game->world);
    game->state = GAME_STATE_PLAYING;
}

static void
player_update_physics (VoxelGame *game,
                       gfloat     dt)
{
    gfloat new_y;

    if (game->state != GAME_STATE_PLAYING)
        return;

    /* Jump */
    if (game->cursor_locked &&
        game->player.on_ground &&
        (grl_input_is_key_pressed (GRL_KEY_SPACE) ||
         grl_input_is_key_pressed (GRL_KEY_ENTER)))
    {
        game->player.vel_y = JUMP_VELOCITY;
        game->player.on_ground = FALSE;
    }

    game->player.vel_y -= GRAVITY * dt;

    new_y = game->player.pos_y + game->player.vel_y * dt;

    if (player_check_collision (game, game->player.pos_x, new_y, game->player.pos_z))
    {
        if (game->player.vel_y < 0)
            game->player.on_ground = TRUE;
        game->player.vel_y = 0.0f;
    }
    else
    {
        game->player.pos_y = new_y;
        game->player.on_ground = FALSE;
    }

    if (game->player.pos_y < 1.0f)
    {
        game->player.pos_y = 1.0f;
        game->player.vel_y = 0.0f;
        game->player.on_ground = TRUE;
    }

    player_check_fall_damage (game);
}

/* ============================================================================
 * RAYCASTING
 * ============================================================================ */

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

    ray_x = game->player.pos_x;
    ray_y = game->player.pos_y + PLAYER_EYE_OFFSET;
    ray_z = game->player.pos_z;

    dir_x = cosf (game->player.pitch) * sinf (game->player.yaw);
    dir_y = sinf (game->player.pitch);
    dir_z = cosf (game->player.pitch) * cosf (game->player.yaw);

    block_x = (gint)floorf (ray_x);
    block_y = (gint)floorf (ray_y);
    block_z = (gint)floorf (ray_z);

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

    for (iterations = 0; iterations < 100; iterations++)
    {
        BlockType block = world_get_block (&game->world, block_x, block_y, block_z);

        if (block != BLOCK_AIR && !(BLOCK_PROPS[block].flags & BLOCK_FLAG_LIQUID))
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

/* ============================================================================
 * BLOCK INTERACTION
 * ============================================================================ */

static void
update_block_breaking (VoxelGame *game,
                       gfloat     dt)
{
    BlockType              block;
    const BlockProperties *props;

    if (!game->cursor_locked || game->state != GAME_STATE_PLAYING)
        return;

    if (grl_input_is_mouse_button_down (GRL_MOUSE_BUTTON_LEFT))
    {
        if (!game->target_block.hit)
        {
            game->breaking.active = FALSE;
            return;
        }

        if (!game->breaking.active ||
            game->breaking.target_x != game->target_block.block_x ||
            game->breaking.target_y != game->target_block.block_y ||
            game->breaking.target_z != game->target_block.block_z)
        {
            block = world_get_block (&game->world,
                                     game->target_block.block_x,
                                     game->target_block.block_y,
                                     game->target_block.block_z);
            props = &BLOCK_PROPS[block];

            if (props->flags & BLOCK_FLAG_UNBREAKABLE)
            {
                game->breaking.active = FALSE;
                return;
            }

            game->breaking.active = TRUE;
            game->breaking.target_x = game->target_block.block_x;
            game->breaking.target_y = game->target_block.block_y;
            game->breaking.target_z = game->target_block.block_z;
            game->breaking.progress = 0.0f;
            game->breaking.total_time = props->hardness;
        }

        if (game->breaking.total_time > 0.0f)
            game->breaking.progress += dt / game->breaking.total_time;
        else
            game->breaking.progress = 1.0f;

        if (game->breaking.progress >= 1.0f)
        {
            world_set_block (&game->world,
                             game->breaking.target_x,
                             game->breaking.target_y,
                             game->breaking.target_z,
                             BLOCK_AIR);
            game->breaking.active = FALSE;

            update_gravity_blocks (game,
                                   game->breaking.target_x,
                                   game->breaking.target_y + 1,
                                   game->breaking.target_z);
        }
    }
    else
    {
        game->breaking.active = FALSE;
    }
}

static void
place_block (VoxelGame *game)
{
    gint      place_x, place_y, place_z;
    BlockType block_type;
    BlockType existing;

    if (!game->target_block.hit || game->state != GAME_STATE_PLAYING)
        return;

    place_x = game->target_block.face_x;
    place_y = game->target_block.face_y;
    place_z = game->target_block.face_z;

    existing = world_get_block (&game->world, place_x, place_y, place_z);
    if (existing != BLOCK_AIR && !(BLOCK_PROPS[existing].flags & BLOCK_FLAG_REPLACEABLE))
        return;

    if (place_x == (gint)floorf (game->player.pos_x) &&
        place_z == (gint)floorf (game->player.pos_z) &&
        place_y >= (gint)floorf (game->player.pos_y) &&
        place_y <= (gint)floorf (game->player.pos_y + PLAYER_HEIGHT))
    {
        return;
    }

    block_type = game->inventory.slots[game->inventory.selected];
    world_set_block (&game->world, place_x, place_y, place_z, block_type);
}

static void
handle_block_interaction (VoxelGame *game)
{
    if (!game->cursor_locked || game->state != GAME_STATE_PLAYING)
        return;

    if (grl_input_is_mouse_button_pressed (GRL_MOUSE_BUTTON_RIGHT))
        place_block (game);
}

static void
inventory_handle_input (VoxelGame *game)
{
    gfloat wheel;

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
    if (grl_input_is_key_pressed (GRL_KEY_SEVEN))
        game->inventory.selected = 6;
    if (grl_input_is_key_pressed (GRL_KEY_EIGHT))
        game->inventory.selected = 7;
    if (grl_input_is_key_pressed (GRL_KEY_NINE))
        game->inventory.selected = 8;

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

/* ============================================================================
 * SKY AND LIGHTING
 * ============================================================================ */

static void
get_sky_colors (gfloat     time,
                GrlColor **horizon,
                GrlColor **zenith,
                GrlColor **fog)
{
    /* time: 0.0 = midnight, 0.25 = sunrise, 0.5 = noon, 0.75 = sunset */
    if (time < 0.2f || time > 0.8f)
    {
        /* Night */
        *horizon = grl_color_new (20, 30, 60, 255);
        *zenith = grl_color_new (5, 10, 30, 255);
        *fog = grl_color_new (15, 20, 45, 255);
    }
    else if (time < 0.3f)
    {
        /* Sunrise */
        gfloat t = (time - 0.2f) / 0.1f;
        *horizon = grl_color_new (20 + (gint)(180 * t), 30 + (gint)(100 * t), 60 + (gint)(140 * t), 255);
        *zenith = grl_color_new (5 + (gint)(95 * t), 10 + (gint)(140 * t), 30 + (gint)(225 * t), 255);
        *fog = grl_color_new (15 + (gint)(145 * t), 20 + (gint)(180 * t), 45 + (gint)(195 * t), 255);
    }
    else if (time < 0.7f)
    {
        /* Day */
        *horizon = grl_color_new (180, 220, 255, 255);
        *zenith = grl_color_new (100, 150, 255, 255);
        *fog = grl_color_new (160, 200, 240, 255);
    }
    else
    {
        /* Sunset */
        gfloat t = (time - 0.7f) / 0.1f;
        *horizon = grl_color_new (180 + (gint)(75 * t) - (gint)(235 * t), 220 - (gint)(120 * t), 255 - (gint)(155 * t), 255);
        *zenith = grl_color_new (100 - (gint)(95 * t), 150 - (gint)(140 * t), 255 - (gint)(225 * t), 255);
        *fog = grl_color_new (160 - (gint)(145 * t), 200 - (gint)(180 * t), 240 - (gint)(195 * t), 255);
    }
}

static gfloat
get_ambient_light (gfloat time)
{
    if (time < 0.2f || time > 0.8f)
        return 0.4f;
    else if (time < 0.3f)
        return 0.4f + 0.6f * ((time - 0.2f) / 0.1f);
    else if (time < 0.7f)
        return 1.0f;
    else
        return 1.0f - 0.6f * ((time - 0.7f) / 0.1f);
}

/* ============================================================================
 * RENDERING
 * ============================================================================ */

static void
draw_lit_block (VoxelGame      *game,
                gint            block_x,
                gint            block_y,
                gint            block_z,
                const GrlColor *base_color,
                gfloat          ambient,
                const GrlColor *fog_color,
                gfloat          fog_factor)
{
    g_autoptr(GrlVector3) pos = NULL;
    g_autoptr(GrlColor) lit_color = NULL;
    g_autoptr(GrlColor) final_color = NULL;
    gfloat x, y, z;
    gfloat avg_light;

    (void)game; /* Unused after simplification */

    /* Average face lighting for uniform block color */
    avg_light = 0.8f * ambient;

    x = (gfloat)block_x + 0.5f;
    y = (gfloat)block_y + 0.5f;
    z = (gfloat)block_z + 0.5f;

    pos = grl_vector3_new (x, y, z);
    lit_color = grl_color_brightness (base_color, avg_light - 1.0f);
    final_color = grl_color_lerp (lit_color, fog_color, fog_factor);

    grl_draw_cube (pos, 0.98f, 0.98f, 0.98f, final_color);
}

static void
draw_world (VoxelGame   *game,
            GrlCamera3D *camera)
{
    g_autoptr(GrlColor) fog_color = NULL;
    g_autoptr(GrlColor) horizon = NULL;
    g_autoptr(GrlColor) zenith = NULL;
    gint      cx, cz, x, y, z;
    gint      world_x, world_z;
    BlockType block;
    Chunk    *chunk;
    gfloat    wx, wy, wz;
    gfloat    dx, dy, dz, dist_sq, distance, fog_factor;
    gfloat    ambient;

    get_sky_colors (game->world_time, &horizon, &zenith, &fog_color);
    ambient = get_ambient_light (game->world_time);

    grl_camera3d_begin (camera);

    /* Draw opaque blocks first */
    for (cz = 0; cz < CHUNK_COUNT_Z; cz++)
    {
        for (cx = 0; cx < CHUNK_COUNT_X; cx++)
        {
            chunk = &game->world.chunks[cz][cx];

            for (y = 0; y < CHUNK_HEIGHT; y++)
            {
                for (z = 0; z < CHUNK_SIZE_XZ; z++)
                {
                    for (x = 0; x < CHUNK_SIZE_XZ; x++)
                    {
                        g_autoptr(GrlColor) base_color = NULL;

                        block = chunk->blocks[y][z][x];
                        if (block == BLOCK_AIR)
                            continue;
                        if (BLOCK_PROPS[block].flags & BLOCK_FLAG_TRANSPARENT)
                            continue;

                        world_x = chunk->origin_x + x;
                        world_z = chunk->origin_z + z;

                        wx = (gfloat)world_x + 0.5f;
                        wy = (gfloat)y + 0.5f;
                        wz = (gfloat)world_z + 0.5f;

                        dx = wx - game->player.pos_x;
                        dz = wz - game->player.pos_z;
                        dist_sq = dx * dx + dz * dz;

                        if (dist_sq > RENDER_DISTANCE * RENDER_DISTANCE)
                            continue;

                        dy = wy - (game->player.pos_y + PLAYER_EYE_OFFSET);
                        distance = sqrtf (dx * dx + dy * dy + dz * dz);

                        if (distance < FOG_START)
                            fog_factor = 0.0f;
                        else if (distance > FOG_END)
                            fog_factor = 1.0f;
                        else
                            fog_factor = (distance - FOG_START) / (FOG_END - FOG_START);

                        base_color = grl_color_new (BLOCK_PROPS[block].r,
                                                    BLOCK_PROPS[block].g,
                                                    BLOCK_PROPS[block].b,
                                                    255);

                        draw_lit_block (game, world_x, y, world_z,
                                        base_color, ambient, fog_color, fog_factor);
                    }
                }
            }
        }
    }

    /* Draw transparent blocks */
    for (cz = 0; cz < CHUNK_COUNT_Z; cz++)
    {
        for (cx = 0; cx < CHUNK_COUNT_X; cx++)
        {
            chunk = &game->world.chunks[cz][cx];

            for (y = 0; y < CHUNK_HEIGHT; y++)
            {
                for (z = 0; z < CHUNK_SIZE_XZ; z++)
                {
                    for (x = 0; x < CHUNK_SIZE_XZ; x++)
                    {
                        g_autoptr(GrlColor) base_color = NULL;

                        block = chunk->blocks[y][z][x];
                        if (block == BLOCK_AIR)
                            continue;
                        if (!(BLOCK_PROPS[block].flags & BLOCK_FLAG_TRANSPARENT))
                            continue;

                        world_x = chunk->origin_x + x;
                        world_z = chunk->origin_z + z;

                        wx = (gfloat)world_x + 0.5f;
                        wy = (gfloat)y + 0.5f;
                        wz = (gfloat)world_z + 0.5f;

                        dx = wx - game->player.pos_x;
                        dz = wz - game->player.pos_z;
                        dist_sq = dx * dx + dz * dz;

                        if (dist_sq > RENDER_DISTANCE * RENDER_DISTANCE)
                            continue;

                        dy = wy - (game->player.pos_y + PLAYER_EYE_OFFSET);
                        distance = sqrtf (dx * dx + dy * dy + dz * dz);

                        if (distance < FOG_START)
                            fog_factor = 0.0f;
                        else if (distance > FOG_END)
                            fog_factor = 1.0f;
                        else
                            fog_factor = (distance - FOG_START) / (FOG_END - FOG_START);

                        /* Transparent blocks get alpha */
                        base_color = grl_color_new (BLOCK_PROPS[block].r,
                                                    BLOCK_PROPS[block].g,
                                                    BLOCK_PROPS[block].b,
                                                    (block == BLOCK_WATER) ? 180 : 220);

                        draw_lit_block (game, world_x, y, world_z,
                                        base_color, ambient, fog_color, fog_factor);
                    }
                }
            }
        }
    }

    /* Draw sun/moon */
    {
        g_autoptr(GrlVector3) sun_pos = NULL;
        g_autoptr(GrlColor) sun_color = NULL;
        gfloat angle = game->world_time * 2.0f * G_PI;
        gfloat sun_x = game->player.pos_x + cosf (angle) * 80.0f;
        gfloat sun_y = sinf (angle) * 50.0f + 40.0f;
        gfloat sun_z = game->player.pos_z;

        sun_pos = grl_vector3_new (sun_x, sun_y, sun_z);

        if (game->world_time > 0.2f && game->world_time < 0.8f)
        {
            sun_color = grl_color_new (255, 255, 200, 255);
            grl_draw_sphere (sun_pos, 4.0f, sun_color);
        }
        else
        {
            sun_color = grl_color_new (220, 220, 240, 255);
            grl_draw_sphere (sun_pos, 2.5f, sun_color);
        }
    }

    /* Draw block highlight */
    if (game->target_block.hit)
    {
        g_autoptr(GrlVector3) highlight_pos = NULL;
        g_autoptr(GrlColor) highlight_color = NULL;
        gfloat pulse = (sinf (game->game_time * 5.0f) + 1.0f) / 2.0f;
        guint8 alpha = (guint8)(150 + pulse * 105);
        gfloat size = 1.01f + pulse * 0.02f;

        highlight_pos = grl_vector3_new (
            (gfloat)game->target_block.block_x + 0.5f,
            (gfloat)game->target_block.block_y + 0.5f,
            (gfloat)game->target_block.block_z + 0.5f
        );
        highlight_color = grl_color_new (255, 255, 255, alpha);
        grl_draw_cube_wires (highlight_pos, size, size, size, highlight_color);

        /* Breaking overlay */
        if (game->breaking.active && game->breaking.progress > 0.0f)
        {
            g_autoptr(GrlColor) crack_color = NULL;
            gint crack_stage = (gint)(game->breaking.progress * 10.0f);
            if (crack_stage > 9) crack_stage = 9;

            crack_color = grl_color_new (0, 0, 0, (guint8)(crack_stage * 20));
            grl_draw_cube (highlight_pos, 1.002f, 1.002f, 1.002f, crack_color);
        }
    }

    grl_camera3d_end (camera);
}

static void
draw_sky_background (VoxelGame *game)
{
    g_autoptr(GrlColor) horizon = NULL;
    g_autoptr(GrlColor) zenith = NULL;
    g_autoptr(GrlColor) fog = NULL;

    get_sky_colors (game->world_time, &horizon, &zenith, &fog);

    /* Clear screen and depth buffer first! */
    grl_draw_clear_background (horizon);

    grl_draw_rectangle_gradient_v (0, 0, WINDOW_WIDTH, WINDOW_HEIGHT / 2,
                                   zenith, horizon);
    grl_draw_rectangle (0, WINDOW_HEIGHT / 2, WINDOW_WIDTH, WINDOW_HEIGHT / 2, horizon);
}

static void
draw_crosshair (void)
{
    g_autoptr(GrlColor) color = grl_color_new (255, 255, 255, 200);
    gint cx = WINDOW_WIDTH / 2;
    gint cy = WINDOW_HEIGHT / 2;

    grl_draw_line (cx - 10, cy, cx + 10, cy, color);
    grl_draw_line (cx, cy - 10, cx, cy + 10, color);
}

static void
draw_health_ui (VoxelGame *game)
{
    g_autoptr(GrlColor) heart_full = grl_color_new (200, 50, 50, 255);
    g_autoptr(GrlColor) heart_empty = grl_color_new (60, 30, 30, 255);
    g_autoptr(GrlColor) heart_border = grl_color_new (100, 40, 40, 255);
    gint i;
    gint heart_size = 20;
    gint spacing = 4;
    gint start_x = WINDOW_WIDTH / 2 - (PLAYER_MAX_HEALTH * (heart_size + spacing)) / 2;
    gint y = WINDOW_HEIGHT - 130;

    for (i = 0; i < PLAYER_MAX_HEALTH; i++)
    {
        gint x = start_x + i * (heart_size + spacing);

        grl_draw_rectangle (x, y, heart_size, heart_size, heart_border);

        if (i < game->player.health)
            grl_draw_rectangle (x + 2, y + 2, heart_size - 4, heart_size - 4, heart_full);
        else
            grl_draw_rectangle (x + 2, y + 2, heart_size - 4, heart_size - 4, heart_empty);
    }
}

static void
draw_hotbar (VoxelGame *game)
{
    g_autoptr(GrlColor) bg_color = grl_color_new (60, 60, 60, 200);
    g_autoptr(GrlColor) selected_color = grl_color_new (255, 255, 255, 255);
    g_autoptr(GrlColor) text_color = grl_color_new (255, 255, 255, 255);
    gint slot_size = 45;
    gint hotbar_width = INVENTORY_SLOTS * slot_size + (INVENTORY_SLOTS - 1) * 4;
    gint start_x = (WINDOW_WIDTH - hotbar_width) / 2;
    gint start_y = WINDOW_HEIGHT - slot_size - 20;
    gint i;
    gchar text[32];

    for (i = 0; i < INVENTORY_SLOTS; i++)
    {
        g_autoptr(GrlColor) block_color = NULL;
        gint slot_x = start_x + i * (slot_size + 4);

        grl_draw_rectangle (slot_x, start_y, slot_size, slot_size, bg_color);

        if (i == game->inventory.selected)
            grl_draw_rectangle_lines (slot_x, start_y, slot_size, slot_size, selected_color);

        block_color = grl_color_new (BLOCK_PROPS[game->inventory.slots[i]].r,
                                     BLOCK_PROPS[game->inventory.slots[i]].g,
                                     BLOCK_PROPS[game->inventory.slots[i]].b,
                                     255);
        grl_draw_rectangle (slot_x + 5, start_y + 5, slot_size - 10, slot_size - 10, block_color);

        g_snprintf (text, sizeof (text), "%d", i + 1);
        grl_draw_text (text, slot_x + 3, start_y + 2, 12, text_color);
    }

    /* Selected block name */
    grl_draw_text (BLOCK_PROPS[game->inventory.slots[game->inventory.selected]].name,
                   start_x, start_y - 25, 20, text_color);
}

static void
draw_debug (VoxelGame *game)
{
    g_autoptr(GrlColor) text_color = grl_color_new (255, 255, 255, 255);
    g_autoptr(GrlColor) shadow_color = grl_color_new (0, 0, 0, 200);
    gchar text[128];
    gint y = 10;
    gint line_height = 18;

    if (!game->show_debug)
        return;

    g_snprintf (text, sizeof (text), "FPS: %d", grl_window_get_fps (NULL));
    grl_draw_text (text, 11, y + 1, 16, shadow_color);
    grl_draw_text (text, 10, y, 16, text_color);
    y += line_height;

    g_snprintf (text, sizeof (text), "Pos: %.1f, %.1f, %.1f",
                game->player.pos_x, game->player.pos_y, game->player.pos_z);
    grl_draw_text (text, 11, y + 1, 16, shadow_color);
    grl_draw_text (text, 10, y, 16, text_color);
    y += line_height;

    g_snprintf (text, sizeof (text), "Time: %.2f (%.0f%%)",
                game->world_time, game->world_time * 100.0f);
    grl_draw_text (text, 11, y + 1, 16, shadow_color);
    grl_draw_text (text, 10, y, 16, text_color);
    y += line_height;

    g_snprintf (text, sizeof (text), "Health: %d/%d",
                game->player.health, PLAYER_MAX_HEALTH);
    grl_draw_text (text, 11, y + 1, 16, shadow_color);
    grl_draw_text (text, 10, y, 16, text_color);
    y += line_height;

    if (game->target_block.hit)
    {
        BlockType block = world_get_block (&game->world,
                                           game->target_block.block_x,
                                           game->target_block.block_y,
                                           game->target_block.block_z);
        g_snprintf (text, sizeof (text), "Target: %s @ %d,%d,%d",
                    BLOCK_PROPS[block].name,
                    game->target_block.block_x,
                    game->target_block.block_y,
                    game->target_block.block_z);
        grl_draw_text (text, 11, y + 1, 16, shadow_color);
        grl_draw_text (text, 10, y, 16, text_color);
    }
}

static void
draw_controls (VoxelGame *game)
{
    g_autoptr(GrlColor) gray = grl_color_new (150, 150, 150, 255);
    g_autoptr(GrlColor) white = grl_color_new (255, 255, 255, 255);

    grl_draw_text ("WASD: Move  Space: Jump  Shift: Sprint  LMB(hold): Break  RMB: Place  1-9: Select  F3: Debug",
                   10, WINDOW_HEIGHT - 18, 14, gray);

    if (!game->cursor_locked && game->state == GAME_STATE_PLAYING)
    {
        grl_draw_text ("CURSOR UNLOCKED - Click to lock",
                       WINDOW_WIDTH / 2 - 120, 50, 18, white);
    }
}

static void
draw_death_screen (VoxelGame *game)
{
    g_autoptr(GrlColor) overlay = grl_color_new (100, 0, 0, 150);
    g_autoptr(GrlColor) title_color = grl_color_new (255, 80, 80, 255);
    g_autoptr(GrlColor) text_color = grl_color_new (255, 255, 255, 255);
    gint title_width;

    grl_draw_rectangle (0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, overlay);

    title_width = grl_measure_text ("You Died!", 60);
    grl_draw_text ("You Died!", WINDOW_WIDTH / 2 - title_width / 2,
                   WINDOW_HEIGHT / 2 - 60, 60, title_color);

    grl_draw_text ("Press SPACE to respawn",
                   WINDOW_WIDTH / 2 - 100, WINDOW_HEIGHT / 2 + 20, 20, text_color);
}

static void
apply_camera_state (VoxelGame   *game,
                    GrlCamera3D *camera)
{
    gfloat target_x, target_y, target_z;
    gfloat eye_y;

    eye_y = game->player.pos_y + PLAYER_EYE_OFFSET;

    target_x = game->player.pos_x + cosf (game->player.pitch) * sinf (game->player.yaw);
    target_y = eye_y + sinf (game->player.pitch);
    target_z = game->player.pos_z + cosf (game->player.pitch) * cosf (game->player.yaw);

    grl_camera3d_set_position_xyz (camera, game->player.pos_x, eye_y, game->player.pos_z);
    grl_camera3d_set_target_xyz (camera, target_x, target_y, target_z);
}

/* ============================================================================
 * MAIN
 * ============================================================================ */

int
main (int    argc,
      char **argv)
{
    g_autoptr(GrlApplication) app = NULL;
    g_autoptr(GrlWindow) window = NULL;
    g_autoptr(GrlCamera3D) camera = NULL;
    VoxelGame game;
    gfloat dt;

    app = grl_application_new ("com.example.voxelcraft3d");
    window = grl_window_new (WINDOW_WIDTH, WINDOW_HEIGHT, "Voxelcraft 3D");

    camera = grl_camera3d_new ();
    grl_camera3d_set_fovy (camera, 70.0f);

    grl_window_set_target_fps (window, 60);

    game_init (&game);

    grl_window_disable_cursor (window);

    while (!grl_window_should_close (window))
    {
        dt = grl_window_get_frame_time (window);

        game.game_time += dt;
        game.world_time += dt / DAY_LENGTH;
        if (game.world_time >= 1.0f)
            game.world_time -= 1.0f;

        grl_window_poll_input (window);

        /* Handle death respawn */
        if (game.state == GAME_STATE_DEAD &&
            grl_input_is_key_pressed (GRL_KEY_SPACE))
        {
            player_respawn (&game);
            grl_window_disable_cursor (window);
        }

        /* Cursor lock toggle */
        if (grl_input_is_key_pressed (GRL_KEY_ESCAPE))
        {
            game.cursor_locked = !game.cursor_locked;
            if (game.cursor_locked)
                grl_window_disable_cursor (window);
            else
                grl_window_enable_cursor (window);
        }

        /* Debug toggle */
        if (grl_input_is_key_pressed (GRL_KEY_F3))
            game.show_debug = !game.show_debug;

        /* Click to re-lock */
        if (!game.cursor_locked && game.state == GAME_STATE_PLAYING &&
            grl_input_is_mouse_button_pressed (GRL_MOUSE_BUTTON_LEFT))
        {
            game.cursor_locked = TRUE;
            grl_window_disable_cursor (window);
        }

        /* Update player */
        player_update_camera (&game);
        player_update_movement (&game, dt);
        player_update_physics (&game, dt);

        /* Update targeting and interaction */
        game.target_block = raycast_block (&game);
        inventory_handle_input (&game);
        update_block_breaking (&game, dt);
        handle_block_interaction (&game);

        /* Apply camera */
        apply_camera_state (&game, camera);

        /* Draw */
        grl_window_begin_drawing (window);
        draw_sky_background (&game);
        draw_world (&game, camera);
        draw_crosshair ();
        draw_health_ui (&game);
        draw_hotbar (&game);
        draw_debug (&game);
        draw_controls (&game);

        if (game.state == GAME_STATE_DEAD)
            draw_death_screen (&game);

        grl_window_end_drawing (window);
    }

    return 0;
}
