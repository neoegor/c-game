#ifndef GAME_H
#define GAME_H

#include "common.h"
#include "scene.h"
#include "menu_scene.h"
#include "play_scene.h"

typedef struct {
    bool running;
    Scene* current;
    MenuScene menu;
    PlayScene play;
} Game;

void game_init(Game* game);
void game_run(Game* game);
void game_free(Game* game);

#endif
