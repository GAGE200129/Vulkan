#pragma once

#include "../data/CPUBuffer.hpp"

#include <glm/vec4.hpp>
#include <optional>

namespace tinygltf
{
    class Material;
}

namespace gage::gfx
{
    class Graphics;
}

namespace gage::gfx::draw
{
    class Material
    {
    public:
        Material(Graphics& gfx, const tinygltf::Material& gltf_material);
        ~Material();


        VkDescriptorSet get_desc_set() const;
    private:
        Graphics& gfx;
        struct UniformBuffer
        {
            glm::vec4 color{1, 1, 1, 1};
            float specular_intensity{1.0};
            float specular_power{32};
            float _padding[2];
        } uniform_buffer_data{};

        data::CPUBuffer uniform_buffer;
        VkDescriptorSet descriptor_set{};
    };
}