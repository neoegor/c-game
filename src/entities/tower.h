#ifndef TOWER_H
#define TOWER_H

#include <raylib.h>

#include "common.h"
#include "entities/path.h"
#include "entities/enemy.h"

typedef enum {
    TOWER_ADDITION,
    TOWER_SUBTRACT,
    TOWER_MULTIPLY,
    TOWER_DIVIDE,
    TOWER_EQUALS,
    TOWER_PRIME
} TowerType;

typedef enum {
    OP_ADDITION,
    OP_SUBTRACT,
    OP_MULTIPLY,
    OP_DIVIDE,
    OP_EQUALS,
    OP_PRIME
} OperationType;

typedef struct {
    const char *display_name;
    OperationType operation;
    bool operand_required;
    int default_operand;
    int min_operand;
    int max_operand;
    float attack_interval;
    float attack_radius;
    float projectile_speed;
    float cost;
} TowerDefinition;

typedef struct {
    int target_enemy_id;
    Vector2 position;
    Vector2 velocity;
    Vector2 target_position;
} Projectile;

typedef struct {
    TowerType type;
    OperationType op_type;
    int operand;
    Vector2 position;
    float attack_interval;
    float time_since_attack;
    float attack_radius;
    Projectile projectiles[128];
    int projectile_count;
    float projectile_speed;
} Tower;

typedef enum {
    TOWER_EVENT_PROJECTILE_HIT
} TowerEventType;

typedef struct {
    TowerEventType type;
    EnemyID enemy_id;
    OperationType op_type;
    int operand;
} TowerEvent;

bool operation_can_apply(
    OperationType operation,
    int operand,
    int enemy_value
);
const TowerDefinition *tower_get_definition(TowerType type);
void tower_init(
    Tower* tower,
    TowerType type,
    int operand,
    Vector2 position
);
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
