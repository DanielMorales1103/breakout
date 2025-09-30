#include "Scene/SpriteScene.h"
#include "TextureManager.h"  
#include <raylib.h>
#include <cmath>

static const char* BG_PATH = "assets/backgrounds/fondoAtl.png";
static const char* HERO_PATH = "assets/sprites/SpriteSheet.png";
static const char* OCTO_PATH   = "assets/sprites/pulpo.png";
static const char* MERMAN_PATH = "assets/sprites/sirena.png";

static int NUM_OCTO   = 6;
static int NUM_SIRENA = 3;

// Config de sheet (lógica)
static const int COLUMNS = 4; // Derecha, Izquierda, Arriba, Abajo
static const int ROWS    = 4; // <-- si tu hoja tiene 5 frames por dirección
static const float FPS_ANIM_H = 8.0f;
static const float FPS_ANIM_V = 8.0f;
// Factor de velocidad para animación en idle (1.0 = misma velocidad que en movimiento)
static const float IDLE_FPS_MULT = 1.0f; // si lo quieres más lento, pon 0.6f por ejemplo


enum class Dir { Right, Left, Up, Down };
static Dir   heroDir    = Dir::Right;
static bool  heroMoving = false;

// Estado de animación
static int   heroFrame  = 0;   // 0..3 (la usaremos distinta según dir)
static float heroAcc    = 0.0f;

// Estado de juego (posición/velocidad)
static float heroX, heroY;          // centro de destino
static float heroSpeed = 200.0f;    // px/s

// Calculados al cargar
static int FRAME_W = 0;
static int FRAME_H = 0;

struct Enemy {
    const char* path;
    int columns = 4;
    int rows    = 4;
    int frameW = 0, frameH = 0;

    int offsetX = 0, offsetY = 0;
    int spacingX = 0, spacingY = 0;

    int   frame = 0;
    float acc   = 0.0f;
    float fps   = 6.0f;

    float x = 0, y = 0;
    float speed = 60.0f;
    float scale = 0.30f;
    Vector2 target{0,0};
    float retargetTimer = 0.0f;
};

static std::vector<Enemy> gEnemies;

static float frand(float a, float b) {
    return a + (b - a) * (GetRandomValue(0, 10000) / 10000.0f);
}
static void EnemyPickNewTarget(Enemy& e) {
    const float halfW = (e.frameW * e.scale) * 0.5f;
    const float halfH = (e.frameH * e.scale) * 0.5f;
    const float margin = 8.0f;

    e.target.x = frand(margin + halfW, GetScreenWidth()  - margin - halfW);
    e.target.y = frand(margin + halfH, GetScreenHeight() - margin - halfH);
    e.retargetTimer = frand(1.2f, 3.5f);
}

void SpriteScene::onSetup() {
    TextureManager::LoadTexture(BG_PATH);
    TextureManager::LoadTexture(HERO_PATH);

    SetTextureFilter(TextureManager::GetTexture(BG_PATH),  TEXTURE_FILTER_POINT);
    SetTextureFilter(TextureManager::GetTexture(HERO_PATH), TEXTURE_FILTER_POINT);

    // Centrar al héroe
    heroX = GetScreenWidth()  * 0.5f;
    heroY = GetScreenHeight() * 0.5f;

    // Calcular tamaño de frame desde la textura
    const Texture2D hero = TextureManager::GetTexture(HERO_PATH);
    FRAME_W = hero.width  / COLUMNS;
    FRAME_H = hero.height / ROWS;

    // --- Enemigos ---
    gEnemies.clear();
    TextureManager::LoadTexture(OCTO_PATH);
    TextureManager::LoadTexture(MERMAN_PATH);
    SetTextureFilter(TextureManager::GetTexture(OCTO_PATH),   TEXTURE_FILTER_POINT);
    SetTextureFilter(TextureManager::GetTexture(MERMAN_PATH), TEXTURE_FILTER_POINT);
    gEnemies.reserve(NUM_OCTO + NUM_SIRENA);

    // Pulpos 
    for (int i = 0; i < NUM_OCTO; ++i) {
        Enemy e;
        e.path    = OCTO_PATH;
        e.columns = 4;
        e.rows    = 4;

        const Texture2D t = TextureManager::GetTexture(e.path);
        e.frameH = t.height / e.rows - 10;  
        e.frameW = e.frameH;                

        const int usedW = e.columns * e.frameW;
        const int usedH = e.rows    * e.frameH;
        e.offsetX = (t.width  - usedW) > 0 ? (t.width  - usedW) / 2 : 0;
        e.offsetY = (t.height - usedH) > 0 ? (t.height - usedH) / 2 : 0;

        e.fps   = 6.0f;
        e.speed = 50.0f;
        e.scale = 0.30f;

        e.x = frand(80.0f, GetScreenWidth()  - 80.0f);
        e.y = frand(80.0f, GetScreenHeight() - 80.0f);

        EnemyPickNewTarget(e);
        gEnemies.push_back(e);
    }
    //Sirenas
    for (int i = 0; i < NUM_SIRENA; ++i) {
        Enemy e;
        e.path    = MERMAN_PATH;
        e.columns = 4;
        e.rows    = 3;

        const Texture2D t = TextureManager::GetTexture(e.path);
        e.frameH = t.height / e.rows;   
        e.frameW = 265;                     

        const int usedW = e.columns * e.frameW;
        const int usedH = e.rows    * e.frameH;
        e.offsetX = (t.width  - usedW) > 0 ? (t.width  - usedW) / 2 : 0;
        e.offsetY = (t.height - usedH) > 0 ? (t.height - usedH) / 2 : 0;

        e.fps   = 6.0f;
        e.speed = 70.0f;
        e.scale = 0.30f;

        e.x = frand(80.0f, GetScreenWidth()  - 80.0f);
        e.y = frand(80.0f, GetScreenHeight() - 80.0f);

        EnemyPickNewTarget(e);
        gEnemies.push_back(e);
    }
}

// --- ON RENDER ---
void SpriteScene::onRender() {
    const Texture2D bg = TextureManager::GetTexture(BG_PATH);
    DrawTexturePro(bg, {0,0,(float)bg.width,(float)bg.height},
                      {0,0,(float)GetScreenWidth(),(float)GetScreenHeight()},
                      {0,0}, 0.0f, WHITE);
    // --- Enemigos ---
    for (const auto& e : gEnemies) {
        const Texture2D tex = TextureManager::GetTexture(e.path);

        const int colE = e.frame % e.columns;
        const int rowE = e.frame / e.columns;

        Rectangle srcE{
            (float)(e.offsetX + colE * e.frameW),
            (float)(e.offsetY + rowE * e.frameH),
            (float)e.frameW,
            (float)e.frameH
        };

        const float dstWE = e.frameW * e.scale;
        const float dstHE = e.frameH * e.scale;

        Rectangle dstE{ e.x, e.y, dstWE, dstHE };
        Vector2   originE{ dstWE * 0.5f, dstHE * 0.5f };

        DrawTexturePro(tex, srcE, dstE, originE, 0.0f, WHITE);
    }

    // --- Héroe ---
    const Texture2D hero = TextureManager::GetTexture(HERO_PATH);

    int col = 0;
    switch (heroDir) {
        case Dir::Right: col = 0; break; // columna 1
        case Dir::Left:  col = 1; break; // columna 2
        case Dir::Up:    col = 2; break; // columna 3
        case Dir::Down:  col = 3; break; // columna 4
    }
    const int row = heroFrame % ROWS; // 0..ROWS-1

    Rectangle srcHero {
        (float)(col * FRAME_W),
        (float)(row * FRAME_H),
        (float)FRAME_W,
        (float)FRAME_H
    };

    const float scale = 0.25f;
    const float dstW = FRAME_W * scale;
    const float dstH = FRAME_H * scale;

    // Dibujo centrado + pivote al centro (evita “saltos” al cambiar dirección)
    Rectangle dstHero { heroX, heroY, dstW, dstH };
    Vector2   origin  { dstW * 0.5f, dstH * 0.5f };

    DrawTexturePro(hero, srcHero, dstHero, origin, 0.0f, WHITE);
}

void SpriteScene::onUpdate() {
    const float dt = GetFrameTime();

    // --- Input ---
    float vx = 0.0f, vy = 0.0f;
    if (IsKeyDown(KEY_RIGHT)) { vx += 1.0f; heroDir = Dir::Right; }
    if (IsKeyDown(KEY_LEFT))  { vx -= 1.0f; heroDir = Dir::Left;  }
    if (IsKeyDown(KEY_DOWN))  { vy += 1.0f; heroDir = Dir::Down;  }
    if (IsKeyDown(KEY_UP))    { vy -= 1.0f; heroDir = Dir::Up;    }

    heroMoving = (vx != 0.0f || vy != 0.0f);

    // Normalizar diagonal
    if (heroMoving && vx != 0.0f && vy != 0.0f) {
        const float inv = 1.0f / std::sqrt(2.0f);
        vx *= inv; vy *= inv;
    }

    // --- Movimiento ---
    heroX += vx * heroSpeed * dt;
    heroY += vy * heroSpeed * dt;

    // Clamp considerando tamaño dibujado
    const float scale = 0.25f;
    const float halfW = (FRAME_W * scale) * 0.5f;
    const float halfH = (FRAME_H * scale) * 0.5f;
    const float margin = 8.0f;
    heroX = fmaxf(margin + halfW, fminf(heroX, GetScreenWidth()  - margin - halfW));
    heroY = fmaxf(margin + halfH, fminf(heroY, GetScreenHeight() - margin - halfH));

    // --- Animación (también en idle) ---
    const bool  vertical = (heroDir == Dir::Up || heroDir == Dir::Down);
    const float fpsBase  = vertical ? FPS_ANIM_V : FPS_ANIM_H;
    const float fps      = heroMoving ? fpsBase : fpsBase * IDLE_FPS_MULT;

    if (fps > 0.0f) {
        const float frameTime = 1.0f / fps;
        heroAcc += dt;
        while (heroAcc >= frameTime) {
            heroAcc -= frameTime;
            heroFrame = (heroFrame + 1) % ROWS; // mismo conteo para todas las dirs
        }
    }
    // --- Enemigos (wander + idle loop) ---
    for (auto& e : gEnemies) {
        // retarget por tiempo
        e.retargetTimer -= dt;
        if (e.retargetTimer <= 0.0f) {
            EnemyPickNewTarget(e);
        }

        // mover hacia el destino
        Vector2 d{ e.target.x - e.x, e.target.y - e.y };
        float len  = std::sqrt(d.x*d.x + d.y*d.y);
        float step = e.speed * dt;

        if (len > 1e-3f) {
            if (step >= len) {
                e.x = e.target.x; e.y = e.target.y;
                EnemyPickNewTarget(e);
            } else {
                d.x /= len; d.y /= len;
                e.x += d.x * step;
                e.y += d.y * step;
            }
        } else {
            EnemyPickNewTarget(e);
        }

        // animación idle continua
        const float frameTimeE = 1.0f / e.fps;
        e.acc += dt;
        while (e.acc >= frameTimeE) {
            e.acc -= frameTimeE;
            e.frame = (e.frame + 1) % (e.columns * e.rows);
        }
    }
}
