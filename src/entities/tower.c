#include <raylib.h>
#include <raymath.h>
#include <math.h>
#include <string.h>

#include "common.h"
#include "entities/tower.h"
#include "entities/path.h"
 
static bool tower_calculate_impact_time(Vector2 tower_position, Vector2 enemy_position, Vector2 enemy_velocity, float t_offset, float* out_t) {
    Vector2 r = Vector2Subtract(enemy_position, tower_position);
    float a, b, c, d, t1, t2, t;
    a = Vector2LengthSqr(enemy_velocity) - PROJ_SPEED * PROJ_SPEED;
    b = 2 * (Vector2DotProduct(r, enemy_velocity) - PROJ_SPEED * PROJ_SPEED * t_offset);
    c = Vector2LengthSqr(r) - PROJ_SPEED * PROJ_SPEED * t_offset * t_offset;

    if (fabsf(a) < EPSILON) {
        if (fabsf(b) < EPSILON) {
            return false;
        }

        t = -c / b;

        if (t <= 0.0f) {
            return false;
        }
    } else {
        d = b * b - 4 * a * c;

        if (d >= 0) {
            t1 = (-b + sqrtf(d)) / (2 * a);
            t2 = (-b - sqrtf(d)) / (2 * a);

            t = INFINITY;

            if (t1 > 0.0f && t1 < t) {
                t = t1;
            }

            if (t2 > 0.0f && t2 < t) {
                t = t2;
            }

            if (isinf(t)) return false;
        } else {
            return false;
        }
    }
    *out_t = t;
    return true;
}

static bool tower_calculate_target(Tower* tower, Path* path, Enemy* enemy, Vector2* target_out, float* time_until_impact) {
    Vector2 tower_position = tower->position;
    Vector2 enemy_position = enemy->position;
    Vector2 enemy_velocity = enemy->velocity;
    float t_offset = 0;
    float t;

    for (int i = enemy->target; i < path->count; i++) {
        Vector2 next_waypoint = path->points[i];
        float time_until_next_waypoint = Vector2Length(Vector2Subtract(next_waypoint, enemy_position)) / ENEMY_SPEED;

        if (tower_calculate_impact_time(
            tower_position,
            enemy_position,
            enemy_velocity,
            t_offset,
            &t
        )) {
            if (time_until_next_waypoint >= t) {
                break;
            }
        }

        if (i == path->count - 1) return false;

        t_offset += time_until_next_waypoint;
        enemy_position = next_waypoint;
        enemy_velocity = Vector2Scale(path_get_next_direction(path, i), ENEMY_SPEED);
    }

    *target_out = Vector2Add(enemy_position, Vector2Scale(enemy_velocity, t));
    *time_until_impact = t_offset + t;
    return true;
}

static void tower_update_projectiles(
    Tower* tower,
    float dt,
    TowerEvent* events,
    int* event_count
) {
    for (int i = 0; i < tower->projectile_count; i++) {
        Projectile* proj = &tower->projectiles[i];
        if (Vector2Length(Vector2Scale(proj->velocity, dt)) < Vector2Length(Vector2Subtract(proj->position, proj->target_position))) {
            proj->position = Vector2Add(
                proj->position,
                Vector2Scale(proj->velocity, dt)
            );
        } else {
            events[(*event_count)++] = (TowerEvent){
                .type = TOWER_EVENT_PROJECTILE_HIT,
                .enemy_id = proj->target_enemy_id
            };
            memmove(
                &tower->projectiles[i],
                &tower->projectiles[i + 1],
                (tower->projectile_count - i - 1) * sizeof(tower->projectiles[0])
            );
            tower->projectile_count--;
            i--;
        }
    }
}

static void tower_attack(Tower* tower, Enemy* enemies, int enemy_count, Path* path) {
    Vector2 potential_target, final_target;
    float time_until_impact, min_time;
    min_time = INFINITY;
    int enemy_id;

    for (int i = 0; i < enemy_count; i++) {
        Enemy* enemy = &enemies[i];
        if (tower_calculate_target(
            tower,
            path,
            enemy,
            &potential_target,
            &time_until_impact
        ) && time_until_impact < min_time && Vector2Length(Vector2Subtract(tower->position, enemy->position)) <= ATTACK_RADIUS) {
            min_time = time_until_impact;
            final_target = potential_target;
            enemy_id = enemies[i].id;
        }
    }

    if (!isinf(min_time)) {
        Projectile* proj = &tower->projectiles[tower->projectile_count++];
        proj->target_enemy_id = enemy_id;
        proj->position = tower->position;
        proj->velocity = Vector2Scale(
            Vector2Normalize(Vector2Subtract(final_target, tower->position)), 
            PROJ_SPEED
        );
        proj->target_position = final_target;
    }
}

void tower_init(Tower* tower, Vector2 position) {
    tower->position = position;
    tower->attack_interval = ATTACK_INTERVAL;
    tower->time_since_attack = 0;
    tower->projectile_count = 0;
}

void tower_update(
    Tower* tower,
    Enemy* enemies,
    int enemy_count,
    Path* path,
    float dt,
    TowerEvent* events,
    int* event_count
) {
    tower->time_since_attack += dt;
    while (tower->time_since_attack >= tower->attack_interval) {
        tower->time_since_attack -= tower->attack_interval;
        tower_attack(
            tower,
            enemies,
            enemy_count,
            path
        );
    }

    tower_update_projectiles(tower, dt, events, event_count);
}
