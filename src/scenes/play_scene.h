#ifndef PLAY_SCENE_H
#define PLAY_SCENE_H

#include "common.h"
#include "scenes/scene.h"
#include "ui/ui.h"

typedef struct {
    Scene scene;
} PlayScene;

void play_init(PlayScene* scene);
SceneRequest play_update(PlayScene* scene, float dt);
void play_draw_world(PlayScene* scene);
SceneRequest play_draw_ui(PlayScene* scene, UiContext* ui);
void play_free(PlayScene* scene);

#endif
