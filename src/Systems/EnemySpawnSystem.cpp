#include "Systems/EnemySpawnSystem.h"
#include "Scene/Scene.h"
#include "../components/Components.h"
#include <raylib.h>
#include <cmath>
#include <algorithm>

// --- Helpers ---
static inline float frand(float a, float b) {
    return a + (float)GetRandomValue(0, 1000000) / 1000000.0f * (b - a);
}
static inline float len2(Vector2 v){ return v.x*v.x + v.y*v.y; }
static inline float clampf(float x, float a, float b){ return std::max(a, std::min(x, b)); }

const char* ENEMY_PATH = "assets/sprites/enemies.png";

// Rectángulo visible en MUNDO
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

// Tamaño de celda (para medir “3 tiles”)
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

// Punto aleatorio del viewport con margen y mínimo a “minDist” del jugador.
// Si falla tras varios intentos, devuelve el último intento igual.
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
static entt::entity spawnEnemy(Scene* scene, Vector2 pos, float scale) {
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

        // ¿terminó todo?
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
                st.waveActive = true;
                st.spawnTimer = 0.f;
                st.spawnedInWave = 0;

                // Elige patrón ALEATORIO entre los 3 existentes
                int r = GetRandomValue(0, 2); // 0..2
                st.currentPattern = (r==0? SpawnPattern::Line
                                   : r==1? SpawnPattern::Circle
                                         : SpawnPattern::RandomArea);
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
                Vector2 p = pickPointInViewAway(view, playerPos, minDist);
                spawnEnemy(scene, p, cfg.scale);
                st.spawnedInWave++;
            } break;

            case SpawnPattern::Line: {
                // Horizontal o vertical al azar
                bool horizontal = (GetRandomValue(0,1) == 0);
                // Espaciado ≈ 1.5 tiles
                const float spacing = 1.5f * ((cw+ch)*0.5f);

                // Longitud total (aprox) de la línea
                const int   need = w.count - st.spawnedInWave;
                const float total = (need-1) * spacing;

                // Punto base dentro de la vista dejando margen para que quepa la línea
                Vector2 base;
                if (horizontal) {
                    float margin = total*0.5f;
                    base.x = frand(view.minX + margin, view.maxX - margin);
                    base.y = frand(view.minY,       view.maxY);
                } else {
                    float margin = total*0.5f;
                    base.x = frand(view.minX,       view.maxX);
                    base.y = frand(view.minY + margin, view.maxY - margin);
                }

                // Corrección: si está muy cerca del player, recoloca
                if (len2(Vector2{base.x-playerPos.x, base.y-playerPos.y}) < (minDist*minDist)) {
                    base = pickPointInViewAway(view, playerPos, minDist);
                }

                // Spawnea tantos como quepan en esta “ronda” (1 por tick)
                // Para no complicar, crea SOLO 1 por intervalo, en posición siguiente sobre la línea.
                int k = st.spawnedInWave; // cuántos van
                int idxInLine = k;        // siguiente índice
                Vector2 p = base;
                if (horizontal) p.x = base.x - total*0.5f + idxInLine*spacing;
                else            p.y = base.y - total*0.5f + idxInLine*spacing;

                // Si cae muy cerca del player, “salta” un paso
                if (len2(Vector2{p.x-playerPos.x, p.y-playerPos.y}) < (minDist*minDist)) {
                    if (horizontal) p.x += spacing;
                    else            p.y += spacing;
                }
                // Asegura que sigue dentro de la vista
                if (!insideView(view, p, 4.f)) {
                    p.x = clampf(p.x, view.minX+4, view.maxX-4);
                    p.y = clampf(p.y, view.minY+4, view.maxY-4);
                }

                spawnEnemy(scene, p, cfg.scale);
                st.spawnedInWave++;
            } break;

            case SpawnPattern::Circle: {
                // Centro y radio dentro de la vista
                const float radius = frand(2.5f, 4.0f) * ((cw+ch)*0.5f);
                ViewRect vr = view;
                // deja margen para que el círculo quepa
                vr.minX += radius; vr.maxX -= radius;
                vr.minY += radius; vr.maxY -= radius;

                Vector2 center = pickPointInViewAway(vr, playerPos, std::max(minDist, radius*0.6f));

                // Coloca 1 enemigo por intervalo en el siguiente ángulo
                int k = st.spawnedInWave;
                int need = w.count;
                float ang = (need > 1) ? (2.0f * PI * (float)k / (float)need) : 0.0f;
                Vector2 p {
                    center.x + std::cos(ang) * radius,
                    center.y + std::sin(ang) * radius
                };

                // Si quedara demasiado cerca del player, empuja un poco
                Vector2 d{ p.x - playerPos.x, p.y - playerPos.y };
                float d2 = len2(d);
                if (d2 < (minDist*minDist)) {
                    float inv = 1.0f / std::max(std::sqrt(d2), 0.001f);
                    p.x += d.x * inv * (minDist - std::sqrt(d2));
                    p.y += d.y * inv * (minDist - std::sqrt(d2));
                }
                // Clamp a vista
                if (!insideView(view, p, 4.f)) {
                    p.x = clampf(p.x, view.minX+4, view.maxX-4);
                    p.y = clampf(p.y, view.minY+4, view.maxY-4);
                }

                spawnEnemy(scene, p, cfg.scale);
                st.spawnedInWave++;
            } break;
        }
    });
}
