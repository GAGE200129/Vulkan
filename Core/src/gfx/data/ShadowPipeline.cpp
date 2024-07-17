#include <pch.hpp>
#include "ShadowPipeline.hpp"

#include "../Graphics.hpp"

#include <Core/src/utils/FileLoader.hpp>
#include <Core/src/utils/VulkanHelper.hpp>

#include "GBuffer.hpp"

namespace gage::gfx::data
{

    ShadowPipeline::ShadowPipeline(Graphics &gfx) : gfx(gfx)
    {

        create_pipeline_layout();
        create_pipeline();

    }

    ShadowPipeline::~ShadowPipeline()
    {
        destroy_pipeline();
        destroy_pipeline_layout();
    }

    void ShadowPipeline::bind(VkCommandBuffer cmd) const
    {

        // Dummy viewport state
        VkViewport viewport = {};
        viewport.x = 0;
        viewport.y = 0;
        viewport.width = gfx.directional_light_shadow_map_resolution;
        viewport.height = gfx.directional_light_shadow_map_resolution;
        viewport.minDepth = 0.f;
        viewport.maxDepth = 1.f;

        VkRect2D scissor = {};
        scissor.offset.x = 0;
        scissor.offset.y = 0;
        scissor.extent.width = gfx.directional_light_shadow_map_resolution;
        scissor.extent.height = gfx.directional_light_shadow_map_resolution;
        
        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, 1, &gfx.frame_datas[gfx.frame_index].global_set, 0, nullptr);
        vkCmdSetViewport(cmd, 0, 1, &viewport);
        vkCmdSetScissor(cmd, 0, 1, &scissor);
    }



    void ShadowPipeline::create_pipeline_layout()
    {
        VkPushConstantRange push_constant{
            VK_SHADER_STAGE_ALL,
            0,
            sizeof(glm::mat4x4)};

        VkPipelineLayoutCreateInfo pipeline_layout_info = {};
        pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipeline_layout_info.pushConstantRangeCount = 1;
        pipeline_layout_info.pPushConstantRanges = &push_constant;
        pipeline_layout_info.pSetLayouts = &gfx.global_set_layout;
        pipeline_layout_info.setLayoutCount = 1;
        vk_check(vkCreatePipelineLayout(gfx.device, &pipeline_layout_info, nullptr, &pipeline_layout));
    }
    void ShadowPipeline::destroy_pipeline_layout()
    {
        vkDestroyPipelineLayout(gfx.device, pipeline_layout, nullptr);
    }

    void ShadowPipeline::create_pipeline()
    {
        std::vector<VkVertexInputBindingDescription> vertex_bindings{
            {.binding = 0, .stride = (sizeof(float) * 3), .inputRate = VK_VERTEX_INPUT_RATE_VERTEX}, // position
        };

        std::vector<VkVertexInputAttributeDescription> vertex_attributes{
            {.location = 0, .binding = 0, .format = VK_FORMAT_R32G32B32_SFLOAT, .offset = 0},
        };

        VkPipelineVertexInputStateCreateInfo vertex_input_info = {};
        vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertex_input_info.vertexBindingDescriptionCount = vertex_bindings.size();
        vertex_input_info.pVertexBindingDescriptions = vertex_bindings.data();
        vertex_input_info.vertexAttributeDescriptionCount = vertex_attributes.size();
        vertex_input_info.pVertexAttributeDescriptions = vertex_attributes.data();

        VkPipelineInputAssemblyStateCreateInfo input_assembly{};
        input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        input_assembly.primitiveRestartEnable = false;
        input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

        VkPipelineRasterizationStateCreateInfo rasterizer{};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizer.lineWidth = 1.f;
        rasterizer.cullMode = VK_CULL_MODE_FRONT_BIT;
        rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

        VkPipelineMultisampleStateCreateInfo multisampling{};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = VK_FALSE;
        // multisampling defaulted to no multisampling (1 sample per pixel)
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        multisampling.minSampleShading = 1.0f;
        multisampling.pSampleMask = nullptr;
        // no alpha to coverage either
        multisampling.alphaToCoverageEnable = VK_FALSE;
        multisampling.alphaToOneEnable = VK_FALSE;

        VkPipelineDepthStencilStateCreateInfo depth_stencil{};
        depth_stencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depth_stencil.depthTestEnable = VK_TRUE;
        depth_stencil.depthWriteEnable = VK_TRUE;
        depth_stencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
        depth_stencil.depthBoundsTestEnable = VK_FALSE;
        depth_stencil.stencilTestEnable = VK_FALSE;
        depth_stencil.front = {};
        depth_stencil.back = {};
        depth_stencil.minDepthBounds = 0.0f;
        depth_stencil.maxDepthBounds = 1.f;

        // Dummy viewport state
        VkViewport viewport = {};
        viewport.x = 0;
        viewport.y = 0;
        viewport.width = gfx.directional_light_shadow_map_resolution;
        viewport.height = gfx.directional_light_shadow_map_resolution;
        viewport.minDepth = 0.f;
        viewport.maxDepth = 1.f;

        VkRect2D scissor = {};
        scissor.offset.x = 0;
        scissor.offset.y = 0;
        scissor.extent.width = gfx.directional_light_shadow_map_resolution;
        scissor.extent.height = gfx.directional_light_shadow_map_resolution;

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
        color_blending.attachmentCount = 0;
        color_blending.pAttachments = nullptr;

        // build the actual pipeline
        // we now use all of the info structs we have been writing into into this one
        // to create the pipeline

        std::vector<VkPipelineShaderStageCreateInfo> pipeline_shader_stages{};
        VkShaderModule vertex_shader{};
        VkShaderModule geometry_shader{};
        VkShaderModule fragment_shader{};
        
        auto vertex_binary = utils::file_path_to_binary("Core/shaders/compiled/shadow.vert.spv");
        auto geometry_binary = utils::file_path_to_binary("Core/shaders/compiled/shadow.geom.spv");
        auto fragment_binary = utils::file_path_to_binary("Core/shaders/compiled/shadow.frag.spv");

        VkShaderModuleCreateInfo shader_module_ci = {};
        shader_module_ci.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        VkPipelineShaderStageCreateInfo shader_stage_ci = {};
        shader_stage_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;

        // Vertex shader
        shader_module_ci.codeSize = vertex_binary.size();
        shader_module_ci.pCode = (uint32_t *)vertex_binary.data();
        vk_check(vkCreateShaderModule(gfx.device, &shader_module_ci, nullptr, &vertex_shader));
        shader_stage_ci.module = vertex_shader;
        shader_stage_ci.pName = "main";
        shader_stage_ci.stage = VK_SHADER_STAGE_VERTEX_BIT;
        pipeline_shader_stages.push_back(shader_stage_ci);

         // Geometry shader
        shader_module_ci.codeSize = geometry_binary.size();
        shader_module_ci.pCode = (uint32_t *)geometry_binary.data();
        vk_check(vkCreateShaderModule(gfx.device, &shader_module_ci, nullptr, &geometry_shader));
        shader_stage_ci.module = geometry_shader;
        shader_stage_ci.pName = "main";
        shader_stage_ci.stage = VK_SHADER_STAGE_GEOMETRY_BIT;
        pipeline_shader_stages.push_back(shader_stage_ci);

        // Fragment shader
        shader_module_ci.codeSize = fragment_binary.size();
        shader_module_ci.pCode = (uint32_t *)fragment_binary.data();
        vk_check(vkCreateShaderModule(gfx.device, &shader_module_ci, nullptr, &fragment_shader));
        shader_stage_ci.module = fragment_shader;
        shader_stage_ci.pName = "main";
        shader_stage_ci.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        pipeline_shader_stages.push_back(shader_stage_ci);

        std::vector<VkDynamicState> dynamic_states =
        {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR
        };

        VkPipelineDynamicStateCreateInfo dynamic_state_ci{};
        dynamic_state_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamic_state_ci.dynamicStateCount = dynamic_states.size();
        dynamic_state_ci.pDynamicStates = dynamic_states.data();


        VkGraphicsPipelineCreateInfo pipeline_info = {};
        pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        // pipeline_info.pNext = &render_info;
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
        pipeline_info.renderPass = gfx.g_buffer->get_shadowpass_render_pass();

        vk_check(vkCreateGraphicsPipelines(gfx.device, nullptr, 1, &pipeline_info, nullptr, &pipeline));

        vkDestroyShaderModule(gfx.device, vertex_shader, nullptr);
        vkDestroyShaderModule(gfx.device, geometry_shader, nullptr);
        vkDestroyShaderModule(gfx.device, fragment_shader, nullptr);
    }
    void ShadowPipeline::destroy_pipeline()
    {
        vkDestroyPipeline(gfx.device, pipeline, nullptr);
    }


    VkPipelineLayout ShadowPipeline::get_layout() const
    {
        return pipeline_layout;
    }
}