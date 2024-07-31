#pragma once

#include "../components/MeshRenderer.hpp"
#include "../components/TerrainRenderer.hpp"
#include <vulkan/vulkan.h>
#include <vector>
#include <memory>

namespace gage::scene::systems
{
    class Renderer
    {
    public:
        Renderer(gfx::Graphics& gfx);
        ~Renderer() = default;
        
        void init();
        void render_depth(VkCommandBuffer cmd, VkPipelineLayout pipeline_layout) const;
        void render_depth_terrain(VkCommandBuffer cmd, VkPipelineLayout pipeline_layout) const;
        void render_geometry(VkCommandBuffer cmd, VkPipelineLayout pipeline_layout) const;
        void render_geometry_terrain(VkCommandBuffer cmd, VkPipelineLayout pipeline_layout) const;
        void shutdown();

        void add_pbr_mesh_renderer(std::unique_ptr<components::MeshRenderer> mesh_renderer);
        void add_terrain_renderer(std::shared_ptr<components::TerrainRenderer> terrain_renderer);
    private:
        gfx::Graphics& gfx;
        std::vector<std::unique_ptr<components::MeshRenderer>> mesh_renderers; 
        std::vector<std::shared_ptr<components::TerrainRenderer>> terrain_renderers;
    };
}