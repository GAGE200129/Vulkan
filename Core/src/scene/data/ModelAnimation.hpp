#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <glm/vec3.hpp>
#include <glm/gtc/quaternion.hpp>


namespace tinygltf
{
    class Model;
    class Animation;
}

namespace gage::scene::data
{
    class ModelAnimation
    {
    public:
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
    public:
        ModelAnimation(const tinygltf::Model& gltf_model, const tinygltf::Animation& gltf_animation);
        ~ModelAnimation();


    public:
        std::string name{};
        double duration{};
        std::vector<PositionChannel> pos_channels{};
        std::vector<ScaleChannel> scale_channels{};
        std::vector<RotationChannel> rotation_channels{};
    };  
}