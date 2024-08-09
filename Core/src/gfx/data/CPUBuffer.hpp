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
        CPUBuffer(Graphics& gfx, VkBufferUsageFlags flags, size_t size_in_bytes, const void* data);
        CPUBuffer(const CPUBuffer&) = delete;
        CPUBuffer operator=(const CPUBuffer&) = delete;
        ~CPUBuffer();
        CPUBuffer(CPUBuffer&&) = default;



        VkBuffer get_buffer_handle() const;
        void* get_mapped() const;
    private:
        Graphics&           gfx;
        VkBuffer            buffer_handle{};
        VmaAllocation       allocation{};
        VmaAllocationInfo   info{};
    };
}