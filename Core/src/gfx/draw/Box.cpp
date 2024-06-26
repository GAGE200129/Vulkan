#include "Box.hpp"

#include "../Vertex.hpp"
#include "../bind/VertexBuffer.hpp"
#include "../bind/IndexBuffer.hpp"
#include "../bind/TransformPS.hpp"
#include "../bind/Pipeline.hpp"



namespace gage::gfx::draw
{
    Box::Box(Graphics& gfx)
    {
        std::vector<Vertex> vertices 
        {
            {{-1.0f, -1.0f, -1.0f }, {}, {}},
            {{1.0f, -1.0f, -1.0f }, {}, {}},
            {{-1.0f, 1.0f, -1.0f }, {}, {}},
            {{1.0f, 1.0f, -1.0f }, {}, {}},
            {{-1.0f, -1.0f, 1.0f }, {}, {}},
            {{1.0f, -1.0f, 1.0f }, {}, {}},
            {{-1.0f, 1.0f, 1.0f }, {}, {}},
            {{-1.0f, 1.0f, 1.0f }, {}, {}},
        };
        std::vector<uint32_t> indices = {
            0, 2, 1, 2, 3, 1,
            1, 3, 5, 3, 7, 5,
            2, 6, 3, 3, 6, 7,
            4, 5, 7, 4, 7, 6,
            0, 4, 2, 2, 4, 6,
            0, 1, 4, 1, 5, 4,
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
}