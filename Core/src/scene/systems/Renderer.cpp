#include <pch.hpp>
#include "Renderer.hpp"

#include "../data/Model.hpp"
#include "../Node.hpp"

#include <Core/src/gfx/data/PBRPipeline.hpp>
#include <Core/src/gfx/data/terrain/TerrainPipeline.hpp>
#include <glm/gtc/type_ptr.hpp>



namespace gage::scene::systems
{
    Renderer::Renderer(gfx::Graphics &gfx) : gfx(gfx)
    {
    }

    void Renderer::init()
    {
        for (const auto &mesh : mesh_renderers)
        {
            for (uint32_t i = 0; i < gfx::Graphics::FRAMES_IN_FLIGHT; i++)
            {

                mesh->animation_buffers[i] = std::make_unique<gfx::data::CPUBuffer>(gfx, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(components::MeshRenderer::AnimationBuffer), nullptr);
                mesh->animation_descs[i] = gfx.get_pbr_pipeline().allocate_animation_set(sizeof(components::MeshRenderer::AnimationBuffer), mesh->animation_buffers[i]->get_buffer_handle());

                components::MeshRenderer::AnimationBuffer *buffer_ptr = (components::MeshRenderer::AnimationBuffer *)(mesh->animation_buffers[i]->get_mapped());
                buffer_ptr->enabled = false;
            }
        }

        for (const auto &terrain_renderer : terrain_renderers)
        {
            const auto &height_map = terrain_renderer->height_map;
            const auto &size = terrain_renderer->size;
            const auto &min_height = terrain_renderer->min_height;
            const auto &max_height = terrain_renderer->max_height;
            const auto &scale = terrain_renderer->scale;
            auto &vertex_buffer = terrain_renderer->vertex_buffer;
            auto &index_buffer = terrain_renderer->index_buffer;
            auto &image = terrain_renderer->image;
            auto &terrain_data = terrain_renderer->terrain_data;
            auto &terrain_data_buffer = terrain_renderer->terrain_data_buffer;
            auto &terrain_data_desc = terrain_renderer->terrain_data_desc;

            

            // Generate triangle list and upload it to vulkan
            {
                std::vector<components::TerrainRenderer::Vertex> vertex_data;
                std::vector<uint32_t> indices_data;

                // Generate positions and uvs
                for (uint32_t y = 0; y < size; y++)
                    for (uint32_t x = 0; x < size; x++)
                    {
                        float height = height_map.at(x + y * size);
                        components::TerrainRenderer::Vertex v{};
                        v.pos = {x * scale, height, y * scale};
                        v.tex_coord = {(float)x / size, (float)y / size};
                        v.normal = {0, 1, 0};
                        vertex_data.push_back(std::move(v));
                    }

                // Generate indices
                uint32_t num_quad = (size - 1) * (size - 1);
                indices_data.resize(num_quad * 6);
                uint32_t index = 0;
                for (uint32_t y = 0; y < (size - 1); y++)
                    for (uint32_t x = 0; x < (size - 1); x++)
                    {
                        uint32_t index_bottom_left = y * size + x;
                        uint32_t index_top_left = (y + 1) * size + x;
                        uint32_t index_top_right = (y + 1) * size + x + 1;
                        uint32_t index_bottom_right = y * size + x + 1;

                        indices_data[index++] = index_bottom_left;
                        indices_data[index++] = index_top_left;
                        indices_data[index++] = index_top_right;

                        indices_data[index++] = index_bottom_left;
                        indices_data[index++] = index_top_right;
                        indices_data[index++] = index_bottom_right;
                    }

                // Calculate normals
                {
                    for (uint32_t i = 0; i < indices_data.size(); i += 3)
                    {
                        uint32_t i0 = indices_data.at(i + 0);
                        uint32_t i1 = indices_data.at(i + 1);
                        uint32_t i2 = indices_data.at(i + 2);
                        glm::vec3 v1 = vertex_data.at(i1).pos - vertex_data.at(i0).pos;
                        glm::vec3 v2 = vertex_data.at(i2).pos - vertex_data.at(i0).pos;
                        glm::vec3 n = glm::normalize(glm::cross(v1, v2));

                        vertex_data.at(i0).normal += n;
                        vertex_data.at(i1).normal += n;
                        vertex_data.at(i2).normal += n;
                    }

                    for (auto &v : vertex_data)
                    {
                        v.normal = glm::normalize(v.normal);
                    }
                }

                vertex_buffer = std::make_unique<gfx::data::GPUBuffer>(gfx, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, vertex_data.size() * sizeof(components::TerrainRenderer::Vertex), vertex_data.data());
                index_buffer = std::make_unique<gfx::data::GPUBuffer>(gfx, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, indices_data.size() * sizeof(uint32_t), indices_data.data());
            }

            // Load test image
            {
                int w, h, comp;
                stbi_uc *data = stbi_load("res/textures/grass_tiled.jpg", &w, &h, &comp, STBI_rgb);
                if (!data)
                {
                    log().critical("Failed to terrain load image: {}", "res/textures/grass_tiled.jpg");
                    throw SceneException{};
                }

                size_t size_in_bytes = w * h * 3;
                gfx::data::ImageCreateInfo image_ci{VK_FORMAT_R8G8B8_UNORM, VK_FILTER_NEAREST, VK_FILTER_NEAREST, VK_SAMPLER_ADDRESS_MODE_REPEAT};
                image = std::make_unique<gfx::data::Image>(gfx, data, w, h, size_in_bytes, image_ci);
                stbi_image_free(data);
            }

            // Create descriptor
            {
                terrain_data.min_height = min_height;
                terrain_data.max_height = max_height;
                terrain_data.uv_scale = size;

                terrain_data_buffer = std::make_unique<gfx::data::CPUBuffer>(gfx, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(components::TerrainRenderer::TerrainData), nullptr);
                terrain_data_desc = gfx.get_terrain_pipeline().allocate_descriptor_set(sizeof(components::TerrainRenderer::TerrainData),
                                                                                       terrain_data_buffer->get_buffer_handle(),
                                                                                       image->get_image_view(), image->get_sampler());
            }

           
        }
    }

    void Renderer::render_depth_terrain(VkCommandBuffer cmd, VkPipelineLayout pipeline_layout) const
    {
        for (const auto &terrain_renderer : terrain_renderers)
        {
            VkBuffer buffers[] =
                {
                    terrain_renderer->vertex_buffer->get_buffer_handle()};
            VkDeviceSize offsets[] = {
                0};
            glm::mat4x4 transform(1.0f);
            vkCmdBindVertexBuffers(cmd, 0, 1, buffers, offsets);
            vkCmdBindIndexBuffer(cmd, terrain_renderer->index_buffer->get_buffer_handle(), 0, VK_INDEX_TYPE_UINT32);
            vkCmdDrawIndexed(cmd, (terrain_renderer->size - 1) * (terrain_renderer->size - 1) * 6, 1, 0, 0, 0);
        }
    }
    void Renderer::render_depth(VkCommandBuffer cmd, VkPipelineLayout pipeline_layout) const
    {
        for (const auto &mesh : mesh_renderers)
        {
            // Update animation buffer
            std::memcpy(mesh->animation_buffers[gfx.get_current_frame_index()]->get_mapped(), &mesh->animation_buffer_data, sizeof(components::MeshRenderer::AnimationBuffer));
            for (const auto &section : mesh->model_mesh.sections)
            {
                if (section.material_index < 0)
                    continue;

                VkBuffer buffers[] =
                    {
                        section.position_buffer->get_buffer_handle(),
                        section.bone_id_buffer->get_buffer_handle(),
                        section.bone_weight_buffer->get_buffer_handle(),
                    };
                VkDeviceSize offsets[] =
                    {0, 0, 0};

                vkCmdPushConstants(cmd, pipeline_layout, VK_SHADER_STAGE_ALL, 0, sizeof(glm::mat4x4), glm::value_ptr(mesh->node.get_global_transform()));

                vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                        pipeline_layout,
                                        1,
                                        1, &mesh->animation_descs[gfx.get_current_frame_index()], 0, nullptr);

                vkCmdBindVertexBuffers(cmd, 0, sizeof(buffers) / sizeof(buffers[0]), buffers, offsets);
                vkCmdBindIndexBuffer(cmd, section.index_buffer->get_buffer_handle(), 0, VK_INDEX_TYPE_UINT32);
                vkCmdDrawIndexed(cmd, section.vertex_count, 1, 0, 0, 0);
            }
        }
    }
    void Renderer::render_geometry(VkCommandBuffer cmd, VkPipelineLayout pipeline_layout) const
    {
        for (const auto &mesh : mesh_renderers)
        {
            // Update animation buffer
            std::memcpy(mesh->animation_buffers[gfx.get_current_frame_index()]->get_mapped(), &mesh->animation_buffer_data, sizeof(components::MeshRenderer::AnimationBuffer));
            for (const auto &section : mesh->model_mesh.sections)
            {
                if (section.material_index < 0)
                    continue;

                VkBuffer buffers[] =
                    {
                        section.position_buffer->get_buffer_handle(),
                        section.normal_buffer->get_buffer_handle(),
                        section.texcoord_buffer->get_buffer_handle(),
                        section.bone_id_buffer->get_buffer_handle(),
                        section.bone_weight_buffer->get_buffer_handle(),
                    };
                VkDeviceSize offsets[] =
                    {0, 0, 0, 0, 0};

                // Build transform

                const VkDescriptorSet &material_set = mesh->model.materials.at(section.material_index).descriptor_set;
                vkCmdPushConstants(cmd, pipeline_layout, VK_SHADER_STAGE_ALL, 0, sizeof(glm::mat4x4), glm::value_ptr(mesh->node.get_global_transform()));
                vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                        pipeline_layout,
                                        1,
                                        1, &material_set, 0, nullptr);

                vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                        pipeline_layout,
                                        2,
                                        1, &mesh->animation_descs[gfx.get_current_frame_index()], 0, nullptr);

                vkCmdBindVertexBuffers(cmd, 0, sizeof(buffers) / sizeof(buffers[0]), buffers, offsets);
                vkCmdBindIndexBuffer(cmd, section.index_buffer->get_buffer_handle(), 0, VK_INDEX_TYPE_UINT32);
                vkCmdDrawIndexed(cmd, section.vertex_count, 1, 0, 0, 0);
            }
        }
    }

    void Renderer::render_geometry_terrain(VkCommandBuffer cmd, VkPipelineLayout pipeline_layout) const
    {
        for (const auto &terrain_renderer : terrain_renderers)
        {
            std::memcpy(terrain_renderer->terrain_data_buffer->get_mapped(), &terrain_renderer->terrain_data, sizeof(components::TerrainRenderer::TerrainData));
            VkBuffer buffers[] =
                {
                    terrain_renderer->vertex_buffer->get_buffer_handle()};
            VkDeviceSize offsets[] = {
                0};
            glm::mat4x4 transform(1.0f);
            vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, gfx.get_terrain_pipeline().get_layout(), 1, 1, &terrain_renderer->terrain_data_desc, 0, nullptr);
            vkCmdPushConstants(cmd, gfx.get_terrain_pipeline().get_layout(), VK_SHADER_STAGE_ALL, 0, sizeof(glm::mat4x4), glm::value_ptr(transform));
            vkCmdBindVertexBuffers(cmd, 0, 1, buffers, offsets);
            vkCmdBindIndexBuffer(cmd, terrain_renderer->index_buffer->get_buffer_handle(), 0, VK_INDEX_TYPE_UINT32);
            vkCmdDrawIndexed(cmd, (terrain_renderer->size - 1) * (terrain_renderer->size - 1) * 6, 1, 0, 0, 0);
        }
    }

    void Renderer::shutdown()
    {
        for (const auto &mesh : mesh_renderers)
        {
            for (uint32_t i = 0; i < gfx::Graphics::FRAMES_IN_FLIGHT; i++)
            {
                gfx.get_pbr_pipeline().free_descriptor_set(mesh->animation_descs[i]);
            }
        }

        for (const auto &terrain_renderer : terrain_renderers)
        {
            gfx.get_terrain_pipeline().free_descriptor_set(terrain_renderer->terrain_data_desc);
        }
    }

    void Renderer::add_pbr_mesh_renderer(std::unique_ptr<components::MeshRenderer> mesh_renderer)
    {
        this->mesh_renderers.push_back(std::move(mesh_renderer));
    }

    void Renderer::add_terrain_renderer(std::shared_ptr<components::TerrainRenderer> terrain_renderer)
    {
        this->terrain_renderers.push_back(terrain_renderer);
    }
}