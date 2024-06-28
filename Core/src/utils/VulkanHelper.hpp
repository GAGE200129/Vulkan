#pragma once

#include <vulkan/vulkan.h>

namespace gage::utils
{
    uint32_t find_memory_type(VkPhysicalDevice physical_device, uint32_t type_filter, VkMemoryPropertyFlags properties);
}