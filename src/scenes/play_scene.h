#ifndef PLAY_SCENE_H
#define PLAY_SCENE_H

#include <raylib.h>

#include "common.h"
#include "scenes/scene.h"
#include "world/play_world.h"

#define CELL 20.0f

typedef struct {
    Scene scene;
    PlayWorld world;
} PlayScene;

void play_init(PlayScene* scene);
void play_reset(PlayScene* scene);
SceneRequest play_update(PlayScene* scene, float dt);
void play_draw_world(PlayScene* scene);
SceneRequest play_draw_ui(PlayScene* scene);
void play_free(PlayScene* scene);

#endif
