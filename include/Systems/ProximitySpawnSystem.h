#pragma once
#include "Systems/System.h"

struct ProximitySpawnBakedTag {};

struct ProximitySpawnSystem : public System { 
    void update() override; 
};
