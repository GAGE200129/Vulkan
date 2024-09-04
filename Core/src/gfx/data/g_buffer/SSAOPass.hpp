#pragma once

#include <vulkan/vulkan.h>

namespace gage::gfx
{
    class Graphics;

}

namespace gage::gfx::data::g_buffer
{
    
    class SSAOPass
    {
        friend class GBuffer;
    public:
        SSAOPass(const Graphics& gfx);
        ~SSAOPass();

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

        static constexpr VkFormat FORMAT = {VK_FORMAT_R32_SFLOAT};
    public:
        VkImage image{};
        VkImageView image_view{};
        VkDeviceMemory image_memory{};

        VkRenderPass render_pass{};
        VkFramebuffer framebuffer;
    };
}