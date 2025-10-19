#pragma once
#include "Systems/System.h"
#include "Scene/Scene.h"

class TilemapLoaderSystem : public System {
public:
    using System::System;
    void update() override;
};
