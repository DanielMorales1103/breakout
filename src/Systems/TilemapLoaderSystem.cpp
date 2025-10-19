#include "Systems/TilemapLoaderSystem.h"
#include "../components/Components.h"
#include "TextureManager.h"
#include <raylib.h>

void TilemapLoaderSystem::update() {
    auto view = scene->r.view<TilemapComponent, TilesetComponent, TilemapTag>();
    bool didInit = false;

    view.each([&](TilemapComponent& m, TilesetComponent& ts) {
        if (didInit) return;
        didInit = true;

        if (m.tiles.empty() && m.width > 0 && m.height > 0) {
            m.tiles.resize(m.width * m.height, -1);

            // Un tile sólido del A5
            const int FLOOR_COL = 5;    // prueba 1, 2, 3 para cambiar de textura
            const int FLOOR_ROW = 5;
            const int floorIndex = FLOOR_ROW * ts.columns + FLOOR_COL;

            for (int y = 0; y < m.height; ++y)
                for (int x = 0; x < m.width; ++x)
                    m.tiles[y * m.width + x] = floorIndex;
        }
    });
}

