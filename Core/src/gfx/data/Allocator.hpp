#pragma once

#include <vk_mem_alloc.h>

namespace gage::gfx::data
{
    class Device;
    class Instance;
    class Allocator
    {
    public:
        Allocator(const Device& device, const Instance& instance);
        ~Allocator();

    public:
        VmaAllocator allocator{};
    };
}