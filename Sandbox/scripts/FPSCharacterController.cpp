#include "FPSCharacterController.hpp"

#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtx/matrix_decompose.hpp>

#include <Core/src/gfx/data/Camera.hpp>
#include <Core/src/hid/Keyboard.hpp>
#include <Core/src/hid/Mouse.hpp>
#include <Core/ThirdParty/imgui/imgui.h>
#include <Core/src/scene/Node.hpp>



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
        head_node = this->node.search_child_by_name("mixamorig:HeadTop_End");

    }
    void FPSCharacterController::update(float delta, const hid::Keyboard& keyboard, const hid::Mouse& mouse)
    {
        CharacterController::update(delta, keyboard, mouse);

        this->pitch -= mouse.get_delta().y;
        this->yaw -= mouse.get_delta().x;

        if(pitch > 90.0f) pitch = 90.0f;
        else if(pitch < -90.0) pitch = -90.0f;

        if(yaw > 360.0f) yaw = -360.0f;
        else if(yaw < -360.0f) yaw = 360.0f;
        

        glm::vec2 dir{0, 0};
        if(keyboard.get_action("FORWARD"))
        {
            dir -= glm::vec2(glm::sin(glm::radians(yaw)), glm::cos(glm::radians(yaw)));
        }
        if(keyboard.get_action("BACKWARD"))
        {
            dir += glm::vec2(glm::sin(glm::radians(yaw)), glm::cos(glm::radians(yaw)));
        }

        if(keyboard.get_action("LEFT"))
        {
            dir -= glm::vec2(glm::sin(glm::radians(yaw + 90.0)), glm::cos(glm::radians(yaw + 90.0)));
        }
        if(keyboard.get_action("RIGHT"))
        {
            dir += glm::vec2(glm::sin(glm::radians(yaw + 90.0)), glm::cos(glm::radians(yaw + 90.0)));
        }

        if(keyboard.get_action("JUMP"))
        {
            if(CharacterController::get_ground_state() == CharacterController::GroundState::GROUND)
            {
                CharacterController::add_velocity(glm::vec3{0.0f, 100.0f, 0.0f} * 1.0f * delta); 
            }
        }

        if(keyboard.get_action("SPRINT"))
        {
            current_speed = 4.0f;
        }
        else if(keyboard.get_action("WALK"))
        {
            current_speed = 0.5f;
        }
        else
        {
            current_speed = 2.0f;
        }

        auto velocity = CharacterController::get_velocity();
        if(glm::length2(dir) != 0.0f && glm::length2(velocity) < glm::pow(current_speed, 2.0f) && CharacterController::get_ground_state() == CharacterController::GroundState::GROUND)
        {
            dir = glm::normalize(dir);
            CharacterController::add_velocity(glm::vec3{dir.x, 0.0f, dir.y} * 30.0f * delta);
        }
        
        auto head_global_transform = head_node->get_global_transform();
        glm::vec3 scale;
        glm::quat rotation;
        glm::vec3 translation;
        glm::vec3 skew;
        glm::vec4 perspective;
        glm::decompose(head_global_transform, scale, rotation, translation, skew, perspective);

        //camera.position = translation;

        camera.rotation.x = pitch;
        camera.rotation.y = yaw;
        camera.position = glm::vec3{this->node.get_position().x, this->node.get_position().y + camera_y_offset, this->node.get_position().z};

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