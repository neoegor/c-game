#include <raylib.h>

#include "common.h"
#include "play_scene.h"

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

void play_draw(PlayScene* scene) {
    ClearBackground(BLACK);
}

void play_free(PlayScene* scene) {

}
