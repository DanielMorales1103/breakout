#pragma once

#include <string>
#include <raylib.h>
#include <random>

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
    int   baseIndex   = 1;    
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
    IGF_Walkable = 1 << 0,  
    IGF_Block    = 1 << 1, 
    IGF_Hazard   = 1 << 2,  
    IGF_Slow     = 1 << 3,
    IGF_Current  = 1 << 4
};

struct IntGridComponent {
    int width = 0, height = 0;
    std::vector<unsigned char> cells;
};

struct IntGridRules {
    std::vector<int> nonWalkableIndices; 
    std::vector<int> hazardIndices;      
    int slowIndex = 16;    
    std::vector<int> currentIndices;               
    unsigned char defaultFlags = IGF_Walkable; 
};

struct HazardSettings { float pushBack = 60.0f; };
struct SlowSettings   { float speedFactor = 0.30f;  float immediateBrake  = 0.6f; 
    float maxSpeedOnSlow  = 60.0f;};

struct CurrentSettings {
    float force   = 60.0f; 
    float maxSpeed = 180.0f;
};

// --- Definición de una oleada ---
enum class SpawnPattern {
    Line,       
    Circle,
    RandomArea,
};
struct WaveDef {
    int   count         = 5;      
    float interval      = 0.50f;  
    float startDelay    = 1.0f;  
    int waveIndex        = 0; 
    SpawnPattern pattern = SpawnPattern::Line;   
};

struct EnemySpawnSettings {
    std::vector<WaveDef> waves;
    bool   loop   = false;
    float  scale  = 3.0f; 
};

struct EnemySpawnState {
    int   curWave        = 0;
    int   spawnedInWave  = 0;
    float waveTimer      = 0.f;  
    float spawnTimer     = 0.f;  
    bool  waveActive     = false;
    
    SpawnPattern currentPattern = SpawnPattern::Line;
    bool         patternLocked  = false;

    bool     lineInit = false;
    bool     lineHorizontal = false;
    Vector2  lineBase{0,0};
    float    lineSpacing = 0.f;
    bool     circleInit = false;
    Vector2  circleCenter{0,0};
    float    circleRadius = 0.f;
};

struct EnemySpawnerComponent {
    Vector2 basePosition { 0, 0 };
    float   timeAcc        = 0.0f;   
    float   waveInterval   = 3.0f;   
    int     countPerWave   = 4;      
    bool    enabled        = true;
    SpawnPattern pattern   = SpawnPattern::Line;
    Vector2 lineDir        { 1, 0 }; 
    float   lineSpacing    = 40.0f;  
    float   circleRadius   = 80.0f;
    Vector2 areaHalfExtents{ 120, 80 }; 
    float   jitter         = 6.0f;
};
