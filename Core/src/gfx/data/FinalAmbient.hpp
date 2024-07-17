#pragma once

#include <vulkan/vulkan.h>

namespace gage::gfx
{
    class Graphics;
}


namespace gage::gfx::data
{
    class FinalAmbient
    {   
    public:
        FinalAmbient(Graphics& gfx);
        ~FinalAmbient();

        void process(VkCommandBuffer cmd) const;

        void reset();

        VkPipelineLayout get_layout() const;
    private:
        void link_desc_to_g_buffer();
    private:
        Graphics& gfx;
        VkSampler default_sampler{};
        VkDescriptorSetLayout desc_layout{};
        VkDescriptorSet desc{};
        VkPipelineLayout pipeline_layout{};
        VkPipeline pipeline{};
    };
}