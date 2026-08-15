#ifndef PLAY_WORLD_H
#define PLAY_WORLD_H

#include "common.h"
#include "entities/tower.h"
#include "entities/enemy.h"
#include "entities/path.h"
#include "entities/wave.h"

typedef struct {
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
void play_world_spawn_enemy(PlayWorld* world);
void play_world_kill_enemy(PlayWorld* world, int id);
void play_world_place_tower(PlayWorld* world, Vector2 position);

#endif
