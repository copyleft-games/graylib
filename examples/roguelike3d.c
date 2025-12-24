/* roguelike3d.c
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Simple 3D roguelike demonstrating turn-based gameplay,
 * procedural dungeon generation, FOV, combat, and items.
 */

#include <graylib.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

/* Window dimensions */
#define WINDOW_WIDTH      1280
#define WINDOW_HEIGHT     900

/* Map dimensions */
#define MAP_WIDTH         40
#define MAP_HEIGHT        30
#define CELL_SIZE         1.5f

/* Room generation */
#define MAX_ROOMS         15
#define MIN_ROOM_SIZE     4
#define MAX_ROOM_SIZE     10

/* Entity limits */
#define MAX_ENEMIES       20
#define MAX_ITEMS         15
#define MAX_MESSAGES      5
#define MAX_INVENTORY     10

/* Animation */
#define ANIM_DURATION     0.15f

/* FOV */
#define FOV_RADIUS        8

/* Camera */
#define CAM_HEIGHT        35.0f
#define CAM_DISTANCE      25.0f

/* Player stats */
#define PLAYER_BASE_HP    50
#define PLAYER_BASE_ATK   5
#define PLAYER_BASE_DEF   2

/* Enemy stats */
#define ENEMY_BASE_HP     15
#define ENEMY_BASE_ATK    3
#define ENEMY_BASE_DEF    1

/* Item values */
#define POTION_HEAL       20
#define WEAPON_BONUS      3
#define ARMOR_BONUS       2

/* Tile types */
typedef enum
{
    TILE_VOID,
    TILE_FLOOR,
    TILE_WALL,
    TILE_STAIRS
} TileType;

/* Item types */
typedef enum
{
    ITEM_NONE,
    ITEM_HEALTH_POTION,
    ITEM_WEAPON,
    ITEM_ARMOR
} ItemType;

/* Game states */
typedef enum
{
    STATE_PLAYING,
    STATE_ANIMATING,
    STATE_ENEMY_TURN,
    STATE_INVENTORY,
    STATE_STAIRS_PROMPT,
    STATE_DEAD
} GameState;

/* Room structure for generation */
typedef struct
{
    gint x;
    gint y;
    gint w;
    gint h;
} Room;

/* Entity (player/enemies) */
typedef struct
{
    gint        x;
    gint        y;
    gint        hp;
    gint        max_hp;
    gint        attack;
    gint        defense;
    gboolean    alive;
    gfloat      world_x;
    gfloat      world_z;
    gfloat      start_x;
    gfloat      start_z;
    gfloat      target_x;
    gfloat      target_z;
} Entity;

/* Item on floor */
typedef struct
{
    ItemType    type;
    gint        x;
    gint        y;
    gint        value;
    gboolean    collected;
} Item;

/* Message log entry */
typedef struct
{
    gchar   text[128];
    guint8  r;
    guint8  g;
    guint8  b;
} Message;

/* Dungeon map */
typedef struct
{
    TileType    tiles[MAP_HEIGHT][MAP_WIDTH];
    gint        visibility[MAP_HEIGHT][MAP_WIDTH];
} DungeonMap;

/* Main game state */
typedef struct
{
    /* Map */
    DungeonMap  map;
    gint        floor_number;
    gint        stairs_x;
    gint        stairs_y;

    /* Player */
    Entity      player;
    ItemType    inventory[MAX_INVENTORY];
    gint        inventory_count;
    gint        weapon_bonus;
    gint        armor_bonus;
    gint        inv_cursor;

    /* Enemies */
    Entity      enemies[MAX_ENEMIES];
    gint        enemy_count;
    gint        current_enemy;

    /* Items on floor */
    Item        items[MAX_ITEMS];
    gint        item_count;

    /* Messages */
    Message     messages[MAX_MESSAGES];
    gint        message_count;

    /* State */
    GameState   state;
    gfloat      anim_timer;
    gfloat      bob_timer;

    /* Stats */
    gint        kills;
} RoguelikeGame;

/*
 * Helper: convert grid X to world X
 */
static gfloat
grid_to_world_x (gint gx)
{
    return (gfloat)(gx - MAP_WIDTH / 2) * CELL_SIZE;
}

/*
 * Helper: convert grid Y to world Z
 */
static gfloat
grid_to_world_z (gint gy)
{
    return (gfloat)(gy - MAP_HEIGHT / 2) * CELL_SIZE;
}

/*
 * Helper: linear interpolation
 */
static gfloat
lerpf (gfloat a,
       gfloat b,
       gfloat t)
{
    return a + (b - a) * t;
}

/*
 * Helper: clamp integer
 */
static gint
clampi (gint val,
        gint min_val,
        gint max_val)
{
    if (val < min_val)
        return min_val;
    if (val > max_val)
        return max_val;
    return val;
}

/*
 * Add message to log
 */
static void
add_message (RoguelikeGame *game,
             const gchar   *text,
             guint8         r,
             guint8         g,
             guint8         b)
{
    gint i;

    /* Shift messages up */
    if (game->message_count >= MAX_MESSAGES)
    {
        for (i = 0; i < MAX_MESSAGES - 1; i++)
        {
            game->messages[i] = game->messages[i + 1];
        }
        game->message_count = MAX_MESSAGES - 1;
    }

    /* Add new message */
    g_strlcpy (game->messages[game->message_count].text, text,
               sizeof (game->messages[0].text));
    game->messages[game->message_count].r = r;
    game->messages[game->message_count].g = g;
    game->messages[game->message_count].b = b;
    game->message_count++;
}

/*
 * Check if room overlaps with existing rooms
 */
static gboolean
room_overlaps (Room *rooms,
               gint  room_count,
               gint  x,
               gint  y,
               gint  w,
               gint  h)
{
    gint i;

    for (i = 0; i < room_count; i++)
    {
        /* Add 1 tile buffer between rooms */
        if (x < rooms[i].x + rooms[i].w + 1 &&
            x + w + 1 > rooms[i].x &&
            y < rooms[i].y + rooms[i].h + 1 &&
            y + h + 1 > rooms[i].y)
        {
            return TRUE;
        }
    }
    return FALSE;
}

/*
 * Carve a room into the map
 */
static void
carve_room (RoguelikeGame *game,
            gint           x,
            gint           y,
            gint           w,
            gint           h)
{
    gint rx;
    gint ry;

    for (ry = y; ry < y + h; ry++)
    {
        for (rx = x; rx < x + w; rx++)
        {
            if (rx > 0 && rx < MAP_WIDTH - 1 &&
                ry > 0 && ry < MAP_HEIGHT - 1)
            {
                game->map.tiles[ry][rx] = TILE_FLOOR;
            }
        }
    }
}

/*
 * Carve L-shaped corridor between two points
 */
static void
carve_corridor (RoguelikeGame *game,
                gint           x1,
                gint           y1,
                gint           x2,
                gint           y2)
{
    gint x;
    gint y;
    gint min_x;
    gint max_x;
    gint min_y;
    gint max_y;

    min_x = (x1 < x2) ? x1 : x2;
    max_x = (x1 > x2) ? x1 : x2;
    min_y = (y1 < y2) ? y1 : y2;
    max_y = (y1 > y2) ? y1 : y2;

    /* Horizontal then vertical, or vice versa */
    if (rand () % 2)
    {
        for (x = min_x; x <= max_x; x++)
        {
            if (x > 0 && x < MAP_WIDTH - 1)
                game->map.tiles[y1][x] = TILE_FLOOR;
        }
        for (y = min_y; y <= max_y; y++)
        {
            if (y > 0 && y < MAP_HEIGHT - 1)
                game->map.tiles[y][x2] = TILE_FLOOR;
        }
    }
    else
    {
        for (y = min_y; y <= max_y; y++)
        {
            if (y > 0 && y < MAP_HEIGHT - 1)
                game->map.tiles[y][x1] = TILE_FLOOR;
        }
        for (x = min_x; x <= max_x; x++)
        {
            if (x > 0 && x < MAP_WIDTH - 1)
                game->map.tiles[y2][x] = TILE_FLOOR;
        }
    }
}

/*
 * Check if tile is walkable
 */
static gboolean
is_walkable (RoguelikeGame *game,
             gint           x,
             gint           y)
{
    if (x < 0 || x >= MAP_WIDTH || y < 0 || y >= MAP_HEIGHT)
        return FALSE;
    return game->map.tiles[y][x] == TILE_FLOOR ||
           game->map.tiles[y][x] == TILE_STAIRS;
}

/*
 * Get enemy at position (or NULL)
 */
static Entity *
get_enemy_at (RoguelikeGame *game,
              gint           x,
              gint           y)
{
    gint i;

    for (i = 0; i < game->enemy_count; i++)
    {
        if (game->enemies[i].alive &&
            game->enemies[i].x == x &&
            game->enemies[i].y == y)
        {
            return &game->enemies[i];
        }
    }
    return NULL;
}

/*
 * Place enemies in rooms (not first room)
 */
static void
place_enemies (RoguelikeGame *game,
               Room          *rooms,
               gint           room_count)
{
    gint i;
    gint j;
    gint rx;
    gint ry;
    gint floor_bonus;

    game->enemy_count = 0;
    floor_bonus = game->floor_number - 1;

    for (i = 1; i < room_count && game->enemy_count < MAX_ENEMIES; i++)
    {
        /* 1-2 enemies per room */
        gint enemies_in_room = 1 + rand () % 2;

        for (j = 0; j < enemies_in_room && game->enemy_count < MAX_ENEMIES; j++)
        {
            rx = rooms[i].x + 1 + rand () % (rooms[i].w - 2);
            ry = rooms[i].y + 1 + rand () % (rooms[i].h - 2);

            /* Don't place on stairs or other enemies */
            if (game->map.tiles[ry][rx] == TILE_FLOOR &&
                get_enemy_at (game, rx, ry) == NULL)
            {
                game->enemies[game->enemy_count].x = rx;
                game->enemies[game->enemy_count].y = ry;
                game->enemies[game->enemy_count].world_x = grid_to_world_x (rx);
                game->enemies[game->enemy_count].world_z = grid_to_world_z (ry);
                game->enemies[game->enemy_count].target_x = grid_to_world_x (rx);
                game->enemies[game->enemy_count].target_z = grid_to_world_z (ry);
                game->enemies[game->enemy_count].max_hp = ENEMY_BASE_HP + floor_bonus * 5;
                game->enemies[game->enemy_count].hp = game->enemies[game->enemy_count].max_hp;
                game->enemies[game->enemy_count].attack = ENEMY_BASE_ATK + floor_bonus;
                game->enemies[game->enemy_count].defense = ENEMY_BASE_DEF + floor_bonus / 2;
                game->enemies[game->enemy_count].alive = TRUE;
                game->enemy_count++;
            }
        }
    }
}

/*
 * Place items in rooms
 */
static void
place_items (RoguelikeGame *game,
             Room          *rooms,
             gint           room_count)
{
    gint i;
    gint rx;
    gint ry;
    gint room_idx;

    game->item_count = 0;

    /* Place 3-6 items */
    gint num_items = 3 + rand () % 4;

    for (i = 0; i < num_items && game->item_count < MAX_ITEMS; i++)
    {
        room_idx = rand () % room_count;
        rx = rooms[room_idx].x + 1 + rand () % (rooms[room_idx].w - 2);
        ry = rooms[room_idx].y + 1 + rand () % (rooms[room_idx].h - 2);

        if (game->map.tiles[ry][rx] == TILE_FLOOR)
        {
            game->items[game->item_count].x = rx;
            game->items[game->item_count].y = ry;
            game->items[game->item_count].collected = FALSE;

            /* Random item type */
            gint item_roll = rand () % 10;
            if (item_roll < 5)
            {
                game->items[game->item_count].type = ITEM_HEALTH_POTION;
                game->items[game->item_count].value = POTION_HEAL;
            }
            else if (item_roll < 8)
            {
                game->items[game->item_count].type = ITEM_WEAPON;
                game->items[game->item_count].value = WEAPON_BONUS;
            }
            else
            {
                game->items[game->item_count].type = ITEM_ARMOR;
                game->items[game->item_count].value = ARMOR_BONUS;
            }

            game->item_count++;
        }
    }
}

/*
 * Generate dungeon
 */
static void
generate_dungeon (RoguelikeGame *game)
{
    Room rooms[MAX_ROOMS];
    gint room_count;
    gint i;
    gint x;
    gint y;
    gint room_w;
    gint room_h;
    gint room_x;
    gint room_y;

    /* Fill with walls */
    for (y = 0; y < MAP_HEIGHT; y++)
    {
        for (x = 0; x < MAP_WIDTH; x++)
        {
            game->map.tiles[y][x] = TILE_WALL;
            game->map.visibility[y][x] = 0;
        }
    }

    room_count = 0;

    /* Generate rooms */
    for (i = 0; i < MAX_ROOMS * 3 && room_count < MAX_ROOMS; i++)
    {
        room_w = MIN_ROOM_SIZE + rand () % (MAX_ROOM_SIZE - MIN_ROOM_SIZE + 1);
        room_h = MIN_ROOM_SIZE + rand () % (MAX_ROOM_SIZE - MIN_ROOM_SIZE + 1);
        room_x = 1 + rand () % (MAP_WIDTH - room_w - 2);
        room_y = 1 + rand () % (MAP_HEIGHT - room_h - 2);

        if (!room_overlaps (rooms, room_count, room_x, room_y, room_w, room_h))
        {
            rooms[room_count].x = room_x;
            rooms[room_count].y = room_y;
            rooms[room_count].w = room_w;
            rooms[room_count].h = room_h;

            carve_room (game, room_x, room_y, room_w, room_h);

            if (room_count > 0)
            {
                /* Connect to previous room */
                carve_corridor (game,
                    rooms[room_count - 1].x + rooms[room_count - 1].w / 2,
                    rooms[room_count - 1].y + rooms[room_count - 1].h / 2,
                    room_x + room_w / 2,
                    room_y + room_h / 2);
            }

            room_count++;
        }
    }

    /* Ensure at least 2 rooms */
    if (room_count < 2)
    {
        rooms[0].x = 5;
        rooms[0].y = 5;
        rooms[0].w = 8;
        rooms[0].h = 6;
        carve_room (game, 5, 5, 8, 6);

        rooms[1].x = MAP_WIDTH - 15;
        rooms[1].y = MAP_HEIGHT - 10;
        rooms[1].w = 8;
        rooms[1].h = 6;
        carve_room (game, MAP_WIDTH - 15, MAP_HEIGHT - 10, 8, 6);
        carve_corridor (game, 9, 8, MAP_WIDTH - 11, MAP_HEIGHT - 7);
        room_count = 2;
    }

    /* Place player in first room */
    game->player.x = rooms[0].x + rooms[0].w / 2;
    game->player.y = rooms[0].y + rooms[0].h / 2;
    game->player.world_x = grid_to_world_x (game->player.x);
    game->player.world_z = grid_to_world_z (game->player.y);
    game->player.target_x = game->player.world_x;
    game->player.target_z = game->player.world_z;

    /* Place stairs in last room */
    game->stairs_x = rooms[room_count - 1].x + rooms[room_count - 1].w / 2;
    game->stairs_y = rooms[room_count - 1].y + rooms[room_count - 1].h / 2;
    game->map.tiles[game->stairs_y][game->stairs_x] = TILE_STAIRS;

    /* Place enemies and items */
    place_enemies (game, rooms, room_count);
    place_items (game, rooms, room_count);
}

/*
 * Calculate field of view using raycasting
 */
static void
calculate_fov (RoguelikeGame *game)
{
    gint    x;
    gint    y;
    gint    i;
    gint    px;
    gint    py;
    gint    ray_x;
    gint    ray_y;
    gfloat  angle;
    gfloat  fx;
    gfloat  fy;
    gfloat  step;

    px = game->player.x;
    py = game->player.y;

    /* Mark currently visible as seen */
    for (y = 0; y < MAP_HEIGHT; y++)
    {
        for (x = 0; x < MAP_WIDTH; x++)
        {
            if (game->map.visibility[y][x] == 2)
                game->map.visibility[y][x] = 1;
        }
    }

    /* Player tile always visible */
    game->map.visibility[py][px] = 2;

    /* Cast rays in all directions */
    for (i = 0; i < 360; i += 2)
    {
        angle = (gfloat)i * G_PI / 180.0f;
        fx = (gfloat)px + 0.5f;
        fy = (gfloat)py + 0.5f;

        for (step = 0; step < FOV_RADIUS; step += 0.5f)
        {
            fx += cosf (angle) * 0.5f;
            fy += sinf (angle) * 0.5f;

            ray_x = (gint)fx;
            ray_y = (gint)fy;

            if (ray_x < 0 || ray_x >= MAP_WIDTH ||
                ray_y < 0 || ray_y >= MAP_HEIGHT)
                break;

            game->map.visibility[ray_y][ray_x] = 2;

            /* Stop at walls */
            if (game->map.tiles[ray_y][ray_x] == TILE_WALL)
                break;
        }
    }
}

/*
 * Calculate damage
 */
static gint
calculate_damage (gint attack,
                  gint defense)
{
    gint damage;
    gint variance;

    damage = attack - defense / 2;
    variance = (rand () % 3) - 1;
    damage += variance;

    if (damage < 1)
        damage = 1;

    return damage;
}

/*
 * Start animation for entity movement
 */
static void
start_entity_animation (Entity *entity)
{
    entity->start_x = entity->world_x;
    entity->start_z = entity->world_z;
    entity->target_x = grid_to_world_x (entity->x);
    entity->target_z = grid_to_world_z (entity->y);
}

/*
 * Attack an enemy
 */
static void
attack_enemy (RoguelikeGame *game,
              Entity        *enemy)
{
    gint damage;
    gchar msg[128];

    damage = calculate_damage (game->player.attack + game->weapon_bonus,
                               enemy->defense);
    enemy->hp -= damage;

    g_snprintf (msg, sizeof (msg), "You hit for %d damage!", damage);
    add_message (game, msg, 200, 200, 200);

    if (enemy->hp <= 0)
    {
        enemy->alive = FALSE;
        game->kills++;
        add_message (game, "Enemy defeated!", 100, 255, 100);
    }
}

/*
 * Enemy attacks player
 */
static void
enemy_attack_player (RoguelikeGame *game,
                     Entity        *enemy)
{
    gint damage;
    gchar msg[128];

    damage = calculate_damage (enemy->attack,
                               game->player.defense + game->armor_bonus);
    game->player.hp -= damage;

    g_snprintf (msg, sizeof (msg), "Enemy hits you for %d!", damage);
    add_message (game, msg, 255, 100, 100);

    if (game->player.hp <= 0)
    {
        game->player.hp = 0;
        game->player.alive = FALSE;
        game->state = STATE_DEAD;
        add_message (game, "You have died!", 255, 50, 50);
    }
}

/*
 * Try to move player
 */
static void
try_move_player (RoguelikeGame *game,
                 gint           dx,
                 gint           dy)
{
    gint new_x;
    gint new_y;
    Entity *enemy;

    new_x = game->player.x + dx;
    new_y = game->player.y + dy;

    /* Check for enemy */
    enemy = get_enemy_at (game, new_x, new_y);
    if (enemy != NULL)
    {
        attack_enemy (game, enemy);
        game->state = STATE_ANIMATING;
        game->anim_timer = 0.0f;
        game->current_enemy = 0;
        return;
    }

    /* Check walkable */
    if (!is_walkable (game, new_x, new_y))
        return;

    /* Move player */
    game->player.x = new_x;
    game->player.y = new_y;
    start_entity_animation (&game->player);
    game->state = STATE_ANIMATING;
    game->anim_timer = 0.0f;
    game->current_enemy = 0;

    /* Check for items */
    gint i;
    for (i = 0; i < game->item_count; i++)
    {
        if (!game->items[i].collected &&
            game->items[i].x == new_x &&
            game->items[i].y == new_y)
        {
            if (game->inventory_count < MAX_INVENTORY)
            {
                game->inventory[game->inventory_count] = game->items[i].type;
                game->inventory_count++;
                game->items[i].collected = TRUE;

                gchar msg[128];
                switch (game->items[i].type)
                {
                    case ITEM_HEALTH_POTION:
                        g_snprintf (msg, sizeof (msg), "Picked up Health Potion!");
                        break;
                    case ITEM_WEAPON:
                        g_snprintf (msg, sizeof (msg), "Picked up Weapon (+%d ATK)!", game->items[i].value);
                        break;
                    case ITEM_ARMOR:
                        g_snprintf (msg, sizeof (msg), "Picked up Armor (+%d DEF)!", game->items[i].value);
                        break;
                    default:
                        break;
                }
                add_message (game, msg, 100, 255, 100);
            }
            else
            {
                add_message (game, "Inventory full!", 255, 200, 100);
            }
        }
    }
}

/*
 * Process single enemy turn
 */
static gboolean
process_enemy_turn (RoguelikeGame *game,
                    Entity        *enemy)
{
    gint dx;
    gint dy;
    gint new_x;
    gint new_y;
    gint dist_x;
    gint dist_y;

    if (!enemy->alive)
        return FALSE;

    /* Only act if visible to player (knows player is there) */
    if (game->map.visibility[enemy->y][enemy->x] != 2)
        return FALSE;

    dx = game->player.x - enemy->x;
    dy = game->player.y - enemy->y;
    dist_x = abs (dx);
    dist_y = abs (dy);

    /* Adjacent to player? Attack! */
    if (dist_x + dist_y == 1)
    {
        enemy_attack_player (game, enemy);
        return FALSE;
    }

    /* Move toward player (simple greedy) */
    new_x = enemy->x;
    new_y = enemy->y;

    if (dist_x > dist_y)
    {
        new_x += (dx > 0) ? 1 : -1;
    }
    else if (dist_y > 0)
    {
        new_y += (dy > 0) ? 1 : -1;
    }

    /* Check if valid move */
    if (is_walkable (game, new_x, new_y) &&
        get_enemy_at (game, new_x, new_y) == NULL &&
        !(new_x == game->player.x && new_y == game->player.y))
    {
        enemy->x = new_x;
        enemy->y = new_y;
        start_entity_animation (enemy);
        return TRUE;
    }

    return FALSE;
}

/*
 * Use item from inventory
 */
static void
use_item (RoguelikeGame *game,
          gint           slot)
{
    gint i;
    gchar msg[128];

    if (slot < 0 || slot >= game->inventory_count)
        return;

    switch (game->inventory[slot])
    {
        case ITEM_HEALTH_POTION:
            game->player.hp += POTION_HEAL;
            if (game->player.hp > game->player.max_hp)
                game->player.hp = game->player.max_hp;
            g_snprintf (msg, sizeof (msg), "Healed for %d HP!", POTION_HEAL);
            add_message (game, msg, 100, 255, 100);
            break;

        case ITEM_WEAPON:
            game->weapon_bonus += WEAPON_BONUS;
            g_snprintf (msg, sizeof (msg), "Equipped weapon! (+%d ATK)", WEAPON_BONUS);
            add_message (game, msg, 255, 200, 100);
            break;

        case ITEM_ARMOR:
            game->armor_bonus += ARMOR_BONUS;
            g_snprintf (msg, sizeof (msg), "Equipped armor! (+%d DEF)", ARMOR_BONUS);
            add_message (game, msg, 255, 200, 100);
            break;

        default:
            return;
    }

    /* Remove from inventory */
    for (i = slot; i < game->inventory_count - 1; i++)
    {
        game->inventory[i] = game->inventory[i + 1];
    }
    game->inventory_count--;

    if (game->inv_cursor >= game->inventory_count && game->inv_cursor > 0)
        game->inv_cursor--;
}

/*
 * Descend stairs to next floor
 */
static void
descend_stairs (RoguelikeGame *game)
{
    gchar msg[128];

    game->floor_number++;
    generate_dungeon (game);
    calculate_fov (game);

    g_snprintf (msg, sizeof (msg), "Descended to floor %d...", game->floor_number);
    add_message (game, msg, 255, 255, 100);
}

/*
 * Initialize game
 */
static void
roguelike_init (RoguelikeGame *game)
{
    srand ((unsigned int)time (NULL));

    /* Reset stats */
    game->floor_number = 1;
    game->kills = 0;
    game->weapon_bonus = 0;
    game->armor_bonus = 0;
    game->inventory_count = 0;
    game->inv_cursor = 0;
    game->message_count = 0;

    /* Player stats */
    game->player.max_hp = PLAYER_BASE_HP;
    game->player.hp = game->player.max_hp;
    game->player.attack = PLAYER_BASE_ATK;
    game->player.defense = PLAYER_BASE_DEF;
    game->player.alive = TRUE;

    /* State */
    game->state = STATE_PLAYING;
    game->anim_timer = 0.0f;
    game->bob_timer = 0.0f;

    /* Generate first dungeon */
    generate_dungeon (game);
    calculate_fov (game);

    add_message (game, "Welcome to the dungeon!", 255, 255, 255);
    add_message (game, "Find the stairs to descend.", 200, 200, 200);
}

/*
 * Update animations
 */
static void
update_animations (RoguelikeGame *game,
                   gfloat         dt)
{
    gfloat t;
    gint i;

    game->anim_timer += dt;
    t = game->anim_timer / ANIM_DURATION;

    if (t > 1.0f)
        t = 1.0f;

    /* Interpolate player */
    game->player.world_x = lerpf (game->player.start_x, game->player.target_x, t);
    game->player.world_z = lerpf (game->player.start_z, game->player.target_z, t);

    /* Interpolate enemies */
    for (i = 0; i < game->enemy_count; i++)
    {
        if (game->enemies[i].alive)
        {
            game->enemies[i].world_x = lerpf (game->enemies[i].start_x,
                                               game->enemies[i].target_x, t);
            game->enemies[i].world_z = lerpf (game->enemies[i].start_z,
                                               game->enemies[i].target_z, t);
        }
    }

    /* Animation complete */
    if (t >= 1.0f)
    {
        game->anim_timer = 0.0f;

        /* Snap to final positions */
        game->player.world_x = game->player.target_x;
        game->player.world_z = game->player.target_z;
        game->player.start_x = game->player.world_x;
        game->player.start_z = game->player.world_z;

        for (i = 0; i < game->enemy_count; i++)
        {
            game->enemies[i].world_x = game->enemies[i].target_x;
            game->enemies[i].world_z = game->enemies[i].target_z;
            game->enemies[i].start_x = game->enemies[i].world_x;
            game->enemies[i].start_z = game->enemies[i].world_z;
        }

        /* Switch to enemy turn processing */
        game->state = STATE_ENEMY_TURN;
    }
}

/*
 * Process enemy turns
 */
static void
update_enemy_turns (RoguelikeGame *game,
                    gfloat         dt)
{
    gboolean moved;

    if (game->player.hp <= 0)
    {
        game->state = STATE_DEAD;
        return;
    }

    /* Process next enemy */
    moved = FALSE;
    while (game->current_enemy < game->enemy_count && !moved)
    {
        moved = process_enemy_turn (game, &game->enemies[game->current_enemy]);
        game->current_enemy++;
    }

    if (moved)
    {
        /* Animate enemy movement */
        game->state = STATE_ANIMATING;
        game->anim_timer = 0.0f;
    }
    else if (game->current_enemy >= game->enemy_count)
    {
        /* All enemies done, back to player turn */
        calculate_fov (game);
        game->state = STATE_PLAYING;
    }
}

/*
 * Handle input
 */
static void
handle_input (RoguelikeGame *game)
{
    gint dx;
    gint dy;

    switch (game->state)
    {
        case STATE_PLAYING:
            dx = 0;
            dy = 0;

            if (grl_input_is_key_pressed (GRL_KEY_W) ||
                grl_input_is_key_pressed (GRL_KEY_UP))
                dy = -1;
            else if (grl_input_is_key_pressed (GRL_KEY_S) ||
                     grl_input_is_key_pressed (GRL_KEY_DOWN))
                dy = 1;
            else if (grl_input_is_key_pressed (GRL_KEY_A) ||
                     grl_input_is_key_pressed (GRL_KEY_LEFT))
                dx = -1;
            else if (grl_input_is_key_pressed (GRL_KEY_D) ||
                     grl_input_is_key_pressed (GRL_KEY_RIGHT))
                dx = 1;
            else if (grl_input_is_key_pressed (GRL_KEY_I))
            {
                game->state = STATE_INVENTORY;
                game->inv_cursor = 0;
            }
            else if (grl_input_is_key_pressed (GRL_KEY_PERIOD) ||
                     grl_input_is_key_pressed (GRL_KEY_KP_DECIMAL))
            {
                if (game->player.x == game->stairs_x &&
                    game->player.y == game->stairs_y)
                {
                    game->state = STATE_STAIRS_PROMPT;
                }
            }
            else if (grl_input_is_key_pressed (GRL_KEY_SPACE))
            {
                /* Wait a turn */
                game->state = STATE_ANIMATING;
                game->anim_timer = ANIM_DURATION;
                game->current_enemy = 0;
            }

            if (dx != 0 || dy != 0)
            {
                try_move_player (game, dx, dy);
            }
            break;

        case STATE_ANIMATING:
        case STATE_ENEMY_TURN:
            /* Block input during animation */
            break;

        case STATE_INVENTORY:
            if (grl_input_is_key_pressed (GRL_KEY_ESCAPE) ||
                grl_input_is_key_pressed (GRL_KEY_I))
            {
                game->state = STATE_PLAYING;
            }
            else if (grl_input_is_key_pressed (GRL_KEY_W) ||
                     grl_input_is_key_pressed (GRL_KEY_UP))
            {
                if (game->inv_cursor > 0)
                    game->inv_cursor--;
            }
            else if (grl_input_is_key_pressed (GRL_KEY_S) ||
                     grl_input_is_key_pressed (GRL_KEY_DOWN))
            {
                if (game->inv_cursor < game->inventory_count - 1)
                    game->inv_cursor++;
            }
            else if (grl_input_is_key_pressed (GRL_KEY_ENTER) ||
                     grl_input_is_key_pressed (GRL_KEY_SPACE))
            {
                if (game->inventory_count > 0)
                {
                    use_item (game, game->inv_cursor);
                }
            }
            break;

        case STATE_STAIRS_PROMPT:
            if (grl_input_is_key_pressed (GRL_KEY_Y) ||
                grl_input_is_key_pressed (GRL_KEY_ENTER))
            {
                descend_stairs (game);
                game->state = STATE_PLAYING;
            }
            else if (grl_input_is_key_pressed (GRL_KEY_N) ||
                     grl_input_is_key_pressed (GRL_KEY_ESCAPE))
            {
                game->state = STATE_PLAYING;
            }
            break;

        case STATE_DEAD:
            if (grl_input_is_key_pressed (GRL_KEY_SPACE) ||
                grl_input_is_key_pressed (GRL_KEY_ENTER))
            {
                roguelike_init (game);
            }
            break;
    }
}

/*
 * Draw tile
 */
static void
draw_tile (TileType tile,
           gfloat   world_x,
           gfloat   world_z,
           gboolean visible)
{
    g_autoptr(GrlVector3) pos = NULL;
    g_autoptr(GrlColor) color = NULL;
    gfloat brightness;

    brightness = visible ? 1.0f : 0.4f;

    switch (tile)
    {
        case TILE_FLOOR:
            pos = grl_vector3_new (world_x, -0.05f, world_z);
            color = grl_color_new ((guint8)(60 * brightness),
                                   (guint8)(50 * brightness),
                                   (guint8)(40 * brightness), 255);
            grl_draw_cube (pos, CELL_SIZE * 0.95f, 0.1f, CELL_SIZE * 0.95f, color);
            break;

        case TILE_WALL:
            pos = grl_vector3_new (world_x, 0.5f, world_z);
            color = grl_color_new ((guint8)(100 * brightness),
                                   (guint8)(100 * brightness),
                                   (guint8)(110 * brightness), 255);
            grl_draw_cube (pos, CELL_SIZE * 0.98f, 1.0f, CELL_SIZE * 0.98f, color);
            break;

        case TILE_STAIRS:
            pos = grl_vector3_new (world_x, 0.02f, world_z);
            color = grl_color_new ((guint8)(220 * brightness),
                                   (guint8)(200 * brightness),
                                   (guint8)(50 * brightness), 255);
            grl_draw_cube (pos, CELL_SIZE * 0.8f, 0.05f, CELL_SIZE * 0.8f, color);
            break;

        default:
            break;
    }
}

/*
 * Draw 3D health bar above entity
 */
static void
draw_health_bar_3d (gfloat world_x,
                    gfloat world_y,
                    gfloat world_z,
                    gint   hp,
                    gint   max_hp)
{
    g_autoptr(GrlVector3) bg_pos = NULL;
    g_autoptr(GrlVector3) fill_pos = NULL;
    g_autoptr(GrlColor) bg_color = NULL;
    g_autoptr(GrlColor) fill_color = NULL;
    gfloat bar_width;
    gfloat fill_width;
    gfloat ratio;

    if (hp >= max_hp)
        return;

    bar_width = 0.8f;
    ratio = (gfloat)hp / (gfloat)max_hp;
    fill_width = bar_width * ratio;

    /* Background */
    bg_pos = grl_vector3_new (world_x, world_y, world_z);
    bg_color = grl_color_new (50, 50, 50, 200);
    grl_draw_cube (bg_pos, bar_width, 0.08f, 0.05f, bg_color);

    /* Fill */
    if (hp > 0)
    {
        fill_pos = grl_vector3_new (world_x - (bar_width - fill_width) / 2.0f,
                                    world_y, world_z - 0.01f);
        if (ratio > 0.3f)
            fill_color = grl_color_new (50, 200, 50, 255);
        else
            fill_color = grl_color_new (200, 50, 50, 255);
        grl_draw_cube (fill_pos, fill_width, 0.1f, 0.06f, fill_color);
    }
}

/*
 * Draw 3D scene
 */
static void
draw_3d_scene (RoguelikeGame *game,
               GrlCamera3D   *camera)
{
    gint x;
    gint y;
    gint i;
    gfloat world_x;
    gfloat world_z;
    gfloat bob;
    g_autoptr(GrlVector3) pos = NULL;
    g_autoptr(GrlColor) player_color = NULL;
    g_autoptr(GrlColor) enemy_color = NULL;
    g_autoptr(GrlColor) item_color = NULL;
    gfloat radius;

    /* Update camera to follow player */
    grl_camera3d_set_position_xyz (camera,
        game->player.world_x, CAM_HEIGHT, game->player.world_z + CAM_DISTANCE);
    grl_camera3d_set_target_xyz (camera,
        game->player.world_x, 0.0f, game->player.world_z);

    grl_camera3d_begin (camera);

    /* Draw tiles */
    for (y = 0; y < MAP_HEIGHT; y++)
    {
        for (x = 0; x < MAP_WIDTH; x++)
        {
            if (game->map.visibility[y][x] == 0)
                continue;

            world_x = grid_to_world_x (x);
            world_z = grid_to_world_z (y);

            draw_tile (game->map.tiles[y][x], world_x, world_z,
                       game->map.visibility[y][x] == 2);
        }
    }

    /* Draw items (only visible) */
    bob = sinf (game->bob_timer * 3.0f) * 0.15f;
    item_color = grl_color_new (80, 220, 80, 255);

    for (i = 0; i < game->item_count; i++)
    {
        if (!game->items[i].collected &&
            game->map.visibility[game->items[i].y][game->items[i].x] == 2)
        {
            world_x = grid_to_world_x (game->items[i].x);
            world_z = grid_to_world_z (game->items[i].y);

            g_clear_pointer (&pos, grl_vector3_free);
            pos = grl_vector3_new (world_x, 0.3f + bob, world_z);
            grl_draw_sphere (pos, 0.2f, item_color);
        }
    }

    /* Draw enemies (only visible) */
    enemy_color = grl_color_new (200, 60, 60, 255);

    for (i = 0; i < game->enemy_count; i++)
    {
        if (game->enemies[i].alive &&
            game->map.visibility[game->enemies[i].y][game->enemies[i].x] == 2)
        {
            g_clear_pointer (&pos, grl_vector3_free);
            pos = grl_vector3_new (game->enemies[i].world_x,
                                   0.35f,
                                   game->enemies[i].world_z);
            grl_draw_cube (pos, 0.6f, 0.7f, 0.6f, enemy_color);

            /* Health bar */
            draw_health_bar_3d (game->enemies[i].world_x,
                                0.9f,
                                game->enemies[i].world_z,
                                game->enemies[i].hp,
                                game->enemies[i].max_hp);
        }
    }

    /* Draw player */
    radius = 0.4f;
    player_color = grl_color_new (80, 120, 220, 255);
    g_clear_pointer (&pos, grl_vector3_free);
    pos = grl_vector3_new (game->player.world_x, radius, game->player.world_z);
    grl_draw_sphere (pos, radius, player_color);

    /* Player health bar */
    draw_health_bar_3d (game->player.world_x,
                        radius * 2 + 0.2f,
                        game->player.world_z,
                        game->player.hp,
                        game->player.max_hp);

    grl_camera3d_end (camera);
}

/*
 * Draw UI
 */
static void
draw_ui (RoguelikeGame *game)
{
    g_autoptr(GrlColor) white = grl_color_new (255, 255, 255, 255);
    g_autoptr(GrlColor) gray = grl_color_new (150, 150, 150, 255);
    g_autoptr(GrlColor) yellow = grl_color_new (255, 220, 50, 255);
    g_autoptr(GrlColor) hp_bg = grl_color_new (60, 20, 20, 255);
    g_autoptr(GrlColor) hp_fg = NULL;
    g_autoptr(GrlColor) overlay_bg = grl_color_new (0, 0, 0, 180);
    gchar text[128];
    gint i;
    gint y_offset;
    gint hp_width;
    gint text_width;
    gfloat hp_ratio;

    /* Top bar */
    g_snprintf (text, sizeof (text), "Floor: %d", game->floor_number);
    grl_draw_text (text, 20, 20, 24, white);

    g_snprintf (text, sizeof (text), "Kills: %d", game->kills);
    grl_draw_text (text, 20, 50, 18, gray);

    /* HP */
    g_snprintf (text, sizeof (text), "HP: %d/%d",
                game->player.hp, game->player.max_hp);
    grl_draw_text (text, 180, 20, 24, white);

    /* HP bar */
    grl_draw_rectangle (180, 50, 200, 16, hp_bg);
    hp_ratio = (gfloat)game->player.hp / (gfloat)game->player.max_hp;
    hp_width = (gint)(200.0f * hp_ratio);
    if (hp_width > 0)
    {
        if (hp_ratio > 0.3f)
            hp_fg = grl_color_new (50, 180, 50, 255);
        else
            hp_fg = grl_color_new (180, 50, 50, 255);
        grl_draw_rectangle (180, 50, hp_width, 16, hp_fg);
    }

    /* Stats */
    g_snprintf (text, sizeof (text), "ATK: %d  DEF: %d",
                game->player.attack + game->weapon_bonus,
                game->player.defense + game->armor_bonus);
    grl_draw_text (text, 400, 20, 18, gray);

    /* Message log */
    y_offset = WINDOW_HEIGHT - 30 - game->message_count * 20;
    for (i = 0; i < game->message_count; i++)
    {
        g_autoptr(GrlColor) msg_color = grl_color_new (
            game->messages[i].r,
            game->messages[i].g,
            game->messages[i].b, 255);
        grl_draw_text (game->messages[i].text, 20, y_offset, 16, msg_color);
        y_offset += 20;
    }

    /* Controls */
    grl_draw_text ("WASD: Move  SPACE: Wait  I: Inventory  >: Stairs",
                   20, WINDOW_HEIGHT - 25, 14, gray);

    /* Inventory overlay */
    if (game->state == STATE_INVENTORY)
    {
        grl_draw_rectangle (WINDOW_WIDTH / 2 - 200, 100, 400, 400, overlay_bg);
        grl_draw_text ("INVENTORY", WINDOW_WIDTH / 2 - 60, 120, 28, yellow);
        grl_draw_text ("ENTER to use, ESC to close", WINDOW_WIDTH / 2 - 100, 155, 14, gray);

        if (game->inventory_count == 0)
        {
            grl_draw_text ("(empty)", WINDOW_WIDTH / 2 - 30, 200, 18, gray);
        }
        else
        {
            y_offset = 200;
            for (i = 0; i < game->inventory_count; i++)
            {
                g_autoptr(GrlColor) item_color = NULL;
                const gchar *item_name = "";

                switch (game->inventory[i])
                {
                    case ITEM_HEALTH_POTION:
                        item_name = "Health Potion";
                        item_color = grl_color_new (255, 100, 100, 255);
                        break;
                    case ITEM_WEAPON:
                        item_name = "Weapon";
                        item_color = grl_color_new (200, 200, 100, 255);
                        break;
                    case ITEM_ARMOR:
                        item_name = "Armor";
                        item_color = grl_color_new (100, 150, 255, 255);
                        break;
                    default:
                        break;
                }

                if (i == game->inv_cursor)
                {
                    grl_draw_text (">", WINDOW_WIDTH / 2 - 100, y_offset, 20, yellow);
                }
                grl_draw_text (item_name, WINDOW_WIDTH / 2 - 80, y_offset, 20, item_color);
                y_offset += 28;
            }
        }
    }

    /* Stairs prompt */
    if (game->state == STATE_STAIRS_PROMPT)
    {
        grl_draw_rectangle (WINDOW_WIDTH / 2 - 180, WINDOW_HEIGHT / 2 - 60,
                            360, 120, overlay_bg);
        g_snprintf (text, sizeof (text), "Descend to floor %d?",
                    game->floor_number + 1);
        text_width = grl_measure_text (text, 24);
        grl_draw_text (text, WINDOW_WIDTH / 2 - text_width / 2,
                       WINDOW_HEIGHT / 2 - 30, 24, yellow);
        grl_draw_text ("Y: Yes   N: No", WINDOW_WIDTH / 2 - 60,
                       WINDOW_HEIGHT / 2 + 20, 18, white);
    }

    /* Game over */
    if (game->state == STATE_DEAD)
    {
        grl_draw_rectangle (0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, overlay_bg);

        text_width = grl_measure_text ("GAME OVER", 60);
        grl_draw_text ("GAME OVER", WINDOW_WIDTH / 2 - text_width / 2,
                       WINDOW_HEIGHT / 2 - 100, 60, yellow);

        g_snprintf (text, sizeof (text), "Floor: %d   Kills: %d",
                    game->floor_number, game->kills);
        text_width = grl_measure_text (text, 30);
        grl_draw_text (text, WINDOW_WIDTH / 2 - text_width / 2,
                       WINDOW_HEIGHT / 2 - 20, 30, white);

        grl_draw_text ("Press SPACE to restart",
                       WINDOW_WIDTH / 2 - 100, WINDOW_HEIGHT / 2 + 40, 18, gray);
    }
}

int
main (int    argc,
      char **argv)
{
    g_autoptr(GrlApplication) app = NULL;
    g_autoptr(GrlWindow) window = NULL;
    g_autoptr(GrlCamera3D) camera = NULL;
    g_autoptr(GrlColor) bg_color = NULL;
    RoguelikeGame game;
    gfloat dt;

    app = grl_application_new ("com.example.roguelike3d");
    window = grl_window_new (WINDOW_WIDTH, WINDOW_HEIGHT, "3D Roguelike");
    bg_color = grl_color_new (20, 20, 30, 255);

    /* Setup camera */
    camera = grl_camera3d_new ();
    grl_camera3d_set_fovy (camera, 45.0f);

    grl_window_set_target_fps (window, 60);

    roguelike_init (&game);

    while (!grl_window_should_close (window))
    {
        dt = grl_window_get_frame_time (window);
        grl_window_poll_input (window);

        /* Update bob timer */
        game.bob_timer += dt;

        /* Handle input */
        handle_input (&game);

        /* Update based on state */
        switch (game.state)
        {
            case STATE_ANIMATING:
                update_animations (&game, dt);
                break;

            case STATE_ENEMY_TURN:
                update_enemy_turns (&game, dt);
                break;

            default:
                break;
        }

        /* Draw */
        grl_window_begin_drawing (window);
        grl_draw_clear_background (bg_color);

        draw_3d_scene (&game, camera);
        draw_ui (&game);

        grl_window_end_drawing (window);
    }

    return 0;
}
