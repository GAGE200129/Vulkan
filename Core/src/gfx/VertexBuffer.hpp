#pragma once

#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <span>
#include <vector>
#include <vk_mem_alloc.h>

namespace gage::gfx
{
    struct Vertex 
    {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec2 uv;
    };

    struct VertexInputDescription {
        std::vector<VkVertexInputBindingDescription> bindings;
        std::vector<VkVertexInputAttributeDescription> attributes;
        VkPipelineVertexInputStateCreateFlags flags = 0;
    };

    class Graphics;
    class VertexBuffer
    {
        friend class Graphics;
    public:
        VertexBuffer(Graphics& gfx, std::span<Vertex> vertices);
        ~VertexBuffer();
        
        static VertexInputDescription get_vertex_description();
    private:
        VmaAllocator& allocator_ref;
        VkBuffer buffer;
        VmaAllocation allocation;
        VmaAllocationInfo allocation_info;
    };
}