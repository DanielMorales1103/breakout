#include "Systems/TilemapLoaderSystem.h"
#include "../components/Components.h"
#include "TextureManager.h"
#include <raylib.h>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

static std::vector<int> LoadCsv(const std::string& path, int& outW, int& outH) {
    std::ifstream f(path);
    std::vector<int> data;
    outW = 0; outH = 0;
    if (!f.is_open()) return data;

    std::string line;
    while (std::getline(f, line)) {
        if (line.empty()) continue;
        std::stringstream ss(line);
        std::string cell;
        int rowCount = 0;

        while (std::getline(ss, cell, ',')) {
            // recorta espacios
            size_t a = cell.find_first_not_of(" \t\r");
            size_t b = cell.find_last_not_of(" \t\r");
            std::string t = (a == std::string::npos) ? "" : cell.substr(a, b - a + 1);

            // vacío → -1 (tile transparente)
            if (t.empty()) { data.push_back(-1); }
            else {
                // números en CSV son índices 0-based del A5, o -1 para vacío
                data.push_back(std::stoi(t));
            }
            rowCount++;
        }
        if (rowCount > 0) {
            if (outW == 0) outW = rowCount;
            outH++;
        }
    }
    return data;
}

void TilemapLoaderSystem::update() {
    auto view = scene->r.view<TilemapComponent, TilesetComponent, MapSourceComponent, TilemapTag>();

    view.each([&](TilemapComponent& m, TilesetComponent& ts, MapSourceComponent& src) {
        // Solo cargar una vez
        if (!m.tiles.empty()) return;

        int w = 0, h = 0;
        std::vector<int> data = LoadCsv(src.csvPath, w, h);
        if (data.empty() || w <= 0 || h <= 0) {
            TraceLog(LOG_WARNING, "CSV not loaded or empty: %s", src.csvPath.c_str());
            return;
        }

        m.width  = w;
        m.height = h;
        m.tiles  = std::move(data);

        TraceLog(LOG_INFO, "Tilemap loaded from CSV %s  (%dx%d)", src.csvPath.c_str(), m.width, m.height);
    });
}
