#include <pch.hpp>
#include "Physics.hpp"

#include "../scene.hpp"
#include "../Node.hpp"
#include "../data/Model.hpp"

#include <Core/src/phys/Physics.hpp>
#include <Core/src/phys/Layers.hpp>
#include <Jolt/Physics/Character/Character.h>
#include <Jolt/Physics/Collision/Shape/HeightFieldShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>

namespace gage::scene::systems
{
    Physics::Physics(phys::Physics &phys) : phys(phys)
    {
    }

    void Physics::init()
    {
        for (auto &character_controller : character_controllers)
        {
            character_controller->character = phys.create_character(character_controller->node.get_position(), character_controller->node.get_rotation());
        }

        for (auto &terrain_renderer : terrain_renderers)
        {
            // Init height map for physics

            JPH::HeightFieldShapeSettings shape_settings(terrain_renderer.terrain_renderer->height_map.data(), JPH::Vec3(0, 0, 0), JPH::Vec3(1, 1, 1), terrain_renderer.terrain_renderer->size);
            JPH::BodyCreationSettings setting(shape_settings.Create().Get(),
                                              JPH::RVec3(0.0, -1.0, 0.0),
                                              JPH::Quat::sIdentity(), JPH::EMotionType::Static, phys::Layers::NON_MOVING);
            setting.mFriction = 0.2f;

            terrain_renderer.height_map_body = this->phys.get_body_interface()->CreateAndAddBody(setting, JPH::EActivation::DontActivate);
        }
    }

    void Physics::shutdown()
    {
        for (auto &character_controller : character_controllers)
        {
            phys.destroy_character(character_controller->character);
        }

        for (const auto &terrain_renderer : terrain_renderers)
        {
            this->phys.get_body_interface()->RemoveBody(terrain_renderer.height_map_body);
            this->phys.get_body_interface()->DestroyBody(terrain_renderer.height_map_body);
        }
    }

    void Physics::update(float delta)
    {
        for (auto &character_controller : character_controllers)
        {
            character_controller->character->PostSimulation(0.1f);
            auto position = character_controller->character->GetPosition(false);
            character_controller->node.set_position({position.GetX(), position.GetY(), position.GetZ()});

            {
                JPH::CharacterBase::EGroundState state = character_controller->character->GetGroundState();
                const JPH::BodyLockInterface *lock_interface = phys.get_body_lock_interface();
                JPH::BodyLockWrite lock(*lock_interface, character_controller->character->GetBodyID());
                if (lock.Succeeded())
                {
                    JPH::Body &body = lock.GetBody();

                    switch (state)
                    {
                    case JPH::CharacterBase::EGroundState::InAir:
                    {
                        body.GetMotionProperties()->SetLinearDamping(0.0f);
                        break;
                    }

                    case JPH::CharacterBase::EGroundState::NotSupported:
                    case JPH::CharacterBase::EGroundState::OnGround:
                    case JPH::CharacterBase::EGroundState::OnSteepGround:
                    {
                        body.GetMotionProperties()->SetLinearDamping(5.0f);
                        break;
                    }
                    }
                }
            }
        }
    }

    void Physics::add_character_controller(std::unique_ptr<components::CharacterController> character_controller)
    {
        character_controllers.push_back(std::move(character_controller));
    }

    void Physics::add_terrain_renderer(std::shared_ptr<components::Terrain> terrain_renderer)
    {
        Terrain terrain_renderer_additional_datas{};
        terrain_renderer_additional_datas.terrain_renderer = terrain_renderer;

        terrain_renderers.push_back(std::move(terrain_renderer_additional_datas));
    }

    void Physics::character_add_impulse(components::CharacterController *character, const glm::vec3 &vel)
    {
        character->character->AddImpulse(JPH::Vec3(vel.x, vel.y, vel.z));
    }
    void Physics::character_set_velocity(components::CharacterController *character, const glm::vec3 &vel)
    {
        character->character->SetLinearVelocity(JPH::Vec3(vel.x, vel.y, vel.z));
    }
    glm::vec3 Physics::character_get_velocity(components::CharacterController *character)
    {
        auto velocity = character->character->GetLinearVelocity(false);

        return glm::vec3{velocity.GetX(), velocity.GetY(), velocity.GetZ()};
    }
    Physics::GroundState Physics::character_get_ground_state(components::CharacterController *character)
    {
        JPH::CharacterBase::EGroundState state = character->character->GetGroundState();
        switch (state)
        {
        case JPH::CharacterBase::EGroundState::InAir:
            return GroundState::AIR;
        case JPH::CharacterBase::EGroundState::OnGround:
            return GroundState::GROUND;
        default:
            return GroundState::GROUND;
        }

        return GroundState::GROUND;
    }

}