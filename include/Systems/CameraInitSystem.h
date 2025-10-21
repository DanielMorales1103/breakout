#pragma once
#include <entt/entt.hpp>
#include "Systems/System.h"
#include "Scene/Scene.h"
#include "../components/Components.h"

// Coloca la cámara en el centro del primer tilemap cargado (usa zoom/scale).
class CameraInitSystem : public System {
public:
    void update() override;
};
