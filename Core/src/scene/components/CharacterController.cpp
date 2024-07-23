#include <pch.hpp>
#include "CharacterController.hpp"

#include <Core/src/phys/Physics.hpp>

#include <Jolt/Physics/Character/Character.h>

#include "../Node.hpp"

namespace gage::scene::components
{
    CharacterController::CharacterController(SceneGraph& scene, Node& node, phys::Physics& phys) :
        IComponent(scene, node),
        phys(phys)
    {
        character = phys.create_character(node.get_position(), node.get_rotation());
    }

    void CharacterController::init()
    {

    }
    void CharacterController::update(float delta)
    {
        auto position = character->GetPosition();
        node.set_position({position.GetX(), position.GetY(), position.GetZ() });
    }
    void CharacterController::render_depth(VkCommandBuffer cmd, VkPipelineLayout pipeline_layout)
    {

    }
    void CharacterController::render_geometry(VkCommandBuffer cmd, VkPipelineLayout pipeline_layout)
    {

    }
    void CharacterController::shutdown()
    {
        phys.destroy_character(character);
    }
}