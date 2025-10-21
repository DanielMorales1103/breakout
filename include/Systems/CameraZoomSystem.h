#pragma once
#include <entt/entt.hpp>
#include <raylib.h>
#include "Systems/System.h"
#include "Scene/Scene.h"
#include "../components/Components.h"

struct CameraZoomSettings {
    float minZoom   = 0.75f;
    float maxZoom   = 2.00f;
    float step      = 0.10f;   
    float smooth    = 0.20f;   
};

class CameraZoomSystem : public System {
public:
    void update() override;
};
