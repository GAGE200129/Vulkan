#pragma once

#include "DrawableBase.hpp"

#include <glm/vec3.hpp>

namespace gage::gfx::draw
{
    class Box : public DrawableBase<Box>
    {
    public: 
        Box(Graphics& gfx);
        void update(float dt) override;

        glm::mat4 get_world_transform() const override;
    private:
        float time{};
        glm::vec3 position{};
    };
}