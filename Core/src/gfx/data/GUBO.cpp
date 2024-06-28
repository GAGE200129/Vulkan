#include "GUBO.hpp"

#include "../Exception.hpp"
#include "../Graphics.hpp"

#include <cstring>

namespace gage::gfx::data
{
    GUBO::GUBO(Graphics& gfx, VmaAllocator allocator)
    {
        //Create uniform buffer
        VkBufferCreateInfo buffer_ci = {};
        buffer_ci.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        buffer_ci.size = sizeof(Data);
        buffer_ci.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        
        VmaAllocationCreateInfo alloc_ci = {};
        alloc_ci.usage = VMA_MEMORY_USAGE_AUTO;
        alloc_ci.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
        vk_check(vmaCreateBuffer(allocator, &buffer_ci, &alloc_ci, &buffer, &alloc, &alloc_info));
    }


    void GUBO::destroy(VmaAllocator allocator)
    {
        vmaDestroyBuffer(allocator, buffer, alloc);
    }

    void GUBO::update()
    {
        std::memcpy(alloc_info.pMappedData, &data, sizeof(Data));
    }
};