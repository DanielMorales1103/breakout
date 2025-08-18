#pragma once

#include <entt/entt.hpp>
#include <vector>
#include "Systems/System.h"

class Scene {
public:
    Scene() = default;
    virtual ~Scene() = default;

    void setup();
    void update();
    void render();
    void addSystem(System* system);

    entt::registry r;

protected:
    virtual void onSetup() {}
    virtual void onUpdate() {}
    virtual void onRender() {}

private:
    std::vector<System*> systems;
};
