#ifndef GAME_H
#define GAME_H

#include "common.h"
#include "scenes/scene.h"
#include "scenes/menu_scene.h"
#include "scenes/play_scene.h"
#include "ui/ui.h"

typedef struct {
    bool running;
    Scene* current;
    MenuScene menu;
    PlayScene play;
    UiContext ui;
} Game;

void game_init(Game* game);
void game_run(Game* game);
void game_free(Game* game);

#endif
