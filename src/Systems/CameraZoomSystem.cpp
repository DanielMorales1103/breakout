#include "Systems/CameraZoomSystem.h"
#include <algorithm>

static inline float lerp(float a, float b, float t) { return a + (b - a) * t; }

void CameraZoomSystem::update() {
    entt::entity cam = entt::null;
    CameraComponent* camC = nullptr;
    TransformComponent* camT = nullptr;
    ViewportComponent* camV = nullptr;
    const CameraZoomSettings* cfg = nullptr;
    CameraZoomSettings defaults;

    // localizar cámara activa
    auto cv = scene->r.view<CameraComponent, TransformComponent, ViewportComponent, CameraTag>();
    cv.each([&](auto e, CameraComponent &c, TransformComponent &t, ViewportComponent &v) {
        if (cam == entt::null && c.active) { cam=e; camC=&c; camT=&t; camV=&v; }
    });
    if (!camC || !camT || !camV) return;

    // settings (si existen)
    if (scene->r.any_of<CameraZoomSettings>(cam)) cfg = &scene->r.get<CameraZoomSettings>(cam);
    else cfg = &defaults;

    // input: rueda del mouse y teclas (Q/E)
    float targetZoom = camC->zoom;
    targetZoom += GetMouseWheelMove() * cfg->step;
    if (IsKeyPressed(KEY_Q)) targetZoom -= cfg->step;
    if (IsKeyPressed(KEY_E)) targetZoom += cfg->step;

    targetZoom = std::clamp(targetZoom, cfg->minZoom, cfg->maxZoom);

    if (targetZoom == camC->zoom) return;

    // mantener el centro visual estable: ajusta top-left al cambiar zoom
    // centro actual en mundo:
    const float viewW = (float)camV->width;
    const float viewH = (float)camV->height;
    const float curHalfW = (viewW / camC->zoom) * 0.5f;
    const float curHalfH = (viewH / camC->zoom) * 0.5f;
    Vector2 worldCenter { camT->position.x + curHalfW, camT->position.y + curHalfH };

    // lerp del zoom
    const float dt = GetFrameTime();
    const float alpha = std::clamp(cfg->smooth * dt * 60.0f, 0.0f, 1.0f);
    camC->zoom = lerp(camC->zoom, targetZoom, alpha);

    // reposicionar top-left para conservar el mismo centro mundo
    const float newHalfW = (viewW / camC->zoom) * 0.5f;
    const float newHalfH = (viewH / camC->zoom) * 0.5f;
    camT->position.x = worldCenter.x - newHalfW;
    camT->position.y = worldCenter.y - newHalfH;
}
