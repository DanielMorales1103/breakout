#include "Systems/EnemyAISystem.h"
#include "Scene/Scene.h"
#include "../components/Components.h"
#include <raylib.h>
#include <cmath>

void EnemyAISystem::update() {
    const float dt = GetFrameTime();
    (void)dt; // no lo usamos explícitamente, pero por si lo necesitas.

    // 1) Posición del player (tomamos el primero que encontremos)
    Vector2 playerPos{};
    bool havePlayer = false;
    {
        auto pv = scene->r.view<PlayerTag, TransformComponent>();
        pv.each([&](TransformComponent& t){
            if(!havePlayer){ playerPos = t.position; havePlayer = true; }
        });
        if(!havePlayer) return;
    }

    // 2) Perseguir al player
    auto ev = scene->r.view<TransformComponent, VelocityComponent,
                            AnimatorComponent, FollowAIComponent, EnemyTag>();
    ev.each([&](TransformComponent& t, VelocityComponent& v,
                AnimatorComponent& a, FollowAIComponent& ai) {

        Vector2 d{ playerPos.x - t.position.x, playerPos.y - t.position.y };
        const float len = std::sqrt(d.x*d.x + d.y*d.y);

        if (len > (ai.stopRadius > 0.f ? ai.stopRadius : 0.0f)) {
            // normaliza y aplica velocidad
            const float inv = 1.0f / (len > 1e-6f ? len : 1.0f);
            d.x *= inv; d.y *= inv;
            v.velocity = { d.x * ai.speed, d.y * ai.speed };
            a.moving   = true;

            // orientar por eje dominante
            if (std::fabs(d.x) > std::fabs(d.y))
                a.facing = (d.x >= 0) ? AnimatorComponent::Right : AnimatorComponent::Left;
            else
                a.facing = (d.y >= 0) ? AnimatorComponent::Down  : AnimatorComponent::Up;
        } else {
            v.velocity = {0,0};
            a.moving   = false;
        }
    });
}
