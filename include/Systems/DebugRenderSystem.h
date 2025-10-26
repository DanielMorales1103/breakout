#pragma once
#include "Systems/System.h"
#include "Scene/Scene.h"
#include "../components/Components.h"

class DebugRenderSystem : public System {
public:
    void update() override; 
    void render() override;
};
