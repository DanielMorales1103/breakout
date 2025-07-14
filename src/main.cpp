#include "Game.h"

int main() {
    Game game("Breakout", 800, 600);
    game.setup();

    while (game.running()) {
        game.frame_start();
        game.handle_events();
        game.update();
        game.render();
        game.frame_end();
    }

    game.clean();
    return 0;
}
