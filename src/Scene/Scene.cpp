#include "Scene/Scene.h"
#include "Systems/PaddleSystem.h"
#include "Systems/BallSystem.h"
#include "Systems/BlockSystem.h"
#include "Entity.h"
#include "../components/Components.h"
#include <raylib.h>

void Scene::setup() {
    // 1) Registrar tu BallSystem
    addSystem(new PaddleSystem());
    addSystem(new BallSystem());
    addSystem(new BlockSystem());

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

    // --- Bloques ---
    const int rows = 4;
    const int cols = 10;
    const float gap    = 4.0f;
    const float blockW = (sw - (cols - 1) * gap) / cols;
    const float blockH = 20.0f;
    const float startY = 40.0f; // margen superior

    for (int rIdx = 0; rIdx < rows; ++rIdx) {
        for (int cIdx = 0; cIdx < cols; ++cIdx) {
            float x = cIdx * (blockW + gap);
            float y = startY + rIdx * (blockH + gap);

            auto e = r.create();
            Entity block{ e, this };
            block.addComponent<TransformComponent>( Vector2{ x, y } );
            block.addComponent<SizeComponent>     ( Vector2{ blockW, blockH } );
            block.addComponent<BlockComponent>();
        }
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
