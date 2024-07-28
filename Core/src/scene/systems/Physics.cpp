#include <pch.hpp>
#include "Physics.hpp"

#include "../scene.hpp"
#include "../Node.hpp"
#include "../data/Model.hpp"


#include <Core/src/phys/Physics.hpp>
#include <Jolt/Physics/Character/Character.h>

namespace gage::scene::systems
{
    Physics::Physics(phys::Physics& phys) :
        phys(phys)
    {
    }


    void Physics::init()
    {
        for (auto &character_controller : character_controllers)
        {
            character_controller->character = phys.create_character(character_controller->node.get_position(), character_controller->node.get_rotation());
        }
    }

    void Physics::shutdown()
    {
        for (auto &character_controller : character_controllers)
        {
            phys.destroy_character(character_controller->character);
        }
    }

    void Physics::update(float delta)
    {
        for (auto &character_controller : character_controllers)
        {
            character_controller->character->PostSimulation(0.1f);
            auto position = character_controller->character->GetPosition(false);
            character_controller->node.set_position({position.GetX(), position.GetY(), position.GetZ()});
        }
    }

    void Physics::add_character_controller(std::unique_ptr<components::CharacterController> character_controller)
    {
        character_controllers.push_back(std::move(character_controller));
    }

     void Physics::character_add_velocity(components::CharacterController* character, const glm::vec3& vel)
    {
        character->character->AddLinearVelocity(JPH::Vec3(vel.x, vel.y, vel.z), true);
    }
    glm::vec3 Physics::character_get_velocity(components::CharacterController* character)
    {
        auto velocity = character->character->GetLinearVelocity(false);

        return glm::vec3{velocity.GetX(), velocity.GetY(), velocity.GetZ()};
    }
    Physics::GroundState Physics::character_get_ground_state(components::CharacterController* character)
    {
        JPH::CharacterBase::EGroundState state = character->character->GetGroundState();
        switch(state)
        {
            case JPH::CharacterBase::EGroundState::InAir: return GroundState::AIR;
            case JPH::CharacterBase::EGroundState::OnGround: return GroundState::GROUND;
            default: return GroundState::AIR;
        }

        return GroundState::AIR;
    }

}