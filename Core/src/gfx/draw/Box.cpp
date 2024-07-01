#include "Box.hpp"

#include "../bind/VertexBuffer.hpp"
#include "../bind/IndexBuffer.hpp"
#include "../bind/TransformPS.hpp"
#include "../bind/Texture.hpp"
#include "../bind/DescriptorSet.hpp"
#include "../bind/UniformBuffer.hpp"
#include "../data/DefaultPipeline.hpp"

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
        std::uniform_real_distribution<float> radius_dist(0, 50);
        std::uniform_real_distribution<float> color_dist(0, 1);
        std::uniform_real_distribution<float> specular_intensity_dist(0, 1);
        std::uniform_real_distribution<float> specular_power_dist(0, 100);

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

        material.color = glm::vec4{color_dist(e), color_dist(e), color_dist(e), 1.0f};
        material.specular_intensity = specular_intensity_dist(e);
        material.specular_power = specular_power_dist(e);

        bind::Texture* p_texture{};
        if (!is_static_initialized())
        {
            struct Vertex 
            {
                glm::vec3 position;
                glm::vec3 normal;
                glm::vec2 uv;
            };

            std::vector<Vertex> vertices{
                {{-1.0f,-1.0f,-1.0f}, {}, {0.000059f, 0.000004f}}, // triangle 1 : begin
                {{-1.0f,-1.0f, 1.0f}, {}, {0.000103f, 0.336048f}},
                {{-1.0f, 1.0f, 1.0f}, {}, {0.335973f, 0.335903f}}, // triangle 1 : end
                {{1.0f, 1.0f,-1.0f,}, {}, {1.000023f, 0.000013f}},// triangle 2 : begin
                {{-1.0f,-1.0f,-1.0f}, {}, {0.667979f, 0.335851f}},
                {{-1.0f, 1.0f,-1.0f}, {}, {0.999958f, 0.336064f}}, // triangle 2 : end
                {{1.0f,-1.0f, 1.0f},  {}, {0.667979f, 0.335851f}},
                {{-1.0f,-1.0f,-1.0f}, {}, {0.336024f, 0.671877f}},
                {{1.0f,-1.0f,-1.0f},  {}, {0.667969f, 0.671889f}},
                {{1.0f, 1.0f,-1.0f},  {}, {1.000023f, 0.000013f}},
                {{1.0f,-1.0f,-1.0f},  {}, {0.668104f, 0.000013f}},
                {{-1.0f,-1.0f,-1.0f}, {}, {0.667979f, 0.335851f}},
                {{-1.0f,-1.0f,-1.0f}, {}, {0.000059f, 0.000004f}},
                {{-1.0f, 1.0f, 1.0f}, {}, {0.335973f, 0.335903f}},
                {{-1.0f, 1.0f,-1.0f}, {}, {0.336098f, 0.000071f}},
                {{1.0f,-1.0f, 1.0f},  {}, {0.667979f, 0.335851f}},
                {{-1.0f,-1.0f, 1.0f}, {}, {0.335973f, 0.335903f}},
                {{-1.0f,-1.0f,-1.0f}, {}, {0.336024f, 0.671877f}},
                {{-1.0f, 1.0f, 1.0f}, {}, {1.000004f, 0.671847f}},
                {{-1.0f,-1.0f, 1.0f}, {}, {0.999958f, 0.336064f}},
                {{1.0f,-1.0f, 1.0f},  {}, {0.667979f, 0.335851f}},
                {{1.0f, 1.0f, 1.0f},  {}, {0.668104f, 0.000013f}},
                {{1.0f,-1.0f,-1.0f},  {}, {0.335973f, 0.335903f}},
                {{1.0f, 1.0f,-1.0f},  {}, {0.667979f, 0.335851f}},
                {{1.0f,-1.0f,-1.0f},  {}, {0.335973f, 0.335903f}},
                {{1.0f, 1.0f, 1.0f},  {}, {0.668104f, 0.000013f}},
                {{1.0f,-1.0f, 1.0f},  {}, {0.336098f, 0.000071f}},
                {{1.0f, 1.0f, 1.0f},  {}, {0.000103f, 0.336048f}},
                {{1.0f, 1.0f,-1.0f},  {}, {0.000004f, 0.671870f}},
                {{-1.0f, 1.0f,-1.0f}, {}, {0.336024f, 0.671877f}},
                {{1.0f, 1.0f, 1.0f},  {}, {0.000103f, 0.336048f}},
                {{-1.0f, 1.0f,-1.0f}, {}, {0.336024f, 0.671877f}},
                {{-1.0f, 1.0f, 1.0f}, {}, {0.335973f, 0.335903f}},
                {{1.0f, 1.0f, 1.0f},  {}, {0.667969f, 0.671889f}},
                {{-1.0f, 1.0f, 1.0f}, {}, {1.000004f, 0.671847f}},
                {{1.0f,-1.0f, 1.0},   {}, {0.667979f, 0.335851f}},
            };

            
            std::vector<uint32_t> indices;
            for(uint32_t i = 0; i < vertices.size(); i++)
            {
                indices.push_back(i);
            }

            for(uint32_t i = 0; i < vertices.size(); i += 3)
            {
                glm::vec3 v1 = vertices[i + 1].position - vertices[i].position;
                glm::vec3 v2 = vertices[i + 2].position - vertices[i].position;

                glm::vec3 n = glm::normalize(glm::cross(v1, v2));

                vertices[i].normal = n;
                vertices[i + 1].normal = n;
                vertices[i + 2].normal = n;
            }

            
            auto image = utils::file_path_to_image("res/textures/x.jpg", 4);
            auto texture = std::make_unique<bind::Texture>(gfx, image);
            p_texture = texture.get();

            add_static_bind(std::move(texture));
            add_static_index_buffer(std::make_unique<bind::IndexBuffer>(gfx, indices));
            add_static_bind(std::make_unique<bind::VertexBuffer>(gfx, 0, vertices.size() * sizeof(Vertex), vertices.data()));
        }
        else
        {
            this->index_buffer = search_static<bind::IndexBuffer>();
            p_texture = search_static<bind::Texture>();
        }
        auto ubo = std::make_unique<bind::UniformBuffer>(gfx, sizeof(Material));
        ubo->update(&material);
        auto descriptor_set = std::make_unique<bind::DescriptorSet>(gfx, gfx.get_default_pipeline().get_pipeline_layout(), gfx.get_default_pipeline().get_instance_set_layout());
        descriptor_set->set_texture(gfx, 0, *p_texture);
        descriptor_set->set_buffer(gfx, 1, ubo->get(), ubo->get_size(), VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);


        add_bind(std::make_unique<bind::TransformPS>(gfx, gfx.get_default_pipeline().get_pipeline_layout(), *this));
        add_bind(std::move(ubo));
        add_bind(std::move(descriptor_set));
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