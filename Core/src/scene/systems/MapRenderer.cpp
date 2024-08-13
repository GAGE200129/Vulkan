#include <pch.hpp>
#include "MapRenderer.hpp"

#include "../Node.hpp"

#include <Core/src/gfx/Graphics.hpp>
#include <Core/src/utils/FileLoader.hpp>
#include <Core/src/gfx/data/g_buffer/GBuffer.hpp>
#include <Core/src/gfx/data/GPUBuffer.hpp>

#include <glm/gtc/type_ptr.hpp>

namespace gage::scene::systems
{
    MapRenderer::MapRenderer(const gfx::Graphics &gfx) : gfx(gfx)
    {
        create_pipeline();
        create_depth_pipeline();
    }
    MapRenderer::~MapRenderer()
    {
        vkDestroyPipelineLayout(gfx.device, depth_pipeline_layout, nullptr);
        vkDestroyPipeline(gfx.device, depth_pipeline, nullptr);

        vkDestroyDescriptorSetLayout(gfx.device, desc_layout, nullptr);
        vkDestroyPipelineLayout(gfx.device, pipeline_layout, nullptr);
        vkDestroyPipeline(gfx.device, pipeline, nullptr);
    }

    void MapRenderer::init()
    {
        struct MapVertex
        {
            glm::vec3 position, normal;
        };
        for (auto &map : maps)
        {
            std::vector<MapVertex> vertices;
            map.vertex_count = 0;
            for (const auto &aabb_wall : map.map->aabb_walls)
            {
                glm::vec3 max = aabb_wall.a + aabb_wall.b;
                glm::vec3 min = aabb_wall.a - aabb_wall.b;
                glm::vec3 n_front = {0, 0, -1};
                glm::vec3 n_back = {0, 0, 1};
                glm::vec3 n_top = {0, 1, 0};
                glm::vec3 n_bottom = {0, -1, 0};


                // front
                vertices.push_back({min, n_front});
                vertices.push_back({{min.x, max.y, min.z}, n_front});
                vertices.push_back({{max.x, max.y, min.z}, n_front});
                map.vertex_count += 3;

                vertices.push_back({min, n_front});
                vertices.push_back({{max.x, max.y, min.z}, n_front});
                vertices.push_back({{max.x, min.y, min.z}, n_front});
                map.vertex_count += 3;

                //back
                vertices.push_back({max, n_back});
                vertices.push_back({{min.x, min.y, max.z}, n_back});
                vertices.push_back({{max.x, min.y, max.z}, n_back});
                map.vertex_count += 3;

                vertices.push_back({max, n_back});
                vertices.push_back({{min.x, max.y, max.z}, n_back});
                vertices.push_back({{min.x, min.y, max.z}, n_back});
                map.vertex_count += 3;

                //Top
                vertices.push_back({{min.x, max.y, min.z}, n_top});
                vertices.push_back({{min.x, max.y, max.z}, n_top});
                vertices.push_back({{max.x, max.y, min.z}, n_top});
                map.vertex_count += 3;

                vertices.push_back({max, n_top});
                vertices.push_back({{max.x, max.y, min.z}, n_top});
                vertices.push_back({{min.x, max.y, max.z}, n_top});
                map.vertex_count += 3;

                //Bottom
                vertices.push_back({{max.x, min.y, max.z}, n_bottom});
                vertices.push_back({{min.x, min.y, max.z}, n_bottom});
                vertices.push_back({{max.x, min.y, min.z}, n_bottom});
                map.vertex_count += 3;

                vertices.push_back({min, n_bottom});
                vertices.push_back({{max.x, min.y, min.z}, n_bottom});
                vertices.push_back({{min.x, min.y, max.z}, n_bottom});
                map.vertex_count += 3;

                //right
                vertices.push_back({{max.x, min.y, min.z}});
                vertices.push_back({{max.x, max.y, min.z}});
                vertices.push_back({{max.x, min.y, max.z}});
                map.vertex_count += 3;

                vertices.push_back({{max.x, min.y, max.z}});
                vertices.push_back({{max.x, max.y, min.z}});
                vertices.push_back({max});
                map.vertex_count += 3;

                //left
                vertices.push_back({{min.x, max.y, max.z}});
                vertices.push_back({{min.x, max.y, min.z}});
                vertices.push_back({{min.x, min.y, max.z}});
                map.vertex_count += 3;

                vertices.push_back({{min.x, min.y, max.z}});
                vertices.push_back({{min.x, max.y, min.z}});
                vertices.push_back({min});
                map.vertex_count += 3;
            }

            map.vertex_buffer = std::make_unique<gfx::data::GPUBuffer>(gfx, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, sizeof(MapVertex) * vertices.size(), vertices.data());
        }
    }
    void MapRenderer::shutdown()
    {
        maps.clear();
    }
    void MapRenderer::add_map(std::shared_ptr<components::Map> map)
    {
        Map additional_data{};
        additional_data.map = map;

        maps.push_back(std::move(additional_data));
    }
    void MapRenderer::render(VkCommandBuffer cmd) const
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

        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, 1, &gfx.frame_datas[gfx.frame_index].global_set, 0, nullptr);
        vkCmdSetViewport(cmd, 0, 1, &viewport);
        vkCmdSetScissor(cmd, 0, 1, &scissor);
        for (const auto &map : maps)
        {
            VkBuffer buffers[] =
                {
                    map.vertex_buffer->get_buffer_handle()
                };
            VkDeviceSize offsets[] =
                {0, 0, 0, 0, 0};

            vkCmdPushConstants(cmd, pipeline_layout, VK_SHADER_STAGE_ALL, 0, sizeof(glm::mat4x4), glm::value_ptr(map.map->node.global_transform));
            vkCmdBindVertexBuffers(cmd, 0, sizeof(buffers) / sizeof(buffers[0]), buffers, offsets);
            vkCmdDraw(cmd, map.vertex_count, 1, 0, 0);
        }
    }
    void MapRenderer::render_depth(VkCommandBuffer cmd) const
    {
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

        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, depth_pipeline);
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, depth_pipeline_layout, 0, 1, &gfx.frame_datas[gfx.frame_index].global_set, 0, nullptr);
        vkCmdSetViewport(cmd, 0, 1, &viewport);
        vkCmdSetScissor(cmd, 0, 1, &scissor);
        for (const auto &map : maps)
        {
            VkBuffer buffers[] =
                {
                    map.vertex_buffer->get_buffer_handle()
                };
            VkDeviceSize offsets[] =
                {0, 0, 0, 0, 0};
            vkCmdPushConstants(cmd, pipeline_layout, VK_SHADER_STAGE_ALL, 0, sizeof(glm::mat4x4), glm::value_ptr(map.map->node.global_transform));

            vkCmdBindVertexBuffers(cmd, 0, sizeof(buffers) / sizeof(buffers[0]), buffers, offsets);
            vkCmdDraw(cmd, map.vertex_count, 1, 0, 0);
        }
    }

    void MapRenderer::create_pipeline()
    {
        // PER INSTANCE SET LAYOUT
        std::vector<VkDescriptorSetLayoutBinding> instance_bindings{
            //{.binding = 0, .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, .descriptorCount = 1, .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT, .pImmutableSamplers = nullptr},
        };

        VkDescriptorSetLayoutCreateInfo layout_ci{};
        layout_ci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layout_ci.bindingCount = instance_bindings.size();
        layout_ci.pBindings = instance_bindings.data();
        layout_ci.flags = 0;
        vk_check(vkCreateDescriptorSetLayout(gfx.device, &layout_ci, nullptr, &desc_layout));
        std::vector<VkPushConstantRange> push_constants{
            VkPushConstantRange{
                VK_SHADER_STAGE_ALL,
                0,
                sizeof(glm::mat4x4)}};

        std::vector<VkDescriptorSetLayout> layouts = {gfx.global_set_layout, desc_layout};
        VkPipelineLayoutCreateInfo pipeline_layout_info = {};
        pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipeline_layout_info.pushConstantRangeCount = push_constants.size();
        pipeline_layout_info.pPushConstantRanges = push_constants.data();
        pipeline_layout_info.pSetLayouts = layouts.data();
        pipeline_layout_info.setLayoutCount = layouts.size();
        vk_check(vkCreatePipelineLayout(gfx.device, &pipeline_layout_info, nullptr, &pipeline_layout));

        // Create pipelie
        std::vector<VkVertexInputBindingDescription> vertex_bindings{
            {.binding = 0, .stride = (sizeof(float) * 6), .inputRate = VK_VERTEX_INPUT_RATE_VERTEX},
        };

        std::vector<VkVertexInputAttributeDescription> vertex_attributes{
            {.location = 0, .binding = 0, .format = VK_FORMAT_R32G32B32_SFLOAT, .offset = 0}, // position
            {.location = 1, .binding = 0, .format = VK_FORMAT_R32G32B32_SFLOAT, .offset = sizeof(float) * 3}, // normal
            // {.location = 2, .binding = 0, .format = VK_FORMAT_R32G32B32_SFLOAT, .offset = sizeof(float) * 5},
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
        rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
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
        depth_stencil.stencilTestEnable = VK_TRUE;
        depth_stencil.minDepthBounds = 0.0f;
        depth_stencil.maxDepthBounds = 1.f;

        depth_stencil.front = {};
        depth_stencil.front.reference = STENCIL_VALUE;
        depth_stencil.front.compareOp = VK_COMPARE_OP_ALWAYS;
        depth_stencil.front.compareMask = 0x00;
        depth_stencil.front.writeMask = 0xFF;
        depth_stencil.front.failOp = VK_STENCIL_OP_KEEP;
        depth_stencil.front.depthFailOp = VK_STENCIL_OP_KEEP;
        depth_stencil.front.passOp = VK_STENCIL_OP_REPLACE;
        depth_stencil.back = depth_stencil.front;

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

        std::vector<VkPipelineColorBlendAttachmentState> blend_attachments =
            {
                VkPipelineColorBlendAttachmentState{
                    .blendEnable = false,
                    .srcColorBlendFactor = VK_BLEND_FACTOR_ZERO,
                    .dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,
                    .colorBlendOp = VK_BLEND_OP_ADD,
                    .srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
                    .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
                    .alphaBlendOp = VK_BLEND_OP_ADD,
                    .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT},

                VkPipelineColorBlendAttachmentState{
                    .blendEnable = false,
                    .srcColorBlendFactor = VK_BLEND_FACTOR_ZERO,
                    .dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,
                    .colorBlendOp = VK_BLEND_OP_ADD,
                    .srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
                    .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
                    .alphaBlendOp = VK_BLEND_OP_ADD,
                    .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT},

                VkPipelineColorBlendAttachmentState{
                    .blendEnable = false,
                    .srcColorBlendFactor = VK_BLEND_FACTOR_ZERO,
                    .dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,
                    .colorBlendOp = VK_BLEND_OP_ADD,
                    .srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
                    .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
                    .alphaBlendOp = VK_BLEND_OP_ADD,
                    .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT},
            };

        VkPipelineColorBlendStateCreateInfo color_blending = {};
        color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;

        color_blending.logicOpEnable = VK_FALSE;
        color_blending.logicOp = VK_LOGIC_OP_COPY;
        color_blending.attachmentCount = blend_attachments.size();
        color_blending.pAttachments = blend_attachments.data();

        std::vector<VkPipelineShaderStageCreateInfo> pipeline_shader_stages{};
        VkShaderModule vertex_shader{};
        VkShaderModule fragment_shader{};
        auto vertex_binary = utils::file_path_to_binary("Core/shaders/compiled/map.vert.spv");
        auto fragment_binary = utils::file_path_to_binary("Core/shaders/compiled/map.frag.spv");

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
        pipeline_shader_stages.push_back(shader_stage_ci);

        std::vector<VkDynamicState> dynamic_states =
            {
                VK_DYNAMIC_STATE_VIEWPORT,
                VK_DYNAMIC_STATE_SCISSOR};

        VkPipelineDynamicStateCreateInfo dynamic_state_ci{};
        dynamic_state_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamic_state_ci.dynamicStateCount = dynamic_states.size();
        dynamic_state_ci.pDynamicStates = dynamic_states.data();

        VkGraphicsPipelineCreateInfo pipeline_info = {};
        pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
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
        pipeline_info.renderPass = gfx.geometry_buffer->get_mainpass_render_pass();

        vk_check(vkCreateGraphicsPipelines(gfx.device, nullptr, 1, &pipeline_info, nullptr, &pipeline));

        vkDestroyShaderModule(gfx.device, vertex_shader, nullptr);
        vkDestroyShaderModule(gfx.device, fragment_shader, nullptr);
    }

    void MapRenderer::create_depth_pipeline()
    {
        std::vector<VkPushConstantRange> push_constants{};

        std::vector<VkDescriptorSetLayout> layouts = {gfx.global_set_layout};
        VkPipelineLayoutCreateInfo pipeline_layout_info = {};
        pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipeline_layout_info.pushConstantRangeCount = push_constants.size();
        pipeline_layout_info.pPushConstantRanges = push_constants.data();
        pipeline_layout_info.pSetLayouts = layouts.data();
        pipeline_layout_info.setLayoutCount = layouts.size();
        vk_check(vkCreatePipelineLayout(gfx.device, &pipeline_layout_info, nullptr, &depth_pipeline_layout));

        // Create pipelie
        std::vector<VkVertexInputBindingDescription> vertex_bindings{
            {.binding = 0, .stride = (sizeof(float) * 6), .inputRate = VK_VERTEX_INPUT_RATE_VERTEX},
        };

        std::vector<VkVertexInputAttributeDescription> vertex_attributes{
            {.location = 0, .binding = 0, .format = VK_FORMAT_R32G32B32_SFLOAT, .offset = 0}, // position
            {.location = 1, .binding = 0, .format = VK_FORMAT_R32G32B32_SFLOAT, .offset = sizeof(float) * 3}, // normal
            // {.location = 2, .binding = 0, .format = VK_FORMAT_R32G32B32_SFLOAT, .offset = sizeof(float) * 5},
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
        color_blending.attachmentCount = 0;
        color_blending.pAttachments = nullptr;

        // build the actual pipeline
        // we now use all of the info structs we have been writing into into this one
        // to create the pipeline

        std::vector<VkPipelineShaderStageCreateInfo> pipeline_shader_stages{};
        VkShaderModule vertex_shader{};
        VkShaderModule geometry_shader{};
        VkShaderModule fragment_shader{};

        auto vertex_binary = utils::file_path_to_binary("Core/shaders/compiled/map_shadow.vert.spv");
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
                VK_DYNAMIC_STATE_SCISSOR};

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
        pipeline_info.layout = depth_pipeline_layout;
        pipeline_info.renderPass = gfx.geometry_buffer->get_shadowpass_render_pass();

        vk_check(vkCreateGraphicsPipelines(gfx.device, nullptr, 1, &pipeline_info, nullptr, &depth_pipeline));

        vkDestroyShaderModule(gfx.device, vertex_shader, nullptr);
        vkDestroyShaderModule(gfx.device, geometry_shader, nullptr);
        vkDestroyShaderModule(gfx.device, fragment_shader, nullptr);
    }

}