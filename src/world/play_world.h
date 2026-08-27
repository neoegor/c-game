#ifndef PLAY_WORLD_H
#define PLAY_WORLD_H

#include <limits.h>

#include "common.h"
#include "entities/tower.h"
#include "entities/enemy.h"
#include "entities/path.h"
#include "entities/wave.h"
#include "entities/inventory.h"

#define INITIAL_HEALTH 100
#define INITIAL_CURRENCY 99999

typedef enum {
    WORLD_PLAYING,
    WORLD_WON,
    WORLD_LOST
} WorldState;

typedef struct {
    WorldState state;
    int health;
    int currency;
    Inventory inventory;
    Tower towers[128];
    int tower_count;
    int next_enemy_id;
    Enemy enemies[128];
    int enemy_count;
    Path path;
    EnemyWave wave;
} PlayWorld;

void play_world_init(PlayWorld* world);
void play_world_update(PlayWorld* world, float dt);
void play_world_place_tower(PlayWorld* world, Vector2 position, TowerType type, int operand);
bool tower_placement_is_allowed(PlayWorld* world, Vector2 position, TowerType type);

#endif
