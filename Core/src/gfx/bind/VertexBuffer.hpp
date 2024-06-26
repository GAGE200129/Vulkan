#pragma once


#include <span>
#include "IBindable.hpp"
#include "../Vertex.hpp"

namespace gage::gfx::bind
{
    class VertexBuffer : public IBindable
    {
    public:
        VertexBuffer(Graphics& gfx, uint32_t binding, std::span<Vertex> vertices);
        void bind(Graphics& gfx) override;
        void destroy(Graphics& gfx) override;
    private:
        uint32_t binding{};
        VkBuffer buffer{};
        VmaAllocation allocation{};
        VmaAllocationInfo allocation_info{};
    };
}