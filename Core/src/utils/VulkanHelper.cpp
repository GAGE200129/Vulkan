#include <pch.hpp>
#include "VulkanHelper.hpp"

#include <Core/src/gfx/Exception.hpp>
#include <Core/src/gfx/gfx.hpp>

namespace gage::utils
{   
    void transition_image(VkCommandBuffer cmd, VkImage image, VkImageAspectFlags aspect, VkImageLayout current_layout, VkImageLayout new_layout)
    {
        VkImageMemoryBarrier barrier = {};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = current_layout;
        barrier.newLayout = new_layout;

        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

        barrier.image = image;
        barrier.subresourceRange.aspectMask = aspect;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;

        vkCmdPipelineBarrier(
            cmd,
            0 /* TODO */, 0 /* TODO */,
            0,
            0, nullptr,
            0, nullptr,
            1, &barrier
        );
    }

    uint32_t find_memory_type(VkPhysicalDevice physical_device, uint32_t type_filter, VkMemoryPropertyFlags properties)
    {
        VkPhysicalDeviceMemoryProperties mem_properties;
        vkGetPhysicalDeviceMemoryProperties(physical_device, &mem_properties);
        for (uint32_t i = 0; i < mem_properties.memoryTypeCount; i++) {
            if ((type_filter & (1 << i)) && (mem_properties.memoryTypes[i].propertyFlags & properties) == properties) {
                return i;
            }
        }
        gfx::log().critical("Failed to find memory type: {}", string_VkMemoryPropertyFlags(properties));
        throw gfx::GraphicsException{"Failed to find memory type !"};
    }
}