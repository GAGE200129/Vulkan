#pragma once

#include <glm/vec3.hpp>
#include <glm/gtc/quaternion.hpp>
#include <vector>
#include <vulkan/vulkan.hpp>

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

        void draw(VkCommandBuffer cmd) const;

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