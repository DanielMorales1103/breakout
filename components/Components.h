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

// Relleno automático del tilemap alrededor de la cámara
struct AutoTileFillComponent {
    int   baseIndex   = 5;    
    int   padding     = 2;    
    bool  enabled     = true; 
};


struct MapSourceComponent {
    std::string csvPath;   // ruta al CSV con los índices de tiles
};

struct CameraTag {};

struct CameraComponent {
    bool  active = true;  
    float zoom   = 1.0f;  
};

struct ViewportComponent {
    int width  = 320;     
    int height = 180;
};

struct CameraFollowSettings {
    float deadzoneW = 96.0f;  
    float deadzoneH = 64.0f;  
    float damping   = 0.15f;  
};

// -------- IntGrid flags (bitmask) --------
enum IntCellFlags : unsigned char {
    IGF_None     = 0,
    IGF_Walkable = 1 << 0,  // se puede caminar
    IGF_Block    = 1 << 1,  // muro/ruina (bloquea)
    IGF_Hazard   = 1 << 2,  // empuja (piedras/corales)
    IGF_Slow     = 1 << 3   // ralentiza (p.ej. índice 16)
};

struct IntGridComponent {
    int width = 0, height = 0;
    std::vector<unsigned char> cells; // width*height, combinación de IGF_*
};

struct IntGridRules {
    std::vector<int> nonWalkableIndices; // muros/ruinas -> Block
    std::vector<int> hazardIndices;      // piedras     -> Hazard
    int slowIndex = 16;                  // SOLO este índice será Slow
    unsigned char defaultFlags = IGF_Walkable; // arena por defecto
};

struct HazardSettings { float pushBack = 60.0f; };
struct SlowSettings   { float speedFactor = 0.40f; };
