#pragma once

#include "DrawableBase.hpp"

#include "../bind/UniformBuffer.hpp"

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

namespace gage::gfx::draw
{
    class Box : public DrawableBase<Box>
    {
    public: 
        Box(Graphics& gfx);
        void update(float dt) override;

        glm::mat4 get_world_transform() const override;
    private:
        bind::UniformBuffer* p_uniform_buffer{};
        struct Material
        {
            glm::vec4 color{};
        } material{};
        
        float pitch_speed{}, yaw_speed{}, roll_speed{};
        float pitch{}, yaw{}, roll{};

        float pitch_orbit_speed{}, yaw_orbit_speed{}, roll_orbit_speed{};
        float pitch_orbit{}, yaw_orbit{}, roll_orbit{};
        float radius{};
    };
}