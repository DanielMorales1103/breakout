#pragma once
#include "Systems/System.h"
#include "Scene/Scene.h"
#include "../components/Components.h"


class RenderSystem : public System {
public:
    using System::System;
    void render() override; 
    void update() override; 
};
