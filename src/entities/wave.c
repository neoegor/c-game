#include "common.h"
#include "wave.h"

void wave_init(EnemyWave* wave, int total_enemies) {
    wave->spawn_interval = SPAWN_INTERVAL;
    wave->time_since_spawn = 0;
    wave->total_enemies = total_enemies;
    wave->spawned_enemies = 0;
}

bool wave_update(EnemyWave* wave, float dt) {
    if (wave->spawned_enemies >= wave->total_enemies) 
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
