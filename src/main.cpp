#include "Game.h"
#include "Scene/Scene.h"

int main() {
    Game game("Breakout ECS", 800, 600);

    // Creas tu escena y la asignas al game
    Scene* scene = new Scene();
    game.setScene(scene);

    // Lo arranca todo (setup, loop, render, clean)
    game.run();

    delete scene;
    return 0;
}
