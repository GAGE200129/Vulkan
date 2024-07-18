#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "../data/Frustum.hpp"

namespace gage::gfx
{
    class Graphics;
}
namespace gage::gfx::draw
{
    class Model;
    class Node
    {
    public:
        Node(Graphics &gfx, const Model &model,
            const glm::vec3 &position,
            const glm::quat &rotation,
            const glm::vec3 &scale,
            std::vector<uint32_t> children,
            int32_t mesh
        );

        void draw(VkCommandBuffer cmd, VkPipelineLayout layout, const data::Frustum& frustum, glm::mat4x4 accumulated_transform) const;

    private:
        Graphics &gfx;
        const Model &model;
        glm::vec3 position{};
        glm::quat rotation{};
        glm::vec3 scale{};
        std::vector<uint32_t> children{};
        int32_t mesh{};
    };
}