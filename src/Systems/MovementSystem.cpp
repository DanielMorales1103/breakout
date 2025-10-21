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

}
