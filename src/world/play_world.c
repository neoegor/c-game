#include <raylib.h>
#include <raymath.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>

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

void play_world_transform_enemy(PlayWorld* world, EnemyID id, OperationType op_type, int operand) {
    for (int i = 0; i < world->enemy_count; i++) {
        Enemy* enemy = &world->enemies[i];
        if (enemy->id == id) {
            if (!operation_can_apply(op_type, operand, enemy->value)) 
                return;

            switch (op_type) {
                case OP_ADDITION:
                    enemy->value += operand;
                    break;
                case OP_SUBTRACT:
                    enemy->value -= operand;
                    break;
                case OP_MULTIPLY:
                    enemy->value *= operand;
                    break;
                case OP_DIVIDE:
                    enemy->value /= operand;
                    break;
                case OP_EQUALS:
                case OP_PRIME:
                    enemy->value = 0;
                    break;
            }
            
            if (enemy->value == 0) {
                world->currency += 100;
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

bool tower_placement_is_allowed(PlayWorld* world, Vector2 position, TowerType type) {
    // Currency
    float cost = tower_get_definition(type)->cost;
    if (cost > world->currency) return false;

    // Towers
    for (int i = 0; i < world->tower_count; i++) {
        Tower* tower = &world->towers[i];

        if (Vector2Equals(tower->position, position)) {
            return false;
        }
    }

    // Path
    for (int i = 1; i < world->path.count; i++) {
        Vector2 segment_start = world->path.points[i-1];
        Vector2 segment_end = world->path.points[i];

        if (segment_start.x == segment_end.x) {
            if (position.x == segment_start.x) {
                float min_y = fminf(segment_start.y, segment_end.y);
                float max_y = fmaxf(segment_start.y, segment_end.y);

                if (position.y >= min_y && position.y <= max_y) {
                    return false;
                }
            }
        } else if (segment_start.y == segment_end.y) {
            if (position.y == segment_start.y) {
                float min_x = fminf(segment_start.x, segment_end.x);
                float max_x = fmaxf(segment_start.x, segment_end.x);

                if (position.x >= min_x && position.x <= max_x) {
                    return false;
                }
            }
        }
    }
    
    return true;
}

void play_world_place_tower(PlayWorld* world, Vector2 position, TowerType type, int operand) {
    if (!tower_placement_is_allowed(world, position, type)) return;
    float cost = tower_get_definition(type)->cost;
    world->currency -= cost;

    Tower* tower = &world->towers[world->tower_count++];
    tower_init(tower, type, operand, position);
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
            play_world_transform_enemy(world, events[i].enemy_id, events[i].op_type, events[i].operand);
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
            world->health -= abs(enemy->value);

            play_world_kill_enemy(world, enemy->id);
            i--;
        }
    }
}

static void play_world_check_end_condition(PlayWorld* world) {
    if (world->health <= 0) {
        world->health = 0;
        world->state = WORLD_LOST;
    } else if (world->wave.total_enemies == world->wave.spawned_enemies && world->enemy_count == 0) {
        world->state = WORLD_WON;
    }
}

void play_world_init(PlayWorld* world) {
    world->state = WORLD_PLAYING;
    world->health = INITIAL_HEALTH;
    world->currency = INITIAL_CURRENCY;

    inventory_init(&world->inventory);
    inventory_add_tower(&world->inventory, TOWER_ADDITION);
    inventory_add_tower(&world->inventory, TOWER_SUBTRACT);
    inventory_add_tower(&world->inventory, TOWER_MULTIPLY);
    inventory_add_tower(&world->inventory, TOWER_DIVIDE);
    inventory_add_tower(&world->inventory, TOWER_PRIME);
    inventory_add_tower(&world->inventory, TOWER_EQUALS);

    world->tower_count = 0;
    world->enemy_count = 0;
    world->next_enemy_id = 0;

    world->path.count = 0;
    world->path.points[world->path.count++] = (Vector2){-1, 7};
    world->path.points[world->path.count++] = (Vector2){35, 7};
    world->path.points[world->path.count++] = (Vector2){35, 14};
    world->path.points[world->path.count++] = (Vector2){20, 14};
    world->path.points[world->path.count++] = (Vector2){20, 23};
    world->path.points[world->path.count++] = (Vector2){45, 23};

    wave_init(&world->wave, 100);
}

void play_world_update(PlayWorld* world, float dt) {
    if (world->state != WORLD_PLAYING) return;

    if (wave_update(&world->wave, dt)) {
        play_world_spawn_enemy(world);
    }
    towers_update(world, dt);
    enemies_update(world, dt);

    play_world_check_end_condition(world);
}
