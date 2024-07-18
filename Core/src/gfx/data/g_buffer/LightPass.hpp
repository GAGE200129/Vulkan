#pragma once

#include <vulkan/vulkan.h>

namespace gage::gfx
{
    class Graphics;
}

namespace gage::gfx::data::g_buffer
{
    class LightPass
    {
        friend class GBuffer;
    public:
        LightPass(Graphics& gfx);
        ~LightPass();

        void reset();
    private:

        void create_image();
        void create_render_pass();
        void create_framebuffer();

        void destroy_image();
        void destroy_render_pass();
        void destroy_framebuffer();
    private:
        Graphics& gfx;

        static constexpr VkFormat COLOR_FORMAT = {VK_FORMAT_B8G8R8A8_UNORM};

        VkRenderPass finalpass_renderpass{};
        VkFramebuffer finalpass_framebuffer{};

        VkImage finalpass_image{};
        VkImageView finalpass_image_view{};
        VkDeviceMemory finalpass_image_memory{};
    };
}