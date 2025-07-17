#include <raylib.h>
// #include <print>
#include <stdlib.h>
#include <iostream>

#include "Game.h"

Rectangle ball;
Rectangle paddle;

int ball_sx = 150; 
int ball_sy = 150;
int paddle_sx = 200;

Game::Game(const char* title, int width, int height)
    : screen_width(width), screen_height(height) {
    InitWindow(width, height, title);
    //   std::println("GAME STARTED");
    std::cout << "GAME STARTED" << std::endl;
    isRunning = true;
    counter = 0;
}

Game::~Game() {

}

void Game::setup() {
    SetTargetFPS(60);
    
    ball = Rectangle{150, 150, 15, 15};
    paddle = Rectangle{(float)(screen_width/2) - ball.width*5, (float)screen_height - 15, ball.width*10, 15};

    //crear bloques 
    int rows = 4;
    int cols = 10;
    int blockWidth = screen_width / cols;
    int blockHeight = 20;

    for (int row = 0; row < rows; ++row) {
        Color color;
        switch (row) {
            case 0: color = RED; break;
            case 1: color = ORANGE; break;
            case 2: color = YELLOW; break;
            case 3: color = GREEN; break;
            default: color = DARKBLUE; break;
        }
        for (int col = 0; col < cols; ++col) {
            Block block = {
                Rectangle{
                    (float)(col * blockWidth),
                    (float)(row * blockHeight),
                    (float)(blockWidth - 2),
                    (float)(blockHeight - 2)
                },
                color
            };
            bricks.push_back(block);
        }
    }
}

void Game::frame_start() {
    //   std::println("==== FRAME {} START ====", counter);
    std::cout << "==== FRAME " << counter << " START ====" << std::endl;
    BeginDrawing();
}

void Game::frame_end() {
    //   std::println("==== FRAME END ====");
    std::cout << "==== FRAME END ====" << std::endl;
    EndDrawing();
    counter++;
}

void Game::handle_events() {
    float dT = GetFrameTime();  // seconds
    if (WindowShouldClose()) {
        isRunning = false;
    }
    if (IsKeyDown(KEY_RIGHT)) {
        paddle.x += paddle_sx * dT;
    }

    if (IsKeyDown(KEY_LEFT)) {
        paddle.x -= paddle_sx * dT;
    }

    if (paddle.x < 0) {
        paddle.x = 0;
    }
    if (paddle.x + paddle.width > screen_width) {
        paddle.x = screen_width - paddle.width;
    }
}

void Game::update() {
    // Verificar colisiones con bloques
    for (auto it = bricks.begin(); it != bricks.end(); ) {
        if (CheckCollisionRecs(ball, it->rect)) {
            ball_sy *= -1;
            it = bricks.erase(it);
        } else {
            ++it;
        }
    }


    // Revisar victoria
    if (bricks.empty()) {
        std::cout << "YOU WIN!" << std::endl;
        WaitTime(3.0);
        exit(0);
    }

    float dT = GetFrameTime();  // seconds
    if (ball.x + 15 >= screen_width) {
        ball_sx *= -1;
    }
    if (ball.x < 0) {
        ball_sx *= -1;
    } 
    if (ball.y >= screen_height) {
        // std::println("YOU FAIL");
        std::cout << "YOU FAIL" << std::endl;
        WaitTime(3.0);
        exit(1);
    }
    if (CheckCollisionRecs(ball, paddle)) {
        ball_sy *= -1;
        ball_sx = paddle_sx;
    }
    if (ball.y < 0) {
        ball_sy *= -1;
    }
    
    ball.x += ball_sx * dT;
    ball.y += ball_sy * dT;
}

void Game::render() {
    ClearBackground(GRAY);
    DrawText(
        // std::format("FPS: {}", GetFPS()).c_str(),
        TextFormat("FPS: %d", GetFPS()),
        10,     
        10,
        20,
        GRAY
    );
    
    DrawRectangleRec(ball, BLACK);
    DrawRectangleRec(paddle, BLACK);

    for (const auto& block : bricks) {
        DrawRectangleRec(block.rect, block.color);
    }

}

void Game::clean() {
  CloseWindow();
}

bool Game::running() {
  return isRunning;
}