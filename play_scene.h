#ifndef PLAY_SCENE_H
#define PLAY_SCENE_H

#include "common.h"
#include "scene.h"

typedef struct {
    Scene scene;
} PlayScene;

void play_init(PlayScene* scene);
SceneRequest play_update(PlayScene* scene, float dt);
void play_draw(PlayScene* scene);
void play_free(PlayScene* scene);

#endif
