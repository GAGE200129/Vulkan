#pragma once

#include <Core/src/gfx/data/GPUBuffer.hpp>
#include <Core/src/gfx/data/CPUBuffer.hpp>
#include <Core/src/gfx/data/Image.hpp>

#include <vector>
#include <memory>
#include <string>
#include <optional>

#include <glm/vec3.hpp>
#include <glm/gtc/quaternion.hpp>

namespace gage::scene::data
{
    struct ModelNode
    {
        std::string name{};
        glm::vec3 position{0, 0, 0};
        glm::vec3 scale{1, 1, 1};
        glm::quat rotation{1, 0, 0, 0};
        bool has_mesh{false};
        uint32_t mesh_index{};
        std::vector<uint32_t> children;
    };

    struct ModelMesh
    {
        struct MeshSection
        {
            uint32_t vertex_count;
            std::unique_ptr<gfx::data::GPUBuffer> index_buffer; 
            std::unique_ptr<gfx::data::GPUBuffer> position_buffer; 
            std::unique_ptr<gfx::data::GPUBuffer> normal_buffer; 
            std::unique_ptr<gfx::data::GPUBuffer> texcoord_buffer; 
            int32_t material_index;
        };
        std::vector<MeshSection> sections{};
    };

    struct ModelMaterial
    {
        struct UniformBuffer
        {
            glm::vec4 color{1, 1, 1, 1};
            float specular_intensity{1.0};
            float specular_power{32};
            uint32_t has_albedo{};
            uint32_t has_metalic{};
            uint32_t has_normal{};
        } uniform_buffer_data{};

        std::optional<gfx::data::CPUBuffer> uniform_buffer;
        std::optional<gfx::data::Image> albedo_image;
        std::optional<gfx::data::Image> metalic_roughness_image;
        std::optional<gfx::data::Image> normal_image;
        VkDescriptorSet descriptor_set{};
    };
    struct Model
    {
        std::string name{};
        std::vector<ModelNode> nodes{};
        uint32_t root_node{};
        std::vector<std::unique_ptr<ModelMesh>> meshes{};
        std::vector<std::unique_ptr<ModelMaterial>> materials{};
    };
}