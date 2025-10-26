#include "Systems/GridCollisionSystem.h"
#include "../components/Components.h"
#include "Scene/Scene.h"
#include <raylib.h>
#include <cmath>
#include <algorithm>
#include <iostream>

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
                                    bool& anyBlock, bool& anyHazard, bool& anySlow, bool& anyCurrent) {
    int minTX, minTY, maxTX, maxTY;
    worldRectToTileRange(g, minX, minY, maxX, maxY, minTX, minTY, maxTX, maxTY);
    anyBlock = anyHazard = anySlow = anyCurrent = false;

    for (int ty = minTY; ty <= maxTY; ++ty) {
        for (int tx = minTX; tx <= maxTX; ++tx) {
            unsigned char f = g.ig->cells[idx2d(tx, ty, g.ig->width)];
            if (f & IGF_Block)  { anyBlock  = true; }
            if (f & IGF_Hazard) { anyHazard = true; }
            if (f & IGF_Slow)   { anySlow   = true; }
            if (f & IGF_Current) { anyCurrent = true; }
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
    bool b,hz,sl,cur;
    queryFlagsInRect(g, minX, minY, maxX, maxY, b, hz, sl, cur);
    return b;
}

static inline bool testBlockY_center(const GridCtx& g, float baseCenterX, float newCenterY,
                                     float halfW, float halfH) {
    const float minX = baseCenterX - halfW;
    const float maxX = baseCenterX + halfW;
    const float minY = newCenterY - halfH;
    const float maxY = newCenterY + halfH;
    bool b,hz,sl,cur;
    queryFlagsInRect(g, minX, minY, maxX, maxY, b, hz, sl, cur);
    return b;
}

static inline Vector2 contactNormalAABB(
    float px0, float py0, float px1, float py1,   
    float tx0, float ty0, float tx1, float ty1    
) {
    float dLeft   = std::fabs(px1 - tx0); 
    float dRight  = std::fabs(px0 - tx1); 
    float dTop    = std::fabs(py1 - ty0); 
    float dBottom = std::fabs(py0 - ty1);

    float m = dLeft;
    int side = 0; 
    if (dRight  < m) { m = dRight;  side = 1; }
    if (dTop    < m) { m = dTop;    side = 2; }
    if (dBottom < m) { m = dBottom; side = 3; }

    switch (side) {
        case 0: return { -1.f,  0.f }; 
        case 1: return {  1.f,  0.f }; 
        case 2: return {  0.f, -1.f }; 
        case 3: return {  0.f,  1.f }; 
    }
    return {0.f, -1.f};
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
    CurrentSettings*    cr = nullptr;

    auto tv = scene->r.view<TilemapComponent, TilesetComponent, TransformComponent, IntGridComponent, TilemapTag>();
    tv.each([&](auto e, TilemapComponent &m, TilesetComponent &tset, TransformComponent &tr, IntGridComponent &grid){
        if (mapE == entt::null) { mapE = e; tm=&m; ts=&tset; tt=&tr; ig=&grid; }
    });
    if (!ig || ig->width <= 0 || ig->height <= 0) return;

    if (scene->r.any_of<HazardSettings>(mapE)) hz = &scene->r.get<HazardSettings>(mapE);
    if (scene->r.any_of<SlowSettings>(mapE))   sl = &scene->r.get<SlowSettings>(mapE);
    if (scene->r.any_of<CurrentSettings>(mapE)) cr = &scene->r.get<CurrentSettings>(mapE);

    const float pushBack        = hz ? hz->pushBack    : 60.0f;
    const float slowFactor      = sl ? sl->speedFactor : 0.40f;
    const float currentForce    = cr ? cr->force : 60.0f;
    const float curMaxSpd       = cr ? cr->maxSpeed    : 180.0f;

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
        const Vector2 vDesired = v.velocity;
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
        bool anyBlock=false, anyHazard=false, anySlow=false, anyCurrent=false;
        queryFlagsInRect(g, t.position.x - halfW, t.position.y - halfH, 
                            t.position.x + halfW, t.position.y + halfH,
                         anyBlock, anyHazard, anySlow, anyCurrent);

        if (anyHazard) {
            const float px0 = t.position.x - halfW;
            const float py0 = t.position.y - halfH;
            const float px1 = t.position.x + halfW;
            const float py1 = t.position.y + halfH;

            int minTX, minTY, maxTX, maxTY;
            worldRectToTileRange(g, px0, py0, px1, py1, minTX, minTY, maxTX, maxTY);

            Vector2 n = {0.f, 0.f};
            bool found = false;
            for (int ty = minTY; ty <= maxTY && !found; ++ty) {
                for (int tx = minTX; tx <= maxTX && !found; ++tx) {
                    unsigned char f = g.ig->cells[idx2d(tx, ty, g.ig->width)];
                    if (f & IGF_Hazard) {
                        const float tx0 = g.origin.x + tx * g.cellW;
                        const float ty0 = g.origin.y + ty * g.cellH;
                        const float tx1 = tx0 + g.cellW;
                        const float ty1 = ty0 + g.cellH;
                        n = contactNormalAABB(px0, py0, px1, py1, tx0, ty0, tx1, ty1);
                        found = true;
                    }
                }
            }
            if (!found) {
                Vector2 vd = { v.velocity.x, v.velocity.y };
                float len = std::sqrt(vd.x*vd.x + vd.y*vd.y);
                n = (len > 0.0001f) ? Vector2{ -vd.x/len, -vd.y/len } : Vector2{0.f, -1.f};
            }

            const float K = pushBack;     
            t.position.x += n.x * K;      
            t.position.y += n.y * K;

            v.velocity.x = 0.f;
            v.velocity.y = 0.f;
            return; 
        }


        if (anySlow) {
            const float sF  = sl ? sl->speedFactor    : 0.35f;
            const float iB  = sl ? sl->immediateBrake : 0.6f;
            const float cap = sl ? sl->maxSpeedOnSlow : 60.0f;

            t.position.x -= vDesired.x * dt * iB * (1.0f - sF);
            t.position.y -= vDesired.y * dt * iB * (1.0f - sF);

            float len = std::sqrt(vDesired.x*vDesired.x + vDesired.y*vDesired.y);
            if (len > 0.0001f) {
                float target = std::min(len * sF, cap);
                float k = target / len;
                v.velocity.x = vDesired.x * k;
                v.velocity.y = vDesired.y * k;
            } else {
                v.velocity.x *= sF;
                v.velocity.y *= sF;
            }
        }

        if (anyCurrent) {
            // depurar si entra anyCurrent
            std::cout << "Entrando en anyCurrent" << std::endl;
            float mvx = t.position.x - prevCX;
            float mvy = t.position.y - prevCY;
            if (std::fabs(mvx) < 0.0001f && std::fabs(mvy) < 0.0001f) { mvx = v.velocity.x; mvy = v.velocity.y; }

            float len = std::sqrt(mvx*mvx + mvy*mvy);
            if (len > 0.0001f) {
                float nx = mvx / len, ny = mvy / len;

                // 1) Aceleración (se siente en frames siguientes)
                const float force = currentForce > 0 ? currentForce : 400.0f; // súbelo si quieres más
                v.velocity.x += nx * force * dt;
                v.velocity.y += ny * force * dt;

                // Cap de velocidad
                float vlen = std::sqrt(v.velocity.x*v.velocity.x + v.velocity.y*v.velocity.y);
                const float vmax = curMaxSpd > 0 ? curMaxSpd : 220.0f;
                if (vlen > vmax) {
                    float k = vmax / vlen;
                    v.velocity.x *= k;
                    v.velocity.y *= k;
                }

                // 2) "Kick" inmediato (SE NOTA YA): usa dt (no dt*dt)
                const float kick = 150.0f;  // píxeles/seg de desplazamiento instantáneo
                t.position.x += nx * kick * dt;
                t.position.y += ny * kick * dt;
            }
        }
    });
}
