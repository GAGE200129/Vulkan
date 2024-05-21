#include "pch.hpp"
#include "VulkanEngine.hpp"

bool VulkanEngine::initSurface()
{
    VkSurfaceKHR surface;

    unsigned int count;
    const char** extensions = glfwGetRequiredInstanceExtensions(&count);
    for(unsigned int i = 0; i < count; i++)
        spdlog::info("Glfw required extension: {}", extensions[i]);

    if (glfwCreateWindowSurface(gData.instance, gData.window, nullptr, &surface) != VK_SUCCESS)
    {
        spdlog::critical("Failed to create window surface !");
        return false;
    }
    gData.surface = surface;

    return true;
}