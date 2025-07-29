#pragma once

#include "Scene/Scene.h"

class Game {
public:
    Game(const char* title, int width, int height);
    ~Game();

    void setup();
    void frameStart();
    void handleEvents();
    void update();
    void render();
    void frameEnd();
    void clean();
    void run();
    bool running() const;

    void setScene(Scene* newScene);
    Scene* getCurrentScene() const;

private:
    int screen_width;
    int screen_height;
    bool isRunning;
    int frameCount;
    float dT;
    float FPS;
    Scene* currentScene = nullptr;
};
