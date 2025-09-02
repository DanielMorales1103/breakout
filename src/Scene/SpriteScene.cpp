#include "Scene/SpriteScene.h"
#include "TextureManager.h"  
#include <raylib.h>

static const char* BG_PATH = "assets/backgrounds/fondo.jpg";
static const char* HERO_PATH = "assets/sprites/SpriteSheet.png";

// Config de sheet (lógica)
static const int COLUMNS = 3; // 3 columnas
static const int ROWS    = 4; // 4 filas

static const float FPS_ANIM_H = 8.0f; 
static const float FPS_ANIM_V = 8.0f; 

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
}

void SpriteScene::onRender() {
    // --- Fondo ---
    const Texture2D bg = TextureManager::GetTexture(BG_PATH);
    const Rectangle srcBg{0,0,(float)bg.width,(float)bg.height};
    const Rectangle dstBg{0,0,(float)GetScreenWidth(),(float)GetScreenHeight()};
    DrawTexturePro(bg, srcBg, dstBg, {0,0}, 0.0f, WHITE);

    // --- Héroe ---
    const Texture2D hero = TextureManager::GetTexture(HERO_PATH);

    int col = 0;
    int row = 0;
    bool flipX = false;
    bool flipY = false;

    // Selección según lo que pediste:
    // - Horizontal izq/der: columna 1 (index 0), filas 0..3 (4 frames)
    // - Vertical up/down:  columna 3 (index 2), filas 1..3 (3 frames)
    switch (heroDir) {
        case Dir::Right:
            col   = 0;           // 1a columna
            row   = heroFrame;   // 0..3
            flipX = false;
            break;
        case Dir::Left:
            col   = 0;           // 1a columna
            row   = heroFrame;   // 0..3
            flipX = true;        // flip horizontal para mirar a la izquierda
            break;
        case Dir::Down:
            col   = 2;                       // 3a columna
            row   = 1 + (heroFrame % 3);     // filas 1..3
            flipX = false;
            break;
        case Dir::Up:
            col   = 2;                       // 3a columna
            row   = 1 + (heroFrame % 3);     // filas 1..3
            flipX = false;
            flipY = true;                     // opcional: flip vertical si te gusta más la lectura visual
            break;
    }

    // Rect fuente (con flips si aplica)
    Rectangle srcHero {
        (float)(col * FRAME_W),
        (float)(row * FRAME_H),
        (float)FRAME_W,
        (float)FRAME_H
    };
    if (flipX) { srcHero.x += FRAME_W; srcHero.width  = -srcHero.width;  }
    if (flipY) { srcHero.y += FRAME_H; srcHero.height = -srcHero.height; }

    // Rect destino (dibujar centrado, escalar si tu frame es grande/pequeño)
    const float scale = 0.25f; // ajusta a gusto
    const float dstW = FRAME_W * scale;
    const float dstH = FRAME_H * scale;

    Rectangle dstHero {
        heroX - dstW/2.0f,
        heroY - dstH/2.0f,
        dstW, dstH
    };

    DrawTexturePro(hero, srcHero, dstHero, {0,0}, 0.0f, WHITE);
}
void SpriteScene::onUpdate() {
    float dt = GetFrameTime();

    // --- INPUT ---
    float vx = 0.0f, vy = 0.0f;

    if (IsKeyDown(KEY_RIGHT)) { vx += 1.0f; heroDir = Dir::Right; }
    if (IsKeyDown(KEY_LEFT))  { vx -= 1.0f; heroDir = Dir::Left;  }
    if (IsKeyDown(KEY_DOWN))  { vy += 1.0f; heroDir = Dir::Down;  }
    if (IsKeyDown(KEY_UP))    { vy -= 1.0f; heroDir = Dir::Up;    }

    heroMoving = (vx != 0.0f || vy != 0.0f);

    // Normalizar diagonal
    if (heroMoving && std::fabs(vx) > 0.0f && std::fabs(vy) > 0.0f) {
        const float inv = 1.0f / std::sqrt(2.0f);
        vx *= inv; vy *= inv;
    }

    // Mover
    heroX += vx * heroSpeed * dt;
    heroY += vy * heroSpeed * dt;

    // Limites simples
    const float margin = 8.0f;
    heroX = fmaxf(margin, fminf(heroX, GetScreenWidth()  - margin));
    heroY = fmaxf(margin, fminf(heroY, GetScreenHeight() - margin));

    // --- ANIMACIÓN ---
    // Horizontal → 4 frames (filas 0..3) en la columna 0
    // Vertical   → 3 frames (filas 1..3) en la columna 2
    const float fps = (heroDir == Dir::Up || heroDir == Dir::Down) ? FPS_ANIM_V : FPS_ANIM_H;
    const float frameTime = 1.0f / fps;

    if (heroMoving) {
        heroAcc += dt;
        while (heroAcc >= frameTime) {
            heroAcc -= frameTime;
            // Para horizontal usamos 0..3, para vertical solo 0..2
            if (heroDir == Dir::Up || heroDir == Dir::Down) {
                heroFrame = (heroFrame + 1) % 3; // 3 frames
            } else {
                heroFrame = (heroFrame + 1) % 4; // 4 frames
            }
        }
    } else {
        // Idle: frame base
        heroAcc = 0.0f;
        heroFrame = 0;
    }
}