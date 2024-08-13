#pragma once

#include <vulkan/vulkan.h>

namespace gage::gfx
{
    class Graphics;
}

namespace gage::gfx::data
{
    class DebugRenderer
    {
    public:
        DebugRenderer(const Graphics& gfx);
        ~DebugRenderer();


        void process(VkCommandBuffer cmd) const;
    private:
        void create_pipeline();
        void link_desc_to_g_buffer();
    private:
        const Graphics& gfx;
    public:
        VkDescriptorSetLayout desc_layout{};
        VkDescriptorSet desc{};
        VkPipelineLayout pipeline_layout{};
        VkPipeline pipeline{};
    };
}