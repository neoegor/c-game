#ifndef TOWER_H
#define TOWER_H

#include <raylib.h>

#include "common.h"
#include "entities/path.h"
#include "entities/enemy.h"

#define ATTACK_INTERVAL 0.5f
#define ATTACK_RADIUS 5.0f
#define PROJ_SPEED 10.0f
#define EPSILON 0.000001f

typedef struct {
    int target_enemy_id;
    Vector2 position;
    Vector2 velocity;
    Vector2 target_position;
} Projectile;

typedef struct {
    Vector2 position;
    float attack_interval;
    float time_since_attack;
    Projectile projectiles[128];
    int projectile_count;
} Tower;

typedef enum {
    TOWER_EVENT_PROJECTILE_HIT
} TowerEventType;

typedef struct {
    TowerEventType type;
    EnemyID enemy_id;
} TowerEvent;

void tower_init(Tower* tower, Vector2 position);
void tower_update(
    Tower* tower,
    Enemy* enemies,
    int enemy_count,
    Path* path,
    float dt,
    TowerEvent *events,
    int* event_count
);

#endif
