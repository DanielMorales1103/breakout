#include "Systems/ProximitySpawnSystem.h"
#include "Scene/Scene.h"
#include "../components/Components.h"
#include <algorithm>
#include <cmath>
#include <raylib.h>

static inline int idx2d(int x,int y,int w){ return y*w + x; }
static inline float sqr(float x){ return x*x; }
static inline float dist2(Vector2 a, Vector2 b){ return sqr(a.x-b.x)+sqr(a.y-b.y); }

const char* ENEMY_PATH2 = "assets/sprites/mer_8_chars1.png";

// ---- util: fábrica mínima de enemigo quieto (idle) ----
static entt::entity SpawnEnemy(entt::registry& r, Vector2 pos, float scale, const char* scriptPath) {
    auto e = r.create();
    r.emplace<TransformComponent>(e, pos);
    r.get<TransformComponent>(e).scale = scale;

    r.emplace<VelocityComponent>(e, Vector2{0,0});
    r.emplace<SpriteComponent>(e, SpriteComponent{
        ENEMY_PATH2,
        Rectangle{ 0*26.0f, 4*46.0f, 26.0f, 46.0f }, 
        Vector2{26.0f, 46.0f}
    });
    r.emplace<AnimatorComponent>(e, AnimatorComponent{
        /*columns=*/3, /*rows=*/4, /*fps=*/6.0f,
        /*frame=*/0, /*acc=*/0.0f, /*moving=*/false,
        /*facing=*/AnimatorComponent::Down,
        /*baseCol=*/0, /*baseRow=*/0
    });
    r.emplace<EnemyTag>(e);

    r.emplace<MovementParams>(e, MovementParams{35.f});
    const char* path = (scriptPath && *scriptPath) ? scriptPath : "assets/scripts/move_tracking.lua";
    r.emplace<ScriptMove>(e, ScriptMove{ std::string(path) });
    return e;
}

void ProximitySpawnSystem::update() {
    auto& r = scene->r;
    const float dt = GetFrameTime();

    entt::entity mapE = entt::null;
    TilemapComponent*   m  = nullptr;
    TilesetComponent*   ts = nullptr;
    TransformComponent* tr = nullptr;

    auto tv = r.view<TilemapComponent, TilesetComponent, TransformComponent, TilemapTag>();
    tv.each([&](auto e, TilemapComponent& tm, TilesetComponent& tset, TransformComponent& ttr){
        if (!m && tm.width>0 && tm.height>0 && !tm.tiles.empty()) {
            mapE=e; m=&tm; ts=&tset; tr=&ttr;
        }
    });

    if (m && !r.any_of<ProximitySpawnBakedTag>(mapE)) {
        r.emplace<ProximitySpawnBakedTag>(mapE);

        // índices de torres
        const int towerIdxArr[] = {32,33,34,40,41,42,48,49,50,56,57,58,64,65,66};
        const std::vector<int> towerIdx(std::begin(towerIdxArr), std::end(towerIdxArr));
        auto isTower = [&](int ti){
            return std::find(towerIdx.begin(), towerIdx.end(), ti) != towerIdx.end();
        };

        const float S  = (tr->scale<=0.f?1.f:tr->scale);
        const float cw = ts->tileSize.x * S;
        const float ch = ts->tileSize.y * S;

        for (int y=0; y<m->height; ++y) {
            for (int x=0; x<m->width; ++x) {
                const int ti = m->tiles[idx2d(x,y,m->width)];
                if (!isTower(ti)) continue;
                if ((x+y)%2) continue; // evitar demasiadas zonas

                Vector2 center{
                    tr->position.x + (x+0.5f)*cw,
                    tr->position.y + (y+0.5f)*ch
                };

                auto z = r.create();
                r.emplace<ProximitySpawnZone>(z, ProximitySpawnZone{
                    /*center*/        center,
                    /*triggerRadius*/ 3*cw,
                    /*spawnCount*/    3,
                    /*spawnInterval*/ 0.15f,
                    /*pattern*/       SpawnPattern::RandomArea,
                    /*oneShot*/       true,
                    /*moveScript*/    "assets/scripts/move_patrol.lua"
                });
                r.emplace<ProximitySpawnState>(z, ProximitySpawnState{}); 
            }
        }
    }

    // ===== 2) Player (para medir proximidad) =====
    Vector2 playerPos{};
    bool havePlayer=false;
    auto pv = r.view<PlayerTag, TransformComponent>();
    pv.each([&](TransformComponent& t){ if(!havePlayer){ playerPos=t.position; havePlayer=true; }});
    if(!havePlayer) return;

    // ===== 3) Disparar zonas por proximidad =====
    auto zv = r.view<ProximitySpawnZone, ProximitySpawnState>();
    zv.each([&](ProximitySpawnZone& z, ProximitySpawnState& st){
        if (z.oneShot && st.finished) return;

        const float r2 = z.triggerRadius * z.triggerRadius;
        if (!st.triggered && dist2(playerPos, z.center) <= r2) {
            st.triggered = true;
            st.spawning  = true;
            st.spawned   = 0;
            st.timer     = 0.f;
        }

        if (!st.triggered) return;

        st.timer += dt;
        if (st.spawned >= z.spawnCount) {
            st.spawning = false;
            if (z.oneShot) st.finished = true;
            return;
        }
        if (st.timer < z.spawnInterval) return;
        st.timer = 0.f;

        // posición de spawn según patrón
        Vector2 pos = z.center;

        switch (z.pattern) {
            case SpawnPattern::Line: {
                float dx = 24.f * (float)(st.spawned - z.spawnCount/2);
                pos = { z.center.x + dx, z.center.y };
            } break;

            case SpawnPattern::Circle: {
                float radius = 48.f;
                float ang = (2.f*PI) * ( (float)st.spawned / std::max(1, z.spawnCount) );
                pos = { z.center.x + radius*std::cos(ang), z.center.y + radius*std::sin(ang) };
            } break;

            case SpawnPattern::RandomArea: {
                float rx = GetRandomValue(-48, 48);
                float ry = GetRandomValue(-48, 48);
                pos = { z.center.x + rx, z.center.y + ry };
            } break;
        }

        // evita spawnear encima del player (3 tiles ~ 48px si tu tile es 16 y escala 3)
        if (dist2(pos, playerPos) < sqr(48.f)) {
            pos.x += (pos.x < playerPos.x ? -48.f : 48.f);
            pos.y += (pos.y < playerPos.y ? -48.f : 48.f);
        }
        const char* ms = z.moveScript.empty() ? "assets/scripts/move_patrol.lua" : z.moveScript.c_str();
        SpawnEnemy(r, pos, /*scale=*/3.0f, ms);
        st.spawned++;
    });
}
