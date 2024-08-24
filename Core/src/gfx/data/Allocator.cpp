#include <pch.hpp>
#include "Allocator.hpp"

#include "Device.hpp"
#include "Instance.hpp"

namespace gage::gfx::data
{
    Allocator::Allocator(const Device& device, const Instance& instance)
    {
         // Create allocator
        VmaVulkanFunctions vulkanFunctions = {};
        vulkanFunctions.vkGetInstanceProcAddr = &vkGetInstanceProcAddr;
        vulkanFunctions.vkGetDeviceProcAddr = &vkGetDeviceProcAddr;

        VmaAllocatorCreateInfo allocatorCreateInfo = {};
        // allocatorCreateInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
        allocatorCreateInfo.vulkanApiVersion = VK_API_VERSION_1_3;
        allocatorCreateInfo.physicalDevice = device.physical_device;
        allocatorCreateInfo.device = device.device;
        allocatorCreateInfo.instance = instance.instance;
        allocatorCreateInfo.pVulkanFunctions = &vulkanFunctions;

        vmaCreateAllocator(&allocatorCreateInfo, &allocator);
    }

    Allocator::~Allocator()
    {
        vmaDestroyAllocator(allocator);
    }
}