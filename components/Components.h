#pragma once

#include <string>
#include <raylib.h>

struct NameComponent {
    std::string tag;
};

struct TransformComponent {
    Vector2 position;
};

struct VelocityComponent {
    Vector2 velocity;
};

struct PaddleComponent {};
struct BallComponent {};
struct BlockComponent {};
