#pragma once

#include "System.h"
#include "../components/Components.h"
#include <raylib.h>
#include <iostream>

class BlockSystem : public System {
public:
    void update() override {
        constexpr float radius = 8.0f; 
        auto ballView  = scene->r.view<BallComponent>();
        auto blockView = scene->r.view<BlockComponent>();

        for (auto ballEnt : ballView) {
            auto &bt = scene->r.get<TransformComponent>(ballEnt);
            auto &bv = scene->r.get<VelocityComponent>(ballEnt);

            Rectangle ballRec{
                bt.position.x - radius,
                bt.position.y - radius,
                radius * 2,
                radius * 2
            };

            for (auto blockEnt : blockView) {
                auto &kt = scene->r.get<TransformComponent>(blockEnt);
                auto &ks = scene->r.get<SizeComponent>(blockEnt);

                Rectangle blockRec{
                    kt.position.x,
                    kt.position.y,
                    ks.size.x,
                    ks.size.y
                };

                if (CheckCollisionRecs(ballRec, blockRec)) {
                    // Rebote y aceleración
                    bv.velocity.y *= -1.01f;

                    if (bt.position.y < kt.position.y) {
                        bt.position.y = kt.position.y - radius;
                    } else {
                        bt.position.y = kt.position.y + ks.size.y + radius;
                    }

                    // Destruir bloque
                    scene->r.destroy(blockEnt);

                    bool hasBlocks = false;
                    auto check = scene->r.view<BlockComponent>();
                    for (auto _ : check) { hasBlocks = true; break; }

                    if (!hasBlocks) {
                        std::cout << "YOU WIN!" << std::endl;
                        WaitTime(1.0);   
                        exit(0);
                    }

                    break; 
                }
            }
        }
    }

    void render() override {
        auto view = scene->r.view<BlockComponent>();
        for (auto ent : view) {
            auto &t = scene->r.get<TransformComponent>(ent);
            auto &s = scene->r.get<SizeComponent>(ent);
            DrawRectangle(
                int(t.position.x),
                int(t.position.y),
                int(s.size.x),
                int(s.size.y),
                DARKBLUE
            );
        }
    }
};
