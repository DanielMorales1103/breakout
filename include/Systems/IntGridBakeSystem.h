#pragma once
#include "Systems/System.h"
#include "Scene/Scene.h"
#include "../components/Components.h"

class IntGridBakeSystem : public System {
public:
    void update() override;
};
