#include "PipelineBuilder.hpp"

#include "Exception.hpp"

namespace gage::gfx
{
    PipelineBuilder::PipelineBuilder()
    {
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
    }
    VkPipeline PipelineBuilder::build(VkDevice device, VkPipelineLayout layout, VkExtent2D draw_extent)
    {
        // make viewport state from our stored viewport and scissor.
        // at the moment we wont support multiple viewports or scissors

        VkViewport viewport = {};
        viewport.x = 0;
        viewport.y = 0;
        viewport.width = draw_extent.width;
        viewport.height = draw_extent.height;
        viewport.minDepth = 0.f;
        viewport.maxDepth = 1.f;

        VkRect2D scissor = {};
        scissor.offset.x = 0;
        scissor.offset.y = 0;
        scissor.extent.width = draw_extent.width;
        scissor.extent.height = draw_extent.height;


        VkPipelineViewportStateCreateInfo viewport_state = {};
        viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewport_state.pViewports = &viewport;
        viewport_state.pScissors = &scissor;
        viewport_state.viewportCount = 1;
        viewport_state.scissorCount = 1;
        // setup dummy color blending. We arent using transparent objects yet
        // the blending is just "no blend", but we do write to the color attachment
        VkPipelineColorBlendStateCreateInfo color_blending = {};
        color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;

        color_blending.logicOpEnable = VK_FALSE;
        color_blending.logicOp = VK_LOGIC_OP_COPY;
        color_blending.attachmentCount = 1;
        color_blending.pAttachments = &color_blend_attachment;

        // completely clear VertexInputStateCreateInfo, as we have no need for it
        VkPipelineVertexInputStateCreateInfo vertex_input_info = {};
        vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

        // build the actual pipeline
        // we now use all of the info structs we have been writing into into this one
        // to create the pipeline
        VkGraphicsPipelineCreateInfo pipeline_info = {};
        pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        // connect the renderInfo to the pNext extension mechanism
        pipeline_info.pNext = &render_info;

        pipeline_info.stageCount = (uint32_t)shader_stages.size();
        pipeline_info.pStages = shader_stages.data();
        pipeline_info.pVertexInputState = &vertex_input_info;
        pipeline_info.pInputAssemblyState = &input_assembly;
        pipeline_info.pViewportState = &viewport_state;
        pipeline_info.pRasterizationState = &rasterizer;
        pipeline_info.pMultisampleState = &multisampling;
        pipeline_info.pColorBlendState = &color_blending;
        pipeline_info.pDepthStencilState = &depth_stencil;
        pipeline_info.layout = layout;

        //VkDynamicState dyn_states[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};

        //VkPipelineDynamicStateCreateInfo dynamic_state_info = {};
        //dynamic_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;

        //dynamic_state_info.dynamicStateCount = 2;
        //dynamic_state_info.pDynamicStates = dyn_states;
        //pipeline_info.pDynamicState = &dynamic_state_info;
        VkPipeline pipeline;
        vk_check(vkCreateGraphicsPipelines(device, nullptr, 1, &pipeline_info, nullptr, &pipeline));

        return pipeline;
    }
    PipelineBuilder &PipelineBuilder::set_shaders(VkShaderModule vertex_shader, VkShaderModule fragment_shader)
    {
        shader_stages.clear();
        VkPipelineShaderStageCreateInfo shader_stage_ci = {};
        shader_stage_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;

        shader_stage_ci.module = vertex_shader;
        shader_stage_ci.pName = "main";
        shader_stage_ci.stage = VK_SHADER_STAGE_VERTEX_BIT;
        shader_stages.push_back(shader_stage_ci);

        shader_stage_ci.module = fragment_shader;
        shader_stage_ci.pName = "main";
        shader_stage_ci.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        shader_stages.push_back(shader_stage_ci);

        return *this;
    }

    PipelineBuilder &PipelineBuilder::set_topology(VkPrimitiveTopology topology)
    {
        input_assembly.topology = topology;
        // we are not going to use primitive restart on the entire tutorial so leave
        // it on false
        input_assembly.primitiveRestartEnable = VK_FALSE;

        return *this;
    }

    PipelineBuilder &PipelineBuilder::set_polygon_mode(VkPolygonMode mode)
    {
        rasterizer.polygonMode = mode;
        rasterizer.lineWidth = 1.f;

        return *this;
    }

    PipelineBuilder &PipelineBuilder::set_cull_mode(VkCullModeFlags cullMode, VkFrontFace frontFace)
    {
        rasterizer.cullMode = cullMode;
        rasterizer.frontFace = frontFace;
        return *this;
    }

    PipelineBuilder &PipelineBuilder::set_multisampling_none()
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

    PipelineBuilder &PipelineBuilder::set_blending_none()
    {
        // default write mask
        color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        // no blending
        color_blend_attachment.blendEnable = VK_FALSE;
        return *this;
    }

    PipelineBuilder &PipelineBuilder::set_color_attachment_format(VkFormat format)
    {   
        color_attachment_format = format;
        // connect the format to the renderInfo  structure
        render_info.colorAttachmentCount = 1;
        render_info.pColorAttachmentFormats = &color_attachment_format;
        return *this;
    }
    PipelineBuilder &PipelineBuilder::set_depth_format(VkFormat format)
    {
        render_info.depthAttachmentFormat = format;
        return *this;
    }

    PipelineBuilder& PipelineBuilder::disable_depth_test()
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
}