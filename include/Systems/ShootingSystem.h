#pragma once
#include "Systems/System.h"
#include "Scene/Scene.h"

class ShootingSystem : public System {
public:
    void update() override;
};
