#pragma once

#include "IComponent.hpp"

namespace JPH
{
    class Character;
}

namespace gage::phys
{
    class Physics;
}

namespace gage::scene::components
{
    class CharacterController : public IComponent
    {

    public:
        CharacterController(SceneGraph& scene, Node& node, phys::Physics& phys);

        void init() override;
        void update(float delta) override;
        inline void render_depth(VkCommandBuffer, VkPipelineLayout) final {}
        inline void render_geometry(VkCommandBuffer, VkPipelineLayout) final {}
        void shutdown() override;

        void render_imgui() override;
    private:
        phys::Physics& phys;
        JPH::Character* character;

    };
}