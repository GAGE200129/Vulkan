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
    class CharacterController final : public IComponent
    {

    public:
        CharacterController(SceneGraph& scene, Node& node, phys::Physics& phys);

        void init() final;
        void update(float delta);
        void render_depth(VkCommandBuffer cmd, VkPipelineLayout pipeline_layout) final;
        void render_geometry(VkCommandBuffer cmd, VkPipelineLayout pipeline_layout) final;
        void shutdown() final;

    private:
        phys::Physics& phys;
        JPH::Character* character;
    };
}