#include <pch.hpp>
#include "CharacterController.hpp"

#include <Core/src/phys/Physics.hpp>
#include <Core/src/gfx/data/Camera.hpp>
#include <imgui/imgui.h>



#include "../Node.hpp"

namespace gage::scene::components
{
    CharacterController::CharacterController(SceneGraph &scene, Node &node, phys::Physics &phys) : IComponent(scene, node),
                                                                                                   phys(phys)
    {
        
    }

    void CharacterController::render_imgui()
    {

    }

   
}