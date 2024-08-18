#pragma once

#include "../components/Map.hpp"

#include <cstdint>
#include <vector>
#include <memory>
#include <unordered_map>

#include <vulkan/vulkan.h>
#include <glm/mat4x4.hpp>
#include <Core/ThirdParty/tiny_gltf.h>

namespace gage::gfx
{
    class Graphics;
    namespace data
    {
        class GPUBuffer;
        class Image;
    }
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
            StaticModelData() = default;
            ~StaticModelData() = default;

            StaticModelData(const StaticModelData&) = delete;
            StaticModelData operator=(const StaticModelData&) = delete;
            StaticModelData& operator=(StaticModelData&&) = default;
            StaticModelData(StaticModelData&&) = default;
        public:
            std::unique_ptr<gfx::data::GPUBuffer> vertex_buffer{};
            std::unique_ptr<gfx::data::GPUBuffer> index_buffer{};
            uint32_t vertex_count{0};  
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