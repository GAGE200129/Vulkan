#pragma once

#include <vulkan/vulkan.h>
#include <vector>

namespace gage::gfx
{
    class Graphics;
}

namespace gage::gfx::data
{
    class Swapchain
    {
    public:
        Swapchain(Graphics& gfx);
        ~Swapchain();

        std::tuple<uint32_t, uint32_t> get_color_image_external() const;
        std::tuple<uint32_t, uint32_t> get_depth_image_external() const;

        VkImage get_color_image_handle() const;
        VkImage get_depth_image_handle() const;
        VkImageView get_color_image_view() const;
        VkImageView get_depth_image_view() const;

        VkFormat get_image_format() const;
        VkFormat get_depth_format() const;

        VkSwapchainKHR get() const;


        VkImage at(size_t i) const;
    private:
        const Graphics& gfx;
        VkSwapchainKHR swapchain{};
        VkFormat image_format{VK_FORMAT_B8G8R8A8_UNORM};
        VkColorSpaceKHR image_color_space{VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
        VkFormat depth_format{VK_FORMAT_D32_SFLOAT};
        VkPresentModeKHR present_mode{VK_PRESENT_MODE_IMMEDIATE_KHR};
        std::vector<VkImage> images{};
        std::vector<VkImageView> image_views{};

        VkDeviceMemory depth_image_memory{};
        VkDeviceSize depth_image_memory_size{};
        VkImage depth_image{};
        VkImageView depth_image_view{};

        VkDeviceMemory color_image_memory{};
        VkDeviceSize color_image_memory_size{};
        VkImage color_image{};
        VkImageView color_image_view{};
    };  
}