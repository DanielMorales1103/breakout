#include "Scene/SpriteScene.h"
#include "TextureManager.h"  
#include <raylib.h>
#include <cmath>
#include "Systems/InputSystem.h"
#include "Systems/MovementSystem.h"
#include "Systems/AnimationSystem.h"
#include "Systems/EnemyAISystem.h"
#include "Systems/RenderSystem.h"
#include "../components/Components.h"
#include "Systems/TilemapLoaderSystem.h"
#include "Systems/TilemapRenderSystem.h"
#include "Systems/AutotileSystem.h"
#include "Systems/CameraFollowSystem.h"
#include "Systems/CameraZoomSystem.h"
#include "Systems/CameraInitSystem.h"
#include "Systems/GridCollisionSystem.h"
#include "Systems/IntGridBakeSystem.h"
#include "Systems/DebugRenderSystem.h"

static const char* BG_PATH = "assets/backgrounds/fondoAtl.png";
static const char* HERO_PATH = "assets/sprites/mer_8_chars1.png";
static const char* ENEMY_PATH = "assets/sprites/enemies.png";
static const char* TILES = "assets/backgrounds/tf_A5_atlantisA.png";


enum class Dir { Right, Left, Up, Down };
static Dir   heroDir    = Dir::Right;
static bool  heroMoving = false;

// Estado de animación
static int   heroFrame  = 0;   

// Estado de juego (posición/velocidad)
static float heroX, heroY;          


void SpriteScene::onSetup() {
    // --- Cargar texturas ---
    TextureManager::LoadTexture(BG_PATH);
    TextureManager::LoadTexture(HERO_PATH);
    TextureManager::LoadTexture(ENEMY_PATH);

    SetTextureFilter(TextureManager::GetTexture(BG_PATH),    TEXTURE_FILTER_POINT);
    SetTextureFilter(TextureManager::GetTexture(HERO_PATH),  TEXTURE_FILTER_POINT);
    SetTextureFilter(TextureManager::GetTexture(ENEMY_PATH), TEXTURE_FILTER_POINT);

    // --- Registrar systems (update en orden; render va al final) ---
    addSystem(new TilemapLoaderSystem());
    addSystem(new AutotileSystem());    
    addSystem(new IntGridBakeSystem()); 
    addSystem(new InputSystem());
    addSystem(new EnemyAISystem());
    addSystem(new MovementSystem());
    addSystem(new GridCollisionSystem());
    addSystem(new AnimationSystem());  
    addSystem(new CameraFollowSystem());
    addSystem(new CameraZoomSystem());
    addSystem(new TilemapRenderSystem());
    addSystem(new DebugRenderSystem());
    addSystem(new RenderSystem());

    // ---------- ENTIDAD: Fondo ----------
    // {
    //     auto e = r.create();
    //     r.emplace<TransformComponent>(e, Vector2{0, 0});

    //     const Texture2D bg = TextureManager::GetTexture(BG_PATH);
    //     r.emplace<SpriteComponent>(e, SpriteComponent{
    //         BG_PATH,
    //         Rectangle{0, 0, (float)bg.width, (float)bg.height},
    //         Vector2{(float)bg.width, (float)bg.height}
    //     });
    //     r.emplace<BackgroundTag>(e);
    // }

    // --- TILEMAP: capa base ---
    {
        const int tileW = 16, tileH = 16;

        TextureManager::LoadTexture(TILES);
        SetTextureFilter(TextureManager::GetTexture(TILES), TEXTURE_FILTER_POINT);

        auto e = r.create();
        r.emplace<TransformComponent>(e, Vector2{0,0});
        r.get<TransformComponent>(e).scale = 3.0f; 
        r.emplace<TilesetComponent>(e, TilesetComponent{
            /*texturePath=*/TILES,
            /*tileSize=*/Vector2{(float)tileW,(float)tileH},
            /*columns=*/8,          // 128 / 16
            /*rows=*/16,            // 256 / 16
            /*margin=*/0,
            /*spacing=*/0,
            /*clip=*/Rectangle{0,0,128,256} // toda la A5
        });

        // width/height los rellena el loader al leer el CSV
        r.emplace<TilemapComponent>(e, TilemapComponent{ 0, 0 });
        r.emplace<MapSourceComponent>(e, MapSourceComponent{ "assets/maps/level.csv" });
        r.emplace<TilemapTag>(e);
        r.emplace<AutoTileFillComponent>(e, AutoTileFillComponent{
            /*baseIndex=*/7,    
            /*padding=*/2,    
            /*enabled=*/true  
        });
    }

    // ---------- ENTIDAD: Player (morado sin sombra: columnas 7–9, filas 5–8) ----------
    {
        auto e = r.create();
        r.emplace<TransformComponent>(e, Vector2{
            (float)GetScreenWidth() * 0.5f,
            (float)GetScreenHeight() * 0.5f
        });
        auto &pt = r.get<TransformComponent>(e);
        pt.scale = 3.0f;

        r.emplace<VelocityComponent>(e, Vector2{0, 0});
        r.emplace<SpriteComponent>(e, SpriteComponent{
            HERO_PATH,
            Rectangle{ 6*26.0f, 4*46.0f, 26.0f, 46.0f },   // primer frame del bloque
            Vector2{26.0f, 46.0f}
        });
        r.emplace<AnimatorComponent>(e, AnimatorComponent{
            /*columns=*/3, /*rows=*/4, /*fps=*/8.0f,
            /*frame=*/0, /*acc=*/0.0f, /*moving=*/false,
            /*facing=*/AnimatorComponent::Down,
            /*baseCol=*/6, /*baseRow=*/4
        });
        r.emplace<BoundsClampComponent>(e, BoundsClampComponent{8.0f});
        r.emplace<PlayerTag>(e);
    }

    // ---------- ENEMIGOS: spawns en esquinas, lejos del player ----------
    {
        const int sw = GetScreenWidth();
        const int sh = GetScreenHeight();
        const float margin = 40.0f;   // separa del borde
        const float minDistFromPlayer = 200.0f;

        // Posición del player (la acabamos de crear arriba)
        Vector2 playerPos { (float)sw * 0.5f, (float)sh * 0.5f };
        {
            auto pv = r.view<PlayerTag, TransformComponent>();
            pv.each([&](TransformComponent& t){ playerPos = t.position; });
        }

        // 4 esquinas
        Vector2 corners[4] = {
            { margin,        margin        },   // TL
            { sw - margin,   margin        },   // TR
            { margin,        sh - margin   },   // BL
            { sw - margin,   sh - margin   }    // BR
        };

        auto sqr = [](float x){ return x*x; };
        auto dist2 = [&](Vector2 a, Vector2 b){
            return sqr(a.x-b.x) + sqr(a.y-b.y);
        };

        const int ENEMIES_COUNT = 4;  // sube si quieres; si >4, cicla esquinas
        for (int i = 0; i < ENEMIES_COUNT; ++i) {
            // elige esquina por índice y, si está muy cerca del player, usa la opuesta
            int idx = i % 4;
            Vector2 pos = corners[idx];
            if (dist2(pos, playerPos) < minDistFromPlayer*minDistFromPlayer) {
                pos = corners[(idx + 2) % 4]; // esquina opuesta
            }

            auto e = r.create();
            r.emplace<TransformComponent>(e, pos);
            auto &tr = r.get<TransformComponent>(e);
            tr.scale = 3.0f;

            r.emplace<VelocityComponent>(e, Vector2{0, 0});
            r.emplace<SpriteComponent>(e, SpriteComponent{
                ENEMY_PATH,
                Rectangle{ 0*26.0f, 0*46.0f, 26.0f, 46.0f }, // primer frame del primer bloque del atlas
                Vector2{26.0f, 46.0f}
            });
            r.emplace<AnimatorComponent>(e, AnimatorComponent{
                /*columns=*/3, /*rows=*/4, /*fps=*/6.0f,
                /*frame=*/0, /*acc=*/0.0f, /*moving=*/false,
                /*facing=*/AnimatorComponent::Down,
                /*baseCol=*/0, /*baseRow=*/0   // primer cuadrante del atlas
            });

            // seguir al player
            r.emplace<FollowAIComponent>(e, FollowAIComponent{
                /*speed=*/50.0f, /*stopRadius=*/0.0f
            });

            r.emplace<EnemyTag>(e);
        }
    }

    {
        auto cam = r.create();
        r.emplace<CameraTag>(cam);
        r.emplace<CameraComponent>(cam, CameraComponent{ true, 1.0f });
        r.emplace<ViewportComponent>(cam, ViewportComponent{ GetScreenWidth(), GetScreenHeight() });
        r.emplace<TransformComponent>(cam, Vector2{0,0});
        r.emplace<CameraFollowSettings>(cam, CameraFollowSettings{128.0f, 80.0f, 0.18f});
        r.emplace<CameraZoomSettings>(cam, CameraZoomSettings{ 0.75f, 2.0f, 0.10f, 0.20f });

        // centrar al player si existe
        Vector2 playerPos{0,0}; 
        bool hasPlayer=false;
        auto pv = r.view<PlayerTag, TransformComponent>();
        pv.each([&](auto /*e*/, TransformComponent& t){ if(!hasPlayer){playerPos=t.position; hasPlayer=true;} });
        if (hasPlayer) {
            auto &vp  = r.get<ViewportComponent>(cam);
            auto &ctf = r.get<TransformComponent>(cam);
            auto &cc  = r.get<CameraComponent>(cam);
            ctf.position.x = playerPos.x - (vp.width  / cc.zoom) * 0.5f;
            ctf.position.y = playerPos.y - (vp.height / cc.zoom) * 0.5f;
        }
    }
}

// --- ON RENDER ---
void SpriteScene::onRender() {}

void SpriteScene::onUpdate() {
    const float dt = GetFrameTime();

    auto pview = r.view<PlayerTag, TransformComponent, VelocityComponent, AnimatorComponent>();
    bool synced = false;
    pview.each([&](TransformComponent& t, VelocityComponent& v, AnimatorComponent& a) {
        heroX = t.position.x;
        heroY = t.position.y;
        

        // mover/idle para tu animación vieja
        heroMoving = (fabsf(v.velocity.x) > 0.001f || fabsf(v.velocity.y) > 0.001f);

        // mapear facing del Animator a tu Dir
        switch (a.facing) {
            case AnimatorComponent::Up:    heroDir = Dir::Up;    break;
            case AnimatorComponent::Down:  heroDir = Dir::Down;  break;
            case AnimatorComponent::Left:  heroDir = Dir::Left;  break;
            case AnimatorComponent::Right: heroDir = Dir::Right; break;
        }
        heroFrame = a.frame; 
        synced = true;
    });    

}
