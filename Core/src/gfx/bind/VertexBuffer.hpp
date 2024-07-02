#pragma once


#include <span>
#include "IBindable.hpp"

namespace gage::gfx::bind
{
    class VertexBuffer : public IBindable
    {
    public:
        VertexBuffer(Graphics& gfx, uint32_t binding, uint32_t size_in_bytes,const void* vertices);
        virtual ~VertexBuffer();
        void bind(Graphics& gfx) final;
    private:
        uint32_t binding{};
        VkBuffer buffer{};
        VmaAllocation allocation{};
        VmaAllocationInfo allocation_info{};
    };
}