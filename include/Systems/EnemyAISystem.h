#pragma once
#include "Systems/System.h"
#include "Scene/Scene.h"
#include "../components/Components.h"


class EnemyAISystem : public System {
public:
    using System::System;
    void update() override;
};
