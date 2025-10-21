#pragma once
#include <entt/entt.hpp>
#include <raylib.h>
#include "Systems/System.h"        
#include "Scene/Scene.h"
#include "../components/Components.h"

// Sistema: centra la cámara en el player (follow duro, sin suavizado)
class CameraFollowSystem : public System {
public:
    void update() override;
};
