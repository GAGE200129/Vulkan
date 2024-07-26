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
    void CharacterController::update(float delta, const hid::Keyboard& keyboard, const hid::Mouse& mouse)
    {
        character->PostSimulation(0.1f);
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

    void CharacterController::add_velocity(const glm::vec3& vel)
    {
        character->AddLinearVelocity(JPH::Vec3(vel.x, vel.y, vel.z), true);
    }
    glm::vec3 CharacterController::get_velocity() const
    {
        auto velocity = character->GetLinearVelocity(false);

        return {velocity.GetX(), velocity.GetY(), velocity.GetZ()};
    }
    CharacterController::GroundState CharacterController::get_ground_state() const
    {
        JPH::CharacterBase::EGroundState state = character->GetGroundState();
        switch(state)
        {
            case JPH::CharacterBase::EGroundState::InAir: return GroundState::AIR;
            case JPH::CharacterBase::EGroundState::OnGround: return GroundState::GROUND;
        }

        return GroundState::AIR;
    }
}