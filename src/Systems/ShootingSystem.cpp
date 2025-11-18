#include "Systems/ShootingSystem.h"
#include "../components/Components.h"
#include <raylib.h>
#include <cmath>

void ShootingSystem::update() {
    auto& r = scene->r;

    entt::entity player = entt::null;
    TransformComponent*    ptr  = nullptr;
    VelocityComponent*     pvel = nullptr;
    AnimatorComponent*     panim = nullptr;

    r.view<PlayerTag, TransformComponent, VelocityComponent, AnimatorComponent>()
     .each([&](entt::entity e,
               TransformComponent& t,
               VelocityComponent& v,
               AnimatorComponent& a) {
        if (player == entt::null) {
            player = e;
            ptr    = &t;
            pvel   = &v;
            panim  = &a;
        }
    });

    if (player == entt::null) return;

    if (IsKeyPressed(KEY_SPACE)) {
        Vector2 dir{ pvel->velocity.x, pvel->velocity.y };

        const float eps = 0.01f;
        if (std::fabs(dir.x) < eps && std::fabs(dir.y) < eps) {
            dir = Vector2{0, 1}; // default

            if (panim) {
                switch (panim->facing) {
                    case AnimatorComponent::Up:
                        dir = Vector2{0, -1};
                        break;
                    case AnimatorComponent::Down:
                        dir = Vector2{0, 1};
                        break;
                    case AnimatorComponent::Left:
                        dir = Vector2{-1, 0};
                        break;
                    case AnimatorComponent::Right:
                        dir = Vector2{1, 0};
                        break;
                    default:
                        break;
                }
            }
        }

        float d = std::sqrt(dir.x*dir.x + dir.y*dir.y);
        if (d < eps) return; 
        dir.x /= d;
        dir.y /= d;

        auto e = r.create();

        r.emplace<TransformComponent>(e, ptr->position);

        r.emplace<VelocityComponent>(e, Vector2{
            dir.x * 250.0f,
            dir.y * 250.0f
        });

        r.emplace<SpriteComponent>(e, SpriteComponent{
            "",                         
            Rectangle{0,0,0,0},
            Vector2{4,4}
        });

        r.emplace<ProjectileTag>(e);
        r.emplace<ProjectileData>(e);
    }
}
