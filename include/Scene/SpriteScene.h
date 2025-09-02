#pragma once
#include "Scene/Scene.h"

class SpriteScene : public Scene {
public:
    void onSetup() override;
    void onUpdate() override;
    void onRender() override;
};