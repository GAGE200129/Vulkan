#pragma once

#include <glm/vec4.hpp>
#include <string>
#include <vector>
#include <vulkan/vulkan.h>
#include <memory>
#include <thread>
#include <optional>

#include "Mesh.hpp"
#include "Node.hpp"

namespace gage::gfx
{
    class Graphics;
}

namespace gage::gfx::draw
{
    class Model
    {
        friend class Node;
    public:
        Model(Graphics& gfx, const std::string& file_path);
        ~Model();

        void draw(VkCommandBuffer cmd) const;
    private:
        void load_async(const std::string& file_path);
    private:
        Graphics& gfx;
        std::optional<std::thread> thread{};
        bool ready{};

        std::vector<Mesh> meshes{};

        uint32_t root_node{};
        std::vector<Node> nodes{};
    };
}