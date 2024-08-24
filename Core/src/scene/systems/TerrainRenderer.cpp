#include <pch.hpp>
#include "TerrainRenderer.hpp"

#include <Core/src/gfx/Graphics.hpp>
#include <Core/src/gfx/data/Camera.hpp>
#include <Core/src/utils/FileLoader.hpp>
#include <Core/src/gfx/data/g_buffer/GBuffer.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "../scene.hpp"

namespace gage::scene::systems
{
    TerrainRenderer::TerrainRenderer(const gfx::Graphics &gfx, const gfx::data::Camera &camera) : gfx(gfx),
                                                                                            camera(camera)
    {
        create_pipeline();
        create_depth_pipeline();
    }
    TerrainRenderer::~TerrainRenderer()
    {
        vkDestroyPipelineLayout(gfx.device.device, depth_pipeline_layout, nullptr);
        vkDestroyPipeline(gfx.device.device, depth_pipeline, nullptr);

        vkDestroyDescriptorSetLayout(gfx.device.device, desc_layout, nullptr);
        vkDestroyPipelineLayout(gfx.device.device, pipeline_layout, nullptr);
        vkDestroyPipeline(gfx.device.device, pipeline, nullptr);
    }

    void TerrainRenderer::init()
    {
        // Init terrain renderers
        for (auto &terrain : terrains)
        {
            terrain.vertex_buffer =
                std::make_unique<gfx::data::GPUBuffer>(gfx,
                                                       VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                                                       terrain.terrain->vertex_data.size() * sizeof(components::Terrain::Vertex),
                                                       terrain.terrain->vertex_data.data());

            terrain.index_buffer =
                std::make_unique<gfx::data::GPUBuffer>(gfx,
                                                       VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                                                       terrain.terrain->indices_data.size() * sizeof(uint32_t),
                                                       terrain.terrain->indices_data.data());

            // Load test image
            {
                int w, h, comp;
                stbi_uc *data = stbi_load("res/textures/grass_tiled.jpg", &w, &h, &comp, STBI_rgb);
                if (!data)
                {
                    log().critical("Failed to terrain load image: {}", "res/textures/grass_tiled.jpg");
                    throw SceneException{};
                }

                uint32_t size_in_bytes = w * h * 3;
                gfx::data::ImageCreateInfo image_ci{data, w, h, 1, size_in_bytes, VK_FORMAT_R8G8B8_UNORM, VK_FILTER_NEAREST, VK_FILTER_NEAREST, VK_SAMPLER_ADDRESS_MODE_REPEAT};

                image_ci.mip_levels = std::floor(std::log2(std::max(w, h))) + 1;
                terrain.image = std::make_unique<gfx::data::Image>(gfx, image_ci);
                stbi_image_free(data);
            }

            // Create descriptor
            {
                // terrain.uniform_buffer_data.min_height = terrain.terrain->min_height;
                // terrain.uniform_buffer_data.max_height = terrain.terrain->max_height;
                // terrain.uniform_buffer_data.uv_scale = terrain.terrain->size;

                // terrain.uniform_buffer = std::make_unique<gfx::data::CPUBuffer>(gfx, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(Terrain::UniformBuffer), nullptr);
                terrain.descriptor = this->allocate_descriptor_set(terrain.image->get_image_view(), terrain.image->get_sampler());
            }
        }
    }
    void TerrainRenderer::shutdown()
    {
        for (const auto &terrain : terrains)
        {
            vkFreeDescriptorSets(gfx.device.device, gfx.desc_pool.pool, 1, &terrain.descriptor);
        }
        terrains.clear();
    }

    void TerrainRenderer::add_terrain(std::shared_ptr<components::Terrain> terrain)
    {
        Terrain additional_terrain_datas{};
        additional_terrain_datas.terrain = terrain;

        this->terrains.push_back(std::move(additional_terrain_datas));
    }

    void TerrainRenderer::render(VkCommandBuffer cmd) const
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

        for (const auto &terrain : terrains)
        {

            terrain.terrain->update_lod_regons(camera.position);

            // std::memcpy(terrain.uniform_buffer->get_mapped(), &terrain.uniform_buffer_data, sizeof(Terrain::UniformBuffer));
            VkBuffer buffers[] =
                {
                    terrain.vertex_buffer->get_buffer_handle()

                };
            VkDeviceSize offsets[] = {
                0};
            glm::mat4x4 transform(1.0f);
            vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, this->pipeline_layout, 1, 1, &terrain.descriptor, 0, nullptr);
            vkCmdPushConstants(cmd, this->pipeline_layout, VK_SHADER_STAGE_ALL, 0, sizeof(glm::mat4x4), glm::value_ptr(transform));
            vkCmdBindVertexBuffers(cmd, 0, 1, buffers, offsets);
            vkCmdBindIndexBuffer(cmd, terrain.index_buffer->get_buffer_handle(), 0, VK_INDEX_TYPE_UINT32);
            for (uint32_t patch_y = 0; patch_y < terrain.terrain->patch_count; patch_y++)
                for (uint32_t patch_x = 0; patch_x < terrain.terrain->patch_count; patch_x++)
                {
                    uint32_t x = patch_x * (terrain.terrain->patch_size - 1);
                    uint32_t y = patch_y * (terrain.terrain->patch_size - 1);

                    uint32_t lod = terrain.terrain->get_current_lod(patch_x, patch_y);
                    if (lod != 0)
                    {
                        auto proj = glm::perspectiveFovRH_ZO(glm::radians(camera.get_field_of_view()),
                                                             (float)gfx.get_scaled_draw_extent().width, (float)gfx.get_scaled_draw_extent().height,
                                                             0.001f, camera.get_far());
                        if (!terrain.terrain->is_inside_frustum(x, y, camera.get_view(), proj))
                        {
                            continue;
                        }
                    }
                    uint32_t base_vertex = x + y * terrain.terrain->size;
                    uint32_t base_index = terrain.terrain->lod_infos.at(lod).start;
                    uint32_t vertex_count = terrain.terrain->lod_infos.at(lod).count;
                    vkCmdDrawIndexed(cmd, vertex_count, 1, base_index, base_vertex, 0);
                }
        }
    }
    void TerrainRenderer::render_depth(VkCommandBuffer cmd) const
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

        for (const auto &terrain : terrains)
        {
            terrain.terrain->update_lod_regons(camera.position);

            VkBuffer buffers[] =
                {
                    terrain.vertex_buffer->get_buffer_handle()

                };
            VkDeviceSize offsets[] = {
                0};
            glm::mat4x4 transform(1.0f);
            vkCmdBindVertexBuffers(cmd, 0, 1, buffers, offsets);
            vkCmdBindIndexBuffer(cmd, terrain.index_buffer->get_buffer_handle(), 0, VK_INDEX_TYPE_UINT32);
            for (uint32_t patch_y = 0; patch_y < terrain.terrain->patch_count; patch_y++)
                for (uint32_t patch_x = 0; patch_x < terrain.terrain->patch_count; patch_x++)
                {
                    uint32_t x = patch_x * (terrain.terrain->patch_size - 1);
                    uint32_t y = patch_y * (terrain.terrain->patch_size - 1);

                    uint32_t lod = terrain.terrain->get_current_lod(patch_x, patch_y);
                    if (lod != 0)
                    {
                        auto proj = glm::perspectiveFovRH_ZO(glm::radians(camera.get_field_of_view()),
                                                             (float)gfx.get_scaled_draw_extent().width, (float)gfx.get_scaled_draw_extent().height,
                                                             0.001f, camera.get_far());
                        if (!terrain.terrain->is_inside_frustum(x, y, camera.get_view(), proj))
                        {
                            continue;
                        }
                    }
                    uint32_t base_vertex = x + y * terrain.terrain->size;
                    uint32_t base_index = terrain.terrain->lod_infos.at(lod).start;
                    uint32_t vertex_count = terrain.terrain->lod_infos.at(lod).count;
                    vkCmdDrawIndexed(cmd, vertex_count, 1, base_index, base_vertex, 0);
                }
        }
    }

    VkDescriptorSet TerrainRenderer::allocate_descriptor_set(VkImageView image_view, VkSampler sampler) const
    {
        VkDescriptorSet res{};
        VkDescriptorSetAllocateInfo alloc_info{};
        alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        alloc_info.descriptorSetCount = 1;
        alloc_info.descriptorPool = gfx.desc_pool.pool;
        alloc_info.pSetLayouts = &desc_layout;
        vk_check(vkAllocateDescriptorSets(gfx.device.device, &alloc_info, &res));

        // // Set 1 binding 0 = uniform buffer
        // VkDescriptorBufferInfo buffer_desc_info{};
        // buffer_desc_info.buffer = buffer;
        // buffer_desc_info.offset = 0;
        // buffer_desc_info.range = size_in_bytes;

        VkWriteDescriptorSet descriptor_write{};
        descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        // descriptor_write.dstSet = res;
        // descriptor_write.dstBinding = 0;
        // descriptor_write.dstArrayElement = 0;
        // descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        // descriptor_write.descriptorCount = 1;
        // descriptor_write.pBufferInfo = &buffer_desc_info;
        // descriptor_write.pImageInfo = nullptr;
        // descriptor_write.pTexelBufferView = nullptr;
        // vkUpdateDescriptorSets(gfx.device.device, 1, &descriptor_write, 0, nullptr);

        // Set 1 binding 0 = texture

        VkDescriptorImageInfo img_info{};
        img_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        img_info.imageView = image_view;
        img_info.sampler = sampler;

        descriptor_write.dstSet = res;
        descriptor_write.dstBinding = 0;
        descriptor_write.dstArrayElement = 0; // Array index 0
        descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptor_write.descriptorCount = 1;
        descriptor_write.pBufferInfo = nullptr;
        descriptor_write.pImageInfo = &img_info;
        descriptor_write.pTexelBufferView = nullptr;
        vkUpdateDescriptorSets(gfx.device.device, 1, &descriptor_write, 0, nullptr);

        return res;
    }

    void TerrainRenderer::create_pipeline()
    {
         // PER INSTANCE SET LAYOUT
        std::vector<VkDescriptorSetLayoutBinding> instance_bindings{
            {.binding = 0, .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, .descriptorCount = 1, .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT, .pImmutableSamplers = nullptr},
            //{.binding = 1, .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, .descriptorCount = 1, .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT, .pImmutableSamplers = nullptr},
        };

        VkDescriptorSetLayoutCreateInfo layout_ci{};
        layout_ci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layout_ci.bindingCount = instance_bindings.size();
        layout_ci.pBindings = instance_bindings.data();
        layout_ci.flags = 0;
        vk_check(vkCreateDescriptorSetLayout(gfx.device.device, &layout_ci, nullptr, &desc_layout));
        std::vector<VkPushConstantRange> push_constants{
            VkPushConstantRange{
                VK_SHADER_STAGE_ALL,
                0,
                sizeof(glm::mat4x4)}};

        std::vector<VkDescriptorSetLayout> layouts = {gfx.global_desc_layout.layout, desc_layout};
        VkPipelineLayoutCreateInfo pipeline_layout_info = {}; 
        pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipeline_layout_info.pushConstantRangeCount = push_constants.size();
        pipeline_layout_info.pPushConstantRanges = push_constants.data();
        pipeline_layout_info.pSetLayouts = layouts.data();
        pipeline_layout_info.setLayoutCount = layouts.size();
        vk_check(vkCreatePipelineLayout(gfx.device.device, &pipeline_layout_info, nullptr, &pipeline_layout));

        // Create pipelie
        std::vector<VkVertexInputBindingDescription> vertex_bindings{
            {.binding = 0, .stride = (sizeof(float) * 8), .inputRate = VK_VERTEX_INPUT_RATE_VERTEX},
            //{.binding = 0, .stride = (sizeof(float) * 5), .inputRate = VK_VERTEX_INPUT_RATE_VERTEX},
            //{.binding = 2, .stride = (sizeof(float) * 2), .inputRate = VK_VERTEX_INPUT_RATE_VERTEX},
        };

        std::vector<VkVertexInputAttributeDescription> vertex_attributes{
            {.location = 0, .binding = 0, .format = VK_FORMAT_R32G32B32_SFLOAT, .offset = 0},              // position
            {.location = 1, .binding = 0, .format = VK_FORMAT_R32G32_SFLOAT, .offset = sizeof(float) * 3}, // tex coord
            {.location = 2, .binding = 0, .format = VK_FORMAT_R32G32B32_SFLOAT, .offset = sizeof(float) * 5},
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
        rasterizer.cullMode = VK_CULL_MODE_NONE;
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
                // VkPipelineColorBlendAttachmentState{
                //     .blendEnable = false,
                //     .srcColorBlendFactor = VK_BLEND_FACTOR_ZERO,
                //     .dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,
                //     .colorBlendOp = VK_BLEND_OP_ADD,
                //     .srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
                //     .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
                //     .alphaBlendOp = VK_BLEND_OP_ADD,
                //     .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT},

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

        // setup dummy color blending. We arent using transparent objects yet
        // the blending is just "no blend", but we do write to the color attachment
        VkPipelineColorBlendStateCreateInfo color_blending = {};
        color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;

        color_blending.logicOpEnable = VK_FALSE;
        color_blending.logicOp = VK_LOGIC_OP_COPY;
        color_blending.attachmentCount = blend_attachments.size();
        color_blending.pAttachments = blend_attachments.data();

        // build the actual pipeline
        // we now use all of the info structs we have been writing into into this one
        // to create the pipeline

        std::vector<VkPipelineShaderStageCreateInfo> pipeline_shader_stages{};
        VkShaderModule vertex_shader{};
        // VkShaderModule geometry_shader{};
        VkShaderModule fragment_shader{};
        auto vertex_binary = utils::file_path_to_binary("Core/shaders/compiled/terrain.vert.spv");
        auto fragment_binary = utils::file_path_to_binary("Core/shaders/compiled/terrain.frag.spv");

        VkShaderModuleCreateInfo shader_module_ci = {};
        shader_module_ci.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        VkPipelineShaderStageCreateInfo shader_stage_ci = {};
        shader_stage_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;

        // Vertex shader
        shader_module_ci.codeSize = vertex_binary.size();
        shader_module_ci.pCode = (uint32_t *)vertex_binary.data();
        vk_check(vkCreateShaderModule(gfx.device.device, &shader_module_ci, nullptr, &vertex_shader));
        shader_stage_ci.module = vertex_shader;
        shader_stage_ci.pName = "main";
        shader_stage_ci.stage = VK_SHADER_STAGE_VERTEX_BIT;
        pipeline_shader_stages.push_back(shader_stage_ci);

        // Geometry shader
        // shader_module_ci.codeSize = geometry_binary.size();
        // shader_module_ci.pCode = (uint32_t *)geometry_binary.data();
        // vk_check(vkCreateShaderModule(gfx.device.device, &shader_module_ci, nullptr, &geometry_shader));
        // shader_stage_ci.module = geometry_shader;
        // shader_stage_ci.pName = "main";
        // shader_stage_ci.stage = VK_SHADER_STAGE_GEOMETRY_BIT;
        // pipeline_shader_stages.push_back(shader_stage_ci);

        // Fragment shader
        shader_module_ci.codeSize = fragment_binary.size();
        shader_module_ci.pCode = (uint32_t *)fragment_binary.data();
        vk_check(vkCreateShaderModule(gfx.device.device, &shader_module_ci, nullptr, &fragment_shader));
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

        // VkPipelineRenderingCreateInfo render_info{};
        // render_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
        // render_info.colorAttachmentCount = 1;
        // render_info.pColorAttachmentFormats = &color_attachment_format;
        // render_info.depthAttachmentFormat = depth_attachment_format;

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
        pipeline_info.renderPass = gfx.geometry_buffer.get_mainpass_render_pass();

        vk_check(vkCreateGraphicsPipelines(gfx.device.device, nullptr, 1, &pipeline_info, nullptr, &pipeline));

        vkDestroyShaderModule(gfx.device.device, vertex_shader, nullptr);
        // vkDestroyShaderModule(gfx.device.device, geometry_shader, nullptr);
        vkDestroyShaderModule(gfx.device.device, fragment_shader, nullptr);
    }


    void TerrainRenderer::create_depth_pipeline()
    {
        std::vector<VkPushConstantRange> push_constants{};

        std::vector<VkDescriptorSetLayout> layouts = {gfx.global_desc_layout.layout};
        VkPipelineLayoutCreateInfo pipeline_layout_info = {};
        pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipeline_layout_info.pushConstantRangeCount = push_constants.size();
        pipeline_layout_info.pPushConstantRanges = push_constants.data();
        pipeline_layout_info.pSetLayouts = layouts.data();
        pipeline_layout_info.setLayoutCount = layouts.size();
        vk_check(vkCreatePipelineLayout(gfx.device.device, &pipeline_layout_info, nullptr, &depth_pipeline_layout));

        // Create pipelie
        std::vector<VkVertexInputBindingDescription> vertex_bindings{
            {.binding = 0, .stride = (sizeof(float) * 8), .inputRate = VK_VERTEX_INPUT_RATE_VERTEX},
            //{.binding = 0, .stride = (sizeof(float) * 5), .inputRate = VK_VERTEX_INPUT_RATE_VERTEX},
            //{.binding = 2, .stride = (sizeof(float) * 2), .inputRate = VK_VERTEX_INPUT_RATE_VERTEX},
        };

        std::vector<VkVertexInputAttributeDescription> vertex_attributes{
            {.location = 0, .binding = 0, .format = VK_FORMAT_R32G32B32_SFLOAT, .offset = 0}, // position
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

        auto vertex_binary = utils::file_path_to_binary("Core/shaders/compiled/terrain_shadow.vert.spv");
        auto geometry_binary = utils::file_path_to_binary("Core/shaders/compiled/shadow.geom.spv");
        auto fragment_binary = utils::file_path_to_binary("Core/shaders/compiled/shadow.frag.spv");

        VkShaderModuleCreateInfo shader_module_ci = {};
        shader_module_ci.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        VkPipelineShaderStageCreateInfo shader_stage_ci = {};
        shader_stage_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;

        // Vertex shader
        shader_module_ci.codeSize = vertex_binary.size();
        shader_module_ci.pCode = (uint32_t *)vertex_binary.data();
        vk_check(vkCreateShaderModule(gfx.device.device, &shader_module_ci, nullptr, &vertex_shader));
        shader_stage_ci.module = vertex_shader;
        shader_stage_ci.pName = "main";
        shader_stage_ci.stage = VK_SHADER_STAGE_VERTEX_BIT;
        pipeline_shader_stages.push_back(shader_stage_ci);

        // Geometry shader
        shader_module_ci.codeSize = geometry_binary.size();
        shader_module_ci.pCode = (uint32_t *)geometry_binary.data();
        vk_check(vkCreateShaderModule(gfx.device.device, &shader_module_ci, nullptr, &geometry_shader));
        shader_stage_ci.module = geometry_shader;
        shader_stage_ci.pName = "main";
        shader_stage_ci.stage = VK_SHADER_STAGE_GEOMETRY_BIT;
        pipeline_shader_stages.push_back(shader_stage_ci);

        // Fragment shader
        shader_module_ci.codeSize = fragment_binary.size();
        shader_module_ci.pCode = (uint32_t *)fragment_binary.data();
        vk_check(vkCreateShaderModule(gfx.device.device, &shader_module_ci, nullptr, &fragment_shader));
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
        pipeline_info.renderPass = gfx.geometry_buffer.get_shadowpass_render_pass();

        vk_check(vkCreateGraphicsPipelines(gfx.device.device, nullptr, 1, &pipeline_info, nullptr, &depth_pipeline));

        vkDestroyShaderModule(gfx.device.device, vertex_shader, nullptr);
        vkDestroyShaderModule(gfx.device.device, geometry_shader, nullptr);
        vkDestroyShaderModule(gfx.device.device, fragment_shader, nullptr);
    }
}