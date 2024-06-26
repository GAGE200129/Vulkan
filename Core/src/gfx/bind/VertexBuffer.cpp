#include "VertexBuffer.hpp"

#include "../Exception.hpp"
#include "../Graphics.hpp"

#include <vk_mem_alloc.h>
#include <cstring>

namespace gage::gfx::bind
{
    VertexBuffer::VertexBuffer(Graphics &gfx, uint32_t binding, std::span<Vertex> vertices) :
        binding(binding)
    {
        VkBufferCreateInfo buffer_info = {};
        buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        // this is the total size, in bytes, of the buffer we are allocating
        buffer_info.size = vertices.size() * sizeof(Vertex);
        // this buffer is going to be used as a Vertex Buffer
        buffer_info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

        // let the VMA library know that this data should be writeable by CPU, but also readable by GPU
        VmaAllocationCreateInfo vmaallocInfo = {};
        vmaallocInfo.usage = VMA_MEMORY_USAGE_AUTO;
        vmaallocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;

        vk_check(vmaCreateBuffer(get_allocator(gfx), &buffer_info, &vmaallocInfo, &buffer, &allocation, &allocation_info));

        void *data;
        vmaMapMemory(get_allocator(gfx), allocation, &data);
        std::memcpy(data, vertices.data(), vertices.size() * sizeof(Vertex));
        vmaUnmapMemory(get_allocator(gfx), allocation);
    }
    void VertexBuffer::destroy(Graphics& gfx)
    {
        vmaDestroyBuffer(get_allocator(gfx), buffer, allocation);
    }

    void VertexBuffer::bind(Graphics& gfx)
    {
        VkDeviceSize offsets{0};
        vkCmdBindVertexBuffers(get_cmd(gfx), binding, 1, &buffer, &offsets);
    }

    
}