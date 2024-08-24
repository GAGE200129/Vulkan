#include <pch.hpp>
#include "DescriptorPool.hpp"

#include "Device.hpp"
#include "../Exception.hpp"

namespace gage::gfx::data
{
    DescriptorPool::DescriptorPool(const Device& device) :
        device(device)
    {
        VkDescriptorPoolSize pool_sizes[] = {
            {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1024},
            {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1024},
        };

        // Create descriptor pool
        VkDescriptorPoolCreateInfo desc_pool_ci{};
        desc_pool_ci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        desc_pool_ci.maxSets = 4;
        desc_pool_ci.pPoolSizes = pool_sizes;
        desc_pool_ci.poolSizeCount = sizeof(pool_sizes) / sizeof(VkDescriptorPoolSize);
        desc_pool_ci.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        vk_check(vkCreateDescriptorPool(device.device, &desc_pool_ci, nullptr, &pool));
    }

    DescriptorPool::~DescriptorPool()
    {
        vkDestroyDescriptorPool(device.device, pool, nullptr);
    }
}