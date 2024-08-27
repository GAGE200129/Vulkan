#pragma once

#include <vk_mem_alloc.h>


namespace gage::gfx
{
    class Graphics;
}
namespace gage::gfx::data
{
    class CPUBuffer
    {
    public:
        CPUBuffer(const Graphics& gfx, VkBufferUsageFlags flags, size_t size_in_bytes, const void* data);
        ~CPUBuffer();

        CPUBuffer(const CPUBuffer&) = delete;
        CPUBuffer operator=(const CPUBuffer&) = delete;
        CPUBuffer& operator=(CPUBuffer&&) = delete;
        CPUBuffer(CPUBuffer&& other) : gfx(other.gfx)
        {
            this->buffer_handle = other.buffer_handle;
            this->allocation = other.allocation;
            other.buffer_handle = VK_NULL_HANDLE;
            other.allocation = VK_NULL_HANDLE;
        }



        VkBuffer get_buffer_handle() const;
        void* get_mapped() const;
    private:
        const Graphics&           gfx;
        VkBuffer            buffer_handle{};
        VmaAllocation       allocation{};
        VmaAllocationInfo   info{};
    };
}