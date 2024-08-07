#pragma once


#include <vulkan/vulkan.h>

namespace gage::gfx
{
    class Graphics;
}

namespace gage::gfx::data
{
    class Sky
    {
    public:
        Sky(const gfx::Graphics& gfx);
        ~Sky();

         void process(VkCommandBuffer cmd) const;

        void reset();
        void link_desc_to_g_buffer();
    private:
        void create_pipeline();
    private:
        const gfx::Graphics& gfx;
        VkDescriptorSetLayout desc_layout{};
        VkPipelineLayout pipeline_layout{};
        VkPipeline pipeline{};
        VkDescriptorSet desc{};
    };
}