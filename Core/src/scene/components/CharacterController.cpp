#include <pch.hpp>
#include "CharacterController.hpp"

#include <Core/src/phys/Physics.hpp>
#include <Core/src/gfx/data/Camera.hpp>
#include <imgui/imgui.h>

#include <Jolt/Jolt.h>
#include <Jolt/Physics/Character/Character.h>

#include "../Node.hpp"

namespace gage::scene::components
{
    CharacterController::CharacterController(SceneGraph &scene, Node &node) : IComponent(scene, node)
    {
    }

    CharacterController::~CharacterController()
    {

    }

    void CharacterController::render_imgui()
    {
    }

    nlohmann::json CharacterController::to_json() const
    {
        return {{"type", get_name()}};
    }

}