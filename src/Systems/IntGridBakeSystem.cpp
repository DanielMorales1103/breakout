#include "Systems/IntGridBakeSystem.h"
#include "../components/Components.h"
#include "Scene/Scene.h"
#include <unordered_set>

void IntGridBakeSystem::update() {
    // Tilemap válido
    entt::entity mapE = entt::null;
    TilemapComponent* tm = nullptr;
    TilesetComponent* ts = nullptr;
    TransformComponent* tt = nullptr;

    auto tv = scene->r.view<TilemapComponent, TilesetComponent, TransformComponent, TilemapTag>();
    tv.each([&](auto e, TilemapComponent &m, TilesetComponent &tset, TransformComponent &tr){
        if (mapE == entt::null && m.width>0 && m.height>0 && !m.tiles.empty()) {
            mapE = e; tm = &m; ts = &tset; tt = &tr;
        }
    });
    if (!tm) return;

    // Reglas por defecto si no existen
    if (!scene->r.any_of<IntGridRules>(mapE)) {
        scene->r.emplace<IntGridRules>(mapE, IntGridRules{
            /*nonWalkableIndices=*/{17,18,24,25,26,32,33,34,40,41,42,48,49,50,56,57,58,64,65,66},
            /*hazardIndices=*/{8,9,11,12},
            /*slowIndex=*/16,
            /*currentIndices=*/{3},
            /*defaultFlags=*/(unsigned char)IGF_Walkable
        });
    }
    if (!scene->r.any_of<HazardSettings>(mapE)) scene->r.emplace<HazardSettings>(mapE, HazardSettings{60.0f});
    if (!scene->r.any_of<SlowSettings>(mapE))   scene->r.emplace<SlowSettings>(mapE, SlowSettings{0.30f, 0.6f, 60.0f});
    if (!scene->r.any_of<CurrentSettings>(mapE)) scene->r.emplace<CurrentSettings>(mapE, CurrentSettings{60.0f, 180.0f});

    auto &rules = scene->r.get<IntGridRules>(mapE);

    // Hornear flags
    IntGridComponent ig;
    ig.width  = tm->width;
    ig.height = tm->height;
    ig.cells.assign(ig.width * ig.height, (unsigned char)rules.defaultFlags);

    std::unordered_set<int> blocks(rules.nonWalkableIndices.begin(), rules.nonWalkableIndices.end());
    std::unordered_set<int> hazards(rules.hazardIndices.begin(), rules.hazardIndices.end());
    std::unordered_set<int> currents(rules.currentIndices.begin(), rules.currentIndices.end()); 


    for (int y=0; y<tm->height; ++y) {
        for (int x=0; x<tm->width; ++x) {
            const int ti = tm->tiles[y*tm->width + x];
            unsigned char f = (unsigned char)rules.defaultFlags; // walkable

            if (blocks.count(ti))           
                f = (unsigned char)IGF_Block; // bloquea (no walkable)
            else {
                if (hazards.count(ti))      f = (unsigned char)(IGF_Walkable | IGF_Hazard);
                if (ti == rules.slowIndex)  f = (unsigned char)(IGF_Walkable | IGF_Slow);
                if (currents.count(ti))     f = (unsigned char)(IGF_Walkable | IGF_Current);
            }

            ig.cells[y*ig.width + x] = f;
        }
    }

    if (scene->r.any_of<IntGridComponent>(mapE)) scene->r.replace<IntGridComponent>(mapE, std::move(ig));
    else scene->r.emplace<IntGridComponent>(mapE, std::move(ig));
}
