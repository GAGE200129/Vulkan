#include "Box.hpp"

#include "../bind/VertexBuffer.hpp"
#include "../bind/IndexBuffer.hpp"
#include "../bind/TransformPS.hpp"
#include "../bind/Pipeline.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <random>


namespace gage::gfx::draw
{
    Box::Box(Graphics &gfx)
    {
        std::random_device rd;
        std::mt19937 e(rd());

        std::uniform_real_distribution<float> rotation_dist(0, 10);
        std::uniform_real_distribution<float> rotation_dist2(0, 360);
        std::uniform_real_distribution<float> radius_dist(0, 100);

        pitch = rotation_dist2(e);
        yaw = rotation_dist2(e);
        roll = rotation_dist2(e);

        pitch_orbit = rotation_dist2(e);
        yaw_orbit = rotation_dist2(e);
        roll_orbit = rotation_dist2(e);
        
        this->pitch_speed = rotation_dist(e);
        this->yaw_speed = rotation_dist(e);
        this->roll_speed = rotation_dist(e);
        this->radius = radius_dist(e);

        this->pitch_orbit_speed = rotation_dist(e);
        this->yaw_orbit_speed = rotation_dist(e);
        this->roll_orbit_speed = rotation_dist(e);

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
            VkVertexInputBindingDescription main_binding = {};
            main_binding.binding = 0;
            main_binding.stride = sizeof(Vertex);
            main_binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
            bindings.push_back(main_binding);

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
        pitch += pitch_speed * dt;
        yaw += yaw_speed * dt;
        roll += roll_speed * dt;

        pitch_orbit += pitch_orbit_speed * dt;
        yaw_orbit += yaw_orbit_speed * dt;
        roll_orbit += roll_orbit_speed * dt;
    }

    glm::mat4 Box::get_world_transform() const
    {
        glm::mat4x4 final_transform = glm::mat4x4(1.0f);
        final_transform = glm::translate(final_transform, {0, 0, -100});
        final_transform = glm::rotate(final_transform, glm::radians(pitch_orbit), {1.0f, 0.0f, 0.0f});
        final_transform = glm::rotate(final_transform, glm::radians(yaw_orbit), {0.0f, 1.0f, 0.0f});
        final_transform = glm::rotate(final_transform, glm::radians(roll_orbit), {0.0f, 0.0f, 1.0f});

        final_transform = glm::translate(final_transform, {radius, 0, 0});
        final_transform = glm::rotate(final_transform, glm::radians(pitch), {1.0f, 0.0f, 0.0f});
        final_transform = glm::rotate(final_transform, glm::radians(yaw), {0.0f, 1.0f, 0.0f});
        final_transform = glm::rotate(final_transform, glm::radians(roll), {0.0f, 0.0f, 1.0f});
        return final_transform;
    }
}