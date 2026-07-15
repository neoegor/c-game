#include <raylib.h>

#include "common.h"
#include "scenes/play_scene.h"
#include "ui/ui.h"

void play_init(PlayScene* scene) {
    scene->scene.type = PLAY_SCENE;
}

SceneRequest play_update(PlayScene* scene, float dt) {
    SceneRequest request = {.type = REQUEST_NONE};

    if (IsKeyPressed(KEY_A)) {
        request.type = REQUEST_SWITCH;
        request.target = MENU_SCENE;
    }
    
    return request;
}

void play_draw_world(PlayScene* scene) {
    ClearBackground(BLACK);
}

SceneRequest play_draw_ui(PlayScene* scene, UiContext* ui) {
    SceneRequest request = {.type = REQUEST_NONE};

    return request;
}

void play_free(PlayScene* scene) {

}
