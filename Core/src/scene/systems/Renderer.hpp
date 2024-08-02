#pragma once

#include "../components/MeshRenderer.hpp"
#include "../components/TerrainRenderer.hpp"
#include <Core/src/gfx/data/GPUBuffer.hpp>
#include <Core/src/gfx/data/CPUBuffer.hpp>
#include <Core/src/gfx/data/Image.hpp>

#include <vector>
#include <memory>

namespace gage::gfx::data
{
    class Camera;
}

namespace gage::scene::systems
{
    class Renderer
    {
    private:
        struct TerrainRenderer
        {
            //Additional datas
            std::unique_ptr<gfx::data::GPUBuffer> vertex_buffer{};
            std::unique_ptr<gfx::data::GPUBuffer> index_buffer{};
            std::unique_ptr<gfx::data::Image> image{};
            std::unique_ptr<gfx::data::CPUBuffer> uniform_buffer{};
            struct UniformBuffer
            {
                float min_height{};
                float max_height{};
                float uv_scale{};
            } uniform_buffer_data;
            VkDescriptorSet descriptor{};

            //Original data
            std::shared_ptr<components::TerrainRenderer> terrain_renderer;
        };

    public:
        Renderer(gfx::Graphics &gfx, const gfx::data::Camera& camera);
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
        gfx::Graphics &gfx;
        const gfx::data::Camera& camera;
        std::vector<std::unique_ptr<components::MeshRenderer>> mesh_renderers;
        std::vector<TerrainRenderer> terrain_renderers;
    };
}