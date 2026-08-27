#include <raylib.h>
#include <raymath.h>
#include <math.h>
#include <string.h>

#include "common.h"
#include "entities/tower.h"
#include "entities/path.h"

#define EPSILON 0.000001f

typedef struct {
    bool found;
    EnemyID enemy_id;
    Vector2 impact_position;
    float impact_time;
} TargetSolution;

static bool is_prime(int n) {
    if (n < 2) {
        return false;
    }

    if (n % 2 == 0) {
        return n == 2;
    }

    for (int divisor = 3; divisor <= n / divisor; divisor += 2) {
        if (n % divisor == 0) {
            return false;
        }
    }

    return true;
}
 
static bool tower_calculate_impact_time(Vector2 tower_position, Vector2 enemy_position, Vector2 enemy_velocity, float projectile_speed, float t_offset, float* out_t) {
    Vector2 r = Vector2Subtract(enemy_position, tower_position);
    float a, b, c, d, t1, t2, t;
    a = Vector2LengthSqr(enemy_velocity) - projectile_speed * projectile_speed;
    b = 2 * (Vector2DotProduct(r, enemy_velocity) - projectile_speed * projectile_speed * t_offset);
    c = Vector2LengthSqr(r) - projectile_speed * projectile_speed * t_offset * t_offset;

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

static TargetSolution tower_predict_intercept(Tower* tower, Path* path, Enemy* enemy) {
    TargetSolution result = {.found = false, .enemy_id = enemy->id};

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
            tower->projectile_speed,
            t_offset,
            &t
        )) {
            if (time_until_next_waypoint >= t) {
                break;
            }
        }

        if (i == path->count - 1) return result;

        t_offset += time_until_next_waypoint;
        enemy_position = next_waypoint;
        enemy_velocity = Vector2Scale(path_get_next_direction(path, i), ENEMY_SPEED);
    }

    result.found = true;
    result.impact_position = Vector2Add(enemy_position, Vector2Scale(enemy_velocity, t));
    result.impact_time = t_offset + t;
    return result;
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
                .enemy_id = proj->target_enemy_id,
                .op_type = tower->op_type,
                .operand = tower->operand
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

static bool tower_can_affect_enemy(Tower* tower, Enemy* enemy) {
    return operation_can_apply(tower->op_type, tower->operand, enemy->value);
}

static bool tower_enemy_in_range(Tower* tower, Enemy* enemy) {
    return Vector2Length(Vector2Subtract(tower->position, enemy->position)) <= tower->attack_radius;
}

static void tower_fire_at(Tower* tower, EnemyID enemy_id, Vector2 impact_position) {
        Projectile* proj = &tower->projectiles[tower->projectile_count++];
        proj->target_enemy_id = enemy_id;
        proj->position = tower->position;
        proj->velocity = Vector2Scale(
            Vector2Normalize(
                Vector2Subtract(
                    impact_position,
                    tower->position
                )
            ), 
            tower->projectile_speed
        );
        proj->target_position = impact_position;
}

static bool tower_attack(Tower* tower, Enemy* enemies, int enemy_count, Path* path) {
    TargetSolution result = {.found = false, .impact_time = INFINITY};

    for (int i = 0; i < enemy_count; i++) {
        Enemy* enemy = &enemies[i];

        if (
            !tower_enemy_in_range(tower, enemy) ||
            !tower_can_affect_enemy(tower, enemy) 
        ) continue;

        TargetSolution solution = tower_predict_intercept(
            tower,
            path,
            enemy
        );

        if (solution.found && solution.impact_time < result.impact_time) {
            result = solution;
        }
    }

    if (result.found) {
        tower_fire_at(tower, result.enemy_id, result.impact_position);
        return true;
    }

    return false;
}

static const TowerDefinition definitions[] = {
    [TOWER_ADDITION] = {
        .display_name = "+",
        .operation = OP_ADDITION,
        .operand_required = true,
        .default_operand = 1,
        .min_operand = 1,
        .max_operand = 100,
        .attack_interval = 0.5f,
        .attack_radius = 5.5f,
        .projectile_speed = 10.0f,
        .cost = 500.0f
    },
    [TOWER_SUBTRACT] = {
        .display_name = "-",
        .operation = OP_SUBTRACT,
        .operand_required = true,
        .default_operand = 1,
        .min_operand = 1,
        .max_operand = 100,
        .attack_interval = 0.5f,
        .attack_radius = 5.5f,
        .projectile_speed = 10.0f,
        .cost = 500.0f
    },
    [TOWER_MULTIPLY] = {
        .display_name = "*",
        .operation = OP_MULTIPLY,
        .operand_required = true,
        .default_operand = 2,
        .min_operand = 2,
        .max_operand = 5,
        .attack_interval = 0.5f,
        .attack_radius = 5.5f,
        .projectile_speed = 10.0f,
        .cost = 500.0f
    },
    [TOWER_DIVIDE] = {
        .display_name = "/",
        .operation = OP_DIVIDE,
        .operand_required = true,
        .default_operand = 2,
        .min_operand = 2,
        .max_operand = 5,
        .attack_interval = 0.5f,
        .attack_radius = 5.5f,
        .projectile_speed = 10.0f,
        .cost = 500.0f
    },
    [TOWER_EQUALS] = {
        .display_name = "=",
        .operation = OP_EQUALS,
        .operand_required = true,
        .default_operand = 1,
        .min_operand = -100,
        .max_operand = 100,
        .attack_interval = 0.5f,
        .attack_radius = 8.5f,
        .projectile_speed = 8.0f,
        .cost = 1000.0f
    },
    [TOWER_PRIME] = {
        .display_name = "Prime",
        .operation = OP_PRIME,
        .operand_required = false,
        .default_operand = 0,
        .min_operand = 0,
        .max_operand = 0,
        .attack_interval = 0.5f,
        .attack_radius = 8.5f,
        .projectile_speed = 30.0f,
        .cost = 1000.0f
    },
    [TOWER_ABSOLUTE_VALUE] = {
        .display_name = "|x|",
        .operation = OP_ABSOLUTE_VALUE,
        .operand_required = false,
        .default_operand = 0,
        .min_operand = 0,
        .max_operand = 0,
        .attack_interval = 0.5f,
        .attack_radius = 5.5f,
        .projectile_speed = 8.0f,
        .cost = 1000.0f
    }
};

bool operation_can_apply(
    OperationType operation,
    int operand,
    int enemy_value
) {
    switch (operation) {
        case OP_EQUALS:
            if (enemy_value != operand) return false;
            break;
        case OP_PRIME:
            if (!is_prime(enemy_value)) return false;
            break;
        case OP_ABSOLUTE_VALUE:
            if (enemy_value > 0) return false;
            break;
        default:
            break;
    }

    return true;
}

const TowerDefinition *tower_get_definition(TowerType type) {
    return &definitions[type];
}

void tower_init(
    Tower* tower,
    TowerType type,
    int operand,
    Vector2 position
) {
    tower->type = type;
    tower->op_type = definitions[type].operation;
    tower->operand = operand;
    tower->position = position;
    tower->attack_interval = definitions[type].attack_interval;
    tower->time_since_attack = 0;
    tower->attack_radius = definitions[type].attack_radius;
    tower->projectile_count = 0;
    tower->projectile_speed = definitions[type].projectile_speed;
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
        if (!tower_attack(
            tower,
            enemies,
            enemy_count,
            path
        )) {
            tower->time_since_attack = tower->attack_interval;
            break;
        }

        tower->time_since_attack -= tower->attack_interval;
    }

    tower_update_projectiles(tower, dt, events, event_count);
}
