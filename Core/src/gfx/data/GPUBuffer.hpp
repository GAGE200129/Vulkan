#pragma once


namespace gage::gfx
{
    class Graphics;
}

namespace gage::gfx::data
{
    class GPUBuffer
    {
    public:
        GPUBuffer(Graphics& gfx, VkBufferUsageFlags flags, size_t size_in_bytes, const void* data);
        ~GPUBuffer();

        GPUBuffer(const GPUBuffer&) = delete;
        GPUBuffer(GPUBuffer&&) = delete;
        GPUBuffer operator=(const GPUBuffer&) = delete;

        VkBuffer get_buffer_handle() const;
    private:
        Graphics&           gfx;
        VkBuffer            buffer_handle{};
        VmaAllocation       allocation{};
        VmaAllocationInfo   info{};
    };
}