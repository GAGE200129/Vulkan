#include "Box.hpp"

#include "../bind/VertexBuffer.hpp"
#include "../bind/IndexBuffer.hpp"
#include "../bind/TransformPS.hpp"
#include "../bind/Pipeline.hpp"

#include <glm/gtc/matrix_transform.hpp>



namespace gage::gfx::draw
{
    Box::Box(Graphics &gfx)
    {
        VkPipelineLayout pipeline_layout{};
        if (!is_static_initialized())
        {
            struct Vertex 
            {
                glm::vec3 position;
                glm::vec3 normal;
                glm::vec2 uv;
            };
            std::vector<VkVertexInputBindingDescription> bindings;
            std::vector<VkVertexInputAttributeDescription> attributes;

            // we will have just 1 vertex buffer binding, with a per-vertex rate
            VkVertexInputBindingDescription mainBinding = {};
            mainBinding.binding = 0;
            mainBinding.stride = sizeof(Vertex);
            mainBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
            bindings.push_back(mainBinding);

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

            attributes.push_back(position_attribute);
            attributes.push_back(normal_attribute);
            attributes.push_back(uv_attribute);

            std::vector<Vertex> vertices{
                {{-1.0f, -1.0f, 1.0f}, {}, {}},
                {{1.0f, -1.0f, 1.0f}, {}, {}},
                {{-1.0f, 1.0f, 1.0f}, {}, {}},
                {{1.0f, 1.0f, 1.0f}, {}, {}},
                {{-1.0f, -1.0f, -1.0f}, {}, {}},
                {{1.0f, -1.0f, -1.0f}, {}, {}},
                {{-1.0f, 1.0f, -1.0f}, {}, {}},
                {{1.0f, 1.0f, -1.0f}, {}, {}},
            };
            std::vector<uint32_t> indices = {
                // Top
                2, 6, 7,
                2, 3, 7,

                // Bottom
                0, 4, 5,
                0, 1, 5,

                // Left
                0, 2, 6,
                0, 4, 6,

                // Right
                1, 3, 7,
                1, 5, 7,

                // Front
                0, 2, 3,
                0, 1, 3,

                // Back
                4, 6, 7,
                4, 5, 7};

            PipelineBuilder builder{};
            builder.set_vertex_layout(bindings, attributes)
                .set_vertex_shader("Core/shaders/compiled/colored_triangle.vert.spv", "main")
                .set_fragment_shader("Core/shaders/compiled/colored_triangle.frag.spv", "main")
                .set_topology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
                .set_polygon_mode(VK_POLYGON_MODE_FILL)
                .set_cull_mode(VK_CULL_MODE_NONE, VK_FRONT_FACE_CLOCKWISE)
                .set_multisampling_none()
                .set_blending_none()
                .enable_depth_test();
            add_static_bind(std::make_unique<bind::VertexBuffer>(gfx, 0, vertices.size() * sizeof(Vertex), vertices.data()));

            auto pipeline = std::make_unique<bind::Pipeline>(gfx, builder);
            pipeline_layout = pipeline->get_layout();
            add_static_bind(std::move(pipeline));
            add_static_index_buffer(std::make_unique<bind::IndexBuffer>(gfx, indices));
        }
        else
        {
            this->index_buffer = search_static_index_buffer();
            pipeline_layout = search_static_pipeline()->get_layout();
        }
        add_bind(std::make_unique<bind::TransformPS>(gfx, pipeline_layout, *this));
    }

    void Box::update(float dt)
    {
        time += dt;
        position.z = -10;
        position.y = glm::sin(glm::radians(time)) * 5;
        position.x = glm::cos(glm::radians(time)) * 5;
    }

    glm::mat4 Box::get_world_transform() const
    {
        return glm::translate(glm::mat4(1.0f), position);
    }
}