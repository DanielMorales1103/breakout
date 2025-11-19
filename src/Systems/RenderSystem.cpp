#include "Systems/RenderSystem.h"
#include "Scene/Scene.h"
#include "../components/Components.h"
#include "TextureManager.h"
#include <raylib.h>

static void DrawSprite(const Texture2D& tex,
                       const Rectangle& src,
                       const Vector2& worldPos,
                       float rotation,
                       float scale) {
    Rectangle dest{
        worldPos.x,
        worldPos.y,
        src.width * scale,
        src.height * scale
    };
    Vector2 origin{ dest.width * 0.5f, dest.height * 0.5f };
    DrawTexturePro(tex, src, dest, origin, rotation, WHITE);
}

void RenderSystem::update() { }

void RenderSystem::render() {
    if (scene->gameOver) {
        DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), BLACK);

        const char* msg = "GAME OVER";
        int fontSize = 40;
        int sw = GetScreenWidth();
        int sh = GetScreenHeight();
        int tw = MeasureText(msg, fontSize);

        DrawText(msg, sw/2 - tw/2, sh/2 - fontSize/2, fontSize, RED);
        return; 
    }

    if (scene->victory) {
        DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), BLACK);

        const char* msg = "VICTORIA";
        int fontSize = 40;
        int sw = GetScreenWidth();
        int sh = GetScreenHeight();
        int tw = MeasureText(msg, fontSize);

        DrawText(msg, sw/2 - tw/2, sh/2 - fontSize/2, fontSize, GREEN);

        const char* sub = "Has derrotado todas las oleadas";
        int fontSize2 = 20;
        int tw2 = MeasureText(sub, fontSize2);
        DrawText(sub, sw/2 - tw2/2, sh/2 + 40, fontSize2, RAYWHITE);

        return; // no dibujamos nada más
    }

    {
        auto bgView = scene->r.view<SpriteComponent, BackgroundTag>();
        bgView.each([&](SpriteComponent& s) {
            const Texture2D tex = TextureManager::GetTexture(s.texturePath);
            if (tex.id == 0) return;

            const int sw = GetScreenWidth();
            const int sh = GetScreenHeight();

            Rectangle src = s.src.width > 0 ? s.src
                                            : Rectangle{0,0,(float)tex.width,(float)tex.height};
            Rectangle dest{ 0, 0, (float)sw, (float)sh };
            Vector2  origin{ 0, 0 };
            DrawTexturePro(tex, src, dest, origin, 0.0f, WHITE);
        });
    }

    Vector2 camPos{0,0};
    float camZoom = 1.0f;
    {
        auto cv = scene->r.view<CameraComponent, TransformComponent, ViewportComponent, CameraTag>();
        cv.each([&](auto /*e*/, CameraComponent &c, TransformComponent &t, ViewportComponent &) {
            if (c.active) { camPos = t.position; camZoom = c.zoom; }
        });
    }


    {
        auto view = scene->r.view<TransformComponent, SpriteComponent>(entt::exclude<BackgroundTag>);
        view.each([&](TransformComponent& t, SpriteComponent& s) {
            const Texture2D tex = TextureManager::GetTexture(s.texturePath);
            if (tex.id == 0) return;

            Rectangle src = s.src.width > 0 ? s.src
                                            : Rectangle{0,0,(float)tex.width,(float)tex.height};

            const float S = (t.scale <= 0.0f) ? 1.0f : t.scale;

            Rectangle dest{
                (t.position.x - camPos.x) * camZoom,
                (t.position.y - camPos.y) * camZoom,
                src.width  * S * camZoom,
                src.height * S * camZoom
            };
            Vector2 origin{ dest.width * 0.5f, dest.height * 0.5f };
            DrawTexturePro(tex, src, dest, origin, t.rotation, WHITE);
        });
    }

    // --- Proyectiles ---
    {
        auto view = scene->r.view<ProjectileTag, TransformComponent>();
        view.each([&](auto, TransformComponent& tr) {

            // Aplicar cámara igual que a los sprites
            float screenX = (tr.position.x - camPos.x) * camZoom;
            float screenY = (tr.position.y - camPos.y) * camZoom;

            float radius = 4.0f * camZoom;  // por si quieres que el zoom también afecte a la bala

            DrawCircleV(Vector2{screenX, screenY}, radius, WHITE);
        });
    }
}
