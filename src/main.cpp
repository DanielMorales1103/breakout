#include "Game.h"
#include "Scene/BreakoutScene.h"
#include "Scene/SpriteScene.h"

int main() {
    Game game("Breakout ECS", 800, 600);

    // Para correr Breakout:
    BreakoutScene scene; 
    game.setScene(&scene);

    // Para trabajar con sprites:
    // SpriteScene scene; 
    //game.setScene(&scene);

    // Lo arranca todo (setup, loop, render, clean)
    game.run();

    return 0;
}
