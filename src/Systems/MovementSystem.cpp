#include "Systems/MovementSystem.h"
#include "Scene/Scene.h"
#include "../components/Components.h"
#include <raylib.h>
#include <algorithm>

static float clampf(float x, float a, float b) { return x < a ? a : (x > b ? b : x); }

void MovementSystem::update() {
    const float dt = GetFrameTime();

    // Mover todo lo que tenga Transform + Velocity
    auto moverView = scene->r.view<TransformComponent, VelocityComponent>();
    moverView.each([&](TransformComponent& t, VelocityComponent& v) {
        t.position.x += v.velocity.x * dt;
        t.position.y += v.velocity.y * dt;
    });

    // Clamp a pantalla cuando exista BoundsClampComponent
    auto clampView = scene->r.view<TransformComponent, SpriteComponent, BoundsClampComponent>();
    clampView.each([&](TransformComponent& t, SpriteComponent& s, BoundsClampComponent& b) {
        const int sw = GetScreenWidth();
        const int sh = GetScreenHeight();

        const float halfW = (s.sizePx.x * t.scale) * 0.5f;
        const float halfH = (s.sizePx.y * t.scale) * 0.5f;

        t.position.x = clampf(t.position.x, b.margin + halfW, sw - b.margin - halfW);
        t.position.y = clampf(t.position.y, b.margin + halfH, sh - b.margin - halfH);
    });
}
