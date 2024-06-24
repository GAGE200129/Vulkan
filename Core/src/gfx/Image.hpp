#pragma once

#include <vulkan/vulkan.h>

namespace gage::gfx
{
    void transition_image(VkCommandBuffer cmd, VkImage image, VkImageLayout current_layout, VkImageLayout new_layout);
};