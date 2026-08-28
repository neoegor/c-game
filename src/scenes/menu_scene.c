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

    if (IsKeyPressed(KEY_ESCAPE)) {
        request.type = REQUEST_QUIT;
    }
    
    return request;
}

void menu_draw_world(MenuScene* scene) {
    ClearBackground(WHITE);
}

SceneRequest menu_draw_ui(MenuScene* scene) {
    SceneRequest request = {.type = REQUEST_NONE};

    int old_size = GuiGetStyle(DEFAULT, TEXT_SIZE);
    GuiSetStyle(DEFAULT, TEXT_SIZE, 40);
    GuiLabel((Rectangle){350, 190, 200, 50}, "Menu");
    GuiSetStyle(DEFAULT, TEXT_SIZE, old_size);

    if (GuiButton((Rectangle){350, 250, 200, 50}, "Play")) {
        request.type = REQUEST_SWITCH;
        request.target = PLAY_SCENE;
    } else if (GuiButton((Rectangle){350, 310, 200, 50}, "Quit")) {
        request.type = REQUEST_QUIT;
    }

    return request;
}

void menu_free(MenuScene* scene) {

}
