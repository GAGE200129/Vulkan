#include "FPSCharacterController.hpp"

#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/common.hpp>
#include <iostream>

#include <Core/src/gfx/data/Camera.hpp>
#include <Core/src/hid/Keyboard.hpp>
#include <Core/src/hid/Mouse.hpp>
#include <Core/ThirdParty/imgui/imgui.h>
#include <Core/src/scene/Node.hpp>
#include <Core/src/scene/components/Animator.hpp>

#include <Core/src/scene/systems/Animation.hpp>
#include <Core/src/scene/systems/Physics.hpp>

#include <Jolt/Jolt.h>
#include <Jolt/Physics/Character/CharacterBase.h>
#include <Jolt/Physics/Character/Character.h>

using namespace gage;

FPSCharacterController::FPSCharacterController(scene::SceneGraph &scene, scene::Node &node, const scene::systems::Physics &phys, gfx::data::Camera &camera) : Script(scene, node),
                                                                                                                                                              phys(phys),
                                                                                                                                                              camera(camera)
{
}

void FPSCharacterController::init()
{
    character_controller = (scene::components::CharacterController *)this->node.get_requested_component(typeid(scene::components::CharacterController).name());

    head_node = this->node.search_child_by_name("mixamorig:Head");
    spine1 = this->node.search_child_by_name("mixamorig:Spine1");
    spine = this->node.search_child_by_name("mixamorig:Spine");
    hips = this->node.search_child_by_name("mixamorig:Hips");

    // hips->set_position({0.0f, -0.8f, 0.0f});

    animator = (scene::components::Animator *)node.get_requested_component(typeid(scene::components::Animator).name());
    scene::systems::Animation::set_animator_animation(animator, "idle");
}
void FPSCharacterController::update(float delta, const hid::Keyboard &keyboard, const hid::Mouse &mouse)
{
    

    this->pitch -= mouse.get_delta().y * 0.3f;
    this->yaw -= mouse.get_delta().x * 0.3f;

    if (pitch > 90.0f)
        pitch = 90.0f;
    else if (pitch < -90.0)
        pitch = -90.0f;

    if (yaw > 360.0f)
        yaw = -360.0f;
    else if (yaw < -360.0f)
        yaw = 360.0f;

    target_roll = 0;
    if (keyboard.get_action("LEAN_LEFT"))
    {
        target_roll -= 30.0f;
    }
    if (keyboard.get_action("LEAN_RIGHT"))
    {
        target_roll += 30.0f;
    }
    roll = std::lerp(roll, target_roll, delta * 5.0f);

    glm::vec2 dir{0, 0};
    if (keyboard.get_action("FORWARD"))
    {
        dir -= glm::vec2(glm::sin(glm::radians(yaw)), glm::cos(glm::radians(yaw)));
    }
    if (keyboard.get_action("BACKWARD"))
    {
        dir += glm::vec2(glm::sin(glm::radians(yaw)), glm::cos(glm::radians(yaw)));
    }

    if (keyboard.get_action("LEFT"))
    {
        dir -= glm::vec2(glm::sin(glm::radians(yaw + 90.0)), glm::cos(glm::radians(yaw + 90.0)));
    }
    if (keyboard.get_action("RIGHT"))
    {
        dir += glm::vec2(glm::sin(glm::radians(yaw + 90.0)), glm::cos(glm::radians(yaw + 90.0)));
    }

    if (keyboard.get_action_once("JUMP"))
    {
        if (scene::systems::Physics::character_get_ground_state(character_controller) == scene::systems::Physics::GroundState::GROUND && current_state == State::NORMAL)
        {
            scene::systems::Physics::character_add_impulse(character_controller, glm::vec3{0.0f, 100000.0f, 0.0f} * delta);
        }
    }

    if (keyboard.get_action_once("FLY"))
    {
        if (current_state == State::NORMAL)
        {
            current_state = State::FLYING;
            phys.character_set_gravity_factor(character_controller, 0.0f);
        }
        else if (current_state == State::FLYING)
        {
            current_state = State::NORMAL;
            phys.character_set_gravity_factor(character_controller, 1.0f);
        }
    }

    if (keyboard.get_action("SPRINT"))
    {
        current_speed = 40.0f;
    }
    else if (keyboard.get_action("WALK"))
    {
        current_speed = 0.5f;
    }
    else
    {
        current_speed = 2.0f;
    }

    if (glm::length2(dir) != 0.0f)
    {
        dir = glm::normalize(dir);
        if (current_state == State::NORMAL)
        {

            auto velocity = scene::systems::Physics::character_get_velocity(character_controller);
            if (glm::length2(velocity) < glm::pow(current_speed, 2.0f) &&
                scene::systems::Physics::character_get_ground_state(character_controller) == scene::systems::Physics::GroundState::GROUND)
            {
                scene::systems::Physics::character_add_impulse(character_controller, glm::vec3{dir.x, 0.0f, dir.y} * 6400.0f * delta);
            }
        }
        else if (current_state == State::FLYING)
        {
            scene::systems::Physics::character_add_impulse(character_controller, glm::vec3{dir.x, 0.0f, dir.y} * 3200.0f * delta);
        }
    }

    // camera.position = translation;
    spine->rotation *= glm::quat(glm::vec3(glm::radians(-pitch), 0.0f, glm::radians(roll)));
    this->node.rotation = glm::quat(glm::vec3(0.0f, glm::radians(yaw), 0.0f));
}

nlohmann::json FPSCharacterController::to_json() const
{
    nlohmann::json script = Script::to_json();
    script["name"] = "FPSCharacterController";
    return script;
}

void FPSCharacterController::late_update(float delta, const hid::Keyboard &keyboard, const hid::Mouse &mouse)
{
    auto head_global_transform = head_node->global_transform;
    glm::vec3 scale;
    glm::quat rotation;
    glm::vec3 translation;
    glm::vec3 skew;
    glm::vec4 perspective;
    glm::decompose(head_global_transform, scale, rotation, translation, skew, perspective);

    camera.rotation.x = pitch;
    camera.rotation.y = yaw;
    camera.rotation.z = -roll;
    camera.position = translation;
}

void FPSCharacterController::shutdown()
{
}
