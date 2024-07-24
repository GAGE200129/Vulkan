#include <pch.hpp>
#include "FPSCharacterController.hpp"

#include <Core/src/gfx/data/Camera.hpp>
#include <Core/src/hid/Keyboard.hpp>
#include <imgui/imgui.h>

#include <Jolt/Physics/Character/Character.h>

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
    void FPSCharacterController::update(float delta, const hid::Keyboard& keyboard)
    {
        CharacterController::update(delta, keyboard);
        camera.position = glm::vec3{node.get_position().x, node.get_position().y + camera_y_offset, node.get_position().z};

        glm::vec2 dir{0, 0};
        if(keyboard.get_action("FORWARD"))
        {
            dir += glm::vec2(0, -1);
        }
        if(keyboard.get_action("BACKWARD"))
        {
            dir += glm::vec2(0, 1);
        }

        if(keyboard.get_action("LEFT"))
        {
            dir += glm::vec2(-1, 0);
        }
        if(keyboard.get_action("RIGHT"))
        {
            dir += glm::vec2(1, 0);
        }

        if(keyboard.get_action("JUMP"))
        {
            if(character->GetGroundState() == JPH::CharacterBase::EGroundState::OnGround)
            {
                character->AddLinearVelocity(JPH::Vec3(0, 100, 0) * 1.0f * delta, true);
            }
        }

        auto velocity = character->GetLinearVelocity(false);
        if(glm::length2(dir) != 0.0f && velocity.LengthSq() < glm::pow(2.0f, 2.0f))
        {
            dir = glm::normalize(dir);
            character->AddLinearVelocity(JPH::Vec3(dir.x, 0, dir.y) * 30.0f * delta, true);
        }

        
        // if(character->GetGroundState() == JPH::CharacterBase::EGroundState::InAir)
        // {
        //     character->AddLinearVelocity(JPH::Vec3(0, -9.8, 0) * delta, true);
        // }
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