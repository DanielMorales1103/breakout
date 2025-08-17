#include "Systems/ImGuiSystem.h"
#include "Scene/Scene.h"
#include "rlImGui.h"
#include "imgui.h"
#include "../components/Components.h"
#include "Globals.h"

// ------------- helpers -------------
static void ShowComponentProperties(entt::registry& r, entt::entity e) {
    if (r.all_of<NameComponent>(e)) {
        auto& n = r.get<NameComponent>(e);
        ImGui::InputText("Name", n.tag.data(), n.tag.capacity() + 1);
    }
    if (r.all_of<TransformComponent>(e)) {
        auto& t = r.get<TransformComponent>(e);
        ImGui::DragFloat2("Position", &t.position.x, 1.0f);
    }
    if (r.all_of<VelocityComponent>(e)) {
        auto& v = r.get<VelocityComponent>(e);
        ImGui::DragFloat2("Velocity", &v.velocity.x, 1.0f);
    }
    if (r.all_of<SizeComponent>(e)) {
        auto& s = r.get<SizeComponent>(e);
        ImGui::DragFloat2("Size", &s.size.x, 1.0f, 0.0f, 2000.0f);
    }
    // agrega aquí más componentes que tengas (PaddleComponent, BallComponent, etc)
}
// -----------------------------------

ImGuiSystem::ImGuiSystem() { rlImGuiSetup(true); }
ImGuiSystem::~ImGuiSystem() { rlImGuiShutdown(); }
void ImGuiSystem::setup() {}

void ImGuiSystem::render() {
    rlImGuiBegin();

    ImGui::Begin("Inspector");

    bool paused = !g_RunUpdate;
    if (ImGui::Checkbox("Paused (stop update)", &paused)) {
        g_RunUpdate = !paused;
    }
    ImGui::Separator();

    auto view = scene->r.view<NameComponent>();

    size_t count = 0;
    view.each([&](entt::entity, NameComponent&) { ++count; });
    ImGui::Text("Entities with NameComponent: %zu", count);
    ImGui::Separator();

    view.each([&](entt::entity e, NameComponent &name){
        ImGui::PushID((int)entt::to_integral(e));
        if (ImGui::CollapsingHeader(name.tag.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {

            if (scene->r.all_of<TransformComponent>(e)) {
                auto &t = scene->r.get<TransformComponent>(e);
                ImGui::TextUnformatted("Transform");
                ImGui::DragFloat("pos.x", &t.position.x, 1.0f);
                ImGui::DragFloat("pos.y", &t.position.y, 1.0f);
            }

            if (scene->r.all_of<VelocityComponent>(e)) {
                auto &v = scene->r.get<VelocityComponent>(e);
                ImGui::TextUnformatted("Velocity");
                ImGui::DragFloat("vel.x", &v.velocity.x, 1.0f);
                ImGui::DragFloat("vel.y", &v.velocity.y, 1.0f);
            }

            if (scene->r.all_of<SizeComponent>(e)) {
                auto &s = scene->r.get<SizeComponent>(e);
                ImGui::TextUnformatted("Size");
                ImGui::DragFloat("w", &s.size.x, 1.0f, 1.0f, 1000.0f);
                ImGui::DragFloat("h", &s.size.y, 1.0f, 1.0f, 1000.0f);
            }
        }
        ImGui::PopID();
    });


    ImGui::End();
    rlImGuiEnd();
}
