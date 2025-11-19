#include "Systems/HealthSystem.h"
#include "../components/Components.h"
#include <raylib.h>
#include <cmath>
#include <iostream>

void HealthSystem::update() {
    auto& r = scene->r;
    const float dt = GetFrameTime();

    auto pv = r.view<PlayerTag, TransformComponent, PlayerHealth, SpriteComponent>();
    auto ev = r.view<EnemyTag, TransformComponent, SpriteComponent>();

    for (auto [pe, ptr, ph, pspr] : pv.each()) {
        ph.damageCooldown += dt;
        if (ph.current <= 0) continue;

        float pScale = (ptr.scale <= 0.0f) ? 1.0f : ptr.scale;
        float pW = pspr.src.width  * pScale;
        float pH = pspr.src.height * pScale;
        float pRadius = 0.5f * std::min(pW, pH);

        for (auto [ee, etr, espr] : ev.each()) {
            float eScale = (etr.scale <= 0.0f) ? 1.0f : etr.scale;
            float eW = espr.src.width  * eScale;
            float eH = espr.src.height * eScale;
            float eRadius = 0.5f * std::min(eW, eH);

            float dx = etr.position.x - ptr.position.x;
            float dy = etr.position.y - ptr.position.y;
            float d2 = dx*dx + dy*dy;

            float hitR = pRadius + eRadius;
            float hitR2 = hitR * hitR;

            if (d2 <= hitR2 && ph.damageCooldown >= ph.damageInterval) {
                ph.current -= 1;
                ph.damageCooldown = 0.0f;

                std::cout << "Player hit! HP = "
                          << ph.current << "/" << ph.max << "\n";

                if (ph.current <= 0) {
                    std::cout << "GAME OVER\n";
                    scene->gameOver = true;
                }

                break; 
            }
        }
    }
}
