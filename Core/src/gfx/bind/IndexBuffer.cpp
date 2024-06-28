#include "IndexBuffer.hpp"

#include "../Graphics.hpp"

#include <cstring>
#include <cassert>

namespace gage::gfx::bind
{
    IndexBuffer::IndexBuffer(Graphics& gfx, std::span<uint32_t> indices) :
        vertex_count(indices.size())
    {
        assert(indices.size() != 0);

        auto external_memory_ci = gfx.get_external_buffer_memory_ci();
        VkBufferCreateInfo buffer_info = {};
        buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        // this is the total size, in bytes, of the buffer we are allocating
        buffer_info.size = indices.size() * sizeof(uint32_t);
        // this buffer is going to be used as a Vertex Buffer
        buffer_info.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
        buffer_info.pNext = &external_memory_ci;

        // let the VMA library know that this data should be writeable by CPU, but also readable by GPU
        VmaAllocationCreateInfo vmaallocInfo = {};
        vmaallocInfo.usage = VMA_MEMORY_USAGE_AUTO;
        vmaallocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;

        vk_check(vmaCreateBuffer(get_allocator(gfx), &buffer_info, &vmaallocInfo, &buffer, &allocation, &allocation_info));

        void *data;
        vmaMapMemory(get_allocator(gfx), allocation, &data);
        std::memcpy(data, indices.data(), indices.size() * sizeof(uint32_t));
        vmaUnmapMemory(get_allocator(gfx), allocation);
    }   

    void IndexBuffer::destroy(Graphics& gfx)
    {
        vmaDestroyBuffer(get_allocator(gfx), buffer, allocation);
    }

    void IndexBuffer::bind(Graphics& gfx)
    {
        vkCmdBindIndexBuffer(get_cmd(gfx), buffer, 0, VK_INDEX_TYPE_UINT32);
    }

    uint32_t IndexBuffer::get_vertex_count() const
    {
        return vertex_count;
    }
}