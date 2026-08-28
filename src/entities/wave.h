#ifndef WAVE_H
#define WAVE_H

#include "common.h"

#define SPAWN_INTERVAL 1.0f
#define ENEMY_MIN_VALUE -25
#define ENEMY_MAX_VALUE 25

typedef struct {
    float spawn_interval;
    float time_since_spawn;
    int total_enemies;
    int spawned_enemies;
} EnemyWave;

void wave_init(EnemyWave* wave, int total_enemies);
bool wave_update(EnemyWave* wave, float dt);

#endif
