#include "Scene/BreakoutScene.h"
#include "Systems/PaddleSystem.h"
#include "Systems/BallSystem.h"
#include "Systems/BlockSystem.h"
#include "Systems/ImGuiSystem.h"
#include "Entity.h"
#include "../components/Components.h"
#include <raylib.h>

void BreakoutScene::onSetup() {
    addSystem(new PaddleSystem());
    addSystem(new BallSystem());
    addSystem(new BlockSystem());
    addSystem(new ImGuiSystem());

    float swP = float(GetScreenWidth());
    float shP = float(GetScreenHeight());

    auto paddleEnt = r.create();
    Entity paddle{ paddleEnt, this };
    paddle.addComponent<NameComponent>(NameComponent{ "Paddle" });
    paddle.addComponent<TransformComponent>(
        Vector2{ swP * 0.5f - PaddleSystem::paddleWidth() * 0.5f, shP - 30.0f }
    );
    paddle.addComponent<VelocityComponent>(Vector2{ 400.0f, 0.0f });
    paddle.addComponent<PaddleComponent>();

    float sw = (float)GetScreenWidth();
    float sh = (float)GetScreenHeight();

    auto ballEnt = r.create();
    Entity ball{ ballEnt, this };
    ball.addComponent<NameComponent>(NameComponent{ "Ball" });
    ball.addComponent<TransformComponent>(Vector2{ sw * 0.5f, sh * 0.5f });
    ball.addComponent<VelocityComponent>(Vector2{ 200.0f, 200.0f });
    ball.addComponent<BallComponent>();

    const int rows = 4;
    const int cols = 10;
    const float gap    = 4.0f;
    const float blockW = (sw - (cols - 1) * gap) / cols;
    const float blockH = 20.0f;
    const float startY = 40.0f;

    for (int rIdx = 0; rIdx < rows; ++rIdx) {
        for (int cIdx = 0; cIdx < cols; ++cIdx) {
            float x = cIdx * (blockW + gap);
            float y = startY + rIdx * (blockH + gap);

            auto e = r.create();
            Entity block{ e, this };
            block.addComponent<TransformComponent>(Vector2{ x, y });
            block.addComponent<SizeComponent>(Vector2{ blockW, blockH });
            block.addComponent<BlockComponent>();
        }
    }
}
