#include <pch.hpp>
#include "Renderer.hpp"

#include "../data/Model.hpp"
#include "../Node.hpp"

#include <Core/src/gfx/data/PBRPipeline.hpp>
#include <Core/src/gfx/data/terrain/TerrainPipeline.hpp>
#include <Core/src/gfx/data/Camera.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace gage::scene::systems
{
    Renderer::Renderer(gfx::Graphics &gfx, const gfx::data::Camera &camera) : gfx(gfx), camera(camera)
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

        // Init terrain renderers
        for (auto &terrain_renderer : terrain_renderers)
        {
            terrain_renderer.vertex_buffer = std::make_unique<gfx::data::GPUBuffer>(gfx,
                                                                                    VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, terrain_renderer.terrain_renderer->vertex_data.size() * sizeof(components::TerrainRenderer::Vertex),
                                                                                    terrain_renderer.terrain_renderer->vertex_data.data());

            terrain_renderer.index_buffer = std::make_unique<gfx::data::GPUBuffer>(gfx,
                                                                                   VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                                                                                   terrain_renderer.terrain_renderer->indices_data.size() * sizeof(uint32_t),
                                                                                   terrain_renderer.terrain_renderer->indices_data.data());

            // Load test image
            {
                int w, h, comp;
                stbi_uc *data = stbi_load("res/textures/grass_tiled.jpg", &w, &h, &comp, STBI_rgb);
                if (!data)
                {
                    log().critical("Failed to terrain load image: {}", "res/textures/grass_tiled.jpg");
                    throw SceneException{};
                }

                uint32_t size_in_bytes = w * h * 3;
                gfx::data::ImageCreateInfo image_ci{data, w, h, 1, size_in_bytes, VK_FORMAT_R8G8B8_UNORM, VK_FILTER_NEAREST, VK_FILTER_NEAREST, VK_SAMPLER_ADDRESS_MODE_REPEAT};

                image_ci.mip_levels = std::floor(std::log2(std::max(w, h))) + 1;
                terrain_renderer.image = std::make_unique<gfx::data::Image>(gfx, image_ci);
                stbi_image_free(data);
            }

            // Create descriptor
            {
                terrain_renderer.uniform_buffer_data.min_height = terrain_renderer.terrain_renderer->min_height;
                terrain_renderer.uniform_buffer_data.max_height = terrain_renderer.terrain_renderer->max_height;
                terrain_renderer.uniform_buffer_data.uv_scale = terrain_renderer.terrain_renderer->size;

                terrain_renderer.uniform_buffer = std::make_unique<gfx::data::CPUBuffer>(gfx, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(TerrainRenderer::UniformBuffer), nullptr);
                terrain_renderer.descriptor = gfx.get_terrain_pipeline().allocate_descriptor_set(sizeof(TerrainRenderer::UniformBuffer),
                                                                                                 terrain_renderer.uniform_buffer->get_buffer_handle(),
                                                                                                 terrain_renderer.image->get_image_view(), terrain_renderer.image->get_sampler());
            }
        }
    }

    void Renderer::render_depth_terrain(VkCommandBuffer cmd, VkPipelineLayout pipeline_layout) const
    {
        for (const auto &terrain_renderer : terrain_renderers)
        {

            terrain_renderer.terrain_renderer->update_lod_regons(camera.position);

            std::memcpy(terrain_renderer.uniform_buffer->get_mapped(), &terrain_renderer.uniform_buffer_data, sizeof(TerrainRenderer::UniformBuffer));
            VkBuffer buffers[] =
                {
                    terrain_renderer.vertex_buffer->get_buffer_handle()

                };
            VkDeviceSize offsets[] = {
                0};
            glm::mat4x4 transform(1.0f);
            vkCmdBindVertexBuffers(cmd, 0, 1, buffers, offsets);
            vkCmdBindIndexBuffer(cmd, terrain_renderer.index_buffer->get_buffer_handle(), 0, VK_INDEX_TYPE_UINT32);
            for (uint32_t patch_y = 0; patch_y < terrain_renderer.terrain_renderer->patch_count; patch_y++)
                for (uint32_t patch_x = 0; patch_x < terrain_renderer.terrain_renderer->patch_count; patch_x++)
                {
                    uint32_t x = patch_x * (terrain_renderer.terrain_renderer->patch_size - 1);
                    uint32_t y = patch_y * (terrain_renderer.terrain_renderer->patch_size - 1);

                    uint32_t lod = terrain_renderer.terrain_renderer->get_current_lod(patch_x, patch_y);
                    if (lod != 0)
                    {   
                        auto proj = glm::perspectiveFovRH_ZO(glm::radians(camera.get_field_of_view()),
                             (float)gfx.get_scaled_draw_extent().width, (float)gfx.get_scaled_draw_extent().height,
                             0.001f, camera.get_far());
                        if (!terrain_renderer.terrain_renderer->is_inside_frustum(x, y, camera.get_view(), proj))
                        {
                            continue;
                        }
                    }
                    uint32_t base_vertex = x + y * terrain_renderer.terrain_renderer->size;
                    uint32_t base_index = terrain_renderer.terrain_renderer->lod_infos.at(lod).start;
                    uint32_t vertex_count = terrain_renderer.terrain_renderer->lod_infos.at(lod).count;
                    vkCmdDrawIndexed(cmd, vertex_count, 1, base_index, base_vertex, 0);
                }
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

            terrain_renderer.terrain_renderer->update_lod_regons(camera.position);

            std::memcpy(terrain_renderer.uniform_buffer->get_mapped(), &terrain_renderer.uniform_buffer_data, sizeof(TerrainRenderer::UniformBuffer));
            VkBuffer buffers[] =
                {
                    terrain_renderer.vertex_buffer->get_buffer_handle()

                };
            VkDeviceSize offsets[] = {
                0};
            glm::mat4x4 transform(1.0f);
            vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, gfx.get_terrain_pipeline().get_layout(), 1, 1, &terrain_renderer.descriptor, 0, nullptr);
            vkCmdPushConstants(cmd, gfx.get_terrain_pipeline().get_layout(), VK_SHADER_STAGE_ALL, 0, sizeof(glm::mat4x4), glm::value_ptr(transform));
            vkCmdBindVertexBuffers(cmd, 0, 1, buffers, offsets);
            vkCmdBindIndexBuffer(cmd, terrain_renderer.index_buffer->get_buffer_handle(), 0, VK_INDEX_TYPE_UINT32);
            for (uint32_t patch_y = 0; patch_y < terrain_renderer.terrain_renderer->patch_count; patch_y++)
                for (uint32_t patch_x = 0; patch_x < terrain_renderer.terrain_renderer->patch_count; patch_x++)
                {
                    uint32_t x = patch_x * (terrain_renderer.terrain_renderer->patch_size - 1);
                    uint32_t y = patch_y * (terrain_renderer.terrain_renderer->patch_size - 1);

                    uint32_t lod = terrain_renderer.terrain_renderer->get_current_lod(patch_x, patch_y);
                    if (lod != 0)
                    {   
                        auto proj = glm::perspectiveFovRH_ZO(glm::radians(camera.get_field_of_view()),
                             (float)gfx.get_scaled_draw_extent().width, (float)gfx.get_scaled_draw_extent().height,
                             0.001f, camera.get_far());
                        if (!terrain_renderer.terrain_renderer->is_inside_frustum(x, y, camera.get_view(), proj))
                        {
                            continue;
                        }
                    }
                    uint32_t base_vertex = x + y * terrain_renderer.terrain_renderer->size;
                    uint32_t base_index = terrain_renderer.terrain_renderer->lod_infos.at(lod).start;
                    uint32_t vertex_count = terrain_renderer.terrain_renderer->lod_infos.at(lod).count;
                    vkCmdDrawIndexed(cmd, vertex_count, 1, base_index, base_vertex, 0);
                }
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
            gfx.get_terrain_pipeline().free_descriptor_set(terrain_renderer.descriptor);
        }
    }

    void Renderer::add_pbr_mesh_renderer(std::unique_ptr<components::MeshRenderer> mesh_renderer)
    {
        this->mesh_renderers.push_back(std::move(mesh_renderer));
    }

    void Renderer::add_terrain_renderer(std::shared_ptr<components::TerrainRenderer> terrain_renderer)
    {
        TerrainRenderer additional_terrain_renderer_datas{};
        additional_terrain_renderer_datas.terrain_renderer = terrain_renderer;

        this->terrain_renderers.push_back(std::move(additional_terrain_renderer_datas));
    }
}