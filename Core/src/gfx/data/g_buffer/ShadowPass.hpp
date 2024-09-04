#pragma once

#include <vulkan/vulkan.h>

namespace gage::gfx
{
    class Graphics;
}

namespace gage::gfx::data::g_buffer
{
    class ShadowPass
    {
        friend class GBuffer;
    public:
        ShadowPass(const Graphics& gfx);
        ~ShadowPass();

        void reset();
    private:

        void create_image();
        void create_render_pass();
        void create_framebuffer();
        void destroy_image();
        void destroy_render_pass();
        void destroy_framebuffer();
    private:
        const Graphics& gfx;

        static constexpr VkFormat SHADOW_FORMAT = {VK_FORMAT_D32_SFLOAT};
    public:
        VkRenderPass shadowpass_renderpass{};
        VkFramebuffer shadowpass_framebuffer{};

        VkImage shadowpass_image{};
        VkImageView shadowpass_image_view{};
        VkDeviceMemory shadowpass_image_memory{};
    };
}