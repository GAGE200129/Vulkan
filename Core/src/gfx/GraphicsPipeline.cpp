#include "GraphicsPipeline.hpp"

#include <Core/src/utils/FileLoader.hpp>
#include <Core/src/log/Log.hpp>
#include <Core/src/utils/Exception.hpp>
#include "Exception.hpp"
#include "PipelineBuilder.hpp"


namespace gage::gfx
{
    GraphicsPipeline::GraphicsPipeline(VkDevice device, VkFormat color_attachment_format, VkExtent2D draw_extent, std::stack<std::function<void()>>& delete_stack)
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
        //Todo descriptor layout

	    vk_check(vkCreatePipelineLayout(device, &pipeline_layout_info, nullptr, &pipeline_layout));

        PipelineBuilder builder;
        //connecting the vertex and pixel shaders to the pipeline
        builder.set_shaders(vertex_shader, fragment_shader);
        //it will draw triangles
        builder.set_topology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
        //filled triangles
        builder.set_polygon_mode(VK_POLYGON_MODE_FILL);
        //no backface culling
        builder.set_cull_mode(VK_CULL_MODE_NONE, VK_FRONT_FACE_CLOCKWISE);
        //no multisampling
        builder.set_multisampling_none();
        //no blending
        builder.set_blending_none();
        //no depth testing
        builder.disable_depth_test();

        //connect the image format we will draw into, from draw image
        builder.set_color_attachment_format(color_attachment_format);
        builder.set_depth_format(VK_FORMAT_UNDEFINED);

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