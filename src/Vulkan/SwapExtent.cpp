#include "pch.hpp"
#include "VulkanEngine.hpp"

bool VulkanEngine::initSwapExtent()
{
    if (gData.surfaceCapabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
    {
        gData.swapExtent = gData.surfaceCapabilities.currentExtent;
    }
    else
    {
        int width, height;
        glfwGetFramebufferSize(gData.window, &width, &height);

        VkExtent2D actualExtent = {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)};

        actualExtent.width = std::clamp(actualExtent.width, gData.surfaceCapabilities.minImageExtent.width, gData.surfaceCapabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, gData.surfaceCapabilities.minImageExtent.height, gData.surfaceCapabilities.maxImageExtent.height);
        gData.swapExtent = actualExtent;
    }

    return true;
}