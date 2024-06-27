#include "Pipeline.hpp"

#include "../Exception.hpp"



namespace gage::gfx::bind
{
    Pipeline::Pipeline(Graphics &gfx, PipelineBuilder& builder)
    {
        builder.set_color_attachment_format(get_swapchain_image_format(gfx))
            .set_depth_format(get_swapchain_depth_format(gfx));

        pipeline_layout = builder.build_layout(get_device(gfx), layouts);
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

        for(auto& [name, layout] : layouts)
        {
            vkDestroyDescriptorSetLayout(get_device(gfx), layout, nullptr);
        }
    }

    VkDescriptorSetLayout Pipeline::get_desc_set_layout(std::string name)
    {
        assert(layouts.find(name) != layouts.end());
        return layouts.at(name);
    }

    const VkPipelineLayout& Pipeline::get_layout() const
    {
        return pipeline_layout;
    }
}