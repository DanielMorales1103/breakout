#pragma once
#include <raylib.h>
#include <vector>

struct Block {
    Rectangle rect;
    Color color;
};

class Game {
public:
    Game(const char* title, int width, int height);
    ~Game();

    void setup();
    void frame_start();
    void handle_events();
    void update();
    void render();
    void frame_end();
    void clean();
    bool running();

private:
    int counter;
    int screen_width;
    int screen_height;
    bool isRunning;

    std::vector<Block> bricks;
};