#include <pch.hpp>
#include "MeshRenderer.hpp"

#include "../data/Model.hpp"
#include "../Node.hpp"


#include <Core/src/gfx/data/PBRPipeline.hpp>

#include <glm/gtc/type_ptr.hpp>

namespace gage::scene::components
{
    MeshRenderer::MeshRenderer(SceneGraph &scene, Node &node, gfx::Graphics &gfx, const data::Model &model, const data::ModelMesh &model_mesh, const data::ModelSkin* model_skin) : 
        IComponent(scene, node),
        gfx(gfx),
        model(model),
        model_mesh(model_mesh),
        model_skin(model_skin)

    {

        for(uint32_t i = 0; i < gfx::Graphics::FRAMES_IN_FLIGHT; i++)
        {
            
            animation_buffers[i] = std::make_unique<gfx::data::CPUBuffer>(gfx, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(AnimationBuffer), nullptr);
            animation_descs[i] = gfx.get_pbr_pipeline().allocate_animation_set(sizeof(AnimationBuffer), animation_buffers[i]->get_buffer_handle());

            AnimationBuffer* buffer_ptr = (AnimationBuffer*)(animation_buffers[i]->get_mapped());
            buffer_ptr->enabled = false;
        }
    }


    void MeshRenderer::render_depth(VkCommandBuffer cmd, VkPipelineLayout pipeline_layout)
    {
        for (const auto &section : model_mesh.sections)
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

   
            vkCmdPushConstants(cmd, pipeline_layout, VK_SHADER_STAGE_ALL, 0, sizeof(glm::mat4x4), glm::value_ptr(node.get_global_transform()));

            vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                    pipeline_layout,
                                    1,
                                    1, &animation_descs[gfx.get_current_frame_index()], 0, nullptr);

            vkCmdBindVertexBuffers(cmd, 0, sizeof(buffers) / sizeof(buffers[0]), buffers, offsets);
            vkCmdBindIndexBuffer(cmd, section.index_buffer->get_buffer_handle(), 0, VK_INDEX_TYPE_UINT32);
            vkCmdDrawIndexed(cmd, section.vertex_count, 1, 0, 0, 0);
        }
    }
    void MeshRenderer::render_geometry(VkCommandBuffer cmd, VkPipelineLayout pipeline_layout)
    {
        for (const auto &section : model_mesh.sections)
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

            const VkDescriptorSet &material_set = model.materials.at(section.material_index).descriptor_set;
            vkCmdPushConstants(cmd, pipeline_layout, VK_SHADER_STAGE_ALL, 0, sizeof(glm::mat4x4), glm::value_ptr(node.get_global_transform()));
            vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                    pipeline_layout,
                                    1,
                                    1, &material_set, 0, nullptr);
                                    
            vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                    pipeline_layout,
                                    2,
                                    1, &animation_descs[gfx.get_current_frame_index()], 0, nullptr);

            vkCmdBindVertexBuffers(cmd, 0, sizeof(buffers) / sizeof(buffers[0]), buffers, offsets);
            vkCmdBindIndexBuffer(cmd, section.index_buffer->get_buffer_handle(), 0, VK_INDEX_TYPE_UINT32);
            vkCmdDrawIndexed(cmd, section.vertex_count, 1, 0, 0, 0);
        }
    }

    void MeshRenderer::shutdown()
    {
        for(uint32_t i = 0; i < gfx::Graphics::FRAMES_IN_FLIGHT; i++)
        {
            gfx.get_pbr_pipeline().free_descriptor_set(animation_descs[i]);
        }
    }

    MeshRenderer::AnimationBuffer* MeshRenderer::get_animation_buffer()
    {
        return (AnimationBuffer*)animation_buffers[gfx.get_current_frame_index()]->get_mapped();
    }

    const data::ModelSkin* MeshRenderer::get_skin()
    {
        return model_skin;
    }
}