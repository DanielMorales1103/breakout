#include "Game.h"

int main() {
    Game game("Breakout ECS", 800, 600);
    game.setScene(new Scene());
    game.run();
    return 0;
}
