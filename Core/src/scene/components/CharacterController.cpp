#include <pch.hpp>
#include "CharacterController.hpp"

#include <Core/src/phys/Physics.hpp>
#include <Core/src/gfx/data/Camera.hpp>
#include <imgui/imgui.h>

#include <Jolt/Physics/Character/Character.h>

#include "../Node.hpp"

namespace gage::scene::components
{
    CharacterController::CharacterController(SceneGraph &scene, Node &node, phys::Physics &phys) : IComponent(scene, node),
                                                                                                   phys(phys)
    {
        character = phys.create_character(node.get_position(), node.get_rotation());
    }

    void CharacterController::init()
    {
    }
    void CharacterController::update(float)
    {
        auto position = character->GetPosition(false);
        node.set_position({position.GetX(), position.GetY(), position.GetZ()});
    }

    void CharacterController::shutdown()
    {
        phys.destroy_character(character);
    }

    void CharacterController::render_imgui()
    {
        ImGui::Text("CharacterController");

        ImGui::Separator();
    }
}