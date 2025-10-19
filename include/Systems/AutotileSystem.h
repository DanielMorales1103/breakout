#pragma once
#include "Systems/System.h"

class AutotileSystem : public System {
public:
    void update() override;   // la llenamos en el próximo checkpoint
};
