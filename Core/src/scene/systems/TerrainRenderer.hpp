#pragma once

#include <cstdint>
#include <vulkan/vulkan.h>
#include <memory>
#include <Core/src/gfx/data/GPUBuffer.hpp>
#include <Core/src/gfx/data/CPUBuffer.hpp>
#include <Core/src/gfx/data/Image.hpp>

#include "../components/Terrain.hpp"

namespace gage::gfx
{
    class Graphics;

    namespace data
    {
        class Camera;
    }
}

namespace gage::scene
{
    class SceneGraph;
}


namespace gage::scene::systems
{
    class TerrainRenderer
    {
        friend class scene::SceneGraph;
    public:
        struct Terrain
        {
            //Additional datas
            std::unique_ptr<gfx::data::GPUBuffer> vertex_buffer{};
            std::unique_ptr<gfx::data::GPUBuffer> index_buffer{};
            std::unique_ptr<gfx::data::Image> image{};
            std::unique_ptr<gfx::data::CPUBuffer> uniform_buffer{};
            VkDescriptorSet descriptor{};

            //Original data
            std::shared_ptr<components::Terrain> terrain;
        };
    public:
        TerrainRenderer(const  gfx::Graphics &gfx, const gfx::data::Camera& camera);
        ~TerrainRenderer();

        void init();
        void shutdown();

        void add_terrain(std::shared_ptr<components::Terrain> terrain);


        void render(VkCommandBuffer cmd) const;
        void render_depth(VkCommandBuffer cmd) const;

        VkDescriptorSet allocate_descriptor_set(VkImageView image_view, VkSampler sampler) const;
    private:
        void create_pipeline();
        void create_depth_pipeline();
    private:
        static constexpr uint8_t STENCIL_VALUE = 0x02;
        const gfx::Graphics& gfx;
        const gfx::data::Camera& camera;

        VkPipelineLayout pipeline_layout{};
        VkPipeline pipeline{};
        VkDescriptorSetLayout desc_layout{};

        //Shadow map
        VkPipelineLayout depth_pipeline_layout{};
        VkPipeline depth_pipeline{};


        //Terrains
        std::vector<Terrain> terrains;
    };
}