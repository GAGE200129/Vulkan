#pragma once

#include <vector>
#include <vulkan/vulkan.hpp>

namespace gage::gfx
{
    class PipelineBuilder
    {
    public:
        
    private:
        std::vector<VkPipelineShaderStageCreateInfo> shader_stages{};

        VkPipelineInputAssemblyStateCreateInfo input_assembly{};
        VkPipelineRasterizationStateCreateInfo rasterizer{};
        VkPipelineColorBlendAttachmentState color_blend_attachment{};
        VkPipelineMultisampleStateCreateInfo multisampling{};
        VkPipelineLayout pipeline_Layout{};
        VkPipelineDepthStencilStateCreateInfo depth_stencil{};
        VkPipelineRenderingCreateInfo render_info{};
        VkFormat color_attachment_format{};
    };
};