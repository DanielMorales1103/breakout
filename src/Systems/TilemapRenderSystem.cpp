#include "Systems/TilemapRenderSystem.h"
#include "Scene/Scene.h"
#include "../components/Components.h"
#include "TextureManager.h"
#include <raylib.h>

static Rectangle tilesetSrcRect(const TilesetComponent& ts, int index) {
    if (index < 0) return Rectangle{0,0,0,0};
    const int c = index % ts.columns;
    const int r = index / ts.columns;

    const float baseX = (ts.clip.width  > 0 ? ts.clip.x : 0);
    const float baseY = (ts.clip.height > 0 ? ts.clip.y : 0);

    const float sx = baseX + ts.margin + c * (ts.tileSize.x + ts.spacing);
    const float sy = baseY + ts.margin + r * (ts.tileSize.y + ts.spacing);
    return Rectangle{ sx, sy, ts.tileSize.x, ts.tileSize.y };
}

void TilemapRenderSystem::update() {
    auto view = scene->r.view<TilemapComponent, TilesetComponent, TransformComponent, TilemapTag>();
    view.each([&](TilemapComponent& m, TilesetComponent& ts, TransformComponent& tr) {
        const Texture2D tex = TextureManager::GetTexture(ts.texturePath ? ts.texturePath : "");
        if (tex.id == 0 || m.tiles.empty()) return;
        
        const float S = (tr.scale <= 0.0f) ? 1.0f : tr.scale; 
        const Vector2 origin = tr.position; 

        for (int y = 0; y < m.height; ++y) {
            for (int x = 0; x < m.width; ++x) {
                const int idx = m.tiles[y * m.width + x];
                if (idx < 0) continue;

                Rectangle src = tilesetSrcRect(ts, idx);
                Rectangle dst{
                    origin.x + x * ts.tileSize.x * S,
                    origin.y + y * ts.tileSize.y * S,
                    ts.tileSize.x * S,
                    ts.tileSize.y * S
                };
                DrawTexturePro(tex, src, dst, {0,0}, 0.f, WHITE);
            }
        }
    });
}
