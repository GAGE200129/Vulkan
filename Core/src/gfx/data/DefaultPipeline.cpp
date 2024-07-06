#include "DefaultPipeline.hpp"

#include "../Graphics.hpp"
#include "../data/Swapchain.hpp"

#include <Core/src/utils/FileLoader.hpp>

#include <cstring>

namespace gage::gfx::data
{
    DefaultPipeline::DefaultPipeline(Graphics &gfx) : gfx(gfx)
    {
        create_pipeline_layout();
        create_pipeline();
        create_global_uniform_buffer();
        create_default_image_sampler();
    }

    DefaultPipeline::~DefaultPipeline()
    {
        destroy_default_image_sampler();
        destroy_global_uniform_buffer();
        destroy_pipeline();
        destroy_pipeline_layout();
    }

    void DefaultPipeline::bind(VkCommandBuffer cmd)
    {
        std::memcpy(global_alloc_info.pMappedData, &ubo, sizeof(GlobalUniform));
        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, 1, &global_set, 0, nullptr);
    }

    VkPipelineLayout DefaultPipeline::get_pipeline_layout() const
    {
        return pipeline_layout;
    }

    void DefaultPipeline::set_push_constant(VkCommandBuffer cmd, const glm::mat4x4 &transform)
    {
        vkCmdPushConstants(cmd, pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4x4), &transform);
    }

    VkDescriptorSet DefaultPipeline::allocate_instance_set(size_t size_in_bytes, VkBuffer buffer,
                                                           VkImageView albedo_view, VkSampler albedo_sampler,
                                                           VkImageView metalic_roughness_view, VkSampler metalic_roughness_sampler) const
    {
        gfx.uploading_mutex.lock();
        VkDescriptorSet res{};
        VkDescriptorSetAllocateInfo alloc_info{};
        alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        alloc_info.descriptorSetCount = 1;
        alloc_info.descriptorPool = gfx.desc_pool;
        alloc_info.pSetLayouts = &instance_set_layout;
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

        gfx.uploading_mutex.unlock();

        return res;
    }

    void DefaultPipeline::free_instance_set(VkDescriptorSet set) const
    {
        vkFreeDescriptorSets(gfx.device, gfx.desc_pool, 1, &set);
    }

    void DefaultPipeline::create_default_image_sampler()
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
                0, 255, 0, 255, 255, 0, 0, 255,
                255, 255, 0, 255, 255, 0, 255, 255};
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

    void DefaultPipeline::destroy_default_image_sampler()
    {
        vkDestroySampler(gfx.device, default_sampler, nullptr);
        vkDestroyImageView(gfx.device, default_image_view, nullptr);
        vmaDestroyImage(gfx.allocator, default_image, default_image_alloc);
    }

    void DefaultPipeline::reset_pipeline()
    {
        destroy_pipeline();
        create_pipeline();
    }

    void DefaultPipeline::create_pipeline()
    {
        std::vector<VkVertexInputBindingDescription> vertex_bindings{
            {.binding = 0, .stride = (sizeof(float) * 3), .inputRate = VK_VERTEX_INPUT_RATE_VERTEX},
            {.binding = 1, .stride = (sizeof(float) * 3), .inputRate = VK_VERTEX_INPUT_RATE_VERTEX},
            {.binding = 2, .stride = (sizeof(float) * 2), .inputRate = VK_VERTEX_INPUT_RATE_VERTEX}};

        std::vector<VkVertexInputAttributeDescription> vertex_attributes{
            {.location = 0, .binding = 0, .format = VK_FORMAT_R32G32B32_SFLOAT, .offset = 0},
            {.location = 1, .binding = 1, .format = VK_FORMAT_R32G32B32_SFLOAT, .offset = 0},
            {.location = 2, .binding = 2, .format = VK_FORMAT_R32G32_SFLOAT, .offset = 0}};

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
        viewport.width = gfx.get_scaled_draw_extent().width;
        viewport.height = gfx.get_scaled_draw_extent().height;
        viewport.minDepth = 0.f;
        viewport.maxDepth = 1.f;

        VkRect2D scissor = {};
        scissor.offset.x = 0;
        scissor.offset.y = 0;
        scissor.extent.width = gfx.get_scaled_draw_extent().width;
        scissor.extent.height = gfx.get_scaled_draw_extent().height;

        VkPipelineViewportStateCreateInfo viewport_state{};
        viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewport_state.viewportCount = 1;
        viewport_state.scissorCount = 1;
        viewport_state.pViewports = &viewport;
        viewport_state.pScissors = &scissor;

        VkPipelineColorBlendAttachmentState color_blend_attachment{};
        // default write mask
        color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        // no blending
        color_blend_attachment.blendEnable = VK_FALSE;

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
        VkShaderModule vertex_shader{};
        VkShaderModule fragment_shader{};
        auto vertex_binary = utils::file_path_to_binary("Core/shaders/compiled/colored_triangle.vert.spv");
        auto fragment_binary = utils::file_path_to_binary("Core/shaders/compiled/colored_triangle.frag.spv");

        VkShaderModuleCreateInfo vertex_shader_module_ci = {};
        vertex_shader_module_ci.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        vertex_shader_module_ci.codeSize = vertex_binary.size();
        vertex_shader_module_ci.pCode = (uint32_t *)vertex_binary.data();
        vk_check(vkCreateShaderModule(gfx.device, &vertex_shader_module_ci, nullptr, &vertex_shader));
        VkShaderModuleCreateInfo fragment_shader_module_ci = {};
        fragment_shader_module_ci.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        fragment_shader_module_ci.codeSize = fragment_binary.size();
        fragment_shader_module_ci.pCode = (uint32_t *)fragment_binary.data();
        vk_check(vkCreateShaderModule(gfx.device, &fragment_shader_module_ci, nullptr, &fragment_shader));

        VkPipelineShaderStageCreateInfo shader_stage_ci = {};
        shader_stage_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shader_stage_ci.module = vertex_shader;
        shader_stage_ci.pName = "main";
        shader_stage_ci.stage = VK_SHADER_STAGE_VERTEX_BIT;
        pipeline_shader_stages.push_back(shader_stage_ci);

        shader_stage_ci.module = fragment_shader;
        shader_stage_ci.pName = "main";
        shader_stage_ci.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        pipeline_shader_stages.push_back(shader_stage_ci);

        VkPipelineDynamicStateCreateInfo dynamic_state_ci{};
        dynamic_state_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamic_state_ci.dynamicStateCount = 0;

        VkFormat color_attachment_format = gfx.get_swapchain().get_image_format();
        VkFormat depth_attachment_format = gfx.get_swapchain().get_depth_format();

        VkPipelineRenderingCreateInfo render_info{};
        render_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
        render_info.colorAttachmentCount = 1;
        render_info.pColorAttachmentFormats = &color_attachment_format;
        render_info.depthAttachmentFormat = depth_attachment_format;

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

        vk_check(vkCreateGraphicsPipelines(gfx.device, nullptr, 1, &pipeline_info, nullptr, &pipeline));

        vkDestroyShaderModule(gfx.device, vertex_shader, nullptr);
        vkDestroyShaderModule(gfx.device, fragment_shader, nullptr);
    }
    void DefaultPipeline::destroy_pipeline()
    {
        vkDestroyPipeline(gfx.device, pipeline, nullptr);
    }

    void DefaultPipeline::create_global_uniform_buffer()
    {
        VkBufferCreateInfo buffer_ci = {};
        buffer_ci.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        buffer_ci.size = sizeof(GlobalUniform);
        buffer_ci.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

        VmaAllocationCreateInfo alloc_ci = {};
        alloc_ci.usage = VMA_MEMORY_USAGE_AUTO;
        alloc_ci.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
        vk_check(vmaCreateBuffer(gfx.allocator, &buffer_ci, &alloc_ci, &global_buffer, &global_alloc, &global_alloc_info));

        // Update descriptor set
        VkDescriptorSetAllocateInfo global_set_alloc_info{};
        global_set_alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        global_set_alloc_info.descriptorPool = gfx.desc_pool;
        global_set_alloc_info.descriptorSetCount = 1;
        global_set_alloc_info.pSetLayouts = &global_set_layout;
        vkAllocateDescriptorSets(gfx.device, &global_set_alloc_info, &global_set);

        VkDescriptorBufferInfo buffer_info{};
        buffer_info.buffer = global_buffer;
        buffer_info.offset = 0;
        buffer_info.range = sizeof(GlobalUniform);

        VkWriteDescriptorSet descriptor_write{};
        descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptor_write.descriptorCount = 1;
        descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptor_write.dstBinding = 0;
        descriptor_write.dstSet = global_set;
        descriptor_write.pBufferInfo = &buffer_info;

        vkUpdateDescriptorSets(gfx.device, 1, &descriptor_write, 0, nullptr);
    }

    void DefaultPipeline::destroy_global_uniform_buffer()
    {
        vmaDestroyBuffer(gfx.allocator, global_buffer, global_alloc);
        vkFreeDescriptorSets(gfx.device, gfx.desc_pool, 1, &global_set);
    }
    void DefaultPipeline::create_pipeline_layout()
    {
        // GLOBAL SET LAYOUT
        VkDescriptorSetLayoutBinding global_binding{};
        global_binding.binding = 0;
        global_binding.descriptorCount = 1;
        global_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        global_binding.stageFlags = VK_SHADER_STAGE_ALL;

        VkDescriptorSetLayoutCreateInfo layout_ci{};
        layout_ci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layout_ci.bindingCount = 1;
        layout_ci.pBindings = &global_binding;
        layout_ci.flags = 0;
        vk_check(vkCreateDescriptorSetLayout(gfx.device, &layout_ci, nullptr, &global_set_layout));

        // PER INSTANCE SET LAYOUT
        std::vector<VkDescriptorSetLayoutBinding> instance_bindings{
            {.binding = 0, .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, .descriptorCount = 1, .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT, .pImmutableSamplers = nullptr},
            {.binding = 1, .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, .descriptorCount = 2, .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT, .pImmutableSamplers = nullptr},
        };

        layout_ci.bindingCount = instance_bindings.size();
        layout_ci.pBindings = instance_bindings.data();
        layout_ci.flags = 0;
        vk_check(vkCreateDescriptorSetLayout(gfx.device, &layout_ci, nullptr, &instance_set_layout));
        std::vector<VkPushConstantRange> push_constants{
            VkPushConstantRange{
                VK_SHADER_STAGE_VERTEX_BIT,
                0,
                sizeof(glm::mat4x4)}};

        std::vector<VkDescriptorSetLayout> layouts = {global_set_layout, instance_set_layout};
        VkPipelineLayoutCreateInfo pipeline_layout_info = {};
        pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipeline_layout_info.pushConstantRangeCount = push_constants.size();
        pipeline_layout_info.pPushConstantRanges = push_constants.data();
        pipeline_layout_info.pSetLayouts = layouts.data();
        pipeline_layout_info.setLayoutCount = layouts.size();
        vk_check(vkCreatePipelineLayout(gfx.device, &pipeline_layout_info, nullptr, &pipeline_layout));
    }
    void DefaultPipeline::destroy_pipeline_layout()
    {
        vkDestroyPipelineLayout(gfx.device, pipeline_layout, nullptr);
        vkDestroyDescriptorSetLayout(gfx.device, global_set_layout, nullptr);
        vkDestroyDescriptorSetLayout(gfx.device, instance_set_layout, nullptr);
    }
}