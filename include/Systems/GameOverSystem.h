#pragma once
#include "Systems/System.h"
#include "Scene/Scene.h"
#include <raylib.h>

class GameOverSystem : public System {
public:
    void update() override;
};
