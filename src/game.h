#ifndef GAME_H
#define GAME_H

#include "common.h"
#include "scenes/scene.h"
#include "scenes/menu_scene.h"
#include "scenes/play_scene.h"

#define WIDTH 900
#define HEIGHT 600

typedef struct {
    bool running;
    Scene* current;
    MenuScene menu;
    PlayScene play;
    // Font font;
} Game;

void game_init(Game* game);
void game_run(Game* game);
void game_free(Game* game);

#endif
