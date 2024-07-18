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
        MainPass(Graphics& gfx);
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
        Graphics& gfx;

        static constexpr VkFormat POSITION_FORMAT = {VK_FORMAT_R16G16B16_SFLOAT};
        static constexpr VkFormat NORMAL_FORMAT = {VK_FORMAT_R16G16B16_SFLOAT};
        static constexpr VkFormat ALBEDO_FORMAT = {VK_FORMAT_R16G16B16_SFLOAT};
        static constexpr VkFormat METALIC_ROUGHENSS_FORMAT = {VK_FORMAT_R16G16B16_SFLOAT};
        static constexpr VkFormat DEPTH_FORMAT = {VK_FORMAT_D32_SFLOAT};

        VkRenderPass render_pass{};
        VkFramebuffer framebuffer{};

        VkDeviceMemory depth_image_memory{};
        VkImage depth_image{};
        VkImageView depth_image_view{};

        VkDeviceMemory position_memory{};
        VkImage position{};
        VkImageView position_view{};

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