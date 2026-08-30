#ifndef PLAY_WORLD_H
#define PLAY_WORLD_H

#include <limits.h>

#include "common.h"
#include "entities/tower.h"
#include "entities/enemy.h"
#include "entities/path.h"
#include "entities/wave.h"
#include "entities/inventory.h"

#define INITIAL_HEALTH 1000
#define INITIAL_CURRENCY 2000
#define ENEMY_KILL_REWARD 100
#define PATH_VERTICAL_SEGMENTS 3
#define WAVE_SIZE 100

typedef enum {
    WORLD_PLAYING,
    WORLD_WON,
    WORLD_LOST
} WorldState;

typedef struct {
    int grid_width;
    int grid_height;
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

typedef enum {
    PLACEMENT_SUCCESS,
    PLACEMENT_INSUFFICIENT_CURRENCY,
    PLACEMENT_OCCUPIED,
    PLACEMENT_ON_PATH,
} TowerPlacementResult;

void play_world_init(PlayWorld* world, int world_width, int world_height);
void play_world_update(PlayWorld* world, float dt);
void play_world_place_tower(PlayWorld* world, Vector2 position, TowerType type, int operand);
TowerPlacementResult tower_placement_validate(PlayWorld* world, Vector2 position, TowerType type);

#endif
