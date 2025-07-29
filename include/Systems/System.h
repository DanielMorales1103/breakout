#pragma once

class Scene;

class System {
protected:
    Scene* scene = nullptr;

public:
    virtual ~System() = default;

    virtual void setup() {}
    virtual void update() {}
    virtual void render() {}

    virtual void setScene(Scene* s) { scene = s; }
};
