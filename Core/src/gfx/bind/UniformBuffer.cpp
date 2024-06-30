#include "UniformBuffer.hpp"

namespace gage::gfx::bind
{
    UniformBuffer::UniformBuffer(Graphics &gfx, size_t size) :
     size_in_bytes(size)
    {
        VkBufferCreateInfo buffer_info = {};
        buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        buffer_info.size = size;
        buffer_info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        VmaAllocationCreateInfo alloc_info = {};
        alloc_info.usage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST;
        alloc_info.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;

        vk_check(vmaCreateBuffer(get_allocator(gfx), &buffer_info, &alloc_info, &buffer, &allocation, nullptr));

        vmaMapMemory(get_allocator(gfx), allocation, &mapped_data);
    }


    void UniformBuffer::update(const void *data)
    {
        std::memcpy(mapped_data, data, size_in_bytes);
    }
    void UniformBuffer::destroy(Graphics &gfx)
    {
        vmaUnmapMemory(get_allocator(gfx), allocation);
        vmaDestroyBuffer(get_allocator(gfx), buffer, allocation);
    }

    VkBuffer UniformBuffer::get() const
    {
        return buffer;
    }

    size_t UniformBuffer::get_size() const
    {
        return size_in_bytes;
    }
}