#pragma once

#include "System.h"
#include "../components/Components.h"
#include <raylib.h>

class PaddleSystem : public System {
public:
    void update() override {
        float dt = GetFrameTime();
        float sw = static_cast<float>(GetScreenWidth());

        // Iteramos todas las entidades que tengan estos 3 componentes:
        auto view = scene->r.view<PaddleComponent>();
        for (auto ent : view) {
            auto &t = scene->r.get<TransformComponent>(ent);
            auto &v = scene->r.get<VelocityComponent>(ent);

            // 1) Lectura de input
            if (IsKeyDown(KEY_LEFT))  t.position.x -= v.velocity.x * dt;
            if (IsKeyDown(KEY_RIGHT)) t.position.x += v.velocity.x * dt;

            // 2) Limitar dentro de la pantalla
            if (t.position.x < 0)             t.position.x = 0;
            if (t.position.x + paddleWidth() > sw)
                t.position.x = sw - paddleWidth();
        }
    }

    void render() override {
        // Dibujamos cada paddle como un rectángulo
        auto view = scene->r.view<PaddleComponent>();
        for (auto ent : view) {
            auto &t = scene->r.get<TransformComponent>(ent);
            DrawRectangle(
                int(t.position.x),
                int(t.position.y),
                int(paddleWidth()),
                int(paddleHeight()),
                RAYWHITE
            );
        }
    }

    // Tamaño del paddle
    static float paddleWidth()  { return 120.0f; }
    static float paddleHeight() { return 20.0f; }
};
