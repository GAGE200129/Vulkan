#pragma once

#include <Vulkan/VulkanEngine.hpp>

namespace EngineConstants
{
    constexpr unsigned int DISPLAY_WIDTH = 1600;
    constexpr unsigned int DISPLAY_HEIGHT = 600;
    constexpr const char*  DISPLAY_TITLE = "EnGAGE";
    constexpr double TICK_TIME = 1.0 / 60.0;
    constexpr size_t TILE_SET_SIZE = 64;
    constexpr size_t ANIMATION_MAX_BONES = 100;
    constexpr vk::Format VULKAN_SURFACE_FORMAT = vk::Format::eB8G8R8A8Srgb;
    constexpr vk::ColorSpaceKHR VULKAN_COLOR_SPACE = vk::ColorSpaceKHR::eSrgbNonlinear;
    constexpr vk::PresentModeKHR VULKAN_PRESENT_MODE = vk::PresentModeKHR::eFifo;

    constexpr const char* VULKAN_EXTENSIONS[] = 
    {
        "VK_EXT_debug_utils",
        "VK_KHR_surface",
        "VK_KHR_xcb_surface"
    };

    constexpr const char* VULKAN_LAYERS[] = 
    {
        "VK_LAYER_KHRONOS_validation"
    };

    constexpr const char* VULKAN_DEVICE_EXTENSIONS[] = 
    {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };



    #ifdef DEBUG
    constexpr bool DEBUG_ENABLED = true;
    #else
    constexpr bool DEBUG_ENABLED = false;
    #endif
}