#pragma once
#include "Systems/System.h"
#include "Scene/Scene.h"
#include "../components/Components.h"
#include <sol/sol.hpp>
#include <unordered_map>

class ScriptMovementSystem : public System {
public:
    void update() override;

private:
    bool inited = false;
    sol::state lua;
    std::unordered_map<std::string, sol::table> modules; 
    std::unordered_map<entt::entity, sol::table> stateMap;

    void lazyInit_() {
        if (inited) return;
        lua.open_libraries(sol::lib::base, sol::lib::math, sol::lib::table, sol::lib::string);
        inited = true;
    }
};
