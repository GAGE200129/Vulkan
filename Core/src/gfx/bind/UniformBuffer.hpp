#pragma once
#include "IBindable.hpp"


#include <vk_mem_alloc.h>
#include <cstring>

namespace gage::gfx::bind
{
    class UniformBuffer : public IBindable
    {
    public:
        UniformBuffer(Graphics& gfx, size_t size);
        virtual  ~UniformBuffer();

        void bind(Graphics&) final {}

        void update(const void* data);
        VkBuffer get() const;
        size_t get_size() const ;
    private:
        VkBuffer buffer{};
        void* mapped_data{};
        size_t size_in_bytes{};
        VmaAllocation allocation{};
        VmaAllocationInfo allocation_info{};
    };
}