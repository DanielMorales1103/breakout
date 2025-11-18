#include "Systems/ProjectileSystem.h"
#include <raylib.h>

void ProjectileSystem::update() {
    auto& r = scene->r;
    float dt = GetFrameTime();

    auto view = r.view<ProjectileTag, TransformComponent, VelocityComponent, ProjectileData>();

    for (auto [e, tr, vel, proj] : view.each()) {
        // mover
        tr.position.x += vel.velocity.x * dt;
        tr.position.y += vel.velocity.y * dt;

        // tiempo de vida
        proj.age += dt;
        if (proj.age >= proj.lifetime) {
            r.destroy(e);
        }
    }
}
