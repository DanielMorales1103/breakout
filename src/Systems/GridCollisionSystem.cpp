#include "Systems/GridCollisionSystem.h"
#include "../components/Components.h"
#include "Scene/Scene.h"
#include <raylib.h>
#include <cmath>
#include <algorithm>

static inline int idx2d(int x, int y, int w) { return y * w + x; }

struct GridCtx {
    const IntGridComponent* ig = nullptr;
    Vector2 origin; // tt->position
    float cellW = 16.f, cellH = 16.f;
};

static inline void worldRectToTileRange(const GridCtx& g, float minX, float minY, float maxX, float maxY,
                                        int& minTX, int& minTY, int& maxTX, int& maxTY) {
    minTX = (int)std::floor((minX - g.origin.x) / g.cellW);
    minTY = (int)std::floor((minY - g.origin.y) / g.cellH);
    maxTX = (int)std::floor((maxX - g.origin.x - 0.001f) / g.cellW);
    maxTY = (int)std::floor((maxY - g.origin.y - 0.001f) / g.cellH);
    // clamp
    minTX = std::max(0, std::min(minTX, g.ig->width  - 1));
    minTY = std::max(0, std::min(minTY, g.ig->height - 1));
    maxTX = std::max(0, std::min(maxTX, g.ig->width  - 1));
    maxTY = std::max(0, std::min(maxTY, g.ig->height - 1));
}

static inline void queryFlagsInRect(const GridCtx& g, float minX, float minY, float maxX, float maxY,
                                    bool& anyBlock, bool& anyHazard, bool& anySlow) {
    int minTX, minTY, maxTX, maxTY;
    worldRectToTileRange(g, minX, minY, maxX, maxY, minTX, minTY, maxTX, maxTY);
    anyBlock = anyHazard = anySlow = false;

    for (int ty = minTY; ty <= maxTY; ++ty) {
        for (int tx = minTX; tx <= maxTX; ++tx) {
            unsigned char f = g.ig->cells[idx2d(tx, ty, g.ig->width)];
            if (f & IGF_Block)  { anyBlock  = true; }
            if (f & IGF_Hazard) { anyHazard = true; }
            if (f & IGF_Slow)   { anySlow   = true; }
        }
    }
}

// prueba colisión moviendo en X; mantiene el CENTRO-Y base
static inline bool testBlockX_center(const GridCtx& g, float baseCenterY, float newCenterX,
                                     float halfW, float halfH) {
    const float minX = newCenterX - halfW;
    const float maxX = newCenterX + halfW;
    const float minY = baseCenterY - halfH;
    const float maxY = baseCenterY + halfH;
    bool b,hz,sl;
    queryFlagsInRect(g, minX, minY, maxX, maxY, b, hz, sl);
    return b;
}

// prueba colisión moviendo en Y; mantiene el CENTRO-X base
static inline bool testBlockY_center(const GridCtx& g, float baseCenterX, float newCenterY,
                                     float halfW, float halfH) {
    const float minX = baseCenterX - halfW;
    const float maxX = baseCenterX + halfW;
    const float minY = newCenterY - halfH;
    const float maxY = newCenterY + halfH;
    bool b,hz,sl;
    queryFlagsInRect(g, minX, minY, maxX, maxY, b, hz, sl);
    return b;
}


void GridCollisionSystem::update() {
    const float dt = GetFrameTime();

    // Contexto de grid
    entt::entity mapE = entt::null;
    TilemapComponent*   tm = nullptr;
    TilesetComponent*   ts = nullptr;
    TransformComponent* tt = nullptr;
    IntGridComponent*   ig = nullptr;
    HazardSettings*     hz = nullptr;
    SlowSettings*       sl = nullptr;

    auto tv = scene->r.view<TilemapComponent, TilesetComponent, TransformComponent, IntGridComponent, TilemapTag>();
    tv.each([&](auto e, TilemapComponent &m, TilesetComponent &tset, TransformComponent &tr, IntGridComponent &grid){
        if (mapE == entt::null) { mapE = e; tm=&m; ts=&tset; tt=&tr; ig=&grid; }
    });
    if (!ig || ig->width <= 0 || ig->height <= 0) return;

    if (scene->r.any_of<HazardSettings>(mapE)) hz = &scene->r.get<HazardSettings>(mapE);
    if (scene->r.any_of<SlowSettings>(mapE))   sl = &scene->r.get<SlowSettings>(mapE);

    const float pushBack   = hz ? hz->pushBack    : 60.0f;
    const float slowFactor = sl ? sl->speedFactor : 0.40f;

    const float S = (tt->scale <= 0.f) ? 1.f : tt->scale;

    GridCtx g;
    g.ig     = ig;
    g.origin = tt->position;
    g.cellW  = ts->tileSize.x * S;
    g.cellH  = ts->tileSize.y * S;

    // Jugador/es: resolvemos por ejes con AABB completo
    auto pv = scene->r.view<TransformComponent, VelocityComponent, SpriteComponent>();
    pv.each([&](entt::entity e, TransformComponent &t, VelocityComponent &v, SpriteComponent &s) {
        if (!scene->r.any_of<PlayerTag>(e)) return;

        const float scale = (t.scale <= 0.f) ? 1.f : t.scale;
        const float w = s.src.width  * scale;
        const float h = s.src.height * scale;
        const float halfW = w * 0.5f;
        const float halfH = h * 0.5f;

        // Posiciones antes/después del MovementSystem (Movement ya movió)
        const float dx = v.velocity.x * dt;
        const float dy = v.velocity.y * dt;
        const float prevCX = t.position.x - dx; 
        const float prevCY = t.position.y - dy;

        // 1) Resolver X primero (snap sencillo: si choca, cancelar el avance en X)
        if (dx != 0.0f) {
            const float newCX = t.position.x; // ya movido por Movement
            if (testBlockX_center(g, prevCY, newCX, halfW, halfH)) {
                t.position.x = prevCX; // cancela X
                v.velocity.x = 0.0f;
            }
        }

        // 2) Luego resolver Y
        if (dy != 0.0f) {
            const float newCY = t.position.y; // ya movido por Movement
            if (testBlockY_center(g, t.position.x, newCY, halfW, halfH)) {
                t.position.y = prevCY; // cancela Y
                v.velocity.y = 0.0f;
            }
        }

        // 3) Efectos (hazard/slow) sobre el AABB final
        bool anyBlock=false, anyHazard=false, anySlow=false;
        queryFlagsInRect(g, t.position.x - halfW, t.position.y - halfH, 
                            t.position.x + halfW, t.position.y + halfH,
                         anyBlock, anyHazard, anySlow);

        if (anyHazard) {
            const float len = std::sqrt(v.velocity.x*v.velocity.x + v.velocity.y*v.velocity.y);
            if (len > 0.0001f) {
                const float nx = v.velocity.x / len;
                const float ny = v.velocity.y / len;
                t.position.x -= nx * pushBack * dt;
                t.position.y -= ny * pushBack * dt;
            }
            // (daño/FX luego)
        }

        if (anySlow) {
            v.velocity.x *= slowFactor;
            v.velocity.y *= slowFactor;
            t.position.x -= (1.0f - slowFactor) * v.velocity.x * dt;
            t.position.y -= (1.0f - slowFactor) * v.velocity.y * dt;
        }
    });
}
