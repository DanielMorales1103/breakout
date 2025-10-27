#include "Systems/ScriptMovementSystem.h"
#include <raylib.h>
#include <iostream>
#include <cmath>

void ScriptMovementSystem::update() {
    lazyInit_();

    const float dt = GetFrameTime();
    auto& r = scene->r;

    Vector2 player{0,0}; bool hasPlayer = false;
    r.view<PlayerTag, TransformComponent>().each([&](auto, TransformComponent& t){
        if (!hasPlayer) { player = t.position; hasPlayer = true; }
    });

    auto view = r.view<ScriptMove, MovementParams, TransformComponent, VelocityComponent>();

    for (auto [e, sm, prm, tr, vel] : view.each()) {
        sol::table mod;
        if (auto it = modules.find(sm.scriptPath); it == modules.end()) {
            sol::load_result lr = lua.load_file(sm.scriptPath);
            if (!lr.valid()) {
                sol::error err = lr;
                std::cerr << "[Lua load] " << sm.scriptPath << " -> " << err.what() << "\n";
                continue;
            }
            sol::protected_function_result pr = lr();
            if (!pr.valid()) {
                sol::error err = pr;
                std::cerr << "[Lua exec] " << sm.scriptPath << " -> " << err.what() << "\n";
                continue;
            }
            mod = pr;
            modules.emplace(sm.scriptPath, mod);
        } else {
            mod = it->second;
        }

        sol::protected_function fn = mod["update"];
        if (!fn.valid()) {
            std::cerr << "[Lua] " << sm.scriptPath << " no tiene update(self, state, ctx)\n";
            continue;
        }

        sol::table& st = stateMap[e];
        if (!st.valid() || st.get_type() == sol::type::lua_nil) {
            st = lua.create_table();
        }

        sol::table self = lua.create_table();
        self["x"]  = tr.position.x;
        self["y"]  = tr.position.y;
        self["vx"] = vel.velocity.x;   
        self["vy"] = vel.velocity.y;

        sol::table ctx = lua.create_table();
        ctx["dt"] = dt;
        ctx["player_x"] = hasPlayer ? player.x : tr.position.x;
        ctx["player_y"] = hasPlayer ? player.y : tr.position.y;
        ctx["maxSpeed"] = prm.maxSpeed;

        sol::protected_function_result res = fn(self, st, ctx);
        if (!res.valid()) {
            sol::error err = res;
            std::cerr << "[Lua call] " << sm.scriptPath << " -> " << err.what() << "\n";
            continue;
        }

        sol::table out = res.get<sol::table>();
        tr.position.x  = out.get_or("x",  tr.position.x);
        tr.position.y  = out.get_or("y",  tr.position.y);
        vel.velocity.x = out.get_or("vx", vel.velocity.x); 
        vel.velocity.y = out.get_or("vy", vel.velocity.y);
    }
}
