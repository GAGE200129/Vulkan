#include <pch.hpp>
#include "FrameData.hpp"

#include "Device.hpp"
#include "DescriptorPool.hpp"
#include "CommandPool.hpp"
#include "../Graphics.hpp"
#include "../Exception.hpp"

namespace gage::gfx::data
{
    FrameData::FrameData(const Device &device, const DescriptorPool &desc_pool, const CommandPool& cmd_pool, VmaAllocator allocator, VkDescriptorSetLayout global_set_layout) :
        device(device), desc_pool(desc_pool), cmd_pool(cmd_pool), allocator(allocator)
    {
        VkFenceCreateInfo fenceCreateInfo = {};
        fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        // we want to create the fence with the Create Signaled flag, so we can wait on it before using it on a GPU command (for the first frame)
        fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        vk_check(vkCreateFence(device.device, &fenceCreateInfo, nullptr, &render_fence));

        // for the semaphores we don't need any flags
        VkSemaphoreCreateInfo semaphoreCreateInfo = {};
        semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        semaphoreCreateInfo.flags = 0;

        vk_check(vkCreateSemaphore(device.device, &semaphoreCreateInfo, nullptr, &present_semaphore));
        vk_check(vkCreateSemaphore(device.device, &semaphoreCreateInfo, nullptr, &render_semaphore));

        // Allocate command buffer
        VkCommandBufferAllocateInfo cmd_alloc_info = {};
        cmd_alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        cmd_alloc_info.commandPool = cmd_pool.pool;
        cmd_alloc_info.commandBufferCount = 1;
        vk_check(vkAllocateCommandBuffers(device.device, &cmd_alloc_info, &cmd));

        // Create global uniform buffer
        VkBufferCreateInfo buffer_ci = {};
        buffer_ci.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        buffer_ci.size = sizeof(Graphics::GlobalUniform);
        buffer_ci.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

        VmaAllocationCreateInfo alloc_ci = {};
        alloc_ci.usage = VMA_MEMORY_USAGE_AUTO;
        alloc_ci.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
        vk_check(vmaCreateBuffer(allocator, &buffer_ci, &alloc_ci, &global_buffer, &global_alloc, &global_alloc_info));

        // Allocate descriptor set
        VkDescriptorSetAllocateInfo global_set_alloc_info{};
        global_set_alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        global_set_alloc_info.descriptorPool = desc_pool.pool;
        global_set_alloc_info.descriptorSetCount = 1;
        global_set_alloc_info.pSetLayouts = &global_set_layout;
        vkAllocateDescriptorSets(device.device, &global_set_alloc_info, &global_set);

        // Link global uniform to globlal set
        VkDescriptorBufferInfo buffer_info{};
        buffer_info.buffer = global_buffer;
        buffer_info.offset = 0;
        buffer_info.range = sizeof(Graphics::GlobalUniform);

        VkWriteDescriptorSet descriptor_write{};
        descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptor_write.descriptorCount = 1;
        descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptor_write.dstBinding = 0;
        descriptor_write.dstSet = global_set;
        descriptor_write.pBufferInfo = &buffer_info;
        vkUpdateDescriptorSets(device.device, 1, &descriptor_write, 0, nullptr);
    }

    FrameData::~FrameData()
    {
        vkDestroyFence(device.device, render_fence, nullptr);
        vkFreeCommandBuffers(device.device, cmd_pool.pool, 1, &cmd);
        vkDestroySemaphore(device.device, present_semaphore, nullptr);
        vkDestroySemaphore(device.device, render_semaphore, nullptr);
        vmaDestroyBuffer(allocator, global_buffer, global_alloc);
        vkFreeDescriptorSets(device.device, desc_pool.pool, 1, &global_set);
    }
}