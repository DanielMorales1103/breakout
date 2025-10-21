#include "Systems/CameraInitSystem.h"
#include "../components/Components.h"
#include "Scene/Scene.h"
#include <raylib.h>

void CameraInitSystem::update() {
    // cámara activa
    entt::entity cam = entt::null;
    TransformComponent* camT = nullptr;
    ViewportComponent*  camV = nullptr;
    CameraComponent*    camC = nullptr;

    auto cv = scene->r.view<CameraComponent, TransformComponent, ViewportComponent, CameraTag>();
    cv.each([&](auto e, CameraComponent &c, TransformComponent &t, ViewportComponent &v) {
        if (cam == entt::null && c.active) { cam=e; camT=&t; camV=&v; camC=&c; }
    });
    if (!camT || !camV || !camC) return;

    // si ya tenemos un tilemap válido, centra la cámara y desactiva este system (opcional)
    bool done = false;

    auto tv = scene->r.view<TilemapComponent, TilesetComponent, TransformComponent, TilemapTag>();
    tv.each([&](TilemapComponent &m, TilesetComponent &ts, TransformComponent &tt) {
        if (done) return;
        if (m.width <= 0 || m.height <= 0) return; // aún no cargado

        const float S = (tt.scale <= 0.f) ? 1.f : tt.scale;

        // tamaño del mundo del tilemap
        const float worldW = m.width  * ts.tileSize.x * S;
        const float worldH = m.height * ts.tileSize.y * S;

        // centro del mapa en mundo
        const float mapCenterX = tt.position.x + worldW * 0.5f;
        const float mapCenterY = tt.position.y + worldH * 0.5f;

        // colocar top-left de la cámara para que su centro coincida con el centro del mapa
        camT->position.x = mapCenterX - (camV->width  / camC->zoom) * 0.5f;
        camT->position.y = mapCenterY - (camV->height / camC->zoom) * 0.5f;

        done = true;
    });

    // Si no hay tilemap aún, este system volverá a intentar en el siguiente frame.
}
