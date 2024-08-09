#pragma once

#include <cstdint>
#include <memory>
#include <vulkan/vulkan.h>

#include <glm/vec4.hpp>

namespace gage::gfx
{
    class Graphics;
    namespace data
    {
        class CPUBuffer;
        class Image;
    }
}

namespace tinygltf
{
    class Model;
    class Material;
}

namespace gage::scene::systems
{
    class Renderer;
}

namespace gage::scene::data
{
    class ModelMaterial
    {
    public: 
        struct UniformBuffer
        {
            glm::vec4 color{1, 1, 1, 1};
            float specular_intensity{1.0};
            float specular_power{32};
            uint32_t has_albedo{};
            uint32_t has_metalic{};
            uint32_t has_normal{};
        };
    public:
        ModelMaterial(const gfx::Graphics& gfx, const systems::Renderer& renderer, const tinygltf::Model &gltf_model, const tinygltf::Material &gltf_material);
        ~ModelMaterial();

        ModelMaterial(ModelMaterial&&) = default;
        ModelMaterial(const ModelMaterial&) = delete;
        ModelMaterial operator=(const ModelMaterial&) = delete;
    public:
        const systems::Renderer& renderer;
        const gfx::Graphics& gfx;
        
        std::unique_ptr<gfx::data::CPUBuffer> uniform_buffer;
        std::unique_ptr<gfx::data::Image> albedo_image;
        std::unique_ptr<gfx::data::Image> metalic_roughness_image;
        std::unique_ptr<gfx::data::Image> normal_image;
        VkDescriptorSet descriptor_set{};
        UniformBuffer uniform_buffer_data{};
    };
}