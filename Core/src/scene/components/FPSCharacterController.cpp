#include <pch.hpp>
#include "FPSCharacterController.hpp"

#include <Core/src/gfx/data/Camera.hpp>
#include <imgui/imgui.h>


#include "../Node.hpp"

namespace gage::scene::components
{
    FPSCharacterController::FPSCharacterController(SceneGraph& scene, Node& node, phys::Physics& phys, gfx::data::Camera& camera) :
        CharacterController(scene, node, phys),
        camera(camera)
    {

    }

    void FPSCharacterController::init()
    {
        CharacterController::init();

    }
    void FPSCharacterController::update(float delta)
    {
        CharacterController::update(delta);
        camera.position = glm::vec3{node.get_position().x, node.get_position().y + camera_y_offset, node.get_position().z};
    }

    void FPSCharacterController::shutdown()
    {
        CharacterController::shutdown();
    }

    void FPSCharacterController::render_imgui()
    {
        CharacterController::render_imgui();
        ImGui::Text("FPSCharacterController");
        ImGui::DragFloat("Camera y offset", &camera_y_offset, 0.01f, -10.0f, 10.0f);
        ImGui::Separator();
    }
}