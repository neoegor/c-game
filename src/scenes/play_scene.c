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
            tower.attack_radius*CELL,
            BLACK
        );
    }
}

static void draw_path(PlayScene* scene) {
    Vector2 prev = scene->world.path.points[0];
    for (int i = 1; i < scene->world.path.count; i++) {
        Vector2 current = scene->world.path.points[i];
        if (prev.x < current.x) {
            DrawRectangle(
                prev.x*CELL,
                prev.y*CELL,
                CELL + (current.x - prev.x)*CELL,
                CELL + (current.y - prev.y)*CELL,
                LIGHTGRAY
            );
        } else {
            DrawRectangle(
                current.x*CELL,
                prev.y*CELL,
                CELL + (prev.x - current.x)*CELL,
                CELL + (current.y - prev.y)*CELL,
                LIGHTGRAY
            );
        }
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

static void draw_hud(PlayScene* scene) {
    DrawText(TextFormat("Health: %d", scene->world.health), 5, 20, 20, BLACK);
    DrawText(TextFormat("Currency: %d", scene->world.currency), 5, 45, 20, BLACK);
}

void play_init(PlayScene* scene) {
    scene->scene.type = PLAY_SCENE;
    play_world_init(&scene->world);

    GuiSetStyle(DEFAULT, TEXT_ALIGNMENT, TEXT_ALIGN_CENTER);
    GuiSetStyle(DEFAULT, TEXT_ALIGNMENT_VERTICAL, TEXT_ALIGN_MIDDLE);
}

void play_reset(PlayScene* scene) {
    play_world_init(&scene->world);
}

SceneRequest play_update(PlayScene* scene, float dt) {
    SceneRequest request = {.type = REQUEST_NONE};

    if (scene->world.state == WORLD_PLAYING) {
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            Vector2 mouse = GetMousePosition();
            int grid_x = (int)floorf(mouse.x / CELL);
            int grid_y = (int)floorf(mouse.y / CELL);
            play_world_place_tower(&scene->world, (Vector2){grid_x, grid_y});
        }
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
    draw_hud(scene);
}

SceneRequest play_draw_ui(PlayScene* scene) {
    SceneRequest request = {.type = REQUEST_NONE};

    if (scene->world.state != WORLD_PLAYING) {
        DrawRectangle(
            0, 0,
            GetScreenWidth(),
            GetScreenHeight(),
            Fade(BLACK, 0.5f)
        );

        Rectangle popup = {
            GetScreenWidth() / 2 - 150,
            GetScreenHeight() / 2 - 100,
            300,
            200
        };

        GuiPanel(popup, NULL);

        const char* text = scene->world.state == WORLD_WON 
            ? "You won!" : "You lost!";

        GuiLabel(
            (Rectangle){ popup.x + 80, popup.y + 35, 140, 30 },
            text
        );

        if (GuiButton(
            (Rectangle){ popup.x + 75, popup.y + 120, 150, 40 },
            "Go to menu"
        )) {
            request.type = REQUEST_SWITCH;
            request.target = MENU_SCENE;
        }
    }

    return request;
}

void play_free(PlayScene* scene) {

}
