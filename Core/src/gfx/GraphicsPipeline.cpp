#include "GraphicsPipeline.hpp"

#include <Core/src/utils/FileLoader.hpp>
#include <Core/src/log/Log.hpp>
#include <Core/src/utils/Exception.hpp>
#include "Exception.hpp"
#include "PipelineBuilder.hpp"
#include "VertexBuffer.hpp"

namespace gage::gfx
{
    GraphicsPipeline::GraphicsPipeline(VkDevice device, VkFormat color_attachment_format, VkFormat depth_attachment_format, VkExtent2D draw_extent, std::stack<std::function<void()>>& delete_stack)
    {
        VkShaderModule vertex_shader, fragment_shader;
        try
        {
            auto vertex_binary = utils::file_path_to_binary("Core/shaders/compiled/colored_triangle.vert.spv");
            auto fragment_binary = utils::file_path_to_binary("Core/shaders/compiled/colored_triangle.frag.spv");

            VkShaderModuleCreateInfo shader_ci = {};
            shader_ci.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
            shader_ci.codeSize = vertex_binary.size();
            shader_ci.pCode = (uint32_t*)vertex_binary.data();
            vk_check(vkCreateShaderModule(device, &shader_ci, nullptr, &vertex_shader));
            
            shader_ci.codeSize = fragment_binary.size();
            shader_ci.pCode = (uint32_t*)fragment_binary.data();
            vk_check(vkCreateShaderModule(device, &shader_ci, nullptr, &fragment_shader));
        }
        catch (utils::FileLoaderException &e)
        {
            logger.error(std::string("Failed to load shader file: ") + e.what());
            throw GraphicsException{ "Failed to load shader module !"};
        }


        VkPipelineLayoutCreateInfo pipeline_layout_info = {};
        pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        
	    vk_check(vkCreatePipelineLayout(device, &pipeline_layout_info, nullptr, &pipeline_layout));

        VertexInputDescription vertex_description = VertexBuffer::get_vertex_description();
        PipelineBuilder builder{};
        builder.set_vertex_layout(vertex_description.bindings, vertex_description.attributes)
            .set_shaders(vertex_shader, fragment_shader)
            .set_topology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
            .set_polygon_mode(VK_POLYGON_MODE_FILL)
            .set_cull_mode(VK_CULL_MODE_NONE, VK_FRONT_FACE_CLOCKWISE)
            .set_multisampling_none()
            .set_blending_none()
            .enable_depth_test()
            .set_color_attachment_format(color_attachment_format)
            .set_depth_format(depth_attachment_format);

        pipeline = builder.build(device, pipeline_layout, draw_extent);

        vkDestroyShaderModule(device, vertex_shader, nullptr);
	    vkDestroyShaderModule(device, fragment_shader, nullptr);

        delete_stack.push([this, device]()
        {
            vkDestroyPipeline(device, pipeline, nullptr);
            vkDestroyPipelineLayout(device, pipeline_layout, nullptr);
        });
    }

    VkPipeline GraphicsPipeline::get() const noexcept
    {
        return pipeline;
    }
}