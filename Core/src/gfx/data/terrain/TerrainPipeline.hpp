#pragma once

#include <vulkan/vulkan.h>

namespace gage::gfx
{
    class Graphics;
}

namespace gage::gfx::data::terrain
{
    class TerrainPipeline
    {
    public:
        TerrainPipeline(Graphics& gfx);
        ~TerrainPipeline();

        void bind(VkCommandBuffer cmd) const;
        void bind_depth(VkCommandBuffer cmd) const;

        VkDescriptorSet allocate_descriptor_set(size_t size_in_bytes, VkBuffer buffer, 
            VkImageView image_view, VkSampler sampler) const;
        void free_descriptor_set(VkDescriptorSet set) const;

        VkPipelineLayout get_layout() const;
        VkPipelineLayout get_depth_layout() const;
        
        void reset();
    private:
        void create_pipeline();
        void destroy_pipeline();
        void create_pipeline_layout();
        void destroy_pipeline_layout();

        void create_depth_pipeline();
        void destroy_depth_pipeline();

    private:
        static constexpr uint8_t STENCIL_VALUE = 0x02;
        Graphics& gfx;

        VkPipelineLayout pipeline_layout{};
        VkPipeline pipeline{};
        VkDescriptorSetLayout desc_layout{};

        //Shadow map
        VkPipelineLayout depth_pipeline_layout{};
        VkPipeline depth_pipeline{};
    };
}