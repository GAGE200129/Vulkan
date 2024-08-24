#pragma once

#include <vk_mem_alloc.h>

namespace  gage::gfx::data
{
    class Device;
    class Allocator;
    class CommandPool;
    class Default
    {
    public:
        Default(const Device& device, const CommandPool& cmd_pool, const Allocator& allocator);
        ~Default();
    private:
        const Device& device;
        const CommandPool& cmd_pool;
        const Allocator& allocator;
    public:
        VkImage image{};
        VkImageView image_view{};
        VmaAllocation image_alloc{};
        VkSampler sampler{};
    };
}