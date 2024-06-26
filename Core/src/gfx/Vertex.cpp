#include "Vertex.hpp"

namespace gage::gfx
{
    VertexInputDescription Vertex::get_vertex_description()
    {
        VertexInputDescription description;

        // we will have just 1 vertex buffer binding, with a per-vertex rate
        VkVertexInputBindingDescription mainBinding = {};
        mainBinding.binding = 0;
        mainBinding.stride = sizeof(Vertex);
        mainBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        description.bindings.push_back(mainBinding);

        // Position will be stored at Location 0
        VkVertexInputAttributeDescription position_attribute = {};
        position_attribute.binding = 0;
        position_attribute.location = 0;
        position_attribute.format = VK_FORMAT_R32G32B32_SFLOAT;
        position_attribute.offset = offsetof(Vertex, position);

        // Normal will be stored at Location 1
        VkVertexInputAttributeDescription normal_attribute = {};
        normal_attribute.binding = 0;
        normal_attribute.location = 1;
        normal_attribute.format = VK_FORMAT_R32G32B32_SFLOAT;
        normal_attribute.offset = offsetof(Vertex, normal);

        // Color will be stored at Location 2
        VkVertexInputAttributeDescription uv_attribute = {};
        uv_attribute.binding = 0;
        uv_attribute.location = 2;
        uv_attribute.format = VK_FORMAT_R32G32_SFLOAT;
        uv_attribute.offset = offsetof(Vertex, uv);

        description.attributes.push_back(position_attribute);
        description.attributes.push_back(normal_attribute);
        description.attributes.push_back(uv_attribute);
        return description;
    }
}