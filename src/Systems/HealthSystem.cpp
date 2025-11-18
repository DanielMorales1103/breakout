#include "Systems/HealthSystem.h"
#include <raylib.h>
#include <cmath>
#include <iostream>

void HealthSystem::update() {
    auto& r = scene->r;
    const float dt = GetFrameTime();

    auto pv = r.view<PlayerTag, TransformComponent, PlayerHealth>();
    auto ev = r.view<EnemyTag, TransformComponent>();

    const float hitRadius  = 18.0f;
    const float hitRadius2 = hitRadius * hitRadius;

    for (auto [pe, ptr, ph] : pv.each()) {
        ph.damageCooldown += dt;
        if (ph.current <= 0) continue;

        for (auto [ee, etr] : ev.each()) {
            float dx = etr.position.x - ptr.position.x;
            float dy = etr.position.y - ptr.position.y;
            float d2 = dx*dx + dy*dy;

            if (d2 <= hitRadius2 && ph.damageCooldown >= ph.damageInterval) {
                ph.current -= 1;
                ph.damageCooldown = 0.0f;

                std::cout << "Player hit! HP = "
                          << ph.current << "/" << ph.max << "\n";

                if (ph.current <= 0) {
                    std::cout << "GAME OVER\n";
                }
                break;
            }
        }
    }
}
