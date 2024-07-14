#pragma once

#include <thread>
#include <mutex>
#include <optional>

#include "Mesh.hpp"
#include "Node.hpp"
#include "Material.hpp"

#include "../data/Frustum.hpp"

namespace gage::gfx
{
    class Graphics;
}

namespace gage::gfx::draw
{
    class Model
    {
        friend class Node;
        friend class Mesh;
    public:
        enum class Mode
        {
            Binary,
            ASCII
        };
        Model(Graphics& gfx, const std::string& file_path, Mode mode = Mode::Binary);
        ~Model();

        void draw(VkCommandBuffer cmd, VkPipelineLayout layout, data::Frustum frustum = {}) const;
    private:
        void load_async(const std::string& file_path);
    private:
        Graphics& gfx;
        std::optional<std::thread> thread{};
        bool ready{};

        Mode mode{};

        std::vector<std::unique_ptr<Mesh>> meshes{};

        uint32_t root_node{};
        std::vector<std::unique_ptr<Node>>  nodes{};

        std::vector<std::unique_ptr<Material>>  materials{};
    };
}