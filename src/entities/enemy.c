#include <raylib.h>
#include <raymath.h>

#include "common.h"
#include "enemy.h"
#include "path.h"

void enemy_init(
    Enemy* enemy,
    EnemyID id,
    Vector2 position,
    Vector2 velocity
) {
    enemy->id = id;
    enemy->position = position;
    enemy->velocity = velocity;
    enemy->target = 1;
}

EnemyResult enemy_update(
    Enemy* enemy,
    Path* path,
    float dt
) {
    Vector2 to_target = Vector2Subtract(
        path->points[enemy->target],
        enemy->position
    );
    float distance = Vector2Length(to_target);
    float about_to_travel = Vector2Length(
        Vector2Scale(
            enemy->velocity,
            dt
        )
    );

    if (distance <= about_to_travel) {
        enemy->position = path->points[enemy->target];
        if (enemy->target + 1 < path->count) {
            Vector2 velocity = path_get_next_direction(path, enemy->target);
            enemy->target++;

            enemy->position = Vector2Add(enemy->position, Vector2Scale(velocity, about_to_travel - distance));
            velocity = Vector2Scale(velocity, ENEMY_SPEED);
            enemy->velocity = velocity;
        } else {
            return ENEMY_REACHED_EXIT;
        }
    } else {
        enemy->position = Vector2Add(enemy->position, Vector2Scale(enemy->velocity, dt));
    }
    return ENEMY_CONTINUES;
}
