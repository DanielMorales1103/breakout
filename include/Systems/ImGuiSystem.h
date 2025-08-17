#pragma once
#include "System.h"

class ImGuiSystem : public System {
public:
    ImGuiSystem();
    ~ImGuiSystem() override;

    void setup() override;
    void update() override {}
    void render() override;
};
