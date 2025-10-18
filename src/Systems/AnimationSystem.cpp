#include "Systems/AnimationSystem.h"
#include "Scene/Scene.h"
#include "../components/Components.h"
#include <raylib.h>
#include <cmath>

static int facingRow(AnimatorComponent::Facing f) {
    switch (f) {
        case AnimatorComponent::Down:  return 0;
        case AnimatorComponent::Left:  return 1;
        case AnimatorComponent::Right: return 2;
        case AnimatorComponent::Up:    return 3;
    }
    return 0;
}

void AnimationSystem::update() {
    const float dt = GetFrameTime();

    // Entidades con Sprite + Animator
    auto view = scene->r.view<SpriteComponent, AnimatorComponent>();
    view.each([&](SpriteComponent& s, AnimatorComponent& a) {
        if (a.fps <= 0.0f || a.columns <= 0 || a.rows <= 0 || s.sizePx.x <= 0 || s.sizePx.y <= 0) {
            return;
        }

        const float effFps = a.moving ? a.fps : a.fps * 0.5f;
        const float frameTime = 1.0f / effFps;

        a.acc += dt;
        while (a.acc >= frameTime) {
            a.acc -= frameTime;
            a.frame = (a.frame + 1) % a.columns;   
        }

        const int row = facingRow(a.facing);
        const float sx = (a.baseCol + a.frame) * s.sizePx.x;
        const float sy = (a.baseRow + row)     * s.sizePx.y;

        s.src = Rectangle{ sx, sy, s.sizePx.x, s.sizePx.y };
    });
}
