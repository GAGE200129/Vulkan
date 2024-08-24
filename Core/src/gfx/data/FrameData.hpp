#pragma once

#include <vk_mem_alloc.h>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

namespace gage::gfx::data
{
    class Device;
    class DescriptorPool;
    class CommandPool;
    class FrameData
    {
    public:
        FrameData(const Device& device,  const DescriptorPool& desc_pool, const CommandPool& cmd_pool, VmaAllocator allocator,
            VkDescriptorSetLayout global_set_layout);
        ~FrameData();
    private:
        const Device& device;
        const DescriptorPool& desc_pool;
        const CommandPool& cmd_pool;
        VmaAllocator allocator;
    public:
        VkSemaphore present_semaphore{};
        VkSemaphore render_semaphore{};
        VkFence render_fence{};
        VkCommandBuffer cmd{};
        VkDescriptorSet global_set{};
        VkBuffer global_buffer{};
        VmaAllocation global_alloc{};
        VmaAllocationInfo global_alloc_info{};
    };
}