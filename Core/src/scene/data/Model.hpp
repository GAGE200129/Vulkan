#pragma once


#include "ModelNode.hpp"
#include "ModelMesh.hpp"
#include "ModelMaterial.hpp"

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