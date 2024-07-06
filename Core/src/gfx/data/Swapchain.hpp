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
        VkSwapchainKHR get() const;

        VkImage at(size_t i) const;
    private:
        static constexpr VkFormat COLOR_FORMAT = VK_FORMAT_B8G8R8A8_UNORM; 
        static constexpr VkColorSpaceKHR COLOR_SPACE = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR; 
        static constexpr VkPresentModeKHR PRESENT_MODE = VK_PRESENT_MODE_IMMEDIATE_KHR;

        const Graphics& gfx;
        VkSwapchainKHR swapchain{};
       
        std::vector<VkImage> images{};
        std::vector<VkImageView> image_views{};

    };  
}