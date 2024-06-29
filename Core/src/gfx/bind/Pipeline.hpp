#pragma once

#include "IBindable.hpp"


#include <vector>
#include <tuple>
#include <vulkan/vulkan.h>
#include <span>
#include <string>
#include <unordered_map>


namespace gage::gfx::bind
{
    class Pipeline : public IBindable
    {
    public:
        Pipeline();
        void bind(Graphics& gfx);
        void destroy(Graphics& gfx);

        void build(Graphics& gfx);
        

        VkDescriptorSetLayout get_desc_set_layout();
        const VkPipelineLayout& get_layout() const;


        Pipeline& set_descriptor_set_bindings(std::span<VkDescriptorSetLayoutBinding> bindings);
        Pipeline& set_push_constants(std::span<VkPushConstantRange> ps);
        Pipeline& set_vertex_layout(std::span<VkVertexInputBindingDescription> bindings, std::span<VkVertexInputAttributeDescription> attributes);
        Pipeline& set_shaders(VkDevice device, std::vector<char> vertex_bin, std::vector<char> fragment_bin);
        Pipeline& set_vertex_shader(std::string file_path, std::string entry_point);
        Pipeline& set_fragment_shader(std::string file_path, std::string entry_point);
        Pipeline& set_topology(VkPrimitiveTopology topology);
        Pipeline& set_polygon_mode(VkPolygonMode mode);
        Pipeline& set_cull_mode(VkCullModeFlags cullMode, VkFrontFace frontFace);
        Pipeline& set_multisampling_none();
        Pipeline& set_blending_none();
        Pipeline& set_color_attachment_format(VkFormat format);
        Pipeline& set_depth_format(VkFormat format);
        Pipeline& disable_depth_test();
        Pipeline& enable_depth_test();
    private:
        VkPipelineLayout pipeline_layout{};
        VkPipeline pipeline{};
        VkDescriptorSetLayout descriptor_set_layout{};

        std::vector<std::tuple<std::string, std::string, VkShaderStageFlagBits>> shader_stages{};
        std::vector<VkDescriptorSetLayoutBinding> layout_bindings{}; // use only one descriptor set
        std::vector<VkPushConstantRange> push_constants{};
       

        VkPipelineVertexInputStateCreateInfo vertex_input_info{};
        VkPipelineInputAssemblyStateCreateInfo input_assembly{};
        VkPipelineRasterizationStateCreateInfo rasterizer{};
        VkPipelineColorBlendAttachmentState color_blend_attachment{};
        VkPipelineMultisampleStateCreateInfo multisampling{};
        VkPipelineDepthStencilStateCreateInfo depth_stencil{};
        VkPipelineRenderingCreateInfo render_info{};
        VkFormat color_attachment_format{};
    };
}