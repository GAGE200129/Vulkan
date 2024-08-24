#pragma once

#include <vulkan/vulkan.h>
#include <cstdint>

#include "Device.hpp"

namespace gage::gfx::data
{
    class Instance;
    class Device;
    class Swapchain
    {
    public:
        Swapchain(const Instance& instance, const Device& device, VkExtent2D draw_extent);
        ~Swapchain();
        VkSwapchainKHR get() const;

        void reset();

        VkImage at(size_t i) const;
    private:
        static constexpr VkFormat COLOR_FORMAT = VK_FORMAT_B8G8R8A8_UNORM; 
        static constexpr VkColorSpaceKHR COLOR_SPACE = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR; 
        static constexpr VkPresentModeKHR PRESENT_MODE = VK_PRESENT_MODE_IMMEDIATE_KHR;
        const Instance& instance;
        const Device& device;
        VkExtent2D draw_extent;
        std::vector<VkImage> images{};
        std::vector<VkImageView> image_views{};
    public:
        uint32_t image_index{};
        VkSwapchainKHR swapchain{};
       
        

    };  
}