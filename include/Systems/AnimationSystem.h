#pragma once
#include "Systems/System.h"
#include "Scene/Scene.h"
#include "../components/Components.h"


class AnimationSystem : public System {
public:
    using System::System;
    void update() override;
};
