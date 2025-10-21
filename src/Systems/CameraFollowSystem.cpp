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

    // player en pantalla con zoom
    const float screenX = (playerPos.x - camT->position.x) * camC->zoom;
    const float screenY = (playerPos.y - camT->position.y) * camC->zoom;

    // cuánto habría que mover la cámara en pantalla para meterlo a la deadzone
    const float offX_screen = screenX - std::clamp(screenX, left, right);
    const float offY_screen = screenY - std::clamp(screenY, top,  bottom);

    // convertir ese desplazamiento de pantalla a mundo
    const float offX_world = offX_screen / camC->zoom;
    const float offY_world = offY_screen / camC->zoom;

    const float targetCamX = camT->position.x + offX_world;
    const float targetCamY = camT->position.y + offY_world;

    // suavizado
    const float dt = GetFrameTime();
    const float alpha = std::clamp(cfg->damping * dt * 60.0f, 0.0f, 1.0f);
    camT->position.x = lerp(camT->position.x, targetCamX, alpha);
    camT->position.y = lerp(camT->position.y, targetCamY, alpha);
}
