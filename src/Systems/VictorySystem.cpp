#include "Systems/VictorySystem.h"
#include "../components/Components.h"
#include "Systems/EnemySpawnSystem.h"   

void VictorySystem::update() {
    auto& r = scene->r;

    if (scene->gameOver || scene->victory) return;

    bool allWavesFinished = true;
    bool hasSpawner       = false;

    auto sv = r.view<EnemySpawnSettings, EnemySpawnState>();

    sv.each([&](EnemySpawnSettings const& cfg, EnemySpawnState const& st) {
        hasSpawner = true;

        if (!allWavesFinished) return;

        if (cfg.loop) {
            allWavesFinished = false;
            return;
        }

        if (st.curWave < (int)cfg.waves.size()) {
            allWavesFinished = false;
            return;
        }
    });

    if (hasSpawner && !allWavesFinished) return;

    auto ev = r.view<EnemyTag>();
    bool anyEnemy = false;
    ev.each([&](auto /*e*/) {
        anyEnemy = true;
    });

    if (anyEnemy) return;

    scene->victory = true;
}
