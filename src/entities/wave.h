#ifndef WAVE_H
#define WAVE_H

#include "common.h"

#define SPAWN_INTERVAL 1.0f
#define ENEMY_MAX_VALUE_INITIAL 20
#define ENEMY_MAGNITUDE_INCREMENT 10
#define ENEMY_MAX_VALUE_LIMIT 500

#define WAVE_SIZE_INITIAL 10
#define WAVE_SIZE_INCREMENT 20
#define WAVE_SIZE_LIMIT 100

typedef enum {
    VALUE_RANDOM,
    VALUE_POSITIVE,
    VALUE_NEGATIVE,
    VALUE_EVEN,
    VALUE_ODD,
    VALUE_PRIME,
    VALUE_SQUARE,
    VALUE_COUNT
} ValueType;

typedef struct {
    const char* display_name;
    int total_enemies;
    int value_magnitude;
    float value_weights[VALUE_COUNT];
} WaveDefinition;

typedef struct {
    WaveDefinition definition;
    float spawn_interval;
    float time_since_spawn;
    int spawned_enemies;
} EnemyWave;

void wave_init(EnemyWave* wave, WaveDefinition definition);
bool wave_update(EnemyWave* wave, float dt);
int wave_generate_value(EnemyWave* wave);
WaveDefinition wave_definition_generate(int wave_index);

#endif
