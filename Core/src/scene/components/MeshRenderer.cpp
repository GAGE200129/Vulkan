#include <pch.hpp>
#include "MeshRenderer.hpp"

#include "../data/Model.hpp"
#include "../Node.hpp"

#include <Core/src/gfx/Graphics.hpp>
#include <Core/src/gfx/data/PBRPipeline.hpp>

#include <glm/gtc/type_ptr.hpp>

namespace gage::scene::components
{
    void MeshRenderer::init()
    {

    }
    void MeshRenderer::update(float delta)
    {

    }
    void MeshRenderer::render(gfx::Graphics& gfx, VkCommandBuffer cmd, VkPipelineLayout pipeline_layout)
    {
        for (const auto& section : model_mesh.sections)
        {
            if (section.material_index < 0)
                continue;

            VkBuffer buffers[] =
                {
                    section.position_buffer->get_buffer_handle(),
                    section.normal_buffer->get_buffer_handle(),
                    section.texcoord_buffer->get_buffer_handle(),
                };
            VkDeviceSize offsets[] =
                {0, 0, 0};


            //Build transform 

            const VkDescriptorSet& desc_set = model.materials.at(section.material_index)->descriptor_set;
            vkCmdPushConstants(cmd, pipeline_layout, VK_SHADER_STAGE_ALL, 0, sizeof(glm::mat4x4), glm::value_ptr(node.get_global_transform()));
            vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                    gfx.get_pbr_pipeline().get_layout(),
                                    1,
                                    1, &desc_set, 0, nullptr);

            vkCmdBindVertexBuffers(cmd, 0, sizeof(buffers) / sizeof(buffers[0]), buffers, offsets);
            vkCmdBindIndexBuffer(cmd, section.index_buffer->get_buffer_handle(), 0, VK_INDEX_TYPE_UINT32);
            vkCmdDrawIndexed(cmd, section.vertex_count, 1, 0, 0, 0);
        }
    }
    void MeshRenderer::shutdown()
    {

    }
}