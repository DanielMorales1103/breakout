#include "Systems/VictorySystem.h"
#include "../components/Components.h"
#include "Systems/EnemySpawnSystem.h"   // para EnemySpawnSettings / EnemySpawnState

void VictorySystem::update() {
    auto& r = scene->r;

    // Si ya perdiste o ya ganaste, no hacemos nada
    if (scene->gameOver || scene->victory) return;

    // 1) ¿Todas las oleadas terminaron?
    bool allWavesFinished = true;
    bool hasSpawner       = false;

    auto sv = r.view<EnemySpawnSettings, EnemySpawnState>();

    sv.each([&](EnemySpawnSettings const& cfg, EnemySpawnState const& st) {
        hasSpawner = true;

        if (!allWavesFinished) return;

        // Si la oleada está en loop, nunca se “termina”
        if (cfg.loop) {
            allWavesFinished = false;
            return;
        }

        // Mientras curWave sea menor al total, todavía hay oleadas pendientes
        if (st.curWave < (int)cfg.waves.size()) {
            allWavesFinished = false;
            return;
        }
    });

    // Si hay spawners y alguno no ha terminado, todavía no hay victoria
    if (hasSpawner && !allWavesFinished) return;

    // 2) ¿Quedan enemigos vivos (de oleadas o proximidad)?
    auto ev = r.view<EnemyTag>();
    bool anyEnemy = false;
    ev.each([&](auto /*e*/) {
        anyEnemy = true;
    });

    if (anyEnemy) return;

    // 3) Si llegamos aquí: no hay más oleadas y no quedan enemigos → ¡Victoria!
    scene->victory = true;
}
