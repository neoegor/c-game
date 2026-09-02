#include <raylib.h>
#include <math.h>
#include <assert.h>

#include "common.h"
#include "wave.h"
#include "tower.h"

void wave_init(EnemyWave* wave, WaveDefinition definition) {
    wave->definition = definition;
    wave->spawn_interval = SPAWN_INTERVAL;
    wave->time_since_spawn = 0;
    wave->spawned_enemies = 0;
}

bool wave_update(EnemyWave* wave, float dt) {
    if (wave->spawned_enemies >= wave->definition.total_enemies) 
        return false;

    wave->time_since_spawn += dt;
    if (wave->time_since_spawn >= wave->spawn_interval) {
        while (wave->time_since_spawn >= wave->spawn_interval) {
            wave->time_since_spawn -= wave->spawn_interval;
        }
        wave->spawned_enemies += 1;
        return true;
    }
    return false;
}

static int generate_random() {
    int value;

    do {
        value = GetRandomValue(ENEMY_MIN_VALUE, ENEMY_MAX_VALUE);
    } while (value == 0);

    return value;
}

static int generate_positive() {
    return GetRandomValue(1, ENEMY_MAX_VALUE);
}

static int generate_negative() {
    return GetRandomValue(ENEMY_MIN_VALUE, -1);
}

static int generate_even() {
    int value;

    do {
        value = GetRandomValue(ENEMY_MIN_VALUE+1, ENEMY_MAX_VALUE);
        if (value % 2 != 0) {
            value -= 1;
        }

    } while (value == 0);

    return value;
}

static int generate_odd() {
    int value;

    value = GetRandomValue(ENEMY_MIN_VALUE+1, ENEMY_MAX_VALUE);
    if (value % 2 != 0) return value;

    value -= 1;

    return value;
}

static int generate_prime() {
    int value;

    do {
        value = GetRandomValue(2, ENEMY_MAX_VALUE);
    } while (!is_prime(value));

    return value;
}

static int generate_square() {
    int value = GetRandomValue(1, floorf(sqrt(ENEMY_MAX_VALUE)));
    value *= value;

    return value;
}

int wave_generate_value(EnemyWave* wave) {
    assert(ENEMY_MAX_VALUE != 0 || ENEMY_MIN_VALUE != 0);
    assert(ENEMY_MAX_VALUE >= ENEMY_MIN_VALUE);

    int value;
    ValueType type;
    float random = (float)GetRandomValue(0, 999999) / 1000000.0f;
    float total_weight = 0;
    for (int i = 0; i < VALUE_COUNT; i++) {
        total_weight += wave->definition.value_weights[i];
    }
    float roll = total_weight * random;
    float cumulative_weight = 0;

    for (int i = 0; i < VALUE_COUNT; i++) {
        cumulative_weight += wave->definition.value_weights[i];

        if (roll < cumulative_weight) {
            type = i;
            break;
        }
    }

    switch (type) {
        case VALUE_RANDOM:
            value = generate_random();
            break;
        case VALUE_POSITIVE:
            value = generate_positive();
            break;
        case VALUE_NEGATIVE:
            value = generate_negative();
            break;
        case VALUE_EVEN:
            value = generate_even();
            break;
        case VALUE_ODD:
            value = generate_odd();
            break;
        case VALUE_PRIME:
            value = generate_prime();
            break;
        case VALUE_SQUARE:
            value = generate_square();
            break;
        default:
            break;
    }

    return value;
}
