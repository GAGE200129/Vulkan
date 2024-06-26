#pragma once

#include "Drawable.hpp"

#include <glm/vec3.hpp>

namespace gage::gfx::draw
{
    class Box : public Drawable
    {
    public: 
        Box(Graphics& gfx);
        void update(float dt) override;

        glm::mat4 get_world_transform() const override;
    private:
        glm::vec3 position{};
    };
}