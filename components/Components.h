#pragma once

#include <string>
#include <raylib.h>

struct NameComponent {
    std::string tag;
};

struct TransformComponent {
    Vector2 position{0,0};
    float rotation = 0.0f;
    float scale = 1.0f;
};

struct SpriteComponent {
    std::string texturePath; 
    Rectangle src{0,0,0,0}; 
    Vector2 sizePx{0,0};     
};

struct AnimatorComponent {
    int columns = 1;
    int rows = 1;
    float fps = 0.0f;
    int frame = 0;
    float acc = 0.0f;
    bool moving = false;
    enum Facing { Right, Left, Up, Down } facing = Right;

    int baseCol = 0;   
    int baseRow = 0;
};

struct PlayerTag {};
struct EnemyTag {};

struct WanderAIComponent {
    Vector2 target{0,0};
    float retargetTimer = 0.0f;
    float speed = 60.0f;
};

struct BoundsClampComponent {
    float margin = 0.0f;
};

struct VelocityComponent {
    Vector2 velocity{0,0};
};

struct PaddleComponent { 
    int dummy = 0; 
};

struct BlockComponent { 
    int dummy = 0; 
};

struct BallComponent { 
    int dummy = 0; 
};

struct SizeComponent { 
    Vector2 size; 
};

struct BackgroundTag {};

struct FollowAIComponent {
    float speed = 60.0f;     
    float stopRadius = 0.0f; 
};

struct TilesetComponent {
    const char* texturePath = nullptr;
    Vector2 tileSize{16,16};
    int columns = 0;
    int rows    = 0;
    int margin  = 0;
    int spacing = 0;

    Rectangle clip{0,0,0,0};
};

struct TilemapComponent {
    int width  = 0;              // celdas
    int height = 0;              // celdas
    std::vector<int> tiles;      // índice de tile en el atlas; -1 = vacío
};

struct TilemapTag {}; 

struct MapSourceComponent {
    std::string csvPath;   // ruta al CSV con los índices de tiles
};
