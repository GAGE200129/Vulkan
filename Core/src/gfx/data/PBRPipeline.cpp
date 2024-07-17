#include <pch.hpp>
#include "PBRPipeline.hpp"

#include "../Graphics.hpp"
#include "Camera.hpp"
#include "GBuffer.hpp"

#include <Core/src/utils/FileLoader.hpp>
#include <Core/src/utils/VulkanHelper.hpp>

namespace gage::gfx::data
{
    PBRPipeline::PBRPipeline(Graphics &gfx) : 
        gfx(gfx)                                                
    {

        create_pipeline_layout();
        create_pipeline();
        create_default_image_sampler();
    }

    PBRPipeline::~PBRPipeline()
    {
        
        destroy_default_image_sampler();
        destroy_pipeline();
        destroy_pipeline_layout();
    }

    void PBRPipeline::bind(VkCommandBuffer cmd) const
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
    }


    VkPipelineLayout PBRPipeline::get_layout() const
    {
        return pipeline_layout;
    }


    void PBRPipeline::set_push_constant(VkCommandBuffer cmd, const glm::mat4x4 &transform)
    {
        vkCmdPushConstants(cmd, pipeline_layout, VK_SHADER_STAGE_ALL, 0, sizeof(glm::mat4x4), &transform);
    }

    VkDescriptorSet PBRPipeline::allocate_instance_set(size_t size_in_bytes, VkBuffer buffer,
                                                              VkImageView albedo_view, VkSampler albedo_sampler,
                                                              VkImageView metalic_roughness_view, VkSampler metalic_roughness_sampler,
                                                              VkImageView normal_view, VkSampler normal_sampler) const
    {
        gfx.uploading_mutex.lock();
        VkDescriptorSet res{};
        VkDescriptorSetAllocateInfo alloc_info{};
        alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        alloc_info.descriptorSetCount = 1;
        alloc_info.descriptorPool = gfx.desc_pool;
        alloc_info.pSetLayouts = &set_layout;
        vk_check(vkAllocateDescriptorSets(gfx.device, &alloc_info, &res));

        // uniform buffer
        VkDescriptorBufferInfo buffer_desc_info{};
        buffer_desc_info.buffer = buffer;
        buffer_desc_info.offset = 0;
        buffer_desc_info.range = size_in_bytes;

        VkWriteDescriptorSet descriptor_write{};
        descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptor_write.dstSet = res;
        descriptor_write.dstBinding = 0;
        descriptor_write.dstArrayElement = 0;
        descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptor_write.descriptorCount = 1;
        descriptor_write.pBufferInfo = &buffer_desc_info;
        descriptor_write.pImageInfo = nullptr;
        descriptor_write.pTexelBufferView = nullptr;
        vkUpdateDescriptorSets(gfx.device, 1, &descriptor_write, 0, nullptr);

        // Albedo texture
        VkDescriptorImageInfo albedo_img_info{};
        albedo_img_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        albedo_img_info.imageView = albedo_view ? albedo_view : default_image_view;
        albedo_img_info.sampler = albedo_sampler ? albedo_sampler : default_sampler;

        descriptor_write.dstSet = res;
        descriptor_write.dstBinding = 1;
        descriptor_write.dstArrayElement = 0; // Array index 0
        descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptor_write.descriptorCount = 1;
        descriptor_write.pBufferInfo = nullptr;
        descriptor_write.pImageInfo = &albedo_img_info;
        descriptor_write.pTexelBufferView = nullptr;
        vkUpdateDescriptorSets(gfx.device, 1, &descriptor_write, 0, nullptr);

        // Metalic roughness texture
        VkDescriptorImageInfo metalic_roughness_img_info{};
        metalic_roughness_img_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        metalic_roughness_img_info.imageView = metalic_roughness_view ? metalic_roughness_view : default_image_view;
        metalic_roughness_img_info.sampler = metalic_roughness_sampler ? metalic_roughness_sampler : default_sampler;

        descriptor_write.dstSet = res;
        descriptor_write.dstBinding = 1;
        descriptor_write.dstArrayElement = 1; // Array index 1
        descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptor_write.descriptorCount = 1;
        descriptor_write.pBufferInfo = nullptr;
        descriptor_write.pImageInfo = &metalic_roughness_img_info;
        descriptor_write.pTexelBufferView = nullptr;
        vkUpdateDescriptorSets(gfx.device, 1, &descriptor_write, 0, nullptr);

        // Normal map texture
        VkDescriptorImageInfo normal_img_info{};
        normal_img_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        normal_img_info.imageView = normal_view ? normal_view : default_image_view;
        normal_img_info.sampler = normal_sampler ? normal_sampler : default_sampler;

        descriptor_write.dstSet = res;
        descriptor_write.dstBinding = 1;
        descriptor_write.dstArrayElement = 2; // Array index 1
        descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptor_write.descriptorCount = 1;
        descriptor_write.pBufferInfo = nullptr;
        descriptor_write.pImageInfo = &normal_img_info;
        descriptor_write.pTexelBufferView = nullptr;
        vkUpdateDescriptorSets(gfx.device, 1, &descriptor_write, 0, nullptr);

        gfx.uploading_mutex.unlock();

        return res;
    }

    void PBRPipeline::free_instance_set(VkDescriptorSet set) const
    {
        vkFreeDescriptorSets(gfx.device, gfx.desc_pool, 1, &set);
    }

    void PBRPipeline::create_default_image_sampler()
    {
        // Create default sampler
        VkSamplerCreateInfo sampler_info{};
        sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        sampler_info.magFilter = VK_FILTER_NEAREST;
        sampler_info.minFilter = VK_FILTER_NEAREST;
        sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        sampler_info.anisotropyEnable = VK_FALSE;
        sampler_info.maxAnisotropy = 0;
        sampler_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        sampler_info.unnormalizedCoordinates = VK_FALSE;
        sampler_info.compareEnable = VK_FALSE;
        sampler_info.compareOp = VK_COMPARE_OP_ALWAYS;
        sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
        sampler_info.mipLodBias = 0.0f;
        sampler_info.minLod = 0.0f;
        sampler_info.maxLod = 0.0f;

        vk_check(vkCreateSampler(gfx.device, &sampler_info, nullptr, &default_sampler));

        // Create default image and image view

        unsigned char image_data[] =
            {
                255, 255, 255, 255, 255, 255, 255, 255, 255,
                255, 255, 255, 255, 255, 255, 255, 255};
        VkImageCreateInfo img_ci = {};
        img_ci.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        img_ci.imageType = VK_IMAGE_TYPE_2D;
        img_ci.extent.width = 2;
        img_ci.extent.height = 2;
        img_ci.extent.depth = 1;
        img_ci.mipLevels = 1;
        img_ci.arrayLayers = 1;
        img_ci.format = VK_FORMAT_R8G8B8A8_UNORM;
        img_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
        img_ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        img_ci.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        img_ci.samples = VK_SAMPLE_COUNT_1_BIT;

        VmaAllocationCreateInfo alloc_ci = {};
        alloc_ci.usage = VMA_MEMORY_USAGE_AUTO;
        alloc_ci.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;

        vk_check(vmaCreateImage(gfx.allocator, &img_ci, &alloc_ci, &default_image, &default_image_alloc, nullptr));

        // Staging
        VkBufferCreateInfo staging_buffer_info = {};
        staging_buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        staging_buffer_info.size = sizeof(image_data);
        staging_buffer_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        VmaAllocationCreateInfo staging_alloc_info = {};
        staging_alloc_info.usage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST;
        staging_alloc_info.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;

        // Copy staging
        VkBuffer staging_buffer{};
        VmaAllocation staging_allocation{};
        vk_check(vmaCreateBuffer(gfx.allocator, &staging_buffer_info, &staging_alloc_info, &staging_buffer, &staging_allocation, nullptr));

        void *data;
        vmaMapMemory(gfx.allocator, staging_allocation, &data);
        std::memcpy(data, image_data, sizeof(image_data));
        vmaUnmapMemory(gfx.allocator, staging_allocation);

        // Copy to image

        // Allocate cmd buffer
        VkCommandBufferAllocateInfo cmd_alloc_info{};
        cmd_alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        cmd_alloc_info.commandPool = gfx.cmd_pool;
        cmd_alloc_info.commandBufferCount = 1;
        cmd_alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

        VkCommandBuffer cmd{};
        vk_check(vkAllocateCommandBuffers(gfx.device, &cmd_alloc_info, &cmd));

        VkCommandBufferBeginInfo transfer_begin_info{};
        transfer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        transfer_begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        vkBeginCommandBuffer(cmd, &transfer_begin_info);

        VkImageMemoryBarrier barrier = {};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

        barrier.image = default_image;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;

        vkCmdPipelineBarrier(
            cmd,
            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            0,
            0, nullptr,
            0, nullptr,
            1, &barrier);

        VkBufferImageCopy copy_region{};
        copy_region.bufferOffset = 0;
        copy_region.bufferRowLength = 0;
        copy_region.bufferImageHeight = 0;

        copy_region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        copy_region.imageSubresource.mipLevel = 0;
        copy_region.imageSubresource.baseArrayLayer = 0;
        copy_region.imageSubresource.layerCount = 1;

        copy_region.imageOffset = {0, 0, 0};
        copy_region.imageExtent = {
            2,
            2,
            1};

        vkCmdCopyBufferToImage(cmd, staging_buffer, default_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy_region);

        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        vkCmdPipelineBarrier(
            cmd,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            0,
            0, nullptr,
            0, nullptr,
            1, &barrier);

        vkEndCommandBuffer(cmd);

        VkSubmitInfo submit_info{};
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &cmd;

        vkQueueSubmit(gfx.queue, 1, &submit_info, nullptr);
        vkQueueWaitIdle(gfx.queue);

        VkImageViewCreateInfo view_info{};
        view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        view_info.image = default_image;
        view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        view_info.format = VK_FORMAT_R8G8B8A8_UNORM;
        view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        view_info.subresourceRange.baseMipLevel = 0;
        view_info.subresourceRange.levelCount = 1;
        view_info.subresourceRange.baseArrayLayer = 0;
        view_info.subresourceRange.layerCount = 1;

        vk_check(vkCreateImageView(gfx.device, &view_info, nullptr, &default_image_view));
        vmaDestroyBuffer(gfx.allocator, staging_buffer, staging_allocation);
    }

    void PBRPipeline::destroy_default_image_sampler()
    {
        vkDestroySampler(gfx.device, default_sampler, nullptr);
        vkDestroyImageView(gfx.device, default_image_view, nullptr);
        vmaDestroyImage(gfx.allocator, default_image, default_image_alloc);
    }

    void PBRPipeline::reset()
    {
        destroy_pipeline();
        create_pipeline();
    }

    void PBRPipeline::create_pipeline()
    {
        std::vector<VkVertexInputBindingDescription> vertex_bindings{
            {.binding = 0, .stride = (sizeof(float) * 3), .inputRate = VK_VERTEX_INPUT_RATE_VERTEX}, // position
            {.binding = 1, .stride = (sizeof(float) * 3), .inputRate = VK_VERTEX_INPUT_RATE_VERTEX}, // Normal
            {.binding = 2, .stride = (sizeof(float) * 2), .inputRate = VK_VERTEX_INPUT_RATE_VERTEX}, // Texcoord
        };

        std::vector<VkVertexInputAttributeDescription> vertex_attributes{
            {.location = 0, .binding = 0, .format = VK_FORMAT_R32G32B32_SFLOAT, .offset = 0},
            {.location = 1, .binding = 1, .format = VK_FORMAT_R32G32B32_SFLOAT, .offset = 0},
            {.location = 2, .binding = 2, .format = VK_FORMAT_R32G32_SFLOAT, .offset = 0},
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
        VkShaderModule geometry_shader{};
        VkShaderModule fragment_shader{};
        auto vertex_binary = utils::file_path_to_binary("Core/shaders/compiled/pbr.vert.spv");
        auto geometry_binary = utils::file_path_to_binary("Core/shaders/compiled/pbr.geom.spv");
        auto fragment_binary = utils::file_path_to_binary("Core/shaders/compiled/pbr.frag.spv");

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
        pipeline_info.renderPass = gfx.g_buffer->get_render_pass();

        vk_check(vkCreateGraphicsPipelines(gfx.device, nullptr, 1, &pipeline_info, nullptr, &pipeline));

        vkDestroyShaderModule(gfx.device, vertex_shader, nullptr);
        vkDestroyShaderModule(gfx.device, geometry_shader, nullptr);
        vkDestroyShaderModule(gfx.device, fragment_shader, nullptr);
    }
    void PBRPipeline::destroy_pipeline()
    {
        vkDestroyPipeline(gfx.device, pipeline, nullptr);
    }

    void PBRPipeline::create_pipeline_layout()
    {
        // PER INSTANCE SET LAYOUT
        std::vector<VkDescriptorSetLayoutBinding> instance_bindings{
            {.binding = 0, .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, .descriptorCount = 1, .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT, .pImmutableSamplers = nullptr},
            {.binding = 1, .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, .descriptorCount = 3, .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT, .pImmutableSamplers = nullptr},
        };

        VkDescriptorSetLayoutCreateInfo layout_ci{};
        layout_ci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layout_ci.bindingCount = instance_bindings.size();
        layout_ci.pBindings = instance_bindings.data();
        layout_ci.flags = 0;
        vk_check(vkCreateDescriptorSetLayout(gfx.device, &layout_ci, nullptr, &set_layout));
        std::vector<VkPushConstantRange> push_constants{
            VkPushConstantRange{
                VK_SHADER_STAGE_ALL,
                0,
                sizeof(glm::mat4x4)}};

        std::vector<VkDescriptorSetLayout> layouts = {gfx.global_set_layout, set_layout};
        VkPipelineLayoutCreateInfo pipeline_layout_info = {};
        pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipeline_layout_info.pushConstantRangeCount = push_constants.size();
        pipeline_layout_info.pPushConstantRanges = push_constants.data();
        pipeline_layout_info.pSetLayouts = layouts.data();
        pipeline_layout_info.setLayoutCount = layouts.size();
        vk_check(vkCreatePipelineLayout(gfx.device, &pipeline_layout_info, nullptr, &pipeline_layout));
    }
    void PBRPipeline::destroy_pipeline_layout()
    {
        vkDestroyPipelineLayout(gfx.device, pipeline_layout, nullptr);
        vkDestroyDescriptorSetLayout(gfx.device, set_layout, nullptr);
    }


    // void PBRPipeline::create_output_pass()
    // {
    //     // Create descriptor set layout
    //     {
    //         std::vector<VkDescriptorSetLayoutBinding> bindings{
    //             {.binding = 0, .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, .descriptorCount = 3, .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT, .pImmutableSamplers = nullptr},
    //         };

    //         VkDescriptorSetLayoutCreateInfo ci{};
    //         ci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    //         ci.bindingCount = bindings.size();
    //         ci.pBindings = bindings.data();
    //         vk_check(vkCreateDescriptorSetLayout(gfx.device, &ci, nullptr, &finalpass_desc_layout));
    //     }


    //     //Allocate descriptor set
    //     {
    //         //Allocate descriptor set
    //         VkDescriptorSetAllocateInfo ai{};
    //         ai.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    //         ai.descriptorPool = gfx.desc_pool;
    //         ai.pSetLayouts = &finalpass_desc_layout;
    //         ai.descriptorSetCount = 1;
    //         vk_check(vkAllocateDescriptorSets(gfx.device, &ai, &finalpass_desc));


    //         //Link to position g_buffer
    //         VkDescriptorImageInfo img_info{};
    //         img_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    //         img_info.sampler = default_sampler;

    //         VkWriteDescriptorSet descriptor_write{};
    //         descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;


    //         img_info.imageView = position_view;
    //         descriptor_write.dstSet = finalpass_desc;
    //         descriptor_write.dstBinding = 0;
    //         descriptor_write.dstArrayElement = 0;
    //         descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    //         descriptor_write.descriptorCount = 1;
    //         descriptor_write.pBufferInfo = nullptr;
    //         descriptor_write.pImageInfo = &img_info;
    //         descriptor_write.pTexelBufferView = nullptr;
    //         vkUpdateDescriptorSets(gfx.device, 1, &descriptor_write, 0, nullptr);

    //         //Link to normal of g buffer
    //         img_info.imageView = normal_view;
    //         descriptor_write.dstSet = finalpass_desc;
    //         descriptor_write.dstBinding = 0;
    //         descriptor_write.dstArrayElement = 1;
    //         descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    //         descriptor_write.descriptorCount = 1;
    //         descriptor_write.pBufferInfo = nullptr;
    //         descriptor_write.pImageInfo = &img_info;
    //         descriptor_write.pTexelBufferView = nullptr;
    //         vkUpdateDescriptorSets(gfx.device, 1, &descriptor_write, 0, nullptr);

    //         //Link to albedo g_buffer
    //         img_info.imageView = albedo_view;
    //         descriptor_write.dstSet = finalpass_desc;
    //         descriptor_write.dstBinding = 0;
    //         descriptor_write.dstArrayElement = 2;
    //         descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    //         descriptor_write.descriptorCount = 1;
    //         descriptor_write.pBufferInfo = nullptr;
    //         descriptor_write.pImageInfo = &img_info;
    //         descriptor_write.pTexelBufferView = nullptr;
    //         vkUpdateDescriptorSets(gfx.device, 1, &descriptor_write, 0, nullptr);
    //     }


    //      // Create renderpass
    //     {

    //         std::vector<VkAttachmentDescription> color_attachments = {
    //             {
    //                 .flags = 0,
    //                 .format = COLOR_FORMAT,
    //                 .samples = VK_SAMPLE_COUNT_1_BIT,
    //                 .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
    //                 .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
    //                 .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
    //                 .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
    //                 .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    //                 .finalLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
    //             },
    //         };

    //         std::vector<VkAttachmentReference> color_attachment_refs = {
    //             {.attachment = 0, .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
    //         };
    //         VkSubpassDescription subpass{};
    //         subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    //         subpass.colorAttachmentCount = color_attachment_refs.size();
    //         subpass.pColorAttachments = color_attachment_refs.data();

    //         std::vector<VkSubpassDependency> dependencies =
    //             {
    //                 {
    //                     .srcSubpass = VK_SUBPASS_EXTERNAL,
    //                     .dstSubpass = 0,
    //                     .srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
    //                     .dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
    //                     .srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
    //                     .dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT,
    //                     .dependencyFlags = 0,
    //                 },
    //                 {
    //                     .srcSubpass = 0,
    //                     .dstSubpass = VK_SUBPASS_EXTERNAL,
    //                     .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
    //                     .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
    //                     .srcAccessMask = 0,
    //                     .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT,
    //                     .dependencyFlags = 0,
    //                 }
    //         };

    //         VkRenderPassCreateInfo ci{};
    //         ci.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    //         ci.attachmentCount = color_attachments.size();
    //         ci.pAttachments = color_attachments.data();
    //         ci.subpassCount = 1;
    //         ci.pSubpasses = &subpass;
    //         ci.dependencyCount = dependencies.size();
    //         ci.pDependencies = dependencies.data();
    //         vk_check(vkCreateRenderPass(gfx.device, &ci, nullptr, &finalpass_renderpass));
    //     }

    //     // Create pipeline layout
    //     {
    //         VkPipelineLayoutCreateInfo ci{};
    //         ci.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    //         ci.pSetLayouts = &finalpass_desc_layout;
    //         ci.setLayoutCount = 1;

    //         vk_check(vkCreatePipelineLayout(gfx.device, &ci, nullptr, &finalpass_pipeline_layout));
    //     }


    //     // Create pipeline
    //     {

    //         VkPipelineVertexInputStateCreateInfo vertex_input_info = {};
    //         vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

    //         VkPipelineInputAssemblyStateCreateInfo input_assembly{};
    //         input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    //         input_assembly.primitiveRestartEnable = false;
    //         input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    //         VkPipelineRasterizationStateCreateInfo rasterizer{};
    //         rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    //         rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    //         rasterizer.lineWidth = 1.f;
    //         rasterizer.cullMode = VK_CULL_MODE_NONE;
    //         rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

    //         VkPipelineMultisampleStateCreateInfo multisampling{};
    //         multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    //         multisampling.sampleShadingEnable = VK_FALSE;
    //         multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    //         multisampling.minSampleShading = 1.0f;
    //         multisampling.pSampleMask = nullptr;
    //         multisampling.alphaToCoverageEnable = VK_FALSE;
    //         multisampling.alphaToOneEnable = VK_FALSE;

    //         VkPipelineDepthStencilStateCreateInfo depth_stencil{};
    //         depth_stencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    //         depth_stencil.depthTestEnable = VK_FALSE;
    //         depth_stencil.depthWriteEnable = VK_FALSE;
    //         depth_stencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    //         depth_stencil.depthBoundsTestEnable = VK_FALSE;
    //         depth_stencil.stencilTestEnable = VK_FALSE;
    //         depth_stencil.front = {};
    //         depth_stencil.back = {};
    //         depth_stencil.minDepthBounds = 0.0f;
    //         depth_stencil.maxDepthBounds = 1.f;

    //         VkViewport viewport = {};
    //         viewport.x = 0;
    //         viewport.y = 0;
    //         viewport.width = gfx.get_scaled_draw_extent().width;
    //         viewport.height = gfx.get_scaled_draw_extent().height;
    //         viewport.minDepth = 0.f;
    //         viewport.maxDepth = 1.f;

    //         VkRect2D scissor = {};
    //         scissor.offset.x = 0;
    //         scissor.offset.y = 0;
    //         scissor.extent.width = gfx.draw_extent.width;
    //         scissor.extent.height = gfx.draw_extent.height;

    //         VkPipelineViewportStateCreateInfo viewport_state{};
    //         viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    //         viewport_state.viewportCount = 1;
    //         viewport_state.scissorCount = 1;
    //         viewport_state.pViewports = &viewport;
    //         viewport_state.pScissors = &scissor;

    //         std::vector<VkPipelineColorBlendAttachmentState> blend_attachments =
    //             {
    //                 VkPipelineColorBlendAttachmentState{
    //                     .blendEnable = false,
    //                     .srcColorBlendFactor = VK_BLEND_FACTOR_ZERO,
    //                     .dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,
    //                     .colorBlendOp = VK_BLEND_OP_ADD,
    //                     .srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
    //                     .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
    //                     .alphaBlendOp = VK_BLEND_OP_ADD,
    //                     .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT},
    //             };

    //         VkPipelineColorBlendStateCreateInfo color_blending = {};
    //         color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    //         color_blending.logicOpEnable = VK_FALSE;
    //         color_blending.logicOp = VK_LOGIC_OP_COPY;
    //         color_blending.attachmentCount = blend_attachments.size();
    //         color_blending.pAttachments = blend_attachments.data();

    //         std::vector<VkPipelineShaderStageCreateInfo> pipeline_shader_stages{};
    //         VkShaderModule vertex_shader{};
    //         VkShaderModule fragment_shader{};
    //         auto vertex_binary = utils::file_path_to_binary("Core/shaders/compiled/basic_defered.vert.spv");
    //         auto fragment_binary = utils::file_path_to_binary("Core/shaders/compiled/basic_defered.frag.spv");

    //         VkShaderModuleCreateInfo shader_module_ci = {};
    //         shader_module_ci.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    //         VkPipelineShaderStageCreateInfo shader_stage_ci = {};
    //         shader_stage_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;

    //         // Vertex shader
    //         shader_module_ci.codeSize = vertex_binary.size();
    //         shader_module_ci.pCode = (uint32_t *)vertex_binary.data();
    //         vk_check(vkCreateShaderModule(gfx.device, &shader_module_ci, nullptr, &vertex_shader));
    //         shader_stage_ci.module = vertex_shader;
    //         shader_stage_ci.pName = "main";
    //         shader_stage_ci.stage = VK_SHADER_STAGE_VERTEX_BIT;
    //         pipeline_shader_stages.push_back(shader_stage_ci);

    //         // Fragment shader
    //         shader_module_ci.codeSize = fragment_binary.size();
    //         shader_module_ci.pCode = (uint32_t *)fragment_binary.data();
    //         vk_check(vkCreateShaderModule(gfx.device, &shader_module_ci, nullptr, &fragment_shader));
    //         shader_stage_ci.module = fragment_shader;
    //         shader_stage_ci.pName = "main";
    //         shader_stage_ci.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    //         pipeline_shader_stages.push_back(shader_stage_ci);

    //         VkPipelineDynamicStateCreateInfo dynamic_state_ci{};
    //         dynamic_state_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;

    //         VkGraphicsPipelineCreateInfo ci{};
    //         ci.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    //         ci.stageCount = (uint32_t)pipeline_shader_stages.size();
    //         ci.pStages = pipeline_shader_stages.data();
    //         ci.pVertexInputState = &vertex_input_info;
    //         ci.pInputAssemblyState = &input_assembly;
    //         ci.pViewportState = &viewport_state;
    //         ci.pRasterizationState = &rasterizer;
    //         ci.pMultisampleState = &multisampling;
    //         ci.pColorBlendState = &color_blending;
    //         ci.pDynamicState = &dynamic_state_ci;
    //         ci.pDepthStencilState = &depth_stencil;
    //         ci.layout = finalpass_pipeline_layout;
    //         ci.renderPass = finalpass_renderpass;

    //         vk_check(vkCreateGraphicsPipelines(gfx.device, VK_NULL_HANDLE, 1, &ci, nullptr, &finalpass_pipeline));

    //         vkDestroyShaderModule(gfx.device, vertex_shader, nullptr);
    //         vkDestroyShaderModule(gfx.device, fragment_shader, nullptr);
    //     }
    //     // Create Images
    //     {
    //         VkImageCreateInfo image_ci = {};
    //         image_ci.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    //         image_ci.imageType = VK_IMAGE_TYPE_2D;
    //         image_ci.extent.width = gfx.draw_extent.width;
    //         image_ci.extent.height = gfx.draw_extent.height;
    //         image_ci.extent.depth = 1;
    //         image_ci.mipLevels = 1;
    //         image_ci.arrayLayers = 1;
    //         image_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    //         image_ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    //         image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
    //         image_ci.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    //         image_ci.format = COLOR_FORMAT;
    //         vk_check(vkCreateImage(gfx.device, &image_ci, nullptr, &finalpass_image));

    //         VkMemoryRequirements mem_reqs{};
    //         vkGetImageMemoryRequirements(gfx.device, finalpass_image, &mem_reqs);
    //         VkMemoryAllocateInfo mem_alloc_info{};
    //         mem_alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    //         mem_alloc_info.allocationSize = mem_reqs.size;
    //         mem_alloc_info.memoryTypeIndex = utils::find_memory_type(gfx.physical_device, mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    //         vk_check(vkAllocateMemory(gfx.device, &mem_alloc_info, nullptr, &finalpass_image_memory));
    //         vk_check(vkBindImageMemory(gfx.device, finalpass_image, finalpass_image_memory, 0));

    //         VkImageViewCreateInfo image_view_ci = {};
    //         image_view_ci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    //         image_view_ci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    //         image_view_ci.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    //         image_view_ci.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    //         image_view_ci.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    //         image_view_ci.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    //         image_view_ci.subresourceRange.baseMipLevel = 0;
    //         image_view_ci.subresourceRange.baseArrayLayer = 0;
    //         image_view_ci.subresourceRange.layerCount = 1;
    //         image_view_ci.subresourceRange.levelCount = 1;
    //         image_view_ci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    //         image_view_ci.format = COLOR_FORMAT;
    //         image_view_ci.image = finalpass_image;
    //         vk_check(vkCreateImageView(gfx.device, &image_view_ci, nullptr, &finalpass_image_view));

            
            
    //     }
    //     //Create frame buffer
    //     {
    //         std::vector<VkImageView> attachments = {
    //             finalpass_image_view
    //         };
    //         VkFramebufferCreateInfo ci{};
    //         ci.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    //         ci.renderPass = finalpass_renderpass;
    //         ci.attachmentCount = attachments.size();
    //         ci.pAttachments = attachments.data();
    //         ci.width = gfx.draw_extent.width;
    //         ci.height = gfx.draw_extent.height;
    //         ci.layers = 1;
    //         vk_check(vkCreateFramebuffer(gfx.device, &ci, nullptr, &finalpass_framebuffer));
    //     }
    // }
    // void PBRPipeline::destroy_output_pass()
    // {
    //     vkDestroyDescriptorSetLayout(gfx.device, finalpass_desc_layout, nullptr);
    //     vkFreeDescriptorSets(gfx.device, gfx.desc_pool, 1, &finalpass_desc);
    //     vkDestroyPipelineLayout(gfx.device, finalpass_pipeline_layout, nullptr);
    //     vkDestroyPipeline(gfx.device, finalpass_pipeline, nullptr);
    //     vkDestroyImage(gfx.device, finalpass_image, nullptr);
    //     vkDestroyImageView(gfx.device, finalpass_image_view, nullptr);
    //     vkFreeMemory(gfx.device, finalpass_image_memory, nullptr);
    //     vkDestroyRenderPass(gfx.device, finalpass_renderpass, nullptr);
    //     vkDestroyFramebuffer(gfx.device, finalpass_framebuffer, nullptr);
    // }


}