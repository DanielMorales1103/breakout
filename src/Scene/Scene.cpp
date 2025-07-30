#include "Scene/Scene.h"
#include "Systems/PaddleSystem.h"
#include "Systems/BallSystem.h"
#include "Entity.h"
#include "../components/Components.h"
#include <raylib.h>

void Scene::setup() {
    // 1) Registrar tu BallSystem
    addSystem(new PaddleSystem());
    addSystem(new BallSystem());

    // 2) Inicializar todos los sistemas
    for (auto* s : systems) {
        s->setScene(this);
        s->setup();
    }

    float swP = float(GetScreenWidth());
    float shP = float(GetScreenHeight());

    auto paddleEnt = r.create();
    Entity paddle{ paddleEnt, this };

    // Centrado horizontal, a 30px del fondo
    paddle.addComponent<TransformComponent>(
        Vector2{ swP*0.5f - PaddleSystem::paddleWidth()*0.5f, shP - 30.0f }
    );
    // Velocidad X de 400 px/s
    paddle.addComponent<VelocityComponent>(
        Vector2{ 400.0f, 0.0f }
    );
    paddle.addComponent<PaddleComponent>();

    // 3) Crear la entidad pelota
    float sw = (float)GetScreenWidth();
    float sh = (float)GetScreenHeight();

    auto ent = r.create();
    Entity ballEntity{ ent, this };
    ballEntity.addComponent<TransformComponent>(Vector2{ sw*0.5f, sh*0.5f });
    ballEntity.addComponent<VelocityComponent>(Vector2{ 200.0f, 200.0f });
    ballEntity.addComponent<BallComponent>();
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
