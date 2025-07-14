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
  
  ball = Rectangle{10, 10, 15, 15};
  paddle = Rectangle{(float)(screen_width/2) - ball.width*5, (float)screen_height - 15, ball.width*10, 15};
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
}

void Game::update() {
    float dT = GetFrameTime();  // seconds
    if (ball.x >= screen_width) {
        ball_sx *= -1;
    }
    if (ball.x < 0) {
        ball_sx *= -1;
    } 
    if (ball.y >= screen_height) {
        // std::println("YOU FAIL");
        std::cout << "YOU FAIL" << std::endl;
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
}

void Game::clean() {
  CloseWindow();
}

bool Game::running() {
  return isRunning;
}