#include <raylib.h>

#include "common.h"
#include "game.h"
#include "scenes/scene.h"
#include "scenes/menu_scene.h"
#include "scenes/play_scene.h"
#include "ui/ui.h"

static void game_switch_to_scene(Game* game, SceneType scene) {
    switch (scene) {
        case MENU_SCENE:
            game->current = &game->menu.scene;
            break;
        case PLAY_SCENE:
            game->current = &game->play.scene;
            break;
    }
}

static void game_handle_scene_request(Game* game, SceneRequest request) {
    switch (request.type) {
        case REQUEST_NONE:
            return;
        case REQUEST_QUIT:
            game->running = false;
            break;
        case REQUEST_SWITCH:
            game_switch_to_scene(game, request.target);
            break;
    }
}

static void game_update(Game* game, float dt) {
    SceneRequest request = {.type = REQUEST_NONE};

    switch (game->current->type) {
        case MENU_SCENE: {
            request = menu_update((MenuScene*)game->current, dt);
            break;
        }
        case PLAY_SCENE: {
            request = play_update((PlayScene*)game->current, dt);
            break;
        }
    }

    game_handle_scene_request(game, request);
}

static void game_draw(Game* game) {
    SceneRequest request = {.type = REQUEST_NONE};

    BeginDrawing();

    switch (game->current->type) {
        case MENU_SCENE: {
            menu_draw_world((MenuScene*)game->current);
            request = menu_draw_ui((MenuScene*)game->current, &game->ui);
            break;
        }
        case PLAY_SCENE: {
            play_draw_world((PlayScene*)game->current);
            request = play_draw_ui((PlayScene*)game->current, &game->ui);
            break;
        }
    }

    EndDrawing();

    game_handle_scene_request(game, request);
}

void game_init(Game* game) {
    InitWindow(900, 600, "Math Defense!?");
    SetTargetFPS(60);

    game->running = true;
    menu_init(&game->menu);
    play_init(&game->play);
    game->current = &game->menu.scene;

    ui_init(&game->ui);
}

void game_run(Game* game) {
    while (!WindowShouldClose() && game->running) {
        float dt = GetFrameTime();

        ui_update(&game->ui);
        game_update(game, dt);
        game_draw(game);
    }
}

void game_free(Game* game) {
    game->running = false;
    game->current = NULL;

    menu_free(&game->menu);
    play_free(&game->play);

    CloseWindow();
}
