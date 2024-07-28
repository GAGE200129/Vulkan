#pragma once

#include "../components/MeshRenderer.hpp"
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
        void render_geometry(VkCommandBuffer cmd, VkPipelineLayout pipeline_layout) const;
        void shutdown();

        void add_pbr_mesh_renderer(std::unique_ptr<components::MeshRenderer> mesh_renderer);
    private:
        gfx::Graphics& gfx;
        std::vector<std::unique_ptr<components::MeshRenderer>> mesh_renderers; 
    };
}