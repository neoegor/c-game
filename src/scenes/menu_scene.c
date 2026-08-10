#include <stdio.h>
#include <raylib.h>
#include <raygui.h>

#include "common.h"
#include "scenes/menu_scene.h"

void menu_init(MenuScene* scene) {
    scene->scene.type = MENU_SCENE;
}

SceneRequest menu_update(MenuScene* scene, float dt) {
    SceneRequest request = {.type = REQUEST_NONE};

    if (IsKeyPressed(KEY_A)) {
        request.type = REQUEST_SWITCH;
        request.target = PLAY_SCENE;
    }
    
    return request;
}

void menu_draw_world(MenuScene* scene) {
    ClearBackground(WHITE);
}

SceneRequest menu_draw_ui(MenuScene* scene) {
    SceneRequest request = {.type = REQUEST_NONE};

    GuiLabel((Rectangle){0, 0, 100, 20}, "Menu");

    if (GuiButton((Rectangle){300, 250, 200, 50}, "Play")) {
        request.type = REQUEST_SWITCH;
        request.target = PLAY_SCENE;
    }

    return request;
}

void menu_free(MenuScene* scene) {

}
