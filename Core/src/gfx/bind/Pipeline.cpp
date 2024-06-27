#include "Pipeline.hpp"

#include "../Exception.hpp"



namespace gage::gfx::bind
{
    Pipeline::Pipeline(Graphics &gfx, PipelineBuilder& builder)
    {
        builder.set_color_attachment_format(get_swapchain_image_format(gfx))
            .set_depth_format(get_swapchain_depth_format(gfx));

        VkPushConstantRange push_constant_range = {};
        push_constant_range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        push_constant_range.size = sizeof(float) * 16;
        push_constant_range.offset = 0;

        VkPipelineLayoutCreateInfo pipeline_layout_info = {};
        pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipeline_layout_info.pushConstantRangeCount = 1;
        pipeline_layout_info.pPushConstantRanges = &push_constant_range;

        vk_check(vkCreatePipelineLayout(get_device(gfx), &pipeline_layout_info, nullptr, &pipeline_layout));

    
        pipeline = builder.build(get_device(gfx), pipeline_layout, get_draw_extent(gfx));
    }
    void Pipeline::bind(Graphics &gfx)
    {
        vkCmdBindPipeline(get_cmd(gfx), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
    }
    void Pipeline::destroy(Graphics &gfx)
    {
        vkDestroyPipeline(get_device(gfx), pipeline, nullptr);
        vkDestroyPipelineLayout(get_device(gfx), pipeline_layout, nullptr);
    }

    const VkPipelineLayout& Pipeline::get_layout() const
    {
        return pipeline_layout;
    }
}