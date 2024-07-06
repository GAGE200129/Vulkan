#pragma once

#include "../data/CPUBuffer.hpp"
#include "../data/Image.hpp"

#include <glm/vec4.hpp>
#include <optional>

namespace tinygltf
{
    class Material;
    class Model;
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
        Material(Graphics& gfx, const tinygltf::Model& model, const tinygltf::Material& gltf_material);
        ~Material();


        VkDescriptorSet get_desc_set() const;
    private:
        Graphics& gfx;
        struct UniformBuffer
        {
            glm::vec4 color{1, 1, 1, 1};
            float specular_intensity{1.0};
            float specular_power{32};
            uint32_t has_albedo{};
            uint32_t has_metalic{};
        } uniform_buffer_data{};

        std::optional<data::CPUBuffer> uniform_buffer;
        std::optional<data::Image> albedo_image;
        std::optional<data::Image> metalic_roughness_image;
        VkDescriptorSet descriptor_set{};
    };
}