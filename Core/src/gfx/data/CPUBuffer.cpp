#include <pch.hpp>
#include "CPUBuffer.hpp"

#include "../Graphics.hpp"



namespace gage::gfx::data
{
    CPUBuffer::CPUBuffer(const Graphics &gfx, VkBufferUsageFlags flags, size_t size_in_bytes, const void *data) :
        gfx(gfx)
    {
        log().trace("Allocating vulkan cpu buffer: size: {} bytes, address: {}, flags: {}", size_in_bytes, data, string_VkBufferUsageFlags(flags));
        assert(size_in_bytes != 0);
        std::vector<unsigned char> null_buffer(size_in_bytes, 0xAA);
        if(data == nullptr)
        {
            data = null_buffer.data();
        }

        VkBufferCreateInfo staging_buffer_info = {};
        staging_buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        staging_buffer_info.size = size_in_bytes;
        staging_buffer_info.usage = flags;
        VmaAllocationCreateInfo staging_alloc_info = {};
        staging_alloc_info.usage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST;
        staging_alloc_info.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;

        vk_check(vmaCreateBuffer(gfx.allocator.allocator, &staging_buffer_info, &staging_alloc_info, &buffer_handle, &allocation, &info));

        std::memcpy(info.pMappedData, data, size_in_bytes);
    }
    CPUBuffer::~CPUBuffer()
    {
        log().trace("Deallocating vulkan cpu buffer: size: {} bytes", info.size);
        vmaDestroyBuffer(gfx.allocator.allocator, buffer_handle, allocation);
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