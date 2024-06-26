#include "IndexBuffer.hpp"

#include "Graphics.hpp"

#include <cstring>

namespace gage::gfx
{
    IndexBuffer::IndexBuffer(Graphics& gfx, std::span<uint32_t> indices) :
        allocator_ref(gfx.allocator)
    {
        VkBufferCreateInfo buffer_info = {};
        buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        // this is the total size, in bytes, of the buffer we are allocating
        buffer_info.size = indices.size() * sizeof(uint32_t);
        // this buffer is going to be used as a Vertex Buffer
        buffer_info.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;

        // let the VMA library know that this data should be writeable by CPU, but also readable by GPU
        VmaAllocationCreateInfo vmaallocInfo = {};
        vmaallocInfo.usage = VMA_MEMORY_USAGE_AUTO;
        vmaallocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;

        vk_check(vmaCreateBuffer(allocator_ref, &buffer_info, &vmaallocInfo, &buffer, &allocation, &allocation_info));

        void *data;
        vmaMapMemory(allocator_ref, allocation, &data);
        std::memcpy(data, indices.data(), indices.size() * sizeof(uint32_t));
        vmaUnmapMemory(allocator_ref, allocation);
    }   

    IndexBuffer::~IndexBuffer()
    {
        vmaDestroyBuffer(allocator_ref, buffer, allocation);
    }
}