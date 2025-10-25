#pragma once
#include "Systems/System.h"
#include "Scene/Scene.h"
#include "../components/Components.h"

class GridCollisionSystem : public System {
public:
    void update() override; 
};
