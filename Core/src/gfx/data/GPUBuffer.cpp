#include <pch.hpp>
#include "GPUBuffer.hpp"

#include "../Graphics.hpp"



namespace gage::gfx::data
{
    GPUBuffer::GPUBuffer(const Graphics &gfx, VkBufferUsageFlags flags, size_t size_in_bytes, const void *data) : gfx(gfx)
    {
        log().trace("Allocating vulkan gpu buffer: size: {} bytes, address: {}, flags: {}", size_in_bytes, data, string_VkBufferUsageFlags(flags));
        assert(size_in_bytes != 0 && data != nullptr);
        VkBufferCreateInfo staging_buffer_info = {};
        staging_buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        staging_buffer_info.size = size_in_bytes;
        staging_buffer_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        VmaAllocationCreateInfo staging_alloc_info = {};
        staging_alloc_info.usage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST;
        staging_alloc_info.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;

        VkBuffer staging_buffer{};
        VmaAllocation staging_allocation{};
        vk_check(vmaCreateBuffer(gfx.allocator.allocator, &staging_buffer_info, &staging_alloc_info, &staging_buffer, &staging_allocation, nullptr));

        void *mapped;
        vmaMapMemory(gfx.allocator.allocator, staging_allocation, &mapped);
        std::memcpy(mapped, data, size_in_bytes);
        vmaUnmapMemory(gfx.allocator.allocator, staging_allocation);

        // Create this buffer
        VkBufferCreateInfo buffer_info = {};
        buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        // this is the total size, in bytes, of the buffer we are allocating
        buffer_info.size = size_in_bytes;
        // this buffer is going to be used as a Vertex Buffer
        buffer_info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | flags;

        // let the VMA library know that this data should be writeable by CPU, but also readable by GPU
        VmaAllocationCreateInfo alloc_info = {};
        alloc_info.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
        alloc_info.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
        vk_check(vmaCreateBuffer(gfx.allocator.allocator, &buffer_info, &alloc_info, &buffer_handle, &allocation, &info));

        VkCommandBufferAllocateInfo cmd_alloc_info{};
        cmd_alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        cmd_alloc_info.commandPool = gfx.cmd_pool.pool;
        cmd_alloc_info.commandBufferCount = 1;
        cmd_alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

        VkCommandBuffer cmd{};
        vk_check(vkAllocateCommandBuffers(gfx.device.device, &cmd_alloc_info, &cmd));

        // Copy to buffer
        VkCommandBufferBeginInfo transfer_begin_info{};
        transfer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        transfer_begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        vkBeginCommandBuffer(cmd, &transfer_begin_info);

        VkBufferCopy copy_region{};
        copy_region.size = size_in_bytes;
        vkCmdCopyBuffer(cmd, staging_buffer, buffer_handle, 1, &copy_region);
        vkEndCommandBuffer(cmd);

        VkSubmitInfo submit_info{};
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &cmd;

        vkQueueSubmit(gfx.device.queue, 1, &submit_info, nullptr);
        vkQueueWaitIdle(gfx.device.queue);

        vmaDestroyBuffer(gfx.allocator.allocator, staging_buffer, staging_allocation);
        vkFreeCommandBuffers(gfx.device.device, gfx.cmd_pool.pool, 1, &cmd);
    }

    GPUBuffer::~GPUBuffer()
    {
        log().trace("Deallocating vulkan gpu buffer: size: {} bytes", info.size);
        vmaDestroyBuffer(gfx.allocator.allocator, buffer_handle, allocation);
    }

    VkBuffer GPUBuffer::get_buffer_handle() const
    {
        return buffer_handle;
    }
}