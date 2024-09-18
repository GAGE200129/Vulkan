#pragma once

#include "../components/Map.hpp"

#include <cstdint>
#include <vector>
#include <memory>
#include <unordered_map>

#include <vulkan/vulkan.h>
#include <glm/mat4x4.hpp>

#include <Core/src/gfx/data/GPUBuffer.hpp>
#include <Core/src/gfx/data/Image.hpp>

namespace gage::gfx
{
    class Graphics;
}

namespace gage::scene::systems
{
    class MapRenderer
    {
    public:
        struct MapVertex
        {
            glm::vec3 position, normal;
            glm::vec2 uv;
        };

        class GeometryData
        {
        public:
            GeometryData() = default;
            ~GeometryData() = default;

            GeometryData(const GeometryData&) = delete;
            GeometryData operator=(const GeometryData&) = delete;
            GeometryData& operator=(GeometryData&&) = default;
            GeometryData(GeometryData&&) = default;
        public:
            std::unique_ptr<gfx::data::Image> image{};
            uint32_t image_width{}, image_height{};
            VkDescriptorSet image_descriptor_set{};
            
            std::vector<MapVertex> vertices{};
            std::unique_ptr<gfx::data::GPUBuffer> vertex_buffer{};
            uint32_t vertex_count{0};  
        };

        class StaticModelData
        {
        public:
            StaticModelData(gfx::data::GPUBuffer vertex_buffer, gfx::data::GPUBuffer index_buffer, uint32_t vertex_count) :
                vertex_buffer(std::move(vertex_buffer)),
                index_buffer(std::move(index_buffer)),
                vertex_count(vertex_count)
            {}
            ~StaticModelData() = default;

            StaticModelData(const StaticModelData&) = delete;
            StaticModelData operator=(const StaticModelData&) = delete;
            StaticModelData& operator=(StaticModelData&&) = default;
            StaticModelData(StaticModelData&&) = default;
        public:
            gfx::data::GPUBuffer vertex_buffer;
            gfx::data::GPUBuffer index_buffer;
            uint32_t vertex_count;  
        };


    public:
        MapRenderer(const gfx::Graphics &gfx);
        ~MapRenderer();

        void init();
        void shutdown();

        void add_map(std::shared_ptr<components::Map> map);

        void render(VkCommandBuffer cmd) const;
        void render_depth(VkCommandBuffer cmd) const;

    private:
        void create_pipeline();
        void create_depth_pipeline();
        void process_aabb_wall(const components::AABBWall& aabb_wall);
        void create_new_static_model(const std::string& model_path);

    private:
        const gfx::Graphics &gfx;

    public:
        static constexpr uint8_t STENCIL_VALUE = 0x03;

        VkPipelineLayout pipeline_layout{};
        VkPipeline pipeline{};
        VkDescriptorSetLayout desc_layout{};

        // Shadow map
        VkPipelineLayout depth_pipeline_layout{};
        VkPipeline depth_pipeline{};

        std::vector<std::shared_ptr<components::Map>> maps;
        std::unordered_map<std::string, GeometryData> image_path_to_geometry_data_map{};
        std::unordered_map<std::string, StaticModelData> model_path_to_model_map{};
    };
}