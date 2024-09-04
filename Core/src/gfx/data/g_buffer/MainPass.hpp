#pragma once

#include <vulkan/vulkan.h>

namespace gage::gfx
{
    class Graphics;
}

namespace gage::gfx::data::g_buffer
{
    class MainPass
    {
        friend class GBuffer;
    public:
        MainPass(const Graphics& gfx);
        ~MainPass();

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

        static constexpr VkFormat NORMAL_FORMAT = {VK_FORMAT_R16G16B16_SFLOAT};
        static constexpr VkFormat ALBEDO_FORMAT = {VK_FORMAT_R16G16B16_SFLOAT};
        static constexpr VkFormat METALIC_ROUGHENSS_FORMAT = {VK_FORMAT_R16G16B16_SFLOAT};
        static constexpr VkFormat DEPTH_FORMAT = {VK_FORMAT_D24_UNORM_S8_UINT};
        
    public:
        VkRenderPass render_pass{};
        VkFramebuffer framebuffer{};

        VkDeviceMemory depth_image_memory{};
        VkImage depth_image{};
        VkImageView depth_image_view{};
        VkImageView stencil_image_view{};

        VkDeviceMemory normal_memory{};
        VkImage normal{};
        VkImageView normal_view{};

        VkDeviceMemory albedo_memory{};
        VkImage albedo{};
        VkImageView albedo_view{};

        VkDeviceMemory mr_memory{};
        VkImage mr{};
        VkImageView mr_view{};
    };
}