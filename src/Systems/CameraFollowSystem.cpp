#include "Systems/CameraFollowSystem.h"
#include <algorithm>
#include <raylib.h>

static inline float lerp(float a, float b, float t) { return a + (b - a) * t; }

void CameraFollowSystem::update() {
    // cámara activa
    entt::entity cam = entt::null;
    TransformComponent* camT = nullptr;
    ViewportComponent*  camV = nullptr;
    CameraComponent*    camC = nullptr;
    CameraFollowSettings defaults;
    const CameraFollowSettings* cfg = &defaults;

    auto cv = scene->r.view<CameraComponent, TransformComponent, ViewportComponent, CameraTag>();
    cv.each([&](auto e, CameraComponent &c, TransformComponent &t, ViewportComponent &v) {
        if (cam == entt::null && c.active) { cam=e; camT=&t; camV=&v; camC=&c; }
    });
    if (!camT || !camV || !camC) return;

    if (scene->r.any_of<CameraFollowSettings>(cam)) {
        cfg = &scene->r.get<CameraFollowSettings>(cam);
    }

    // player
    Vector2 playerPos{0,0};
    bool hasPlayer = false;
    auto pv = scene->r.view<PlayerTag, TransformComponent>();
    pv.each([&](auto /*e*/, TransformComponent &t) {
        if (!hasPlayer) { playerPos = t.position; hasPlayer = true; }
    });
    if (!hasPlayer) return;

    // deadzone (en píxeles de viewport)
    const float centerX = camV->width  * 0.5f;
    const float centerY = camV->height * 0.5f;
    const float left    = centerX - cfg->deadzoneW * 0.5f;
    const float right   = centerX + cfg->deadzoneW * 0.5f;
    const float top     = centerY - cfg->deadzoneH * 0.5f;
    const float bottom  = centerY + cfg->deadzoneH * 0.5f;

    // === objetivo: jugador al CENTRO del viewport (considerando zoom) ===
    const float halfViewW_world = (camV->width  / camC->zoom) * 0.5f;
    const float halfViewH_world = (camV->height / camC->zoom) * 0.5f;

    const float targetX = playerPos.x - halfViewW_world;
    const float targetY = playerPos.y - halfViewH_world;

    // === mover cámara (suavizado opcional con damping) ===
    const float dt = GetFrameTime();
    const float alpha = std::clamp(cfg->damping * dt * 60.0f, 0.0f, 1.0f); // 0 = instantáneo

    camT->position.x = lerp(camT->position.x, targetX, alpha);
    camT->position.y = lerp(camT->position.y, targetY, alpha);
}
