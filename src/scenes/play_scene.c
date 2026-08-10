#include <raylib.h>
#include <raygui.h>
#include <raymath.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#include "common.h"
#include "scenes/play_scene.h"

static void draw_grid() {
    int sw = GetScreenWidth();
    int sh = GetScreenHeight();

    for (int row = 0; row*CELL < sh; row++) {
        for (int col = 0; col*CELL < sw; col++) {
            Color color = WHITE;
            if ((col + row) % 2 == 0) color = LIGHTGRAY;

            DrawRectangle(col*CELL, row*CELL, CELL, CELL, color);
        }
    }
}

static void draw_towers(PlayScene* scene) {
    for (int i = 0; i < scene->tower_count; i++) {
        Tower tower = scene->towers[i];
        DrawRectangle(tower.position.x*CELL, tower.position.y*CELL, CELL, CELL, BLACK);
    }
}

static void draw_path(PlayScene* scene) {
    for (int i = 0; i < scene->path.count; i++) {
        Vector2 point = scene->path.points[i];
        DrawRectangle(point.x*CELL, point.y*CELL, CELL, CELL, GREEN);
    }
}

static void draw_enemies(PlayScene* scene) {
    for (int i = 0; i < scene->enemy_count; i++) {
        Enemy enemy = scene->enemies[i];
        DrawRectangle(enemy.position.x*CELL, enemy.position.y*CELL, CELL, CELL, RED);
    }
}

static void draw_projectiles(PlayScene* scene) {
    for (int i = 0; i < scene->tower_count; i++) {
        Tower tower = scene->towers[i];
        for (int j = 0; j < tower.projectile_count; j++) {
            DrawCircle(
                tower.projectiles[j].position.x*CELL + CELL/2,
                tower.projectiles[j].position.y*CELL + CELL/2,
                CELL/8,
                BLACK
            );
        }
    }
}

static void enemy_create(PlayScene* scene) {
    Vector2 begining = scene->path.points[0];
    Vector2 end = scene->path.points[1];
    Vector2 velocity = Vector2Subtract(end, begining);
    velocity = Vector2Scale(Vector2Normalize(velocity), ENEMY_SPEED);
    scene->enemies[scene->enemy_count++] = (Enemy){
        scene->next_enemy_id++,
        begining,
        velocity,
        1
    };
}

static void enemy_kill(PlayScene* scene, int id) {
    for (int i = 0; i < scene->enemy_count; i++) {
        if (scene->enemies[i].id == id) {
            memmove(
                &scene->enemies[i],
                &scene->enemies[i + 1],
                (scene->enemy_count - i - 1) * sizeof(scene->enemies[0])
            );
            scene->enemy_count--;
            return;
        }
    }
}

static int enemy_closest(PlayScene* scene, Vector2 position) {
    int closest = -1;
    float closest_distance = INFINITY;

    for (int i = 0; i < scene->enemy_count; i++) {
        Enemy* enemy = &scene->enemies[i];

        float distance = Vector2Length(Vector2Subtract(position, enemy->position));

        if (distance < closest_distance) {
            closest = i;
            closest_distance = distance;
        }
    }

    return closest;
}

static Vector2 path_get_next_direction(Path* path, int i) {
    // may need guards here
    Vector2 direction = Vector2Normalize(Vector2Subtract(path->points[i+1], path->points[i]));

    return direction;
}

static void tower_create(PlayScene* scene, Vector2 position) {
    Tower* tower = &scene->towers[scene->tower_count++];
    tower->position = position;
    tower->attack_interval = 1;
    tower->time_since_attack = 0;
    tower->projectile_count = 0;
}

static bool tower_calculate_impact_time(Vector2 tower_position, Vector2 enemy_position, Vector2 enemy_velocity, float t_offset, float* out_t) {
    Vector2 r = Vector2Subtract(enemy_position, tower_position);
    float a, b, c, d, t1, t2, t;
    a = Vector2LengthSqr(enemy_velocity) - PROJ_SPEED * PROJ_SPEED;
    b = 2 * (Vector2DotProduct(r, enemy_velocity) - PROJ_SPEED * PROJ_SPEED * t_offset);
    c = Vector2LengthSqr(r) - PROJ_SPEED * PROJ_SPEED * t_offset * t_offset;

    if (fabsf(a) < EPSILON) {
        if (fabsf(b) < EPSILON) {
            return false;
        }

        t = -c / b;

        if (t <= 0.0f) {
            return false;
        }
    } else {
        d = b * b - 4 * a * c;

        if (d >= 0) {
            t1 = (-b + sqrtf(d)) / (2 * a);
            t2 = (-b - sqrtf(d)) / (2 * a);

            t = INFINITY;

            if (t1 > 0.0f && t1 < t) {
                t = t1;
            }

            if (t2 > 0.0f && t2 < t) {
                t = t2;
            }

            if (isinf(t)) return false;
        } else {
            return false;
        }
    }
    *out_t = t;
    return true;
}

static bool tower_calculate_target(Tower* tower, Path* path, Enemy* enemy, Vector2* target_out) {
    Vector2 tower_position = tower->position;
    Vector2 enemy_position = enemy->position;
    Vector2 enemy_velocity = enemy->velocity;
    float t_offset = 0;
    float t;

    for (int i = enemy->target; i < path->count; i++) {
        Vector2 next_waypoint = path->points[i];
        float time_until_next_waypoint = Vector2Length(Vector2Subtract(next_waypoint, enemy_position)) / ENEMY_SPEED;

        if (tower_calculate_impact_time(
            tower_position,
            enemy_position,
            enemy_velocity,
            t_offset,
            &t
        )) {
            if (time_until_next_waypoint >= t) {
                break;
            }
        }

        if (i == path->count - 1) return false;

        t_offset += time_until_next_waypoint;
        enemy_position = next_waypoint;
        enemy_velocity = Vector2Scale(path_get_next_direction(path, i), ENEMY_SPEED);
    }

    *target_out = Vector2Add(enemy_position, Vector2Scale(enemy_velocity, t));
    return true;
}

static void tower_attack(PlayScene* scene, Tower* tower) {
    int target_enemy_i = enemy_closest(scene, tower->position);
    if (target_enemy_i == -1) return;

    Vector2 target_position;
    if (tower_calculate_target(
            tower,
            &scene->path,
            &scene->enemies[target_enemy_i], &target_position
        )) {
        Projectile* proj = &tower->projectiles[tower->projectile_count++];
        proj->target_enemy_id = scene->enemies[target_enemy_i].id;
        proj->position = tower->position;
        proj->velocity = Vector2Scale(
            Vector2Normalize(Vector2Subtract( target_position, tower->position)), 
            PROJ_SPEED
        );
        proj->target_position = target_position;
    }
}

static void tower_update_projectiles(PlayScene* scene, Tower* tower, float dt) {
    for (int i = 0; i < tower->projectile_count; i++) {
        Projectile* proj = &tower->projectiles[i];
        if (Vector2Length(Vector2Scale(proj->velocity, dt)) < Vector2Length(Vector2Subtract(proj->position, proj->target_position))) {
            proj->position = Vector2Add(
                proj->position,
                Vector2Scale(proj->velocity, dt)
            );
        } else {
            enemy_kill(scene, proj->target_enemy_id);
            memmove(
                &tower->projectiles[i],
                &tower->projectiles[i + 1],
                (tower->projectile_count - i - 1) * sizeof(tower->projectiles[0])
            );
            tower->projectile_count--;
            i--;
        }
    }
}

static void tower_update(PlayScene* scene, Tower* tower, float dt) {
    tower->time_since_attack += dt;
    while (tower->time_since_attack >= tower->attack_interval) {
        tower->time_since_attack -= tower->attack_interval;
        tower_attack(scene, tower);
    }

    tower_update_projectiles(scene, tower, dt);
}

static void towers_update(PlayScene* scene, float dt) {
    for (int i = 0; i < scene->tower_count; i++) {
        tower_update(scene, &scene->towers[i], dt);
    }
}

static void enemies_update(PlayScene* scene, float dt) {
    for (int i = 0; i < scene->enemy_count; i++) {
        Enemy* enemy = &scene->enemies[i];

        Vector2 to_target = Vector2Subtract(scene->path.points[enemy->target], enemy->position);
        float distance = Vector2Length(to_target);
        float about_to_travel = Vector2Length(Vector2Scale(enemy->velocity, dt));

        if (distance <= about_to_travel) {
            enemy->position = scene->path.points[enemy->target];
            if (enemy->target + 1 < scene->path.count) {
                Vector2 velocity = path_get_next_direction(&scene->path, enemy->target);
                enemy->target++;

                enemy->position = Vector2Add(enemy->position, Vector2Scale(velocity, about_to_travel - distance));
                velocity = Vector2Scale(velocity, ENEMY_SPEED);
                enemy->velocity = velocity;
            } else {
                enemy_kill(scene, enemy->id);
                i--;
            }
        } else {
            enemy->position = Vector2Add(enemy->position, Vector2Scale(enemy->velocity, dt));
        }
    }
}

void play_init(PlayScene* scene) {
    scene->scene.type = PLAY_SCENE;
    scene->tower_count = 0;
    scene->enemy_count = 0;
    scene->next_enemy_id = 0;

    scene->path.count = 0;
    scene->path.points[scene->path.count++] = (Vector2){0, 7};
    scene->path.points[scene->path.count++] = (Vector2){22, 7};
    scene->path.points[scene->path.count++] = (Vector2){22, 22};
    scene->path.points[scene->path.count++] = (Vector2){44, 22};
}

SceneRequest play_update(PlayScene* scene, float dt) {
    SceneRequest request = {.type = REQUEST_NONE};

    if (IsKeyPressed(KEY_A)) {
        request.type = REQUEST_SWITCH;
        request.target = MENU_SCENE;
    } else if (IsKeyPressed(KEY_E)) {
        enemy_create(scene);
    } else if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        Vector2 mouse = GetMousePosition();
        int grid_x = (int)floorf(mouse.x / CELL);
        int grid_y = (int)floorf(mouse.y / CELL);
        tower_create(scene, (Vector2){grid_x, grid_y});
    }

    towers_update(scene, dt);
    enemies_update(scene, dt);
    
    return request;
}

void play_draw_world(PlayScene* scene) {
    ClearBackground(BLACK);
    draw_grid();
    draw_towers(scene);
    draw_path(scene);
    draw_enemies(scene);
    draw_projectiles(scene);
}

SceneRequest play_draw_ui(PlayScene* scene) {
    SceneRequest request = {.type = REQUEST_NONE};

    GuiLabel((Rectangle){0, 0, 100, 20}, "Play");

    return request;
}

void play_free(PlayScene* scene) {

}
