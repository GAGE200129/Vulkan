#pragma once

#include <cstdint>
#include <span>
#include <vk_mem_alloc.h>

namespace gage::gfx
{
    class Graphics;
    class IndexBuffer
    {
        friend class Graphics;
    public:
        IndexBuffer(Graphics& gfx, std::span<uint32_t> indices);
        ~IndexBuffer();
    private:
        VmaAllocator& allocator_ref;
        VkBuffer buffer;
        VmaAllocation allocation;
        VmaAllocationInfo allocation_info;
    };
}