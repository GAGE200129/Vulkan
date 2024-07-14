#pragma once

#include "../Graphics.hpp"

namespace gage::gfx::data
{
    class PostprocessPipeline
    {
    public:
        PostprocessPipeline(Graphics& gfx);
        ~PostprocessPipeline();

        
    private:
        static constexpr VkFormat COLOR_FORMAT = {VK_FORMAT_B8G8R8A8_UNORM};

        Graphics& gfx;
        VkPipelineLayout pipeline_layout{};
        VkPipeline pipeline{};

        VkRenderPass render_pass{};
        VkFramebuffer frame_buffer{};
        VkDeviceMemory color_image_memory{};
        VkImage color_image{};
        VkImageView color_image_view{};
        VkSampler color_sampler{};
    };
}