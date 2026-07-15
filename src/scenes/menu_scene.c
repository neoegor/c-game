#include <raylib.h>

#include "common.h"
#include "scenes/menu_scene.h"
#include "ui/ui.h"

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

SceneRequest menu_draw_ui(MenuScene* scene, UiContext* ui) {
    SceneRequest request = {.type = REQUEST_NONE};

    if (ui_button(ui, 1, "hello", (Rectangle){100, 100, 200, 40})) {
        request.type = REQUEST_QUIT;
    }

    return request;
}

void menu_free(MenuScene* scene) {

}
