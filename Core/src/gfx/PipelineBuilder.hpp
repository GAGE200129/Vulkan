#pragma once

#include <vector>
#include <vulkan/vulkan.hpp>

namespace gage::gfx
{
    class PipelineBuilder
    {
    public:
        PipelineBuilder();
        VkPipeline build(VkDevice device, VkPipelineLayout layout, VkExtent2D draw_extent);

        PipelineBuilder& set_vertex_layout(std::span<VkVertexInputBindingDescription> bindings, std::span<VkVertexInputAttributeDescription> attributes);
        PipelineBuilder& set_shaders(VkShaderModule vertex_shader, VkShaderModule fragment_shader);
        PipelineBuilder& set_topology(VkPrimitiveTopology topology);
        PipelineBuilder& set_polygon_mode(VkPolygonMode mode);
        PipelineBuilder& set_cull_mode(VkCullModeFlags cullMode, VkFrontFace frontFace);
        PipelineBuilder& set_multisampling_none();
        PipelineBuilder& set_blending_none();
        PipelineBuilder& set_color_attachment_format(VkFormat format);
        PipelineBuilder& set_depth_format(VkFormat format);
        PipelineBuilder& disable_depth_test();
        PipelineBuilder& enable_depth_test();
    private:
        std::vector<VkPipelineShaderStageCreateInfo> shader_stages{};

        VkPipelineVertexInputStateCreateInfo vertex_input_info{};
        VkPipelineInputAssemblyStateCreateInfo input_assembly{};
        VkPipelineRasterizationStateCreateInfo rasterizer{};
        VkPipelineColorBlendAttachmentState color_blend_attachment{};
        VkPipelineMultisampleStateCreateInfo multisampling{};
        VkPipelineDepthStencilStateCreateInfo depth_stencil{};
        VkPipelineRenderingCreateInfo render_info{};
        VkFormat color_attachment_format{};
    };
};