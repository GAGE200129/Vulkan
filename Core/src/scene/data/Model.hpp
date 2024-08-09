#pragma once


#include "ModelNode.hpp"
#include "ModelMesh.hpp"

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

        std::unique_ptr<gfx::data::CPUBuffer> uniform_buffer;
        std::unique_ptr<gfx::data::Image> albedo_image;
        std::unique_ptr<gfx::data::Image> metalic_roughness_image;
        std::unique_ptr<gfx::data::Image> normal_image;
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
        std::vector<ModelMesh> meshes{};
        std::vector<ModelMaterial> materials{};
        std::vector<ModelAnimation> animations{};
        std::vector<ModelSkin> skins{};
    };
}