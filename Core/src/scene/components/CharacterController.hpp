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

namespace gage::scene::components
{
    class CharacterController : public IComponent
    {
    protected:
        enum class GroundState
        {
            GROUND,
            AIR
        };
    public:
        CharacterController(SceneGraph& scene, Node& node, phys::Physics& phys);

        void init() override;
        void update(float delta, const hid::Keyboard& keyboard, const hid::Mouse& mouse) override;
        inline void render_depth(VkCommandBuffer, VkPipelineLayout) final {}
        inline void render_geometry(VkCommandBuffer, VkPipelineLayout) final {}
        void shutdown() override;

        void render_imgui() override;

        void add_velocity(const glm::vec3& vel);
        glm::vec3 get_velocity() const;
        GroundState get_ground_state() const;
    protected:
        phys::Physics& phys;
        JPH::Character* character;

    };
}