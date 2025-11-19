#include "Systems/ProjectileHitSystem.h"
#include "../components/Components.h"
#include <raylib.h>
#include <cmath>

void ProjectileHitSystem::update() {
    auto& r = scene->r;

    auto pv = r.view<ProjectileTag, TransformComponent>();
    auto ev = r.view<EnemyTag, TransformComponent, SpriteComponent>();

    const float bulletRadius = 4.0f;  // igual que en el render de la bala

    for (auto [pe, ptr] : pv.each()) {

        bool projectileUsed = false;

        for (auto [ee, etr, espr] : ev.each()) {
            float scale = (etr.scale <= 0.0f) ? 1.0f : etr.scale;
            float w = espr.src.width  * scale;
            float h = espr.src.height * scale;

            // recuerda que dibujas centrado, así que el centro es etr.position
            float halfW = w * 0.5f;
            float halfH = h * 0.5f;

            // caja del enemigo
            float left   = etr.position.x - halfW;
            float right  = etr.position.x + halfW;
            float top    = etr.position.y - halfH;
            float bottom = etr.position.y + halfH;

            // --- Colisión círculo (bala) vs AABB (enemigo) ---

            float bx = ptr.position.x;
            float by = ptr.position.y;

            // clamp punto de la caja más cercano a la bala
            float cx = bx;
            if (cx < left)   cx = left;
            if (cx > right)  cx = right;

            float cy = by;
            if (cy < top)    cy = top;
            if (cy > bottom) cy = bottom;

            float dx = bx - cx;
            float dy = by - cy;
            float dist2 = dx*dx + dy*dy;

            if (dist2 <= bulletRadius * bulletRadius) {
                // hit!
                r.destroy(ee); // enemigo
                r.destroy(pe); // bala
                projectileUsed = true;
                break;         // solo un enemigo por bala
            }
        }

        if (projectileUsed) {
            // ya destruimos la bala, no sigas usándola
            continue;
        }
    }
}
