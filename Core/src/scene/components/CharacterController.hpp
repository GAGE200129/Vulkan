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
        CharacterController(SceneGraph& scene, Node& node);
        ~CharacterController();

        nlohmann::json to_json() const final;
        void render_imgui() override;
        inline const char* get_name() const override { return "CharacterController"; };

    public:
        std::unique_ptr<JPH::Character> character;

    };
}