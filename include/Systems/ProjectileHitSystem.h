#pragma once
#include "Systems/System.h"
#include "Scene/Scene.h"

class ProjectileHitSystem : public System {
public:
    void update() override;
};
