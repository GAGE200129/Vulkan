#pragma once

#include "DrawableBase.hpp"

#include "../bind/UniformBuffer.hpp"

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

namespace gage::gfx::draw
{
    class StaticModel : public DrawableBase<StaticModel>
    {
    public: 
        StaticModel(Graphics& gfx, std::string model_path);

        glm::mat4 get_world_transform() const override;
        void update(float) override {};
    private:
        struct Material
        {
            glm::vec4 color{1, 1, 1, 1};
            float specular_intensity{1};
            float specular_power{32}; float _padding[2]{};
        } material{};
    };
}