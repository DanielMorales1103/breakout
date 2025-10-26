#pragma once
#include "Systems/System.h"
#include "Scene/Scene.h"
#include "../components/Components.h"

class EnemySpawnSystem : public System {
public:
    void update() override;
};
