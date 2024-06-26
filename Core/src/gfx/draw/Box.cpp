#include "Box.hpp"

#include "../Vertex.hpp"
#include "../bind/VertexBuffer.hpp"
#include "../bind/IndexBuffer.hpp"
#include "../bind/TransformPS.hpp"
#include "../bind/Pipeline.hpp"

#include <glm/gtc/matrix_transform.hpp>

namespace gage::gfx::draw
{
    Box::Box(Graphics &gfx)
    {
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
            4, 5, 7
        };

        VertexInputDescription vertex_description = Vertex::get_vertex_description();
        PipelineBuilder builder{};
        builder.set_vertex_layout(vertex_description.bindings, vertex_description.attributes)
            .set_vertex_shader("Core/shaders/compiled/colored_triangle.vert.spv", "main")
            .set_fragment_shader("Core/shaders/compiled/colored_triangle.frag.spv", "main")
            .set_topology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
            .set_polygon_mode(VK_POLYGON_MODE_FILL)
            .set_cull_mode(VK_CULL_MODE_NONE, VK_FRONT_FACE_CLOCKWISE)
            .set_multisampling_none()
            .set_blending_none()
            .enable_depth_test();
        auto pipeline = std::make_unique<bind::Pipeline>(gfx, builder);
        add_bind(std::make_unique<bind::VertexBuffer>(gfx, 0, vertices));
        add_bind(std::make_unique<bind::TransformPS>(gfx, pipeline->get_layout(), *this));
        add_bind(std::move(pipeline));
        add_index_buffer(std::make_unique<bind::IndexBuffer>(gfx, indices));
    }

    void Box::update(float dt)
    {
        static float temp = 0.0f;

        temp += dt;
        position.z = -10;
        position.y = glm::sin(glm::radians(temp)) * 5;
        position.x = glm::cos(glm::radians(temp)) * 5;
    }

    glm::mat4 Box::get_world_transform() const
    {
        return glm::translate(glm::mat4(1.0f), position);
    }
}