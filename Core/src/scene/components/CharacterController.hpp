#pragma once

#include "IComponent.hpp"

#include <glm/vec3.hpp>

namespace JPH
{
    class Character;
}

namespace gage::phys
{
    class Physics;
}

namespace gage::scene::systems
{
    class Physics;
}

namespace gage::scene::components
{
    class CharacterController : public IComponent
    {
        friend class systems::Physics;

    public:
        CharacterController(SceneGraph& scene, Node& node, phys::Physics& phys);


        void render_imgui() override;
        inline const char* get_name() const override { return "CharacterController"; };

    protected:
        phys::Physics& phys;
        JPH::Character* character;

    };
}