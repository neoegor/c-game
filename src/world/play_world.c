#include <raylib.h>
#include <raymath.h>
#include <math.h>
#include <string.h>

#include "common.h"
#include "world/play_world.h"
#include "entities/tower.h"
#include "entities/path.h"

void play_world_spawn_enemy(PlayWorld* world) {
    Vector2 begining = world->path.points[0];
    Vector2 end = world->path.points[1];
    Vector2 velocity = Vector2Subtract(end, begining);
    velocity = Vector2Scale(Vector2Normalize(velocity), ENEMY_SPEED);
    Enemy* enemy = &world->enemies[world->enemy_count++];
    enemy_init(
        enemy,
        world->next_enemy_id++,
        begining,
        velocity,
        GetRandomValue(1, 9)
    );
}

void play_world_kill_enemy(PlayWorld* world, EnemyID id) {
    for (int i = 0; i < world->enemy_count; i++) {
        if (world->enemies[i].id == id) {
            memmove(
                &world->enemies[i],
                &world->enemies[i + 1],
                (world->enemy_count - i - 1) * sizeof(world->enemies[0])
            );
            world->enemy_count--;
            return;
        }
    }
}

void play_world_transform_enemy(PlayWorld* world, EnemyID id) {
    for (int i = 0; i < world->enemy_count; i++) {
        Enemy* enemy = &world->enemies[i];
        if (enemy->id == id) {
            enemy->value -= 1;
            
            if (enemy->value == 0) {
                memmove(
                    &world->enemies[i],
                    &world->enemies[i + 1],
                    (world->enemy_count - i - 1) * sizeof(world->enemies[0])
                );
                world->enemy_count--;
            }

            return;
        }
    }
}

void play_world_place_tower(PlayWorld* world, Vector2 position) {
    Tower* tower = &world->towers[world->tower_count++];
    tower_init(tower, position);
}

static void towers_update(PlayWorld* world, float dt) {
    TowerEvent events[128];
    int event_count = 0;

    for (int i = 0; i < world->tower_count; i++) {
        tower_update(
            &world->towers[i],
            world->enemies,
            world->enemy_count,
            &world->path,
            dt,
            events,
            &event_count
        );
    }

    // temp, should be done as recieved
    for (int i = 0; i < event_count; i++) {
        if (events[i].type == TOWER_EVENT_PROJECTILE_HIT) {
            // play_world_kill_enemy(world, events[i].enemy_id);
            play_world_transform_enemy(world, events[i].enemy_id);
        }
    }
}

static void enemies_update(PlayWorld* world, float dt) {
    for (int i = 0; i < world->enemy_count; i++) {
        Enemy* enemy = &world->enemies[i];
        EnemyResult result = enemy_update(
            enemy,
            &world->path,
            dt
        );
        if (result == ENEMY_REACHED_EXIT) {
            play_world_kill_enemy(world, enemy->id);
            i--;
        }
    }
}

void play_world_init(PlayWorld* world) {
    world->tower_count = 0;
    world->enemy_count = 0;
    world->next_enemy_id = 0;

    world->path.count = 0;
    world->path.points[world->path.count++] = (Vector2){-1, 7};
    world->path.points[world->path.count++] = (Vector2){22, 7};
    world->path.points[world->path.count++] = (Vector2){22, 22};
    world->path.points[world->path.count++] = (Vector2){45, 22};

    wave_init(&world->wave, 10);
}

void play_world_update(PlayWorld* world, float dt) {
    if (wave_update(&world->wave, dt)) {
        play_world_spawn_enemy(world);
    }
    towers_update(world, dt);
    enemies_update(world, dt);
}
