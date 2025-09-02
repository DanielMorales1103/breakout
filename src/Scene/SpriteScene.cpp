#include "Scene/SpriteScene.h"
#include "TextureManager.h"  
#include <raylib.h>
#include <cmath>

static const char* BG_PATH = "assets/backgrounds/fondo.jpg";
static const char* HERO_PATH = "assets/sprites/SpriteSheet.png";

// Config de sheet (lógica)
static const int COLUMNS = 4; // Derecha, Izquierda, Arriba, Abajo
static const int ROWS    = 4; // <-- si tu hoja tiene 5 frames por dirección
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

// --- ON RENDER (reemplaza la parte del héroe completa) ---
void SpriteScene::onRender() {
    // --- Fondo (igual que antes) ---
    const Texture2D bg = TextureManager::GetTexture(BG_PATH);
    DrawTexturePro(bg, {0,0,(float)bg.width,(float)bg.height},
                      {0,0,(float)GetScreenWidth(),(float)GetScreenHeight()},
                      {0,0}, 0.0f, WHITE);

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
    float dt = GetFrameTime();

    float vx = 0.0f, vy = 0.0f;
    if (IsKeyDown(KEY_RIGHT)) { vx += 1.0f; heroDir = Dir::Right; }
    if (IsKeyDown(KEY_LEFT))  { vx -= 1.0f; heroDir = Dir::Left;  }
    if (IsKeyDown(KEY_DOWN))  { vy += 1.0f; heroDir = Dir::Down;  }
    if (IsKeyDown(KEY_UP))    { vy -= 1.0f; heroDir = Dir::Up;    }

    heroMoving = (vx != 0.0f || vy != 0.0f);

    if (heroMoving && vx != 0.0f && vy != 0.0f) {
        const float inv = 1.0f / std::sqrt(2.0f);
        vx *= inv; vy *= inv;
    }

    heroX += vx * heroSpeed * dt;
    heroY += vy * heroSpeed * dt;

    // Clamp considerando tamaño dibujado
    const float scale = 0.25f;
    const float halfW = (FRAME_W * scale) * 0.5f;
    const float halfH = (FRAME_H * scale) * 0.5f;
    const float margin = 8.0f;
    heroX = fmaxf(margin + halfW, fminf(heroX, GetScreenWidth()  - margin - halfW));
    heroY = fmaxf(margin + halfH, fminf(heroY, GetScreenHeight() - margin - halfH));

    // Animación (usa FPS_H para L/R, FPS_V para U/D)
    const bool vertical = (heroDir == Dir::Up || heroDir == Dir::Down);
    const float fps = vertical ? FPS_ANIM_V : FPS_ANIM_H;
    const float frameTime = 1.0f / fps;

    if (heroMoving) {
        heroAcc += dt;
        while (heroAcc >= frameTime) {
            heroAcc -= frameTime;
            heroFrame = (heroFrame + 1) % ROWS; // mismo conteo para todas las dirs
        }
    } else {
        heroAcc = 0.0f;
        heroFrame = 0; // idle (primer frame de la columna de la dir actual)
    }
}
