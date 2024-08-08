#pragma once

#include <glm/vec3.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/mat4x4.hpp>
#include <cstdint>
#include <string>
#include <vector>


namespace tinygltf
{
    class Node;
}

namespace gage::scene::data
{
    class ModelNode
    { 
    public:
        ModelNode(const tinygltf::Node& node, uint32_t node_index);
        ~ModelNode() = default;
    public:
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
}