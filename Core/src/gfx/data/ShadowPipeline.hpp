#pragma once

#include "../Graphics.hpp"

namespace gage::gfx::data
{
    class Camera;
    class ShadowPipeline
    {
        friend class DefaultPipeline;
    public:
        ShadowPipeline(Graphics& gfx);
        ~ShadowPipeline();

        ShadowPipeline(const ShadowPipeline&) = delete;
        ShadowPipeline operator=(const ShadowPipeline&) = delete;

        void begin(VkCommandBuffer cmd);
        void end(VkCommandBuffer cmd);

        void reset();
    private:
        void create_pipeline_layout();
        void destroy_pipeline_layout();

        void create_pipeline();
        void destroy_pipeline();

        void create_render_pass();
        void destroy_render_pass();

        void create_depth_image();
        void destroy_depth_image();

        void create_framebuffer();
        void destroy_framebuffer();

        void link_depth_to_global_set();
    private:
        static constexpr VkFormat DEPTH_FORMAT = {VK_FORMAT_D32_SFLOAT};

        Graphics& gfx;
        bool shadow_map_need_resize{};

        VkPipelineLayout pipeline_layout{};
        VkPipeline pipeline{};

        VkRenderPass render_pass{};
        VkFramebuffer frame_buffer{};
        VkDeviceMemory depth_image_memory{};
        VkImage depth_image{};
        VkImageView depth_image_view{};
        VkSampler depth_sampler{};

    };
}