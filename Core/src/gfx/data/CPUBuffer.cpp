#include "CPUBuffer.hpp"

#include "../Graphics.hpp"

#include <cstring>

namespace gage::gfx::data
{
    CPUBuffer::CPUBuffer(Graphics &gfx, VkBufferUsageFlags flags, size_t size_in_bytes, const void *data) :
        gfx(gfx)
    {
        gfx.uploading_mutex.lock();
        assert(size_in_bytes != 0 && data != nullptr);
        VkBufferCreateInfo staging_buffer_info = {};
        staging_buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        staging_buffer_info.size = size_in_bytes;
        staging_buffer_info.usage = flags;
        VmaAllocationCreateInfo staging_alloc_info = {};
        staging_alloc_info.usage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST;
        staging_alloc_info.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;

        vk_check(vmaCreateBuffer(gfx.allocator, &staging_buffer_info, &staging_alloc_info, &buffer_handle, &allocation, &info));

        std::memcpy(info.pMappedData, data, size_in_bytes);

        gfx.uploading_mutex.unlock();
    }
    CPUBuffer::~CPUBuffer()
    {
        vmaDestroyBuffer(gfx.allocator, buffer_handle, allocation);
    }

    VkBuffer CPUBuffer::get_buffer_handle() const
    {
        return buffer_handle;
    }
    void *CPUBuffer::get_mapped() const
    {
        return info.pMappedData;
    }
}