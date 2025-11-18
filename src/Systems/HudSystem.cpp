#include "Systems/HudSystem.h"
#include <raylib.h>
#include <algorithm>

void HudSystem::update() {
    auto& r = scene->r;

    PlayerHealth* hp = nullptr;
    r.view<PlayerTag, PlayerHealth>().each([&](auto, PlayerHealth& h){
        if (!hp) hp = &h;
    });
    if (!hp) return;

    float ratio = (hp->max > 0) ? (float)hp->current / (float)hp->max : 0.0f;
    ratio = std::clamp(ratio, 0.0f, 1.0f);

    int sw = GetScreenWidth();
    const int margin    = 8;
    const int barWidth  = 150;
    const int barHeight = 14;

    int x = sw - margin - barWidth - 30;
    int y = margin;

    DrawRectangle(x-2, y-2, barWidth+4, barHeight+4, Fade(BLACK, 0.6f));
    DrawRectangle(x,   y,   barWidth,   barHeight,   DARKGRAY);
    DrawRectangle(x,   y,   (int)(barWidth * ratio), barHeight, GREEN);

    const int fontSize = 10;
    char buf[32];
    snprintf(buf, sizeof(buf), "%d/%d", hp->current, hp->max);

    DrawText("HP", x - 28, y + barHeight/2 - fontSize/2, fontSize, WHITE);
    DrawText(buf, x + barWidth + 6, y + barHeight/2 - fontSize/2, fontSize, WHITE);
}
