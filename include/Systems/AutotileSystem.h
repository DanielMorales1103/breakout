#pragma once
#include "Systems/System.h"
#include "Scene/Scene.h"

class AutotileSystem : public System {
public:
    void update() override;   // la llenamos en el próximo checkpoint
};
