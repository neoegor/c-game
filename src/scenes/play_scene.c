#include <raylib.h>
#include <raygui.h>
#include <raymath.h>

#include "common.h"
#include "scenes/play_scene.h"
#include "world/play_world.h"

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
    for (int i = 0; i < scene->world.tower_count; i++) {
        Tower tower = scene->world.towers[i];
        DrawRectangle(tower.position.x*CELL, tower.position.y*CELL, CELL, CELL, BLACK);
        DrawCircleLines(
            tower.position.x*CELL + CELL/2,
            tower.position.y*CELL + CELL/2,
            ATTACK_RADIUS*CELL,
            BLACK
        );
    }
}

static void draw_path(PlayScene* scene) {
    for (int i = 0; i < scene->world.path.count; i++) {
        Vector2 point = scene->world.path.points[i];
        DrawRectangle(point.x*CELL, point.y*CELL, CELL, CELL, GREEN);
    }
}

static void draw_enemies(PlayScene* scene) {
    for (int i = 0; i < scene->world.enemy_count; i++) {
        Enemy enemy = scene->world.enemies[i];
        DrawRectangle(enemy.position.x*CELL, enemy.position.y*CELL, CELL, CELL, RED);
    }
}

static void draw_projectiles(PlayScene* scene) {
    for (int i = 0; i < scene->world.tower_count; i++) {
        Tower tower = scene->world.towers[i];
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

void play_init(PlayScene* scene) {
    scene->scene.type = PLAY_SCENE;
    play_world_init(&scene->world);
}

SceneRequest play_update(PlayScene* scene, float dt) {
    SceneRequest request = {.type = REQUEST_NONE};

    if (IsKeyPressed(KEY_A)) {
        request.type = REQUEST_SWITCH;
        request.target = MENU_SCENE;
    } else if (IsKeyPressed(KEY_E)) {
        play_world_spawn_enemy(&scene->world);
    } else if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        Vector2 mouse = GetMousePosition();
        int grid_x = (int)floorf(mouse.x / CELL);
        int grid_y = (int)floorf(mouse.y / CELL);
        play_world_place_tower(&scene->world, (Vector2){grid_x, grid_y});
    }

    play_world_update(&scene->world, dt);

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
