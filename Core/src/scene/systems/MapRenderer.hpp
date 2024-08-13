#pragma once

#include "../components/Map.hpp"

#include <cstdint>
#include <vector>
#include <memory>

#include <vulkan/vulkan.h>

namespace gage::gfx
{
    class Graphics;
    namespace data
    {
        class GPUBuffer;
    }
}

namespace gage::scene::systems
{
    class MapRenderer
    {
    public:
        struct Map
        {
            //Additional datas
            std::unique_ptr<gfx::data::GPUBuffer> vertex_buffer{};
            uint32_t vertex_count{};

            //Original data
            std::shared_ptr<components::Map> map;
        };

    public:
        MapRenderer(const gfx::Graphics& gfx);
        ~MapRenderer();

        void init();
        void shutdown();

        void add_map(std::shared_ptr<components::Map> map);


        void render(VkCommandBuffer cmd) const;
        void render_depth(VkCommandBuffer cmd) const;
    private:
        void create_pipeline();
        void create_depth_pipeline();
    private:
        const gfx::Graphics& gfx;

    public:
        static constexpr uint8_t STENCIL_VALUE = 0x03;

        VkPipelineLayout pipeline_layout{};
        VkPipeline pipeline{};
        VkDescriptorSetLayout desc_layout{};

        //Shadow map
        VkPipelineLayout depth_pipeline_layout{};
        VkPipeline depth_pipeline{};

        std::vector<Map> maps;
    };
}