#include <raylib.h>
#include <raymath.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "common.h"
#include "world/play_world.h"
#include "entities/tower.h"
#include "entities/path.h"

static void play_world_spawn_enemy(PlayWorld* world) {
    Vector2 beginning = world->path.points[0];
    Vector2 end = world->path.points[1];
    Vector2 velocity = Vector2Subtract(end, beginning);
    velocity = Vector2Scale(Vector2Normalize(velocity), ENEMY_SPEED);
    Enemy* enemy = &world->enemies[world->enemy_count++];

    enemy_init(
        enemy,
        world->next_enemy_id++,
        beginning,
        velocity,
        wave_generate_value(world->wave)
    );
}

static void play_world_kill_enemy(PlayWorld* world, EnemyID id) {
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

static void play_world_transform_enemy(PlayWorld* world, EnemyID id, OperationType op_type, int operand) {
    for (int i = 0; i < world->enemy_count; i++) {
        Enemy* enemy = &world->enemies[i];
        if (enemy->id == id) {
            // TODO do this in enemy.c?
            if (!operation_can_apply(op_type, operand, enemy->value)) 
                return;

            int old_value = enemy->value;

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
                case OP_ABSOLUTE_VALUE:
                    enemy->value = abs(enemy->value);
                    break;
            }

            if (abs(enemy->value) < enemy->min_abs_value) {
                enemy->min_abs_value = abs(enemy->value);
                world->currency += (int)(abs(old_value - enemy->value) * operation_get_reward_multiplier(op_type));
            }
            
            if (enemy->value == 0) {
                world->currency += ENEMY_KILL_REWARD;
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

TowerPlacementResult tower_placement_validate(PlayWorld* world, Vector2 position, TowerType type) {
    // Tower limit
    if (world->tower_count >= world->tower_limit) return PLACEMENT_TOWER_LIMIT;

    // Currency
    float cost = tower_get_definition(type)->cost;
    if (cost > world->currency) return PLACEMENT_INSUFFICIENT_CURRENCY;

    // Towers
    for (int i = 0; i < world->tower_count; i++) {
        Tower* tower = &world->towers[i];

        if (Vector2Equals(tower->position, position)) {
            return PLACEMENT_OCCUPIED;
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
                    return PLACEMENT_ON_PATH;
                }
            }
        } else if (segment_start.y == segment_end.y) {
            if (position.y == segment_start.y) {
                float min_x = fminf(segment_start.x, segment_end.x);
                float max_x = fmaxf(segment_start.x, segment_end.x);

                if (position.x >= min_x && position.x <= max_x) {
                    return PLACEMENT_ON_PATH;
                }
            }
        }
    }
    
    return PLACEMENT_SUCCESS;
}

void play_world_refund_tower(PlayWorld* world, int tower_index) {
    int cost = tower_get_definition(world->towers[tower_index].type)->cost;
    world->currency += (int)(cost * TOWER_REFUND_PROPORTION);

    memmove(
        &world->towers[tower_index],
        &world->towers[tower_index + 1],
        (world->tower_count - tower_index - 1) * sizeof(world->towers[0])
    );
    world->tower_count--;
}

void play_world_change_tower_operand(PlayWorld* world, int tower_index, int operand) {
    // TODO check bounds
    world->currency -= TOWER_OPERAND_CHANGE_COST;
    world->towers[tower_index].operand = operand;
}

void play_world_place_tower(PlayWorld* world, Vector2 position, TowerType type, int operand) {
    if (tower_placement_validate(world, position, type) != PLACEMENT_SUCCESS) return;
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

static void check_wave_switch(PlayWorld* world) {
    if (world->wave->definition.total_enemies == world->wave->spawned_enemies && world->enemy_count == 0) {
        if (world->current_wave_index + 1 >= world->wave_count) return;
        world->current_wave_index++;
        world->wave = &world->waves[world->current_wave_index];
    }
}

static void check_end_condition(PlayWorld* world) {
    if (world->health <= 0) {
        world->health = 0;
        world->state = WORLD_LOST;
    } else if (world->wave->definition.total_enemies == world->wave->spawned_enemies && world->enemy_count == 0) {
        world->state = WORLD_WON;
    }
}

void play_world_init(PlayWorld* world, int world_width, int world_height) {
    world->grid_width = world_width;
    world->grid_height = world_height;
    world->state = WORLD_PLAYING;
    world->health = INITIAL_HEALTH;
    world->currency = INITIAL_CURRENCY;

    inventory_init(&world->inventory);
    inventory_add_tower(&world->inventory, TOWER_ADDITION);
    inventory_add_tower(&world->inventory, TOWER_SUBTRACT);
    inventory_add_tower(&world->inventory, TOWER_MULTIPLY);
    inventory_add_tower(&world->inventory, TOWER_DIVIDE);
    inventory_add_tower(&world->inventory, TOWER_ABSOLUTE_VALUE);
    inventory_add_tower(&world->inventory, TOWER_EQUALS);
    inventory_add_tower(&world->inventory, TOWER_PRIME);

    world->tower_count = 0;
    world->tower_limit = TOWER_LIMIT;
    world->enemy_count = 0;
    world->next_enemy_id = 0;

    assert(path_generate(
        &world->path,
        world->grid_width,
        world->grid_height,
        PATH_VERTICAL_SEGMENTS
    ));

    world->wave_count = 0;
    world->current_wave_index = 0;

    for (int wave_index = 0; wave_index < NUMBER_OF_WAVES; wave_index++) {
        WaveDefinition wave = wave_definition_generate(wave_index);
        wave_init(&world->waves[world->wave_count++], wave);
    }

    world->wave = &world->waves[world->current_wave_index];
}

void play_world_update(PlayWorld* world, float dt) {
    if (world->state != WORLD_PLAYING) return;

    if (wave_update(world->wave, dt)) {
        play_world_spawn_enemy(world);
    }
    towers_update(world, dt);
    enemies_update(world, dt);

    check_wave_switch(world);
    check_end_condition(world);
}
