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
        uint32_t bone_id{};
        glm::vec3 position{0, 0, 0};
        glm::vec3 scale{1, 1, 1};
        glm::quat rotation{1, 0, 0, 0};
        glm::mat4x4 inverse_bind_transform{1.0f};
        bool has_skin{false};
        uint32_t skin_index{};
        
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
            std::unique_ptr<gfx::data::GPUBuffer> bone_id_buffer;
            std::unique_ptr<gfx::data::GPUBuffer> bone_weight_buffer;
            int32_t material_index;
            bool has_skin{};
        };
        std::vector<MeshSection> sections{};
        std::vector<uint32_t> skin_joints;
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

    struct ModelAnimation
    {
        struct PositionChannel
        {
            uint32_t target_node{};
            std::vector<float> time_points{};
            std::vector<glm::vec3> positions{};
        };

        struct ScaleChannel
        {
            uint32_t target_node{};
            std::vector<float> time_points{};
            std::vector<glm::vec3> scales{};
        };

        struct RotationChannel
        {
            uint32_t target_node{};
            std::vector<float> time_points{};
            std::vector<glm::quat> rotations{};
        };

        std::string name{};
        double duration{};
        std::vector<PositionChannel> pos_channels{};
        std::vector<ScaleChannel> scale_channels{};
        std::vector<RotationChannel> rotation_channels{};
        
    };

    struct ModelSkin
    {
        std::vector<uint32_t> joints;
    };

    struct Model
    {
        std::string name{};
        std::vector<ModelNode> nodes{};
        uint32_t root_node{};
        std::vector<std::unique_ptr<ModelMesh>> meshes{};
        std::vector<std::unique_ptr<ModelMaterial>> materials{};
        std::vector<std::unique_ptr<ModelAnimation>> animations{};
        std::vector<std::unique_ptr<ModelSkin>> skins{};
    };
}