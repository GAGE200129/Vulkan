#include <pch.hpp>
#include "CommandPool.hpp"


#include "Device.hpp"
#include "../Exception.hpp"

namespace gage::gfx::data
{
    CommandPool::CommandPool(const Device& device) :
        device(device)
    {
        VkCommandPoolCreateInfo command_pool_info = {};
        command_pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        command_pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        command_pool_info.queueFamilyIndex = device.queue_family;
        vk_check(vkCreateCommandPool(device.device, &command_pool_info, nullptr, &pool));
    }

    CommandPool::~CommandPool()
    {
       vkDestroyCommandPool(device.device, pool, nullptr);
    }
}