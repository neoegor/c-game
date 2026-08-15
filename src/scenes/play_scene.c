#include <raylib.h>
#include <raygui.h>
#include <raymath.h>

#include "common.h"
#include "scenes/play_scene.h"
#include "world/play_world.h"

static void draw_hover_cell(PlayScene* scene) {
    Vector2 mouse = GetMousePosition();
    int grid_x = (int)floorf(mouse.x / CELL);
    int grid_y = (int)floorf(mouse.y / CELL);
    DrawRectangle(grid_x*CELL, grid_y*CELL, CELL, CELL, GRAY);
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
    Vector2 prev = scene->world.path.points[0];
    for (int i = 1; i < scene->world.path.count; i++) {
        Vector2 current = scene->world.path.points[i];
        DrawRectangle(
            prev.x*CELL,
            prev.y*CELL,
            CELL + (current.x - prev.x)*CELL,
            CELL + (current.y - prev.y)*CELL,
            LIGHTGRAY
        );
        prev = current;
    }
}

static void draw_enemies(PlayScene* scene) {
    for (int i = 0; i < scene->world.enemy_count; i++) {
        Enemy enemy = scene->world.enemies[i];
        DrawRectangle(
            enemy.position.x*CELL,
            enemy.position.y*CELL,
            CELL,
            CELL,
            RED
        );
        const char *text = TextFormat("%d", enemy.value);
        int width = MeasureText(text, CELL);
        DrawText(
            text,
            enemy.position.x*CELL + (CELL - width) / 2,
            enemy.position.y*CELL,
            CELL,
            BLACK
        );
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
    ClearBackground(WHITE);
    draw_hover_cell(scene);
    draw_path(scene);
    draw_towers(scene);
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
