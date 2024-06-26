#pragma once

#include "IBindable.hpp"
#include <span>

namespace gage::gfx::bind
{
    class IndexBuffer : public IBindable
    {
    public:
        IndexBuffer(Graphics& gfx, std::span<uint32_t> indices);
        void bind(Graphics& gfx) override;
        void destroy(Graphics& gfx) override;

        uint32_t get_vertex_count() const;
    private:
        uint32_t vertex_count;
        VkBuffer buffer;
        VmaAllocation allocation;
        VmaAllocationInfo allocation_info;
    };
}