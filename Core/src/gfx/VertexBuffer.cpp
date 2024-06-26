#include "VertexBuffer.hpp"

#include "Exception.hpp"
#include "Graphics.hpp"

#include <vk_mem_alloc.h>
#include <cstring>

namespace gage::gfx
{
    VertexBuffer::VertexBuffer(Graphics &gfx, std::span<Vertex> vertices) : allocator_ref(gfx.allocator)
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

        vk_check(vmaCreateBuffer(allocator_ref, &buffer_info, &vmaallocInfo, &buffer, &allocation, &allocation_info));

        void *data;
        vmaMapMemory(allocator_ref, allocation, &data);
        std::memcpy(data, vertices.data(), vertices.size() * sizeof(Vertex));
        vmaUnmapMemory(allocator_ref, allocation);
    }
    VertexBuffer::~VertexBuffer()
    {
        vmaDestroyBuffer(allocator_ref, buffer, allocation);
    }

    VertexInputDescription VertexBuffer::get_vertex_description()
    {
        VertexInputDescription description;

        // we will have just 1 vertex buffer binding, with a per-vertex rate
        VkVertexInputBindingDescription mainBinding = {};
        mainBinding.binding = 0;
        mainBinding.stride = sizeof(Vertex);
        mainBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        description.bindings.push_back(mainBinding);

        // Position will be stored at Location 0
        VkVertexInputAttributeDescription position_attribute = {};
        position_attribute.binding = 0;
        position_attribute.location = 0;
        position_attribute.format = VK_FORMAT_R32G32B32_SFLOAT;
        position_attribute.offset = offsetof(Vertex, position);

        // Normal will be stored at Location 1
        VkVertexInputAttributeDescription normal_attribute = {};
        normal_attribute.binding = 0;
        normal_attribute.location = 1;
        normal_attribute.format = VK_FORMAT_R32G32B32_SFLOAT;
        normal_attribute.offset = offsetof(Vertex, normal);

        // Color will be stored at Location 2
        VkVertexInputAttributeDescription uv_attribute = {};
        uv_attribute.binding = 0;
        uv_attribute.location = 2;
        uv_attribute.format = VK_FORMAT_R32G32_SFLOAT;
        uv_attribute.offset = offsetof(Vertex, uv);

        description.attributes.push_back(position_attribute);
        description.attributes.push_back(normal_attribute);
        description.attributes.push_back(uv_attribute);
        return description;
    }
}