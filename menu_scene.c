#include <raylib.h>

#include "common.h"
#include "menu_scene.h"

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

void menu_draw(MenuScene* scene) {
    ClearBackground(WHITE);
}

void menu_free(MenuScene* scene) {

}
