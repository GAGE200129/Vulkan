#pragma once


namespace gage::utils
{
    void transition_image(VkCommandBuffer cmd, VkImage image, VkImageAspectFlags aspect, VkImageLayout current_layout, VkImageLayout new_layout);
    uint32_t find_memory_type(VkPhysicalDevice physical_device, uint32_t type_filter, VkMemoryPropertyFlags properties);
}