#pragma once

#include <entt/entt.hpp>
#include "Scene/Scene.h"

class Entity {
public:
    Entity(entt::entity e, Scene* s) : handle(e), scene(s) {}

    template<typename T, typename... Args>
    auto& addComponent(Args&&... args) {
        return scene->r.emplace_or_replace<T>(handle, std::forward<Args>(args)...);
    }

    template<typename T>
    void removeComponent() {
        scene->r.remove<T>(handle);
    }

private:
    entt::entity handle;
    Scene* scene;
};
