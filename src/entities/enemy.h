#ifndef ENEMY_H
#define ENEMY_H

#include <raylib.h>

#include "common.h"
#include "path.h"

#define ENEMY_SPEED 5.0f

typedef int EnemyID;

typedef struct {
    EnemyID id;
    Vector2 position;
    Vector2 velocity;
    int target;
} Enemy;

typedef enum {
    ENEMY_CONTINUES,
    ENEMY_REACHED_EXIT
} EnemyResult;

void enemy_init(
    Enemy* enemy,
    EnemyID id,
    Vector2 position,
    Vector2 velocity
);
EnemyResult enemy_update(
    Enemy* enemy,
    Path* path,
    float dt
);

#endif
