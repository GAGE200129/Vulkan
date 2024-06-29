#pragma once
#include "IBindable.hpp"


#include <vk_mem_alloc.h>
#include <cstring>

namespace gage::gfx::bind
{
    class UniformBuffer : public IBindable
    {
    public:
        UniformBuffer(Graphics& gfx, size_t size) :
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

        void bind(Graphics&) override {}

        void update(const void* data)
        {
            std::memcpy(mapped_data, data, size_in_bytes);
        }
        void destroy(Graphics& gfx) override
        {
            vmaUnmapMemory(get_allocator(gfx), allocation);
            vmaDestroyBuffer(get_allocator(gfx), buffer, allocation);
        }

        VkBuffer get() const
        {
            return buffer;
        }

        size_t get_size() const 
        {
            return size_in_bytes;
        }
    private:
        VkBuffer buffer{};
        void* mapped_data{};
        size_t size_in_bytes{};
        VmaAllocation allocation{};
        VmaAllocationInfo allocation_info{};
    };
}