#pragma once

#include "System.h"
#include "../components/Components.h"
#include "PaddleSystem.h"
#include <raylib.h>
#include <iostream>

class BallSystem : public System {
public:
    void update() override {
        float dt = GetFrameTime();
        float sw = static_cast<float>(GetScreenWidth());
        float sh = static_cast<float>(GetScreenHeight());
        constexpr float radius = 8.0f;

        // Obtenemos todas las entidades que tienen BallComponent
        auto view = scene->r.view<BallComponent>();
        for (auto ent : view) {
            // Recuperamos los componentes que necesitamos
            auto &t = scene->r.get<TransformComponent>(ent);
            auto &v = scene->r.get<VelocityComponent>(ent);

            // Movimiento
            t.position.x += v.velocity.x * dt;
            t.position.y += v.velocity.y * dt;

            // Rebotes + aceleración
            if (t.position.x < 0 || t.position.x > sw) {
                v.velocity.x *= -1.0f;
            }
            if (t.position.y < 0) {
                v.velocity.y *= -1.0f;
            }

            Rectangle ballRec {
                t.position.x - radius,
                t.position.y - radius,
                radius * 2,
                radius * 2
            };

            auto paddleView = scene->r.view<TransformComponent, PaddleComponent>();
            for (auto pe : paddleView) {
                auto &pt = scene->r.get<TransformComponent>(pe);
                Rectangle padRec {
                    pt.position.x,
                    pt.position.y,
                    PaddleSystem::paddleWidth(),
                    PaddleSystem::paddleHeight()
                };

                if (CheckCollisionRecs(ballRec, padRec)) {
                    v.velocity.y *= -1.1f;
                    t.position.y = pt.position.y - radius;
                    break;  
                }
            }

            if (t.position.y > sh) {
                std::cout << "Game Over!" << std::endl;
                exit(0);
            }
        }
    }

    void render() override {
        // Mismo view: sólo necesitamos saber qué entidades tienen BallComponent
        auto view = scene->r.view<BallComponent>();
        for (auto ent : view) {
            auto &t = scene->r.get<TransformComponent>(ent);
            DrawCircleV(t.position, 8.0f, RAYWHITE);
        }
    }
};
