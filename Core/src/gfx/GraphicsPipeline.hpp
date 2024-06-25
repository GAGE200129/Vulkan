#pragma once


#include <vulkan/vulkan.h>
#include <stack>
#include <functional>

namespace gage::gfx
{
    class GraphicsPipeline
    {
    public:
        GraphicsPipeline(VkDevice device, VkFormat color_attachment_format, VkExtent2D draw_extent, std::stack<std::function<void()>>& delete_stack);

        VkPipeline get() const noexcept;
    private:
        VkPipelineLayout pipeline_layout;
        VkPipeline pipeline;
    };
}