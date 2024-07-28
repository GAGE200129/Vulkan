#include <pch.hpp>
#include "Renderer.hpp"

#include "../data/Model.hpp"
#include "../Node.hpp"

#include <Core/src/gfx/data/PBRPipeline.hpp>
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
            for(uint32_t i = 0; i < gfx::Graphics::FRAMES_IN_FLIGHT; i++)
            {
                
                mesh->animation_buffers[i] = std::make_unique<gfx::data::CPUBuffer>(gfx, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(components::MeshRenderer::AnimationBuffer), nullptr);
                mesh->animation_descs[i] = gfx.get_pbr_pipeline().allocate_animation_set(sizeof(components::MeshRenderer::AnimationBuffer), mesh->animation_buffers[i]->get_buffer_handle());

                components::MeshRenderer::AnimationBuffer* buffer_ptr = (components::MeshRenderer::AnimationBuffer*)(mesh->animation_buffers[i]->get_mapped());
                buffer_ptr->enabled = false;
            }
        }
    }
    void Renderer::render_depth(VkCommandBuffer cmd, VkPipelineLayout pipeline_layout) const
    {
        for (const auto &mesh : mesh_renderers)
        {
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


    void Renderer::shutdown()
    {
        for (const auto &mesh : mesh_renderers)
        {
            for(uint32_t i = 0; i < gfx::Graphics::FRAMES_IN_FLIGHT; i++)
            {
                gfx.get_pbr_pipeline().free_descriptor_set(mesh->animation_descs[i]);
            }
        }
    }

    void Renderer::add_pbr_mesh_renderer(std::unique_ptr<components::MeshRenderer> mesh_renderer)
    {
        this->mesh_renderers.push_back(std::move(mesh_renderer));
    }
}