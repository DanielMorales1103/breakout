#include "Systems/InputSystem.h"
#include "Scene/Scene.h"
#include "../components/Components.h"
#include <raylib.h>
#include <cmath>

void InputSystem::update() {
    auto view = scene->r.view<PlayerTag, VelocityComponent, AnimatorComponent>();

    float vx = 0.0f, vy = 0.0f;
    if (IsKeyDown(KEY_RIGHT)) vx += 1.0f;
    if (IsKeyDown(KEY_LEFT))  vx -= 1.0f;
    if (IsKeyDown(KEY_DOWN))  vy += 1.0f;
    if (IsKeyDown(KEY_UP))    vy -= 1.0f;

    if (vx != 0.0f && vy != 0.0f) {
        const float inv = 1.0f / std::sqrt(2.0f);
        vx *= inv; vy *= inv;
    }

    const float speed = 120.0f;

    view.each([&](VelocityComponent& vel,
                AnimatorComponent& anim) {
        vel.velocity = { vx * speed, vy * speed };

        anim.moving = (vx != 0.0f || vy != 0.0f);
        if (std::fabs(vx) > std::fabs(vy)) {
            anim.facing = (vx >= 0.0f) ? AnimatorComponent::Right : AnimatorComponent::Left;
        } else if (std::fabs(vy) > 0.0f) {
            anim.facing = (vy >= 0.0f) ? AnimatorComponent::Down : AnimatorComponent::Up;
        }
    });

}
