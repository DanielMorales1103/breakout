#include "Systems/EnemySpawnSystem.h"
#include "Scene/Scene.h"
#include "../components/Components.h"
#include <raylib.h>
#include <cmath>
#include <algorithm>
#include <iostream>

// --- Helpers ---
static inline float frand(float a, float b) {
    return a + (float)GetRandomValue(0, 1000000) / 1000000.0f * (b - a);
}
static inline float len2(Vector2 v){ return v.x*v.x + v.y*v.y; }
static inline float clampf(float x, float a, float b){ return std::max(a, std::min(x, b)); }

const char* ENEMY_PATH = "assets/sprites/enemies.png";

struct ViewRect { float minX, minY, maxX, maxY; };
static ViewRect getWorldView(Scene* scene) {
    Vector2 camPos{0,0}; float zoom=1.f; int vw=320, vh=180;
    auto cv = scene->r.view<CameraComponent, TransformComponent, ViewportComponent, CameraTag>();
    cv.each([&](auto, CameraComponent& c, TransformComponent& t, ViewportComponent& v){
        if(c.active){ camPos=t.position; zoom=c.zoom; vw=v.width; vh=v.height; }
    });
    const float w = vw/zoom, h = vh/zoom;
    return { camPos.x, camPos.y, camPos.x + w, camPos.y + h };
}

// Tamaño para evitar spawnear sobre el jugador
static void getCellSize(Scene* scene, float& cw, float& ch) {
    cw = ch = 16.f;
    auto tv = scene->r.view<TilesetComponent, TransformComponent, TilemapTag>();
    tv.each([&](TilesetComponent& ts, TransformComponent& tr){
        float S = (tr.scale <= 0.f)? 1.f : tr.scale;
        cw = ts.tileSize.x * S; ch = ts.tileSize.y * S;
    });
}

// Posición del player
static bool getPlayerPos(Scene* scene, Vector2& pos) {
    bool ok=false;
    auto pv = scene->r.view<PlayerTag, TransformComponent>();
    pv.each([&](TransformComponent& t){ if(!ok){ pos=t.position; ok=true; }});
    return ok;
}

static Vector2 pickPointInViewAway(const ViewRect& r, Vector2 player, float minDist) {
    const float minDist2 = minDist*minDist;
    Vector2 chosen{ (r.minX+r.maxX)*0.5f, (r.minY+r.maxY)*0.5f };
    for (int tries=0; tries<40; ++tries) {
        Vector2 p{ frand(r.minX, r.maxX), frand(r.minY, r.maxY) };
        if (len2(Vector2{p.x-player.x, p.y-player.y}) >= minDist2) { return p; }
        chosen = p;
    }
    return chosen;
}

// Crea enemigo básico
static entt::entity spawnEnemy(Scene* scene, Vector2 pos, float scale, const char* scriptPath) {
    auto& r = scene->r;
    auto e = r.create();

    r.emplace<TransformComponent>(e, pos);
    auto &tr = r.get<TransformComponent>(e);
    tr.scale = scale;

    r.emplace<VelocityComponent>(e, Vector2{0,0});
    // Ajusta ENEMY_PATH y frame si tu atlas es distinto
    r.emplace<SpriteComponent>(e, SpriteComponent{
        ENEMY_PATH,
        Rectangle{ 0*26.0f, 0*46.0f, 26.0f, 46.0f },
        Vector2{26.0f, 46.0f}
    });
    r.emplace<AnimatorComponent>(e, AnimatorComponent{
        /*columns=*/3, /*rows=*/4, /*fps=*/6.0f,
        /*frame=*/0, /*acc=*/0.0f, /*moving=*/false,
        /*facing=*/AnimatorComponent::Down,
        /*baseCol=*/0, /*baseRow=*/0
    });
    r.emplace<EnemyTag>(e);

    r.emplace<MovementParams>(e, MovementParams{ 35.f });
    const char* path = (scriptPath && *scriptPath) ? scriptPath : "assets/scripts/move_tracking.lua";
    r.emplace<ScriptMove>(e, ScriptMove{ std::string(path)});

    return e;
}

// Asegura que una posición está dentro del viewport (pequeño margen)
static bool insideView(const ViewRect& vr, Vector2 p, float margin=0.f) {
    return p.x >= vr.minX+margin && p.x <= vr.maxX-margin &&
           p.y >= vr.minY+margin && p.y <= vr.maxY-margin;
}

void EnemySpawnSystem::update() {
    const float dt = GetFrameTime();

    // Config y estado (pueden existir varios spawners; iteramos todos)
    auto sv = scene->r.view<EnemySpawnSettings, EnemySpawnState>();
    sv.each([&](EnemySpawnSettings& cfg, EnemySpawnState& st){

        // Avanza reloj global
        st.waveTimer += dt;

        if (st.curWave >= (int)cfg.waves.size()) {
            if (cfg.loop) {
                st.curWave = 0;
                st.waveTimer = 0.f;
                st.spawnTimer = 0.f;
                st.spawnedInWave = 0;
                st.waveActive = false;
                st.patternLocked = false;
            } else {
                return;
            }
        }

        const WaveDef& w = cfg.waves[st.curWave];

        // Espera al startDelay de la oleada
        if (!st.waveActive) {
            if (st.waveTimer >= w.startDelay) {
                st.waveActive    = true;
                st.spawnTimer    = 0.f;
                st.spawnedInWave = 0;

                if (w.waveIndex >= 3) {
                    // Waves 3 y 4 aleatorio entre los tres
                    int r = GetRandomValue(0, 2); // 0..2
                    st.currentPattern = (r == 0 ? SpawnPattern::Line
                                        : r == 1 ? SpawnPattern::Circle
                                                : SpawnPattern::RandomArea);
                }else{
                    st.currentPattern = w.pattern;
                }
                st.patternLocked = true;
            } else {
                return;
            }
        }

        // Si ya completamos la oleada, pasa a la siguiente
        if (st.spawnedInWave >= w.count) {
            st.curWave++;
            st.patternLocked = false;
            st.waveActive = false;
            st.lineInit = false;
            st.circleInit = false;
            return;
        }

        // Espera entre spawns
        st.spawnTimer += dt;
        if (st.spawnTimer < w.interval) return;
        st.spawnTimer = 0.f;

        // --- Cálculos de viewport, jugador y celdas ---
        ViewRect view = getWorldView(scene);
        Vector2 playerPos{ (view.minX+view.maxX)*0.5f, (view.minY+view.maxY)*0.5f };
        (void)getPlayerPos(scene, playerPos);

        float cw, ch; getCellSize(scene, cw, ch);
        const float minDist = 3.0f * 0.5f * (cw+ch); // “3 tiles” aprox

        // --- Spawning según patrón elegido para ESTA oleada ---
        switch (st.currentPattern) {
            case SpawnPattern::RandomArea: {
                // depurar
                std::cout << "Spawning in RandomArea pattern\n";
                Vector2 p = pickPointInViewAway(view, playerPos, minDist);
                const char* ms = w.moveScript.empty() ? "assets/scripts/move_tracking.lua" : w.moveScript.c_str();
                spawnEnemy(scene, p, cfg.scale, ms);
                st.spawnedInWave++;
            } break;

            case SpawnPattern::Line: {
                // Init una sola vez por oleada
                if (!st.lineInit) {
                    st.lineHorizontal = (GetRandomValue(0,1) == 0);

                    // spacing chico para "pegaditos" (≈ 0.8 tile):
                    float tile = (cw + ch) * 0.5f;
                    st.lineSpacing = 1.5f * tile;

                    // Calcula longitud total de la línea de toda la oleada:
                    float total = (w.count > 1 ? (w.count - 1) * st.lineSpacing : 0.f);

                    // Elige base dentro de vista, con margen para que quepa la línea:
                    if (st.lineHorizontal) {
                        float margin = total * 0.5f;
                        st.lineBase.x = frand(view.minX + margin, view.maxX - margin);
                        st.lineBase.y = frand(view.minY, view.maxY);
                    } else {
                        float margin = total * 0.5f;
                        st.lineBase.x = frand(view.minX, view.maxX);
                        st.lineBase.y = frand(view.minY + margin, view.maxY - margin);
                    }

                    // Si quedó muy cerca del player, reubica la base:
                    if (len2(Vector2{st.lineBase.x - playerPos.x, st.lineBase.y - playerPos.y}) < (minDist*minDist)) {
                        st.lineBase = pickPointInViewAway(view, playerPos, minDist);
                    }
                    st.lineInit = true;
                }

                // Posición del N-ésimo enemigo de la línea:
                int idx = st.spawnedInWave;              // 0..count-1
                float total = (w.count > 1 ? (w.count - 1) * st.lineSpacing : 0.f);
                Vector2 p = st.lineBase;

                if (st.lineHorizontal) p.x = st.lineBase.x - total*0.5f + idx * st.lineSpacing;
                else                   p.y = st.lineBase.y - total*0.5f + idx * st.lineSpacing;

                // Clamp suave a vista:
                if (!insideView(view, p, 2.f)) {
                    p.x = clampf(p.x, view.minX + 2.f, view.maxX - 2.f);
                    p.y = clampf(p.y, view.minY + 2.f, view.maxY - 2.f);
                }
                const char* ms = w.moveScript.empty() ? "assets/scripts/move_tracking.lua" : w.moveScript.c_str();
                spawnEnemy(scene, p, cfg.scale, ms);
                st.spawnedInWave++;
            } break;

            case SpawnPattern::Circle: {
                if (!st.circleInit) {
                    float tile = (cw + ch) * 0.5f;

                    // Radio pequeño para que se vean cercanos:
                    st.circleRadius = frand(1.2f, 1.8f) * tile; // más compacto

                    // Margen para que quepa el círculo completo:
                    ViewRect vr = view;
                    vr.minX += st.circleRadius; vr.maxX -= st.circleRadius;
                    vr.minY += st.circleRadius; vr.maxY -= st.circleRadius;

                    st.circleCenter = pickPointInViewAway(vr, playerPos, std::max(minDist, st.circleRadius*0.8f));
                    st.circleInit = true;
                }

                int k = st.spawnedInWave;           // 0..count-1
                int need = std::max(1, w.count);
                float ang = 2.0f * PI * (float)k / (float)need;

                Vector2 p{
                    st.circleCenter.x + std::cos(ang) * st.circleRadius,
                    st.circleCenter.y + std::sin(ang) * st.circleRadius
                };

                // Clamp suave a vista:
                if (!insideView(view, p, 2.f)) {
                    p.x = clampf(p.x, view.minX + 2.f, view.maxX - 2.f);
                    p.y = clampf(p.y, view.minY + 2.f, view.maxY - 2.f);
                }
                const char* ms = w.moveScript.empty() ? "assets/scripts/move_tracking.lua" : w.moveScript.c_str();
                spawnEnemy(scene, p, cfg.scale, ms);
                st.spawnedInWave++;
            } break;

        }
    });
}
