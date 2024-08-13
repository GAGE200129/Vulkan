#pragma once

#include <vulkan/vulkan.h>

namespace gage::gfx
{
    class  Graphics;
}

namespace gage::gfx::data::g_buffer
{
    class DebugPass
    {
    public:
        DebugPass(const Graphics& gfx);
    private:
        const Graphics& gfx;

        static constexpr VkFormat FORMAT = {VK_FORMAT_R8G8B8_UNORM};

        VkImage image{};
        VkImageView image_view{};
        VkDeviceMemory image_memory{};

        VkRenderPass render_pass{};
        VkFramebuffer framebuffer;
    };
}