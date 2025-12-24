/* asteroids3d.c
 *
 * Copyright 2025 Zach Podbielniak
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Classic Asteroids clone in 3D demonstrating rotation-based movement,
 * bullet pooling, entity splitting, screen wrapping, particle effects,
 * and multi-entity collision detection.
 */

#include <graylib.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

/* Window dimensions */
#define WINDOW_WIDTH      1280
#define WINDOW_HEIGHT     900

/* World bounds (centered at origin) */
#define WORLD_HALF_WIDTH  20.0f
#define WORLD_HALF_HEIGHT 14.0f

/* Ship settings */
#define SHIP_SIZE         0.8f
#define SHIP_THRUST       15.0f
#define SHIP_DRAG         0.98f
#define SHIP_ROTATE_SPEED 4.0f
#define SHIP_MAX_SPEED    12.0f
#define INVULN_TIME       2.0f
#define STARTING_LIVES    3

/* Bullet settings */
#define MAX_BULLETS       20
#define BULLET_SPEED      25.0f
#define BULLET_LIFETIME   1.5f
#define BULLET_SIZE       0.15f
#define FIRE_COOLDOWN     0.15f

/* Asteroid settings */
#define MAX_ASTEROIDS     40
#define ASTEROID_LARGE    2.5f
#define ASTEROID_MEDIUM   1.2f
#define ASTEROID_SMALL    0.6f
#define ASTEROID_SPEED_MIN 2.0f
#define ASTEROID_SPEED_MAX 6.0f
#define INITIAL_ASTEROIDS 4
#define SPAWN_MARGIN      5.0f

/* Particle settings */
#define MAX_PARTICLES     150
#define PARTICLE_LIFETIME 0.6f
#define PARTICLE_SPEED    8.0f
#define PARTICLES_PER_EXPLOSION 15

/* Scoring */
#define SCORE_LARGE       20
#define SCORE_MEDIUM      50
#define SCORE_SMALL       100

/* Camera */
#define CAM_HEIGHT        45.0f

/* Asteroid size enum */
typedef enum
{
    SIZE_LARGE,
    SIZE_MEDIUM,
    SIZE_SMALL
} AsteroidSize;

/* Bullet entity */
typedef struct
{
    gfloat x, y;
    gfloat vx, vy;
    gfloat lifetime;
    gboolean active;
} Bullet;

/* Asteroid entity */
typedef struct
{
    gfloat x, y;
    gfloat vx, vy;
    gfloat rotation;
    gfloat rotation_speed;
    AsteroidSize size;
    gboolean active;
} Asteroid;

/* Particle entity */
typedef struct
{
    gfloat x, y;
    gfloat vx, vy;
    gfloat lifetime;
    gboolean active;
} Particle;

/* Game state */
typedef struct
{
    /* Ship */
    gfloat ship_x, ship_y;
    gfloat ship_vx, ship_vy;
    gfloat ship_angle;
    gfloat fire_cooldown;
    gfloat invuln_timer;
    gint lives;

    /* Entity pools */
    Bullet bullets[MAX_BULLETS];
    Asteroid asteroids[MAX_ASTEROIDS];
    Particle particles[MAX_PARTICLES];

    /* Game state */
    gint score;
    gint level;
    gboolean paused;
    gboolean game_over;
} AsteroidsGame;

/*
 * Get radius for asteroid size.
 */
static gfloat
get_asteroid_radius (AsteroidSize size)
{
    switch (size)
    {
    case SIZE_LARGE:
        return ASTEROID_LARGE;
    case SIZE_MEDIUM:
        return ASTEROID_MEDIUM;
    case SIZE_SMALL:
        return ASTEROID_SMALL;
    default:
        return ASTEROID_SMALL;
    }
}

/*
 * Wrap position to stay within world bounds.
 */
static void
wrap_position (gfloat *x,
               gfloat *y)
{
    if (*x < -WORLD_HALF_WIDTH)
        *x += WORLD_HALF_WIDTH * 2.0f;
    else if (*x > WORLD_HALF_WIDTH)
        *x -= WORLD_HALF_WIDTH * 2.0f;

    if (*y < -WORLD_HALF_HEIGHT)
        *y += WORLD_HALF_HEIGHT * 2.0f;
    else if (*y > WORLD_HALF_HEIGHT)
        *y -= WORLD_HALF_HEIGHT * 2.0f;
}

/*
 * Check circle-circle collision.
 */
static gboolean
check_collision (gfloat x1,
                 gfloat y1,
                 gfloat r1,
                 gfloat x2,
                 gfloat y2,
                 gfloat r2)
{
    gfloat dx;
    gfloat dy;
    gfloat dist_sq;
    gfloat radius_sum;

    dx = x2 - x1;
    dy = y2 - y1;
    dist_sq = dx * dx + dy * dy;
    radius_sum = r1 + r2;

    return dist_sq < radius_sum * radius_sum;
}

/*
 * Spawn explosion particles at position.
 */
static void
spawn_explosion (AsteroidsGame *game,
                 gfloat         x,
                 gfloat         y,
                 gint           count)
{
    gint i;
    gint spawned;
    gfloat angle;
    gfloat speed;

    spawned = 0;
    for (i = 0; i < MAX_PARTICLES && spawned < count; i++)
    {
        if (!game->particles[i].active)
        {
            angle = ((gfloat)rand () / (gfloat)RAND_MAX) * 2.0f * G_PI;
            speed = ((gfloat)rand () / (gfloat)RAND_MAX) * PARTICLE_SPEED;

            game->particles[i].x = x;
            game->particles[i].y = y;
            game->particles[i].vx = cosf (angle) * speed;
            game->particles[i].vy = sinf (angle) * speed;
            game->particles[i].lifetime = PARTICLE_LIFETIME;
            game->particles[i].active = TRUE;

            spawned++;
        }
    }
}

/*
 * Spawn asteroid at position with given size.
 */
static void
spawn_asteroid_at (AsteroidsGame *game,
                   gfloat         x,
                   gfloat         y,
                   AsteroidSize   size)
{
    gint i;
    gfloat angle;
    gfloat speed;

    for (i = 0; i < MAX_ASTEROIDS; i++)
    {
        if (!game->asteroids[i].active)
        {
            angle = ((gfloat)rand () / (gfloat)RAND_MAX) * 2.0f * G_PI;
            speed = ASTEROID_SPEED_MIN +
                    ((gfloat)rand () / (gfloat)RAND_MAX) *
                    (ASTEROID_SPEED_MAX - ASTEROID_SPEED_MIN);

            /* Smaller asteroids move faster */
            if (size == SIZE_MEDIUM)
                speed *= 1.3f;
            else if (size == SIZE_SMALL)
                speed *= 1.6f;

            game->asteroids[i].x = x;
            game->asteroids[i].y = y;
            game->asteroids[i].vx = cosf (angle) * speed;
            game->asteroids[i].vy = sinf (angle) * speed;
            game->asteroids[i].rotation = 0.0f;
            game->asteroids[i].rotation_speed =
                (((gfloat)rand () / (gfloat)RAND_MAX) - 0.5f) * 4.0f;
            game->asteroids[i].size = size;
            game->asteroids[i].active = TRUE;

            return;
        }
    }
}

/*
 * Spawn asteroid away from ship.
 */
static void
spawn_asteroid_away_from_ship (AsteroidsGame *game,
                               AsteroidSize   size)
{
    gfloat x, y;
    gfloat dx, dy;
    gfloat dist;
    gint attempts;

    attempts = 0;
    do
    {
        x = ((gfloat)rand () / (gfloat)RAND_MAX) * WORLD_HALF_WIDTH * 2.0f
            - WORLD_HALF_WIDTH;
        y = ((gfloat)rand () / (gfloat)RAND_MAX) * WORLD_HALF_HEIGHT * 2.0f
            - WORLD_HALF_HEIGHT;

        dx = x - game->ship_x;
        dy = y - game->ship_y;
        dist = sqrtf (dx * dx + dy * dy);
        attempts++;
    }
    while (dist < SPAWN_MARGIN && attempts < 50);

    spawn_asteroid_at (game, x, y, size);
}

/*
 * Split asteroid into smaller pieces.
 */
static void
split_asteroid (AsteroidsGame *game,
                gint           index)
{
    Asteroid *ast;
    AsteroidSize new_size;

    ast = &game->asteroids[index];

    /* Spawn explosion */
    spawn_explosion (game, ast->x, ast->y, PARTICLES_PER_EXPLOSION);

    /* Add score */
    switch (ast->size)
    {
    case SIZE_LARGE:
        game->score += SCORE_LARGE;
        break;
    case SIZE_MEDIUM:
        game->score += SCORE_MEDIUM;
        break;
    case SIZE_SMALL:
        game->score += SCORE_SMALL;
        break;
    }

    /* Spawn smaller asteroids */
    if (ast->size != SIZE_SMALL)
    {
        new_size = (ast->size == SIZE_LARGE) ? SIZE_MEDIUM : SIZE_SMALL;
        spawn_asteroid_at (game, ast->x, ast->y, new_size);
        spawn_asteroid_at (game, ast->x, ast->y, new_size);
    }

    ast->active = FALSE;
}

/*
 * Fire bullet from ship.
 */
static void
fire_bullet (AsteroidsGame *game)
{
    gint i;

    if (game->fire_cooldown > 0.0f)
        return;

    for (i = 0; i < MAX_BULLETS; i++)
    {
        if (!game->bullets[i].active)
        {
            game->bullets[i].x = game->ship_x +
                                 cosf (game->ship_angle) * SHIP_SIZE;
            game->bullets[i].y = game->ship_y +
                                 sinf (game->ship_angle) * SHIP_SIZE;
            game->bullets[i].vx = cosf (game->ship_angle) * BULLET_SPEED +
                                  game->ship_vx * 0.5f;
            game->bullets[i].vy = sinf (game->ship_angle) * BULLET_SPEED +
                                  game->ship_vy * 0.5f;
            game->bullets[i].lifetime = BULLET_LIFETIME;
            game->bullets[i].active = TRUE;

            game->fire_cooldown = FIRE_COOLDOWN;
            return;
        }
    }
}

/*
 * Count active asteroids.
 */
static gint
count_active_asteroids (AsteroidsGame *game)
{
    gint i;
    gint count;

    count = 0;
    for (i = 0; i < MAX_ASTEROIDS; i++)
    {
        if (game->asteroids[i].active)
            count++;
    }

    return count;
}

/*
 * Start next level.
 */
static void
start_next_level (AsteroidsGame *game)
{
    gint i;
    gint num_asteroids;

    game->level++;
    num_asteroids = INITIAL_ASTEROIDS + game->level - 1;

    if (num_asteroids > 10)
        num_asteroids = 10;

    for (i = 0; i < num_asteroids; i++)
    {
        spawn_asteroid_away_from_ship (game, SIZE_LARGE);
    }
}

/*
 * Respawn ship at center.
 */
static void
respawn_ship (AsteroidsGame *game)
{
    game->ship_x = 0.0f;
    game->ship_y = 0.0f;
    game->ship_vx = 0.0f;
    game->ship_vy = 0.0f;
    game->ship_angle = G_PI / 2.0f;  /* Point up */
    game->invuln_timer = INVULN_TIME;
}

/*
 * Initialize game state.
 */
static void
asteroids_init (AsteroidsGame *game)
{
    gint i;

    srand ((unsigned int)time (NULL));

    /* Reset ship */
    game->ship_x = 0.0f;
    game->ship_y = 0.0f;
    game->ship_vx = 0.0f;
    game->ship_vy = 0.0f;
    game->ship_angle = G_PI / 2.0f;  /* Point up */
    game->fire_cooldown = 0.0f;
    game->invuln_timer = INVULN_TIME;
    game->lives = STARTING_LIVES;

    /* Clear bullets */
    for (i = 0; i < MAX_BULLETS; i++)
    {
        game->bullets[i].active = FALSE;
    }

    /* Clear asteroids */
    for (i = 0; i < MAX_ASTEROIDS; i++)
    {
        game->asteroids[i].active = FALSE;
    }

    /* Clear particles */
    for (i = 0; i < MAX_PARTICLES; i++)
    {
        game->particles[i].active = FALSE;
    }

    /* Reset state */
    game->score = 0;
    game->level = 0;
    game->paused = FALSE;
    game->game_over = FALSE;

    /* Start first level */
    start_next_level (game);
}

/*
 * Update game logic.
 */
static void
asteroids_update (AsteroidsGame *game,
                  gfloat         dt)
{
    gint i, j;
    gfloat speed;
    Bullet *bullet;
    Asteroid *asteroid;
    Particle *particle;

    if (game->paused || game->game_over)
        return;

    /* Update fire cooldown */
    if (game->fire_cooldown > 0.0f)
        game->fire_cooldown -= dt;

    /* Update invulnerability */
    if (game->invuln_timer > 0.0f)
        game->invuln_timer -= dt;

    /* Ship rotation */
    if (grl_input_is_key_down (GRL_KEY_A) ||
        grl_input_is_key_down (GRL_KEY_LEFT))
    {
        game->ship_angle += SHIP_ROTATE_SPEED * dt;
    }
    if (grl_input_is_key_down (GRL_KEY_D) ||
        grl_input_is_key_down (GRL_KEY_RIGHT))
    {
        game->ship_angle -= SHIP_ROTATE_SPEED * dt;
    }

    /* Ship thrust */
    if (grl_input_is_key_down (GRL_KEY_W) ||
        grl_input_is_key_down (GRL_KEY_UP))
    {
        game->ship_vx += cosf (game->ship_angle) * SHIP_THRUST * dt;
        game->ship_vy += sinf (game->ship_angle) * SHIP_THRUST * dt;
    }

    /* Apply drag */
    game->ship_vx *= SHIP_DRAG;
    game->ship_vy *= SHIP_DRAG;

    /* Clamp speed */
    speed = sqrtf (game->ship_vx * game->ship_vx +
                   game->ship_vy * game->ship_vy);
    if (speed > SHIP_MAX_SPEED)
    {
        game->ship_vx = (game->ship_vx / speed) * SHIP_MAX_SPEED;
        game->ship_vy = (game->ship_vy / speed) * SHIP_MAX_SPEED;
    }

    /* Update ship position */
    game->ship_x += game->ship_vx * dt;
    game->ship_y += game->ship_vy * dt;
    wrap_position (&game->ship_x, &game->ship_y);

    /* Fire bullet */
    if (grl_input_is_key_pressed (GRL_KEY_SPACE))
    {
        fire_bullet (game);
    }

    /* Update bullets */
    for (i = 0; i < MAX_BULLETS; i++)
    {
        bullet = &game->bullets[i];
        if (!bullet->active)
            continue;

        bullet->x += bullet->vx * dt;
        bullet->y += bullet->vy * dt;
        bullet->lifetime -= dt;

        wrap_position (&bullet->x, &bullet->y);

        if (bullet->lifetime <= 0.0f)
            bullet->active = FALSE;
    }

    /* Update asteroids */
    for (i = 0; i < MAX_ASTEROIDS; i++)
    {
        asteroid = &game->asteroids[i];
        if (!asteroid->active)
            continue;

        asteroid->x += asteroid->vx * dt;
        asteroid->y += asteroid->vy * dt;
        asteroid->rotation += asteroid->rotation_speed * dt;

        wrap_position (&asteroid->x, &asteroid->y);
    }

    /* Update particles */
    for (i = 0; i < MAX_PARTICLES; i++)
    {
        particle = &game->particles[i];
        if (!particle->active)
            continue;

        particle->x += particle->vx * dt;
        particle->y += particle->vy * dt;
        particle->lifetime -= dt;

        if (particle->lifetime <= 0.0f)
            particle->active = FALSE;
    }

    /* Check bullet-asteroid collisions */
    for (i = 0; i < MAX_BULLETS; i++)
    {
        bullet = &game->bullets[i];
        if (!bullet->active)
            continue;

        for (j = 0; j < MAX_ASTEROIDS; j++)
        {
            asteroid = &game->asteroids[j];
            if (!asteroid->active)
                continue;

            if (check_collision (bullet->x, bullet->y, BULLET_SIZE,
                                 asteroid->x, asteroid->y,
                                 get_asteroid_radius (asteroid->size)))
            {
                bullet->active = FALSE;
                split_asteroid (game, j);
                break;
            }
        }
    }

    /* Check ship-asteroid collisions (if not invulnerable) */
    if (game->invuln_timer <= 0.0f)
    {
        for (i = 0; i < MAX_ASTEROIDS; i++)
        {
            asteroid = &game->asteroids[i];
            if (!asteroid->active)
                continue;

            if (check_collision (game->ship_x, game->ship_y, SHIP_SIZE * 0.5f,
                                 asteroid->x, asteroid->y,
                                 get_asteroid_radius (asteroid->size)))
            {
                /* Ship destroyed */
                spawn_explosion (game, game->ship_x, game->ship_y,
                                 PARTICLES_PER_EXPLOSION * 2);
                game->lives--;

                if (game->lives <= 0)
                {
                    game->game_over = TRUE;
                }
                else
                {
                    respawn_ship (game);
                }
                break;
            }
        }
    }

    /* Check level complete */
    if (count_active_asteroids (game) == 0)
    {
        start_next_level (game);
    }
}

/*
 * Draw 3D scene.
 */
static void
asteroids_draw_3d (AsteroidsGame *game,
                   GrlCamera3D   *camera)
{
    g_autoptr(GrlColor) ship_color = grl_color_new (100, 200, 255, 255);
    g_autoptr(GrlColor) ship_thrust = grl_color_new (255, 150, 50, 255);
    g_autoptr(GrlColor) bullet_color = grl_color_new (255, 255, 100, 255);
    g_autoptr(GrlColor) asteroid_color = grl_color_new (139, 119, 101, 255);
    g_autoptr(GrlColor) asteroid_wire = grl_color_new (100, 80, 60, 255);
    g_autoptr(GrlColor) particle_color = grl_color_new (255, 200, 100, 255);
    g_autoptr(GrlColor) boundary_color = grl_color_new (60, 60, 80, 100);
    g_autoptr(GrlVector3) pos = NULL;
    gint i;
    gfloat alpha_factor;
    gboolean thrusting;
    gboolean visible;

    grl_camera3d_begin (camera);

    /* Draw boundary walls */
    pos = grl_vector3_new (-WORLD_HALF_WIDTH, 0.0f, 0.0f);
    grl_draw_cube (pos, 0.3f, 1.0f, WORLD_HALF_HEIGHT * 2.0f + 1.0f, boundary_color);
    g_clear_pointer (&pos, grl_vector3_free);

    pos = grl_vector3_new (WORLD_HALF_WIDTH, 0.0f, 0.0f);
    grl_draw_cube (pos, 0.3f, 1.0f, WORLD_HALF_HEIGHT * 2.0f + 1.0f, boundary_color);
    g_clear_pointer (&pos, grl_vector3_free);

    pos = grl_vector3_new (0.0f, 0.0f, -WORLD_HALF_HEIGHT);
    grl_draw_cube (pos, WORLD_HALF_WIDTH * 2.0f + 1.0f, 1.0f, 0.3f, boundary_color);
    g_clear_pointer (&pos, grl_vector3_free);

    pos = grl_vector3_new (0.0f, 0.0f, WORLD_HALF_HEIGHT);
    grl_draw_cube (pos, WORLD_HALF_WIDTH * 2.0f + 1.0f, 1.0f, 0.3f, boundary_color);
    g_clear_pointer (&pos, grl_vector3_free);

    /* Draw ship (if not game over) */
    if (!game->game_over)
    {
        /* Blink when invulnerable */
        visible = TRUE;
        if (game->invuln_timer > 0.0f)
        {
            visible = ((gint)(game->invuln_timer * 10.0f) % 2) == 0;
        }

        if (visible)
        {
            /* Ship body - draw as rotated cube/cone approximation */
            pos = grl_vector3_new (game->ship_x, 0.0f, game->ship_y);

            /* Main body */
            grl_draw_cube (pos, SHIP_SIZE * 1.5f, SHIP_SIZE * 0.5f,
                           SHIP_SIZE, ship_color);

            /* Thrust indicator */
            thrusting = grl_input_is_key_down (GRL_KEY_W) ||
                        grl_input_is_key_down (GRL_KEY_UP);
            if (thrusting)
            {
                g_autoptr(GrlVector3) thrust_pos = NULL;
                gfloat tx, ty;

                tx = game->ship_x - cosf (game->ship_angle) * SHIP_SIZE;
                ty = game->ship_y - sinf (game->ship_angle) * SHIP_SIZE;
                thrust_pos = grl_vector3_new (tx, 0.0f, ty);
                grl_draw_sphere (thrust_pos, SHIP_SIZE * 0.4f, ship_thrust);
            }

            /* Direction indicator */
            {
                g_autoptr(GrlVector3) nose_pos = NULL;
                gfloat nx, ny;

                nx = game->ship_x + cosf (game->ship_angle) * SHIP_SIZE;
                ny = game->ship_y + sinf (game->ship_angle) * SHIP_SIZE;
                nose_pos = grl_vector3_new (nx, 0.0f, ny);
                grl_draw_sphere (nose_pos, SHIP_SIZE * 0.3f, ship_color);
            }
        }
    }

    /* Draw bullets */
    for (i = 0; i < MAX_BULLETS; i++)
    {
        if (!game->bullets[i].active)
            continue;

        pos = grl_vector3_new (game->bullets[i].x, 0.0f, game->bullets[i].y);
        grl_draw_sphere (pos, BULLET_SIZE, bullet_color);
        g_clear_pointer (&pos, grl_vector3_free);
    }

    /* Draw asteroids */
    for (i = 0; i < MAX_ASTEROIDS; i++)
    {
        Asteroid *ast;
        gfloat radius;

        if (!game->asteroids[i].active)
            continue;

        ast = &game->asteroids[i];
        radius = get_asteroid_radius (ast->size);

        pos = grl_vector3_new (ast->x, 0.0f, ast->y);
        grl_draw_sphere (pos, radius, asteroid_color);
        grl_draw_sphere_wires (pos, radius, 8, 8, asteroid_wire);
        g_clear_pointer (&pos, grl_vector3_free);
    }

    /* Draw particles */
    for (i = 0; i < MAX_PARTICLES; i++)
    {
        Particle *part;
        g_autoptr(GrlColor) part_color = NULL;
        guint8 alpha_val;

        if (!game->particles[i].active)
            continue;

        part = &game->particles[i];
        alpha_factor = part->lifetime / PARTICLE_LIFETIME;
        alpha_val = (guint8)(255.0f * alpha_factor);

        part_color = grl_color_new (255, (guint8)(200.0f * alpha_factor),
                                    (guint8)(100.0f * alpha_factor), alpha_val);

        pos = grl_vector3_new (part->x, 0.0f, part->y);
        grl_draw_sphere (pos, 0.1f, part_color);
        g_clear_pointer (&pos, grl_vector3_free);
    }

    grl_camera3d_end (camera);
}

/*
 * Draw 2D UI overlay.
 */
static void
asteroids_draw_ui (AsteroidsGame *game)
{
    g_autoptr(GrlColor) white = grl_color_new (255, 255, 255, 255);
    g_autoptr(GrlColor) gray = grl_color_new (150, 150, 150, 255);
    g_autoptr(GrlColor) yellow = grl_color_new (255, 220, 50, 255);
    g_autoptr(GrlColor) cyan = grl_color_new (100, 200, 255, 255);
    gchar text[64];
    gint text_width;
    gint i;

    /* Score */
    g_snprintf (text, sizeof (text), "SCORE: %d", game->score);
    grl_draw_text (text, 20, 20, 30, white);

    /* Level */
    g_snprintf (text, sizeof (text), "LEVEL: %d", game->level);
    grl_draw_text (text, 20, 55, 20, gray);

    /* Lives */
    grl_draw_text ("LIVES:", 20, 85, 20, gray);
    for (i = 0; i < game->lives; i++)
    {
        grl_draw_text ("*", 100 + i * 25, 85, 20, cyan);
    }

    /* Controls */
    grl_draw_text ("W/UP: Thrust  A/D: Rotate  SPACE: Fire  P: Pause",
                   20, WINDOW_HEIGHT - 30, 14, gray);

    /* Pause overlay */
    if (game->paused && !game->game_over)
    {
        text_width = grl_measure_text ("PAUSED", 50);
        grl_draw_text ("PAUSED", WINDOW_WIDTH / 2 - text_width / 2,
                       WINDOW_HEIGHT / 2 - 50, 50, yellow);
        grl_draw_text ("Press P to resume", WINDOW_WIDTH / 2 - 90,
                       WINDOW_HEIGHT / 2 + 10, 20, gray);
    }

    /* Game over overlay */
    if (game->game_over)
    {
        text_width = grl_measure_text ("GAME OVER", 60);
        grl_draw_text ("GAME OVER", WINDOW_WIDTH / 2 - text_width / 2,
                       WINDOW_HEIGHT / 2 - 100, 60, yellow);

        g_snprintf (text, sizeof (text), "Final Score: %d", game->score);
        text_width = grl_measure_text (text, 30);
        grl_draw_text (text, WINDOW_WIDTH / 2 - text_width / 2,
                       WINDOW_HEIGHT / 2 - 30, 30, white);

        g_snprintf (text, sizeof (text), "Level Reached: %d", game->level);
        text_width = grl_measure_text (text, 20);
        grl_draw_text (text, WINDOW_WIDTH / 2 - text_width / 2,
                       WINDOW_HEIGHT / 2 + 10, 20, gray);

        grl_draw_text ("Press SPACE to restart", WINDOW_WIDTH / 2 - 100,
                       WINDOW_HEIGHT / 2 + 60, 18, gray);
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
    AsteroidsGame game;
    gfloat dt;

    app = grl_application_new ("com.example.asteroids3d");
    window = grl_window_new (WINDOW_WIDTH, WINDOW_HEIGHT, "3D Asteroids");
    bg_color = grl_color_new (10, 10, 20, 255);

    /* Setup 3D camera - top-down view */
    camera = grl_camera3d_new ();
    grl_camera3d_set_position_xyz (camera, 0.0f, CAM_HEIGHT, 0.0f);
    grl_camera3d_set_target_xyz (camera, 0.0f, 0.0f, 0.0f);
    {
        g_autoptr(GrlVector3) up = grl_vector3_new (0.0f, 0.0f, -1.0f);
        grl_camera3d_set_up (camera, up);
    }
    grl_camera3d_set_fovy (camera, 45.0f);

    grl_window_set_target_fps (window, 60);

    asteroids_init (&game);

    while (!grl_window_should_close (window))
    {
        dt = grl_window_get_frame_time (window);

        /* Poll input */
        grl_window_poll_input (window);

        /* Pause toggle */
        if (grl_input_is_key_pressed (GRL_KEY_P))
        {
            if (!game.game_over)
            {
                game.paused = !game.paused;
            }
        }

        /* Restart on SPACE when game over */
        if (game.game_over && grl_input_is_key_pressed (GRL_KEY_SPACE))
        {
            asteroids_init (&game);
        }

        /* Update */
        asteroids_update (&game, dt);

        /* Draw */
        grl_window_begin_drawing (window);
        grl_draw_clear_background (bg_color);

        asteroids_draw_3d (&game, camera);
        asteroids_draw_ui (&game);

        grl_window_end_drawing (window);
    }

    return 0;
}
