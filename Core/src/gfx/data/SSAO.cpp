#include <pch.hpp>
#include "SSAO.hpp"

#include "../Graphics.hpp"
#include "g_buffer/GBuffer.hpp"

#include <Core/src/utils/FileLoader.hpp>

namespace gage::gfx::data
{
    SSAO::SSAO(Graphics &gfx) : gfx(gfx)
    {
        generate_kernel_and_noises();
        create_pipeline();
        link_desc_to_g_buffer();
        link_desc_to_kernel_buffer();
    }
    SSAO::~SSAO()
    {
        vkDestroyDescriptorSetLayout(gfx.device, desc_layout, nullptr);
        vkDestroyPipelineLayout(gfx.device, pipeline_layout, nullptr);
        vkDestroyPipeline(gfx.device, pipeline, nullptr);
        vkFreeDescriptorSets(gfx.device, gfx.desc_pool, 1, &desc);
    }

    void SSAO::reset()
    {
        link_desc_to_g_buffer();
    }

    void SSAO::process(VkCommandBuffer cmd) const
    {
        VkViewport viewport = {};
        viewport.x = 0;
        viewport.y = 0;
        viewport.width = gfx.get_scaled_draw_extent().width;
        viewport.height = gfx.get_scaled_draw_extent().height;
        viewport.minDepth = 0.f;
        viewport.maxDepth = 1.f;

        VkRect2D scissor = {};
        scissor.offset.x = 0;
        scissor.offset.y = 0;
        scissor.extent.width = gfx.get_scaled_draw_extent().width;
        scissor.extent.height = gfx.get_scaled_draw_extent().height;

        PushConstantFragment ps_fs{
            .radius = gfx.ssao_radius,
            .bias = gfx.ssao_bias,
            .noise_scale = glm::vec2(gfx.draw_extent.width / 4.0, gfx.draw_extent.height / 4.0),
            .resolution_scale = gfx.draw_extent_scale};

        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, 1, &gfx.frame_datas[gfx.frame_index].global_set, 0, nullptr);
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 1, 1, &desc, 0, nullptr);
        vkCmdSetViewport(cmd, 0, 1, &viewport);
        vkCmdSetScissor(cmd, 0, 1, &scissor);
        vkCmdPushConstants(cmd, pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(float), &gfx.draw_extent_scale);
        vkCmdPushConstants(cmd, pipeline_layout, VK_SHADER_STAGE_FRAGMENT_BIT, 16, sizeof(PushConstantFragment), &ps_fs);
        vkCmdDraw(cmd, 3, 1, 0, 0);
    }

    void SSAO::link_desc_to_kernel_buffer()
    {
        VkDescriptorBufferInfo buffer_info{};
        buffer_info.buffer = kernel_buffer->get_buffer_handle();
        buffer_info.offset = 0;
        buffer_info.range = KERNEL_SIZE * sizeof(glm::vec4);

        VkWriteDescriptorSet descriptor_write{};
        descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptor_write.dstSet = desc;
        descriptor_write.dstBinding = 1;
        descriptor_write.dstArrayElement = 0;
        descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptor_write.descriptorCount = 1;
        descriptor_write.pBufferInfo = &buffer_info;
        descriptor_write.pImageInfo = nullptr;
        descriptor_write.pTexelBufferView = nullptr;
        vkUpdateDescriptorSets(gfx.device, 1, &descriptor_write, 0, nullptr);
    }
    void SSAO::link_desc_to_g_buffer()
    {
        VkDescriptorImageInfo img_info{};
        img_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        img_info.sampler = gfx.default_sampler;

        VkWriteDescriptorSet descriptor_write{};
        descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;

        // Link to depth g_buffer
        img_info.imageView = gfx.geometry_buffer->get_depth_view();
        descriptor_write.dstSet = desc;
        descriptor_write.dstBinding = 0;
        descriptor_write.dstArrayElement = 0;
        descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptor_write.descriptorCount = 1;
        descriptor_write.pBufferInfo = nullptr;
        descriptor_write.pImageInfo = &img_info;
        descriptor_write.pTexelBufferView = nullptr;
        vkUpdateDescriptorSets(gfx.device, 1, &descriptor_write, 0, nullptr);

        // Link to normal of g buffer
        img_info.imageView = gfx.geometry_buffer->get_normal_view();
        descriptor_write.dstSet = desc;
        descriptor_write.dstBinding = 0;
        descriptor_write.dstArrayElement = 1;
        descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptor_write.descriptorCount = 1;
        descriptor_write.pBufferInfo = nullptr;
        descriptor_write.pImageInfo = &img_info;
        descriptor_write.pTexelBufferView = nullptr;
        vkUpdateDescriptorSets(gfx.device, 1, &descriptor_write, 0, nullptr);

        // Link to noise texture
        img_info.imageView = image->get_image_view();
        descriptor_write.dstSet = desc;
        descriptor_write.dstBinding = 0;
        descriptor_write.dstArrayElement = 2;
        descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptor_write.descriptorCount = 1;
        descriptor_write.pBufferInfo = nullptr;
        descriptor_write.pImageInfo = &img_info;
        descriptor_write.pTexelBufferView = nullptr;
        vkUpdateDescriptorSets(gfx.device, 1, &descriptor_write, 0, nullptr);
    }

    void SSAO::create_pipeline()
    {
        // Create descriptor set layout
        {
            std::vector<VkDescriptorSetLayoutBinding> bindings{
                {.binding = 0, .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, .descriptorCount = 3, .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT, .pImmutableSamplers = nullptr},
                {.binding = 1, .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, .descriptorCount = 1, .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT, .pImmutableSamplers = nullptr},
            };

            VkDescriptorSetLayoutCreateInfo ci{};
            ci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            ci.bindingCount = bindings.size();
            ci.pBindings = bindings.data();
            vk_check(vkCreateDescriptorSetLayout(gfx.device, &ci, nullptr, &desc_layout));
        }
        // Allocate descriptor set
        {
            // Allocate descriptor set
            VkDescriptorSetAllocateInfo ai{};
            ai.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            ai.descriptorPool = gfx.desc_pool;
            ai.pSetLayouts = &desc_layout;
            ai.descriptorSetCount = 1;
            vk_check(vkAllocateDescriptorSets(gfx.device, &ai, &desc));
        }

        // Create pipeline layout
        {

            std::vector<VkDescriptorSetLayout> layouts = {gfx.global_set_layout, desc_layout};
            std::vector<VkPushConstantRange> push_constants =
                {
                    {.stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
                     .offset = 0,
                     .size = sizeof(float)},
                    {.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
                     .offset = 16,
                     .size = sizeof(PushConstantFragment)}};
            VkPipelineLayoutCreateInfo ci = {};
            ci.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
            ci.pSetLayouts = layouts.data();
            ci.setLayoutCount = layouts.size();
            ci.pushConstantRangeCount = push_constants.size();
            ci.pPushConstantRanges = push_constants.data();
            vk_check(vkCreatePipelineLayout(gfx.device, &ci, nullptr, &pipeline_layout));
        }

        // Create pipeline
        {

            VkPipelineVertexInputStateCreateInfo vertex_input_info = {};
            vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

            VkPipelineInputAssemblyStateCreateInfo input_assembly{};
            input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
            input_assembly.primitiveRestartEnable = false;
            input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

            VkPipelineRasterizationStateCreateInfo rasterizer{};
            rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
            rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
            rasterizer.lineWidth = 1.f;
            rasterizer.cullMode = VK_CULL_MODE_NONE;
            rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

            VkPipelineMultisampleStateCreateInfo multisampling{};
            multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
            multisampling.sampleShadingEnable = VK_FALSE;
            multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
            multisampling.minSampleShading = 1.0f;
            multisampling.pSampleMask = nullptr;
            multisampling.alphaToCoverageEnable = VK_FALSE;
            multisampling.alphaToOneEnable = VK_FALSE;

            VkPipelineDepthStencilStateCreateInfo depth_stencil{};
            depth_stencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
            depth_stencil.depthTestEnable = VK_FALSE;
            depth_stencil.depthWriteEnable = VK_FALSE;
            depth_stencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
            depth_stencil.depthBoundsTestEnable = VK_FALSE;
            depth_stencil.stencilTestEnable = VK_FALSE;
            depth_stencil.front = {};
            depth_stencil.back = {};
            depth_stencil.minDepthBounds = 0.0f;
            depth_stencil.maxDepthBounds = 1.f;

            VkViewport viewport = {};
            viewport.x = 0;
            viewport.y = 0;
            viewport.width = gfx.get_scaled_draw_extent().width;
            viewport.height = gfx.get_scaled_draw_extent().height;
            viewport.minDepth = 0.f;
            viewport.maxDepth = 1.f;

            VkRect2D scissor = {};
            scissor.offset.x = 0;
            scissor.offset.y = 0;
            scissor.extent.width = gfx.draw_extent.width;
            scissor.extent.height = gfx.draw_extent.height;

            VkPipelineViewportStateCreateInfo viewport_state{};
            viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
            viewport_state.viewportCount = 1;
            viewport_state.scissorCount = 1;
            viewport_state.pViewports = &viewport;
            viewport_state.pScissors = &scissor;

            std::vector<VkPipelineColorBlendAttachmentState> blend_attachments =
                {
                    VkPipelineColorBlendAttachmentState{
                        .blendEnable = false,
                        .srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
                        .dstColorBlendFactor = VK_BLEND_FACTOR_ONE,
                        .colorBlendOp = VK_BLEND_OP_ADD,
                        .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
                        .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
                        .alphaBlendOp = VK_BLEND_OP_ADD,
                        .colorWriteMask = VK_COLOR_COMPONENT_R_BIT},
                };

            VkPipelineColorBlendStateCreateInfo color_blending = {};
            color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
            color_blending.logicOpEnable = VK_FALSE;
            color_blending.logicOp = VK_LOGIC_OP_COPY;
            color_blending.attachmentCount = blend_attachments.size();
            color_blending.pAttachments = blend_attachments.data();

            // Constants

            struct SpecializationData
            {
                uint32_t kernel_size; 
            } specialization_data;
            specialization_data.kernel_size = KERNEL_SIZE;


            std::vector<VkSpecializationMapEntry> specialization_map_entries = 
            {
                VkSpecializationMapEntry{
                    .constantID = 0,
                    .offset = 0,
                    .size = sizeof(uint32_t),
                }
            };
        

            VkSpecializationInfo specialization_info{};
            specialization_info.dataSize = sizeof(specialization_data);
            specialization_info.mapEntryCount = specialization_map_entries.size();
            specialization_info.pMapEntries = specialization_map_entries.data();
            specialization_info.pData = &specialization_data;

            std::vector<VkPipelineShaderStageCreateInfo> pipeline_shader_stages{};
            VkShaderModule vertex_shader{};
            VkShaderModule fragment_shader{};
            auto vertex_binary = utils::file_path_to_binary("Core/shaders/compiled/vertex_generator.vert.spv");
            auto fragment_binary = utils::file_path_to_binary("Core/shaders/compiled/ssao.frag.spv");

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

            // Fragment shader
            shader_module_ci.codeSize = fragment_binary.size();
            shader_module_ci.pCode = (uint32_t *)fragment_binary.data();
            vk_check(vkCreateShaderModule(gfx.device, &shader_module_ci, nullptr, &fragment_shader));
            shader_stage_ci.module = fragment_shader;
            shader_stage_ci.pName = "main";
            shader_stage_ci.stage = VK_SHADER_STAGE_FRAGMENT_BIT;

            //Constant id = 0
            shader_stage_ci.pSpecializationInfo = &specialization_info;
            pipeline_shader_stages.push_back(shader_stage_ci);

            std::vector<VkDynamicState> dynamic_states =
                {
                    VK_DYNAMIC_STATE_VIEWPORT,
                    VK_DYNAMIC_STATE_SCISSOR};

            VkPipelineDynamicStateCreateInfo dynamic_state_ci{};
            dynamic_state_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
            dynamic_state_ci.dynamicStateCount = dynamic_states.size();
            dynamic_state_ci.pDynamicStates = dynamic_states.data();

            VkGraphicsPipelineCreateInfo ci{};
            ci.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
            ci.stageCount = (uint32_t)pipeline_shader_stages.size();
            ci.pStages = pipeline_shader_stages.data();
            ci.pVertexInputState = &vertex_input_info;
            ci.pInputAssemblyState = &input_assembly;
            ci.pViewportState = &viewport_state;
            ci.pRasterizationState = &rasterizer;
            ci.pMultisampleState = &multisampling;
            ci.pColorBlendState = &color_blending;
            ci.pDynamicState = &dynamic_state_ci;
            ci.pDepthStencilState = &depth_stencil;
            ci.layout = pipeline_layout;
            ci.renderPass = gfx.geometry_buffer->get_ssao_render_pass();

            vk_check(vkCreateGraphicsPipelines(gfx.device, VK_NULL_HANDLE, 1, &ci, nullptr, &pipeline));

            vkDestroyShaderModule(gfx.device, vertex_shader, nullptr);
            vkDestroyShaderModule(gfx.device, fragment_shader, nullptr);
        }
    }

    void SSAO::generate_kernel_and_noises()
    {
        auto lerp = [](float a, float b, float f)
        {
            return a + f * (b - a);
        };
        // Generate kernel
        std::uniform_real_distribution<float> random_floats(0.0, 1.0); // random floats between [0.0, 1.0]
        std::default_random_engine generator;
        kernel.reserve(KERNEL_SIZE);
        for (unsigned int i = 0; i < KERNEL_SIZE; ++i)
        {
            glm::vec3 sample(
                random_floats(generator) * 2.0 - 1.0,
                random_floats(generator) * 2.0 - 1.0,
                random_floats(generator));
            float scale = (float)i / 64.0;
            scale = lerp(0.1f, 1.0f, scale * scale);
            sample = glm::normalize(sample);
            sample *= random_floats(generator);
            sample *= scale;
            kernel.push_back(glm::vec4(sample, 0.0));
        }

        noises.reserve(16);
        for (unsigned int i = 0; i < 16; i++)
        {
            glm::vec3 noise(
                random_floats(generator) * 2.0 - 1.0,
                random_floats(generator) * 2.0 - 1.0,
                0.0f);
            noises.push_back(noise);
        }

        ImageCreateInfo image_ci{noises.data(), 4, 4, 1, 12 * 4 * 4, VK_FORMAT_R32G32B32_SFLOAT, VK_FILTER_NEAREST, VK_FILTER_NEAREST, VK_SAMPLER_ADDRESS_MODE_REPEAT};

        image = std::make_unique<Image>(gfx, image_ci);

        kernel_buffer = std::make_unique<GPUBuffer>(gfx, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(glm::vec4) * KERNEL_SIZE, kernel.data());
    }
}