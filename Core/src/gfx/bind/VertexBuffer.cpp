#include "VertexBuffer.hpp"

#include "../Exception.hpp"
#include "../Graphics.hpp"

#include <vk_mem_alloc.h>
#include <cstring>

namespace gage::gfx::bind
{
    VertexBuffer::VertexBuffer(Graphics &gfx, uint32_t binding, uint32_t size_in_bytes,const void* vertices) :
        IBindable(gfx),
        binding(binding)
    {
        assert(size_in_bytes != 0 && vertices != nullptr);
        VkBufferCreateInfo staging_buffer_info = {};
        staging_buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        staging_buffer_info.size = size_in_bytes;
        staging_buffer_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        VmaAllocationCreateInfo staging_alloc_info = {};
        staging_alloc_info.usage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST;
        staging_alloc_info.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;

        VkBuffer staging_buffer{};
        VmaAllocation staging_allocation{};
        vk_check(vmaCreateBuffer(get_allocator(gfx), &staging_buffer_info, &staging_alloc_info, &staging_buffer, &staging_allocation, nullptr));

        void *data;
        vmaMapMemory(get_allocator(gfx), staging_allocation, &data);
        std::memcpy(data, vertices, size_in_bytes);
        vmaUnmapMemory(get_allocator(gfx), staging_allocation);


        //Create this vertex buffer
       
        VkBufferCreateInfo buffer_info = {};
        buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        // this is the total size, in bytes, of the buffer we are allocating
        buffer_info.size = size_in_bytes;
        // this buffer is going to be used as a Vertex Buffer
        buffer_info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

        // let the VMA library know that this data should be writeable by CPU, but also readable by GPU
        VmaAllocationCreateInfo alloc_info = {};
        alloc_info.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
        alloc_info.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;

        vk_check(vmaCreateBuffer(get_allocator(gfx), &buffer_info, &alloc_info, &buffer, &allocation, &allocation_info));

        //Copy to buffer
        VkCommandBufferBeginInfo transfer_begin_info{};
        transfer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        transfer_begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        VkCommandBuffer cmd = get_transfer_cmd(gfx);
        vkBeginCommandBuffer(cmd, &transfer_begin_info);

        VkBufferCopy copy_region{};
        copy_region.size = size_in_bytes;
        vkCmdCopyBuffer(cmd, staging_buffer, buffer, 1, &copy_region);
        vkEndCommandBuffer(cmd);

        VkSubmitInfo submit_info{};
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &cmd;
    
        vkQueueSubmit(get_queue(gfx), 1, &submit_info, nullptr);
        vkQueueWaitIdle(get_queue(gfx));

        vmaDestroyBuffer(get_allocator(gfx), staging_buffer, staging_allocation);
    }
    VertexBuffer::~VertexBuffer()
    {
        vmaDestroyBuffer(get_allocator(gfx), buffer, allocation);
    }

    void VertexBuffer::bind(Graphics& gfx)
    {
        VkDeviceSize offsets{0};
        vkCmdBindVertexBuffers(get_cmd(gfx), binding, 1, &buffer, &offsets);
    }

    
}