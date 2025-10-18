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

    {
        auto view = scene->r.view<TransformComponent, SpriteComponent>(entt::exclude<BackgroundTag>);
        view.each([&](TransformComponent& t, SpriteComponent& s) {
            const Texture2D tex = TextureManager::GetTexture(s.texturePath);
            if (tex.id == 0) return;

            Rectangle src = s.src.width > 0 ? s.src
                                            : Rectangle{0,0,(float)tex.width,(float)tex.height};
            DrawSprite(tex, src, t.position, t.rotation, t.scale <= 0 ? 1.0f : t.scale);
        });
    }
}
