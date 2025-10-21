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

    Vector2 camPos{0,0};
    float camZoom = 1.0f;
    {
        auto cv = scene->r.view<CameraComponent, TransformComponent, ViewportComponent, CameraTag>();
        cv.each([&](auto /*e*/, CameraComponent &c, TransformComponent &t, ViewportComponent &) {
            if (c.active) {
                camPos = t.position;
                camZoom = c.zoom;
            }
        });
    }

    auto view = scene->r.view<TilemapComponent, TilesetComponent, TransformComponent, TilemapTag>();
    view.each([&](TilemapComponent& m, TilesetComponent& ts, TransformComponent& tr) {
        const Texture2D tex = TextureManager::GetTexture(ts.texturePath ? ts.texturePath : "");
        if (tex.id == 0 || m.tiles.empty() || m.width <= 0 || m.height <= 0) return;
        
        const float S = (tr.scale <= 0.0f) ? 1.0f : tr.scale; 
        const Vector2 origin = tr.position; 

        for (int y = 0; y < m.height; ++y) {
            for (int x = 0; x < m.width; ++x) {
                const int idx = m.tiles[y * m.width + x];
                if (idx < 0) continue;

                const int c = idx % ts.columns;
                const int r = idx / ts.columns;

                Rectangle src {
                    ts.clip.x + c * (ts.tileSize.x + ts.spacing),
                    ts.clip.y + r * (ts.tileSize.y + ts.spacing),
                    ts.tileSize.x,
                    ts.tileSize.y
                };

                const float worldX = origin.x + x * ts.tileSize.x * S;
                const float worldY = origin.y + y * ts.tileSize.y * S;

                Rectangle dst{
                    (worldX - camPos.x) * camZoom,
                    (worldY - camPos.y) * camZoom,
                    ts.tileSize.x * S * camZoom,
                    ts.tileSize.y * S * camZoom
                };
                DrawTexturePro(tex, src, dst, {0,0}, 0.f, WHITE);
            }
        }
    });
}
