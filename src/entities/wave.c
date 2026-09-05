#include <raylib.h>
#include <math.h>
#include <assert.h>

#include "common.h"
#include "wave.h"
#include "tower.h"

#define MIN(a, b) (((a) < (b)) ? (a) : (b))

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

static int generate_random(int value_magnitude) {
    int value;

    do {
        value = GetRandomValue(-value_magnitude, value_magnitude);
    } while (value == 0);

    return value;
}

static int generate_positive(int value_magnitude) {
    return GetRandomValue(1, value_magnitude);
}

static int generate_negative(int value_magnitude) {
    return GetRandomValue(-value_magnitude, -1);
}

static int generate_even(int value_magnitude) {
    int value;

    do {
        value = GetRandomValue(-value_magnitude, value_magnitude);
    } while (value == 0 || value % 2 != 0);

    return value;
}

static int generate_odd(int value_magnitude) {
    int value;

    do {
        value = GetRandomValue(-value_magnitude, value_magnitude);
    } while (value % 2 == 0);

    return value;
}

static int generate_prime(int value_magnitude) {
    int value;

    do {
        value = GetRandomValue(2, value_magnitude);
    } while (!is_prime(value));

    return value;
}

static int generate_square(int value_magnitude) {
    int value = GetRandomValue(1, floorf(sqrt(value_magnitude)));
    value *= value;

    return value;
}

int wave_generate_value(EnemyWave* wave) {
    assert(ENEMY_MAX_VALUE_INITIAL != 0);

    int value;
    ValueType type = VALUE_COUNT;

    float random = (float)GetRandomValue(0, 999999) / 1000000.0f;
    float total_weight = 0;
    for (int i = 0; i < VALUE_COUNT; i++) {
        total_weight += wave->definition.value_weights[i];
    }
    assert(total_weight > 0);

    float roll = total_weight * random;
    float cumulative_weight = 0;

    for (int i = 0; i < VALUE_COUNT; i++) {
        float weight = wave->definition.value_weights[i];

        assert(weight >= 0.0f);

        cumulative_weight += weight;

        if (roll < cumulative_weight) {
            type = i;
            break;
        }
    }

    assert(type != VALUE_COUNT);

    switch (type) {
        case VALUE_RANDOM:
            value = generate_random(wave->definition.value_magnitude);
            break;
        case VALUE_POSITIVE:
            value = generate_positive(wave->definition.value_magnitude);
            break;
        case VALUE_NEGATIVE:
            value = generate_negative(wave->definition.value_magnitude);
            break;
        case VALUE_EVEN:
            value = generate_even(wave->definition.value_magnitude);
            break;
        case VALUE_ODD:
            value = generate_odd(wave->definition.value_magnitude);
            break;
        case VALUE_PRIME:
            value = generate_prime(wave->definition.value_magnitude);
            break;
        case VALUE_SQUARE:
            value = generate_square(wave->definition.value_magnitude);
            break;
        default:
            assert(false);
            value = 0;
            break;
    }

    return value;
}

static const char* wave_display_names[VALUE_COUNT] = {
    [VALUE_RANDOM] = "Random",
    [VALUE_POSITIVE] = "Positive",
    [VALUE_NEGATIVE] = "Negative",
    [VALUE_EVEN] = "Even",
    [VALUE_ODD] = "Odd",
    [VALUE_PRIME] = "Prime",
    [VALUE_SQUARE] = "Square"
};

WaveDefinition wave_definition_generate(int wave_index) {
    ValueType type = (ValueType)GetRandomValue(0, VALUE_COUNT - 1);

    WaveDefinition wave = {
        .display_name = wave_display_names[type],
        .total_enemies = MIN(
            WAVE_SIZE_INITIAL + wave_index * WAVE_SIZE_INCREMENT,
            WAVE_SIZE_LIMIT
        ),
        .value_magnitude = MIN(
            ENEMY_MAX_VALUE_INITIAL + wave_index * ENEMY_MAGNITUDE_INCREMENT,
            ENEMY_MAX_VALUE_LIMIT 
        ),
        .value_weights = {0}
    };

    wave.value_weights[type] = 1.0f;

    return wave;
}
