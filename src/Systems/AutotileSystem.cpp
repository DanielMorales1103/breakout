#include "Systems/AutotileSystem.h"
#include <algorithm>
#include <vector>
#include "Scene/Scene.h"
#include "../components/Components.h"

static inline int idx2d(int x, int y, int w) { return y * w + x; }

void AutotileSystem::update() {
    // 1) Cámara activa (pos y zoom)
    Vector2 camPos{0,0};
    float camZoom = 1.0f;
    int viewW = 320, viewH = 180;

    {
        auto cv = scene->r.view<CameraComponent, TransformComponent, ViewportComponent, CameraTag>();
        cv.each([&](auto /*e*/, CameraComponent &c, TransformComponent &t, ViewportComponent &v) {
            if (c.active) {
                camPos  = t.position;
                camZoom = c.zoom;
                viewW   = v.width;
                viewH   = v.height;
            }
        });
    }

    // 2) Tilemap(s) con relleno activo
    auto tv = scene->r.view<TilemapComponent, TilesetComponent, TransformComponent, AutoTileFillComponent, TilemapTag>();
    tv.each([&](TilemapComponent &m, TilesetComponent &ts, TransformComponent &tr, AutoTileFillComponent &fill) {
        if (!fill.enabled) return;
        if (ts.tileSize.x <= 0 || ts.tileSize.y <= 0) return;

        const float S   = (tr.scale <= 0.f) ? 1.f : tr.scale;
        const float cellW = ts.tileSize.x * S;
        const float cellH = ts.tileSize.y * S;

        // Rectángulo visible en MUNDO (expandido con padding)
        const float screenW = (float)viewW;
        const float screenH = (float)viewH;
        const float worldViewW = screenW / camZoom;
        const float worldViewH = screenH / camZoom;

        // padding en píxeles de mundo
        const float padW_world = fill.padding * cellW;
        const float padH_world = fill.padding * cellH;

        const float needMinX = camPos.x - padW_world;
        const float needMinY = camPos.y - padH_world;
        const float needMaxX = camPos.x + worldViewW + padW_world;
        const float needMaxY = camPos.y + worldViewH + padH_world;

        // Convertir a índices de celda relativos al origen del tilemap (tr.position)
        auto toTileX = [&](float worldX) -> int { return (int)std::floor((worldX - tr.position.x) / cellW); };
        auto toTileY = [&](float worldY) -> int { return (int)std::floor((worldY - tr.position.y) / cellH); };

        int needMinTX = toTileX(needMinX);
        int needMinTY = toTileY(needMinY);
        int needMaxTX = toTileX(needMaxX);
        int needMaxTY = toTileY(needMaxY);

        if (m.width <= 0 || m.height <= 0 || m.tiles.empty()) return;

        // 3) Expandir hacia la IZQUIERDA (prepend columnas)
        if (needMinTX < 0) {
            const int addCols = -needMinTX;
            const int oldW = m.width;
            const int newW = oldW + addCols;
            std::vector<int> newTiles;
            newTiles.resize(newW * m.height, fill.baseIndex);

            for (int y = 0; y < m.height; ++y) {
                // copia fila vieja desplazada addCols a la derecha
                for (int x = 0; x < oldW; ++x) {
                    newTiles[idx2d(x + addCols, y, newW)] = m.tiles[idx2d(x, y, oldW)];
                }
                // las nuevas columnas [0..addCols-1] quedan con baseIndex
            }
            m.width = newW;
            m.tiles.swap(newTiles);

            // mover el origen del tilemap hacia la IZQUIERDA en mundo
            tr.position.x -= addCols * cellW;

            // actualizar las necesidades (porque movimos el origen)
            needMinTX += addCols;
            needMaxTX += addCols;
        }

        // 4) Expandir hacia ARRIBA (prepend filas)
        if (needMinTY < 0) {
            const int addRows = -needMinTY;
            const int oldW = m.width;
            const int oldH = m.height;
            const int newH = oldH + addRows;
            std::vector<int> newTiles;
            newTiles.resize(oldW * newH, fill.baseIndex);

            // copiamos las filas antiguas empezando en y + addRows
            for (int y = 0; y < oldH; ++y) {
                for (int x = 0; x < oldW; ++x) {
                    newTiles[idx2d(x, y + addRows, oldW)] = m.tiles[idx2d(x, y, oldW)];
                }
            }
            m.height = newH;
            m.tiles.swap(newTiles);

            // mover el origen del tilemap hacia ARRIBA en mundo
            tr.position.y -= addRows * cellH;

            // actualizar índices necesarios
            needMinTY += addRows;
            needMaxTY += addRows;
        }

        // 5) Expandir hacia la DERECHA (append columnas)
        if (needMaxTX >= m.width) {
            const int addCols = (needMaxTX - m.width) + 1;
            const int oldW = m.width;
            const int newW = oldW + addCols;

            std::vector<int> newTiles;
            newTiles.resize(newW * m.height, fill.baseIndex);

            for (int y = 0; y < m.height; ++y) {
                // copia fila vieja a la izquierda y rellena lo nuevo a la derecha
                for (int x = 0; x < oldW; ++x) {
                    newTiles[idx2d(x, y, newW)] = m.tiles[idx2d(x, y, oldW)];
                }
                // columnas [oldW..newW-1] quedan con baseIndex
            }
            m.width = newW;
            m.tiles.swap(newTiles);
        }

        // 6) Expandir hacia ABAJO (append filas)
        if (needMaxTY >= m.height) {
            const int addRows = (needMaxTY - m.height) + 1;
            const int oldW = m.width;
            const int oldH = m.height;
            const int newH = oldH + addRows;

            std::vector<int> newTiles = m.tiles;
            newTiles.resize(oldW * newH, fill.baseIndex);
            // las filas nuevas al final ya quedan a baseIndex

            m.height = newH;
            m.tiles.swap(newTiles);
        }
    });
}
