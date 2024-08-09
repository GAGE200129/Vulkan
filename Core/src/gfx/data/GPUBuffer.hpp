#pragma once

#include <vk_mem_alloc.h>

namespace gage::gfx
{
    class Graphics;
}

namespace gage::gfx::data
{
    class GPUBuffer
    {
    public:
        GPUBuffer(const Graphics& gfx, VkBufferUsageFlags flags, size_t size_in_bytes, const void* data);
        ~GPUBuffer();

        GPUBuffer(const GPUBuffer&) = delete;
        GPUBuffer operator=(const GPUBuffer&) = delete;
        GPUBuffer(GPUBuffer&&) = default;

        VkBuffer get_buffer_handle() const;
    private:
        const Graphics&           gfx;
        VkBuffer            buffer_handle{};
        VmaAllocation       allocation{};
        VmaAllocationInfo   info{};
    };
}