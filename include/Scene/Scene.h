#pragma once

#include <entt/entt.hpp>
#include <vector>
#include "Systems/System.h"

class Scene {
public:
    Scene() = default;

    void setup();
    void update();
    void render();
    void addSystem(System* system);

    entt::registry r;

private:
    std::vector<System*> systems;
};
