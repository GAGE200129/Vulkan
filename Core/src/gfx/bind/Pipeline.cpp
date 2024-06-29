#include "Pipeline.hpp"

#include "../Exception.hpp"

#include <Core/src/utils/FileLoader.hpp>

namespace gage::gfx::bind
{
    Pipeline::Pipeline()
    {
        vertex_input_info = {};
        vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

        input_assembly = {};
        input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;

        rasterizer = {};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;

        color_blend_attachment = {};

        multisampling = {};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;

        depth_stencil = {};
        depth_stencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;

        render_info = {};
        render_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;

        shader_stages.clear();
        layout_bindings.clear();
        push_constants.clear();

        
    }
    void Pipeline::bind(Graphics &gfx)
    {
        VkViewport viewport = {};
        viewport.x = 0;
        viewport.y = 0;
        viewport.width = get_draw_extent(gfx).width;
        viewport.height = get_draw_extent(gfx).height;
        viewport.minDepth = 0.f;
        viewport.maxDepth = 1.f;

        VkRect2D scissor = {};
        scissor.offset.x = 0;
        scissor.offset.y = 0;
        scissor.extent.width = get_draw_extent(gfx).width;
        scissor.extent.height = get_draw_extent(gfx).height;

        vkCmdBindPipeline(get_cmd(gfx), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

        vkCmdSetViewport(get_cmd(gfx), 0, 1, &viewport);
        vkCmdSetScissor(get_cmd(gfx), 0, 1, &scissor);
    }
    void Pipeline::destroy(Graphics &gfx)
    {
        vkDestroyPipeline(get_device(gfx), pipeline, nullptr);
        vkDestroyPipelineLayout(get_device(gfx), pipeline_layout, nullptr);
        vkDestroyDescriptorSetLayout(get_device(gfx), descriptor_set_layout, nullptr);
    }

    void Pipeline::build(Graphics &gfx)
    {
        set_color_attachment_format(get_swapchain_image_format(gfx));
        set_depth_format(get_swapchain_depth_format(gfx));

        VkDescriptorSetLayoutCreateInfo layout_ci{};
        layout_ci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layout_ci.bindingCount = layout_bindings.size();
        layout_ci.pBindings = layout_bindings.data();
        layout_ci.flags = 0;
        vk_check(vkCreateDescriptorSetLayout(get_device(gfx), &layout_ci, nullptr, &descriptor_set_layout));

        VkPipelineLayoutCreateInfo pipeline_layout_info = {};
        pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipeline_layout_info.pushConstantRangeCount = push_constants.size();
        pipeline_layout_info.pPushConstantRanges = push_constants.data();
        pipeline_layout_info.pSetLayouts = &descriptor_set_layout;
        pipeline_layout_info.setLayoutCount = 1;
        vk_check(vkCreatePipelineLayout(get_device(gfx), &pipeline_layout_info, nullptr, &pipeline_layout));

        // Dummy viewport state
        VkViewport viewport = {};
        viewport.x = 0;
        viewport.y = 0;
        viewport.width = 0;
        viewport.height = 0;
        viewport.minDepth = 0.f;
        viewport.maxDepth = 1.f;

        VkRect2D scissor = {};
        scissor.offset.x = 0;
        scissor.offset.y = 0;
        scissor.extent.width = 0;
        scissor.extent.height = 0;

        VkPipelineViewportStateCreateInfo viewport_state{};
        viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewport_state.viewportCount = 1;
        viewport_state.scissorCount = 1;
        viewport_state.pViewports = &viewport;
        viewport_state.pScissors = &scissor;

        // setup dummy color blending. We arent using transparent objects yet
        // the blending is just "no blend", but we do write to the color attachment
        VkPipelineColorBlendStateCreateInfo color_blending = {};
        color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;

        color_blending.logicOpEnable = VK_FALSE;
        color_blending.logicOp = VK_LOGIC_OP_COPY;
        color_blending.attachmentCount = 1;
        color_blending.pAttachments = &color_blend_attachment;

        // build the actual pipeline
        // we now use all of the info structs we have been writing into into this one
        // to create the pipeline

        std::vector<VkPipelineShaderStageCreateInfo> pipeline_shader_stages{};
        std::vector<VkShaderModule> shader_modules{};

        for (const auto &stage : shader_stages)
        {
            VkShaderModule shader;
            auto &file_path = std::get<0>(stage);
            auto &entry_point = std::get<1>(stage);
            auto &shader_stage = std::get<2>(stage);

            auto binary = utils::file_path_to_binary(std::move(file_path));
            VkShaderModuleCreateInfo shader_ci = {};
            shader_ci.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
            shader_ci.codeSize = binary.size();
            shader_ci.pCode = (uint32_t *)binary.data();
            vk_check(vkCreateShaderModule(get_device(gfx), &shader_ci, nullptr, &shader));
            shader_modules.push_back(shader);
            VkPipelineShaderStageCreateInfo shader_stage_ci = {};
            shader_stage_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;

            shader_stage_ci.module = shader;
            shader_stage_ci.pName = entry_point.c_str();
            shader_stage_ci.stage = shader_stage;
            pipeline_shader_stages.push_back(shader_stage_ci);
        }

        const VkDynamicState dynamic_states[] =
            {
                VK_DYNAMIC_STATE_VIEWPORT,
                VK_DYNAMIC_STATE_SCISSOR,
            };

        VkPipelineDynamicStateCreateInfo dynamic_state_ci{};
        dynamic_state_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamic_state_ci.dynamicStateCount = sizeof(dynamic_states) / sizeof(dynamic_states[0]);
        dynamic_state_ci.pDynamicStates = dynamic_states;

        VkGraphicsPipelineCreateInfo pipeline_info = {};
        pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipeline_info.pNext = &render_info;
        pipeline_info.stageCount = (uint32_t)pipeline_shader_stages.size();
        pipeline_info.pStages = pipeline_shader_stages.data();
        pipeline_info.pVertexInputState = &vertex_input_info;
        pipeline_info.pInputAssemblyState = &input_assembly;
        pipeline_info.pViewportState = &viewport_state;
        pipeline_info.pRasterizationState = &rasterizer;
        pipeline_info.pMultisampleState = &multisampling;
        pipeline_info.pColorBlendState = &color_blending;
        pipeline_info.pDynamicState = &dynamic_state_ci;
        pipeline_info.pDepthStencilState = &depth_stencil;
        pipeline_info.layout = pipeline_layout;

        vk_check(vkCreateGraphicsPipelines(get_device(gfx), nullptr, 1, &pipeline_info, nullptr, &pipeline));

        for (const auto &shader : shader_modules)
        {
            vkDestroyShaderModule(get_device(gfx), shader, nullptr);
        }
    }

    VkDescriptorSetLayout Pipeline::get_desc_set_layout()
    {
        return descriptor_set_layout;
    }

    const VkPipelineLayout &Pipeline::get_layout() const
    {
        return pipeline_layout;
    }

    Pipeline &Pipeline::set_push_constants(std::span<VkPushConstantRange> ps)
    {
        for (const auto &push_constant : ps)
        {
            push_constants.push_back(push_constant);
        }
        return *this;
    }

    Pipeline &Pipeline::set_descriptor_set_bindings(std::span<VkDescriptorSetLayoutBinding> bindings)
    {
        layout_bindings.clear();
        for (const auto &binding : bindings)
        {
            layout_bindings.push_back(binding);
        }
        return *this;
    }

    Pipeline &Pipeline::set_vertex_shader(std::string file_path, std::string entry_point)
    {
        shader_stages.push_back(std::make_tuple(file_path, entry_point, VK_SHADER_STAGE_VERTEX_BIT));
        return *this;
    }
    Pipeline &Pipeline::set_fragment_shader(std::string file_path, std::string entry_point)
    {
        shader_stages.push_back(std::make_tuple(file_path, entry_point, VK_SHADER_STAGE_FRAGMENT_BIT));
        return *this;
    }

    Pipeline &Pipeline::set_vertex_layout(std::span<VkVertexInputBindingDescription> bindings, std::span<VkVertexInputAttributeDescription> attributes)
    {
        vertex_input_info.pVertexAttributeDescriptions = attributes.data();
        vertex_input_info.vertexAttributeDescriptionCount = attributes.size();
        vertex_input_info.pVertexBindingDescriptions = bindings.data();
        vertex_input_info.vertexBindingDescriptionCount = bindings.size();
        return *this;
    }

    Pipeline &Pipeline::set_topology(VkPrimitiveTopology topology)
    {
        input_assembly.topology = topology;
        // we are not going to use primitive restart on the entire tutorial so leave
        // it on false
        input_assembly.primitiveRestartEnable = VK_FALSE;

        return *this;
    }

    Pipeline &Pipeline::set_polygon_mode(VkPolygonMode mode)
    {
        rasterizer.polygonMode = mode;
        rasterizer.lineWidth = 1.f;

        return *this;
    }

    Pipeline &Pipeline::set_cull_mode(VkCullModeFlags cullMode, VkFrontFace frontFace)
    {
        rasterizer.cullMode = cullMode;
        rasterizer.frontFace = frontFace;
        return *this;
    }

    Pipeline &Pipeline::set_multisampling_none()
    {
        multisampling.sampleShadingEnable = VK_FALSE;
        // multisampling defaulted to no multisampling (1 sample per pixel)
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        multisampling.minSampleShading = 1.0f;
        multisampling.pSampleMask = nullptr;
        // no alpha to coverage either
        multisampling.alphaToCoverageEnable = VK_FALSE;
        multisampling.alphaToOneEnable = VK_FALSE;
        return *this;
    }

    Pipeline &Pipeline::set_blending_none()
    {
        // default write mask
        color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        // no blending
        color_blend_attachment.blendEnable = VK_FALSE;
        return *this;
    }

    Pipeline &Pipeline::set_color_attachment_format(VkFormat format)
    {
        color_attachment_format = format;
        // connect the format to the renderInfo  structure
        render_info.colorAttachmentCount = 1;
        render_info.pColorAttachmentFormats = &color_attachment_format;
        return *this;
    }
    Pipeline &Pipeline::set_depth_format(VkFormat format)
    {
        render_info.depthAttachmentFormat = format;
        return *this;
    }

    Pipeline &Pipeline::disable_depth_test()
    {
        depth_stencil.depthTestEnable = VK_FALSE;
        depth_stencil.depthWriteEnable = VK_FALSE;
        depth_stencil.depthCompareOp = VK_COMPARE_OP_NEVER;
        depth_stencil.depthBoundsTestEnable = VK_FALSE;
        depth_stencil.stencilTestEnable = VK_FALSE;
        depth_stencil.front = {};
        depth_stencil.back = {};
        depth_stencil.minDepthBounds = 0.f;
        depth_stencil.maxDepthBounds = 1.f;

        return *this;
    }

    Pipeline &Pipeline::enable_depth_test()
    {
        depth_stencil.depthTestEnable = VK_TRUE;
        depth_stencil.depthWriteEnable = VK_TRUE;
        depth_stencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
        depth_stencil.depthBoundsTestEnable = VK_FALSE;
        depth_stencil.stencilTestEnable = VK_FALSE;
        depth_stencil.front = {};
        depth_stencil.back = {};
        depth_stencil.minDepthBounds = 0.0f;
        depth_stencil.maxDepthBounds = 1.f;
        return *this;
    }
}