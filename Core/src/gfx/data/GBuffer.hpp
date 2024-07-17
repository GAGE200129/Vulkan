#pragma once

#include <vulkan/vulkan.h>


namespace gage::gfx
{
    class Graphics;
}

namespace gage::gfx::data
{
    class GBuffer
    {
    public:
        GBuffer(Graphics& gfx);
        ~GBuffer();

        void begin(VkCommandBuffer cmd) const;
        void begin_finalpass(VkCommandBuffer cmd) const;
        void end(VkCommandBuffer cmd) const;

        VkRenderPass get_render_pass() const;
        VkRenderPass get_finalpass_render_pass() const;

        VkImageView get_position_view() const;
        VkImageView get_normal_view() const;
        VkImageView get_albedo_view() const;
        VkImage get_final_color() const;

        void reset();

    private:
        void create_images();
        void create_framebuffers();

    private:
        static constexpr VkFormat POSITION_FORMAT = {VK_FORMAT_R16G16B16_SFLOAT};
        static constexpr VkFormat NORMAL_FORMAT = {VK_FORMAT_R16G16B16_SFLOAT};
        static constexpr VkFormat ALBEDO_FORMAT = {VK_FORMAT_R16G16B16_SFLOAT};
        static constexpr VkFormat DEPTH_FORMAT = {VK_FORMAT_D32_SFLOAT};
        static constexpr VkFormat COLOR_FORMAT = {VK_FORMAT_B8G8R8A8_UNORM};
        
        Graphics& gfx;

        VkRenderPass render_pass{};
        VkFramebuffer framebuffer{};
        VkRenderPass finalpass_renderpass{};
        VkFramebuffer finalpass_framebuffer{};



        VkImage finalpass_image{};
        VkImageView finalpass_image_view{};
        VkDeviceMemory finalpass_image_memory;

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
    };
}