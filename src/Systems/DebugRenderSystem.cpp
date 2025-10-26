#include "Systems/DebugRenderSystem.h"
#include "../components/Components.h"
#include <raylib.h>
#include <algorithm>
#include <cmath>

static inline int idx2d(int x, int y, int w) { return y * w + x; }

// === helpers compartidos con colisión ===
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
}

// world → screen (resta cámara y aplica zoom)
static inline Vector2 worldToScreen(const Vector2& world, const TransformComponent& camT, const CameraComponent& camC) {
    return { (world.x - camT.position.x) * camC.zoom,
             (world.y - camT.position.y) * camC.zoom };
}

void DebugRenderSystem::update() {
    // === cámara activa ===
    TransformComponent* camT = nullptr;
    CameraComponent*    camC = nullptr;
    ViewportComponent*  camV = nullptr;

    auto cv = scene->r.view<CameraComponent, TransformComponent, ViewportComponent, CameraTag>();
    cv.each([&](auto, CameraComponent &c, TransformComponent &t, ViewportComponent &v){
        if (!camT && c.active) { camT = &t; camC = &c; camV = &v; }
    });
    if (!camT || !camC || !camV) return;

    // === tilemap + intgrid ===
    TilemapComponent*   tm = nullptr;
    TilesetComponent*   ts = nullptr;
    TransformComponent* tt = nullptr;
    IntGridComponent*   ig = nullptr;

    auto tv = scene->r.view<TilemapComponent, TilesetComponent, TransformComponent, TilemapTag>();
    tv.each([&](auto e, TilemapComponent &m, TilesetComponent &tset, TransformComponent &tr){
        if (!tm && m.width>0 && m.height>0 && !m.tiles.empty()) {
            tm = &m; ts = &tset; tt = &tr;
            if (scene->r.any_of<IntGridComponent>(e)) ig = &scene->r.get<IntGridComponent>(e);
        }
    });
    if (!tm || !ts || !tt) return;

    const float S     = (tt->scale <= 0.f) ? 1.f : tt->scale;
    const float cellW = ts->tileSize.x * S;
    const float cellH = ts->tileSize.y * S;

    GridCtx g;
    g.ig     = ig;
    g.origin = tt->position;
    g.cellW  = cellW;
    g.cellH  = cellH;

    // === rango visible de tiles ===
    const float viewW_world = camV->width  / camC->zoom;
    const float viewH_world = camV->height / camC->zoom;
    const float viewMinX = camT->position.x;
    const float viewMinY = camT->position.y;
    const float viewMaxX = viewMinX + viewW_world;
    const float viewMaxY = viewMinY + viewH_world;

    int minTX = (int)std::floor((viewMinX - tt->position.x) / cellW);
    int minTY = (int)std::floor((viewMinY - tt->position.y) / cellH);
    int maxTX = (int)std::floor((viewMaxX - tt->position.x) / cellW);
    int maxTY = (int)std::floor((viewMaxY - tt->position.y) / cellH);

    minTX = std::max(0, std::min(minTX, tm->width  - 1));
    minTY = std::max(0, std::min(minTY, tm->height - 1));
    maxTX = std::max(0, std::min(maxTX, tm->width  - 1));
    maxTY = std::max(0, std::min(maxTY, tm->height - 1));

    // === dibuja TODOS los tiles visibles con color fijo según IntGrid ===
    for (int ty = minTY; ty <= maxTY; ++ty) {
        for (int tx = minTX; tx <= maxTX; ++tx) {
            Color col = Color{80,200,120,60}; // grilla suave si no hay IG
            if (ig) {
                const unsigned char f = ig->cells[idx2d(tx,ty,ig->width)];
                // colores fijos por flag
                if (f & IGF_Block)         col = Color{255, 60, 60,180};   // rojo
                else if (f & IGF_Hazard)   col = Color{255,140, 0,180};    // naranja
                else if (f & IGF_Slow)     col = Color{ 60,120,255,180};   // azul
                else if (f & IGF_Current)  col = Color{  0,255,255,180};   // cian
                else if (f & IGF_Walkable) col = Color{120,255,120,120};   // verde
            }

            const float wx = tt->position.x + tx * cellW;
            const float wy = tt->position.y + ty * cellH;
            Vector2 a = worldToScreen({wx,          wy},          *camT, *camC);
            Vector2 b = worldToScreen({wx+cellW, wy+cellH}, *camT, *camC);
            DrawRectangleLines((int)a.x, (int)a.y, (int)(b.x-a.x), (int)(b.y-a.y), col);
        }
    }

    // === AABB del player (CENTRADO) ===
    auto pv = scene->r.view<PlayerTag, TransformComponent, SpriteComponent>();
    pv.each([&](TransformComponent &t, SpriteComponent &s){
        const float sc = (t.scale <= 0.f) ? 1.f : t.scale;
        const float w  = s.src.width  * sc;
        const float h  = s.src.height * sc;
        const float halfW = w * 0.5f, halfH = h * 0.5f;

        const float minX = t.position.x - halfW;
        const float minY = t.position.y - halfH;
        const float maxX = t.position.x + halfW;
        const float maxY = t.position.y + halfH;

        Vector2 tl = worldToScreen({minX, minY}, *camT, *camC);
        Vector2 br = worldToScreen({maxX, maxY}, *camT, *camC);
        DrawRectangleLines((int)tl.x, (int)tl.y, (int)(br.x - tl.x), (int)(br.y - tl.y), Color{50,255,50,255});

        // cruz en el centro del jugador
        Vector2 pc = worldToScreen(t.position, *camT, *camC);
        DrawLine((int)pc.x-4,(int)pc.y,(int)pc.x+4,(int)pc.y, Color{255,255,255,160});
        DrawLine((int)pc.x,(int)pc.y-4,(int)pc.x,(int)pc.y+4, Color{255,255,255,160});
    });

    // marca centro del viewport (opcional)
    DrawCircleLines((int)(camV->width*0.5f), (int)(camV->height*0.5f), 4, Color{255,255,255,120});
}
