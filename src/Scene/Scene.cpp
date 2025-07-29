#include "Scene/Scene.h"

void Scene::setup() {
    for (auto* system : systems) {
        system->setScene(this);
        system->setup();
    }
}

void Scene::update() {
    for (auto* system : systems) {
        system->update();
    }
}

void Scene::render() {
    for (auto* system : systems) {
        system->render();
    }
}

void Scene::addSystem(System* system) {
    systems.push_back(system);
}
