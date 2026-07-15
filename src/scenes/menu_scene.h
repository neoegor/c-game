#ifndef MENU_SCENE_H
#define MENU_SCENE_H

#include "common.h"
#include "scenes/scene.h"
#include "ui/ui.h"

typedef struct {
    Scene scene;
} MenuScene;

void menu_init(MenuScene* scene);
SceneRequest menu_update(MenuScene* scene, float dt);
void menu_draw_world(MenuScene* scene);
SceneRequest menu_draw_ui(MenuScene* scene, UiContext* ui);
void menu_free(MenuScene* scene);

#endif
