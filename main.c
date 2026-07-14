#include "game.h"

int main(void) {
    Game game;

    game_init(&game);
    game_run(&game);
    game_free(&game);

    return 0;
}
