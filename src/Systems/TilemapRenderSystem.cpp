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

void TilemapRenderSystem::update() {             // <- antes estaba en render()
    auto view = scene->r.view<TilemapComponent, TilesetComponent, TilemapTag>();
    view.each([&](TilemapComponent& m, TilesetComponent& ts) {
        const Texture2D tex = TextureManager::GetTexture(ts.texturePath ? ts.texturePath : "");
        if (tex.id == 0 || m.tiles.empty()) return;

        for (int y = 0; y < m.height; ++y) {
            for (int x = 0; x < m.width; ++x) {
                const int idx = m.tiles[y * m.width + x];
                if (idx < 0) continue;

                Rectangle src = tilesetSrcRect(ts, idx);
                Rectangle dst{ x * ts.tileSize.x, y * ts.tileSize.y, ts.tileSize.x, ts.tileSize.y };
                DrawTexturePro(tex, src, dst, {0,0}, 0.f, WHITE);
            }
        }
    });
}
