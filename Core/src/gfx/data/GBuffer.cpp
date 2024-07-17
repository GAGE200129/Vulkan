#include <pch.hpp>
#include "GBuffer.hpp"

#include "../Graphics.hpp"

#include <Core/src/utils/FileLoader.hpp>
#include <Core/src/utils/VulkanHelper.hpp>

namespace gage::gfx::data
{
    GBuffer::GBuffer(Graphics &gfx) : gfx(gfx)
    {

        // Create renderpass
        {
            std::vector<VkAttachmentDescription> color_attachments = {
                {
                    .flags = 0,
                    .format = POSITION_FORMAT,
                    .samples = VK_SAMPLE_COUNT_1_BIT,
                    .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                    .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                    .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                    .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                    .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                    .finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                },

                {
                    .flags = 0,
                    .format = NORMAL_FORMAT,
                    .samples = VK_SAMPLE_COUNT_1_BIT,
                    .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                    .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                    .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                    .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                    .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                    .finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                },

                {
                    .flags = 0,
                    .format = ALBEDO_FORMAT,
                    .samples = VK_SAMPLE_COUNT_1_BIT,
                    .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                    .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                    .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                    .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                    .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                    .finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                },

                // Depth

                {
                    .flags = 0,
                    .format = DEPTH_FORMAT,
                    .samples = VK_SAMPLE_COUNT_1_BIT,
                    .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                    .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                    .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                    .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                    .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                    .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                }};

            std::vector<VkAttachmentReference> color_attachment_refs = {
                {.attachment = 0, .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
                {.attachment = 1, .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
                {.attachment = 2, .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
            };

            VkAttachmentReference depth_attachment_ref{};
            depth_attachment_ref.attachment = 3;
            depth_attachment_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

            VkSubpassDescription subpass{};
            subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
            subpass.colorAttachmentCount = color_attachment_refs.size();
            subpass.pColorAttachments = color_attachment_refs.data();
            subpass.pDepthStencilAttachment = &depth_attachment_ref;

            // Subpass dependencies for layout transitions
            std::vector<VkSubpassDependency> dependencies =
                {
                    {
                        .srcSubpass = VK_SUBPASS_EXTERNAL,
                        .dstSubpass = 0,
                        .srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
                        .dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
                        .srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
                        .dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT,
                        .dependencyFlags = 0,
                    },
                    {
                        .srcSubpass = 0,
                        .dstSubpass = VK_SUBPASS_EXTERNAL,
                        .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                        .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                        .srcAccessMask = 0,
                        .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT,
                        .dependencyFlags = 0,
                    }};

            VkRenderPassCreateInfo ci{};
            ci.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
            ci.attachmentCount = color_attachments.size();
            ci.pAttachments = color_attachments.data();
            ci.subpassCount = 1;
            ci.pSubpasses = &subpass;
            ci.dependencyCount = dependencies.size();
            ci.pDependencies = dependencies.data();

            vk_check(vkCreateRenderPass(gfx.device, &ci, nullptr, &render_pass));
        }

        // ======= FINAL PASS ==========//

        //  Create renderpass
        {

            std::vector<VkAttachmentDescription> color_attachments = {
                {
                    .flags = 0,
                    .format = COLOR_FORMAT,
                    .samples = VK_SAMPLE_COUNT_1_BIT,
                    .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                    .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                    .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                    .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                    .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                    .finalLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                },
            };

            std::vector<VkAttachmentReference> color_attachment_refs = {
                {.attachment = 0, .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
            };
            VkSubpassDescription subpass{};
            subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
            subpass.colorAttachmentCount = color_attachment_refs.size();
            subpass.pColorAttachments = color_attachment_refs.data();

            std::vector<VkSubpassDependency> dependencies =
                {
                    {
                        .srcSubpass = VK_SUBPASS_EXTERNAL,
                        .dstSubpass = 0,
                        .srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
                        .dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
                        .srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
                        .dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT,
                        .dependencyFlags = 0,
                    },
                    {
                        .srcSubpass = 0,
                        .dstSubpass = VK_SUBPASS_EXTERNAL,
                        .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                        .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                        .srcAccessMask = 0,
                        .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT,
                        .dependencyFlags = 0,
                    }};

            VkRenderPassCreateInfo ci{};
            ci.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
            ci.attachmentCount = color_attachments.size();
            ci.pAttachments = color_attachments.data();
            ci.subpassCount = 1;
            ci.pSubpasses = &subpass;
            ci.dependencyCount = dependencies.size();
            ci.pDependencies = dependencies.data();
            vk_check(vkCreateRenderPass(gfx.device, &ci, nullptr, &finalpass_renderpass));
        }

        create_images();
        create_framebuffers();
    }
    GBuffer::~GBuffer()
    {

        vkDestroyRenderPass(gfx.device, render_pass, nullptr);
        vkDestroyFramebuffer(gfx.device, framebuffer, nullptr);

        // GBuffer datas

        vkDestroyImageView(gfx.device, position_view, nullptr);
        vkDestroyImage(gfx.device, position, nullptr);
        vkFreeMemory(gfx.device, position_memory, nullptr);

        vkDestroyImageView(gfx.device, normal_view, nullptr);
        vkDestroyImage(gfx.device, normal, nullptr);
        vkFreeMemory(gfx.device, normal_memory, nullptr);

        vkDestroyImageView(gfx.device, albedo_view, nullptr);
        vkDestroyImage(gfx.device, albedo, nullptr);
        vkFreeMemory(gfx.device, albedo_memory, nullptr);

        vkDestroyImage(gfx.device, depth_image, nullptr);
        vkDestroyImageView(gfx.device, depth_image_view, nullptr);
        vkFreeMemory(gfx.device, depth_image_memory, nullptr);

        vkDestroyImage(gfx.device, finalpass_image, nullptr);
        vkDestroyImageView(gfx.device, finalpass_image_view, nullptr);
        vkFreeMemory(gfx.device, finalpass_image_memory, nullptr);
        vkDestroyRenderPass(gfx.device, finalpass_renderpass, nullptr);
        vkDestroyFramebuffer(gfx.device, finalpass_framebuffer, nullptr);
    }

    void GBuffer::reset()
    {
        vkDestroyFramebuffer(gfx.device, finalpass_framebuffer, nullptr);
        vkDestroyFramebuffer(gfx.device, framebuffer, nullptr);

        vkDestroyImageView(gfx.device, position_view, nullptr);
        vkDestroyImage(gfx.device, position, nullptr);
        vkFreeMemory(gfx.device, position_memory, nullptr);

        vkDestroyImageView(gfx.device, normal_view, nullptr);
        vkDestroyImage(gfx.device, normal, nullptr);
        vkFreeMemory(gfx.device, normal_memory, nullptr);

        vkDestroyImageView(gfx.device, albedo_view, nullptr);
        vkDestroyImage(gfx.device, albedo, nullptr);
        vkFreeMemory(gfx.device, albedo_memory, nullptr);

        vkDestroyImage(gfx.device, depth_image, nullptr);
        vkDestroyImageView(gfx.device, depth_image_view, nullptr);
        vkFreeMemory(gfx.device, depth_image_memory, nullptr);

        vkDestroyImage(gfx.device, finalpass_image, nullptr);
        vkDestroyImageView(gfx.device, finalpass_image_view, nullptr);
        vkFreeMemory(gfx.device, finalpass_image_memory, nullptr);
        create_images();
        create_framebuffers();
    }

    void GBuffer::begin(VkCommandBuffer cmd) const
    {
        VkRenderPassBeginInfo render_pass_begin_info{};
        render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        render_pass_begin_info.renderPass = render_pass;
        render_pass_begin_info.framebuffer = framebuffer;
        render_pass_begin_info.renderArea.offset = {0, 0};
        render_pass_begin_info.renderArea.extent = gfx.get_scaled_draw_extent();
        std::array<VkClearValue, 4> clear_values{};
        clear_values[0].color = {{0.0f, 0.0f, 0.0f}};
        clear_values[1].color = {{0.0f, 0.0f, 0.0f}};
        clear_values[2].color = {{0.0f, 0.0f, 0.0f}};
        clear_values[3].depthStencil = {1.0f, 0};
        render_pass_begin_info.clearValueCount = clear_values.size();
        render_pass_begin_info.pClearValues = clear_values.data();
        vkCmdBeginRenderPass(cmd, &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);
    }

    void GBuffer::begin_finalpass(VkCommandBuffer cmd) const
    {
        VkRenderPassBeginInfo render_pass_begin_info{};
        render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        render_pass_begin_info.renderPass = finalpass_renderpass;
        render_pass_begin_info.framebuffer = finalpass_framebuffer;
        render_pass_begin_info.renderArea.offset = {0, 0};
        render_pass_begin_info.renderArea.extent = gfx.get_scaled_draw_extent();
        std::array<VkClearValue, 1> clear_values{};
        clear_values[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
        render_pass_begin_info.clearValueCount = clear_values.size();
        render_pass_begin_info.pClearValues = clear_values.data();
        vkCmdBeginRenderPass(cmd, &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);
    }

    void GBuffer::end(VkCommandBuffer cmd) const
    {
        vkCmdEndRenderPass(cmd);
    }

    void GBuffer::create_images()
    {
        // GBuffer
        {
            VkImageCreateInfo image_ci = {};
            image_ci.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            image_ci.imageType = VK_IMAGE_TYPE_2D;
            image_ci.extent.width = gfx.draw_extent.width;
            image_ci.extent.height = gfx.draw_extent.height;
            image_ci.extent.depth = 1;
            image_ci.mipLevels = 1;
            image_ci.arrayLayers = 1;
            image_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
            image_ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            image_ci.samples = VK_SAMPLE_COUNT_1_BIT;

            image_ci.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
            image_ci.format = POSITION_FORMAT;
            vk_check(vkCreateImage(gfx.device, &image_ci, nullptr, &position));

            image_ci.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
            image_ci.format = NORMAL_FORMAT;
            vk_check(vkCreateImage(gfx.device, &image_ci, nullptr, &normal));

            image_ci.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
            image_ci.format = ALBEDO_FORMAT;
            vk_check(vkCreateImage(gfx.device, &image_ci, nullptr, &albedo));

            image_ci.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
            image_ci.format = DEPTH_FORMAT;
            vk_check(vkCreateImage(gfx.device, &image_ci, nullptr, &depth_image));

            auto allocate_memory = [this](VkImage &image, VkDeviceMemory &memory)
            {
                VkMemoryRequirements mem_reqs{};
                vkGetImageMemoryRequirements(gfx.device, image, &mem_reqs);
                VkMemoryAllocateInfo mem_alloc_info{};
                mem_alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
                mem_alloc_info.allocationSize = mem_reqs.size;
                mem_alloc_info.memoryTypeIndex = utils::find_memory_type(gfx.physical_device, mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
                vk_check(vkAllocateMemory(gfx.device, &mem_alloc_info, nullptr, &memory));
                vk_check(vkBindImageMemory(gfx.device, image, memory, 0));
            };

            allocate_memory(position, position_memory);
            allocate_memory(normal, normal_memory);
            allocate_memory(albedo, albedo_memory);
            allocate_memory(depth_image, depth_image_memory);

            VkImageViewCreateInfo image_view_ci = {};
            image_view_ci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            image_view_ci.viewType = VK_IMAGE_VIEW_TYPE_2D;
            image_view_ci.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            image_view_ci.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            image_view_ci.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            image_view_ci.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            image_view_ci.subresourceRange.baseMipLevel = 0;
            image_view_ci.subresourceRange.baseArrayLayer = 0;
            image_view_ci.subresourceRange.layerCount = 1;
            image_view_ci.subresourceRange.levelCount = 1;

            image_view_ci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            image_view_ci.format = POSITION_FORMAT;
            image_view_ci.image = position;
            vk_check(vkCreateImageView(gfx.device, &image_view_ci, nullptr, &position_view));

            image_view_ci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            image_view_ci.format = NORMAL_FORMAT;
            image_view_ci.image = normal;
            vk_check(vkCreateImageView(gfx.device, &image_view_ci, nullptr, &normal_view));

            image_view_ci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            image_view_ci.format = ALBEDO_FORMAT;
            image_view_ci.image = albedo;
            vk_check(vkCreateImageView(gfx.device, &image_view_ci, nullptr, &albedo_view));

            image_view_ci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
            image_view_ci.format = DEPTH_FORMAT;
            image_view_ci.image = depth_image;
            vk_check(vkCreateImageView(gfx.device, &image_view_ci, nullptr, &depth_image_view));
        }

        // Final
        {
            VkImageCreateInfo image_ci = {};
            image_ci.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            image_ci.imageType = VK_IMAGE_TYPE_2D;
            image_ci.extent.width = gfx.draw_extent.width;
            image_ci.extent.height = gfx.draw_extent.height;
            image_ci.extent.depth = 1;
            image_ci.mipLevels = 1;
            image_ci.arrayLayers = 1;
            image_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
            image_ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
            image_ci.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
            image_ci.format = COLOR_FORMAT;
            vk_check(vkCreateImage(gfx.device, &image_ci, nullptr, &finalpass_image));

            VkMemoryRequirements mem_reqs{};
            vkGetImageMemoryRequirements(gfx.device, finalpass_image, &mem_reqs);
            VkMemoryAllocateInfo mem_alloc_info{};
            mem_alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            mem_alloc_info.allocationSize = mem_reqs.size;
            mem_alloc_info.memoryTypeIndex = utils::find_memory_type(gfx.physical_device, mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
            vk_check(vkAllocateMemory(gfx.device, &mem_alloc_info, nullptr, &finalpass_image_memory));
            vk_check(vkBindImageMemory(gfx.device, finalpass_image, finalpass_image_memory, 0));

            VkImageViewCreateInfo image_view_ci = {};
            image_view_ci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            image_view_ci.viewType = VK_IMAGE_VIEW_TYPE_2D;
            image_view_ci.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            image_view_ci.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            image_view_ci.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            image_view_ci.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            image_view_ci.subresourceRange.baseMipLevel = 0;
            image_view_ci.subresourceRange.baseArrayLayer = 0;
            image_view_ci.subresourceRange.layerCount = 1;
            image_view_ci.subresourceRange.levelCount = 1;
            image_view_ci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            image_view_ci.format = COLOR_FORMAT;
            image_view_ci.image = finalpass_image;
            vk_check(vkCreateImageView(gfx.device, &image_view_ci, nullptr, &finalpass_image_view));
        }
    }
    void GBuffer::create_framebuffers()
    {
        // GBuffer
        {
            std::vector<VkImageView> attachments = {
                position_view,
                normal_view,
                albedo_view,
                depth_image_view};
            VkFramebufferCreateInfo ci{};
            ci.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            ci.renderPass = render_pass;
            ci.attachmentCount = attachments.size();
            ci.pAttachments = attachments.data();
            ci.width = gfx.draw_extent.width;
            ci.height = gfx.draw_extent.height;
            ci.layers = 1;
            vk_check(vkCreateFramebuffer(gfx.device, &ci, nullptr, &framebuffer));
        }

        // Final pass
        //  Create frame buffer
        {
            std::vector<VkImageView> attachments = {
                finalpass_image_view};
            VkFramebufferCreateInfo ci{};
            ci.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            ci.renderPass = finalpass_renderpass;
            ci.attachmentCount = attachments.size();
            ci.pAttachments = attachments.data();
            ci.width = gfx.draw_extent.width;
            ci.height = gfx.draw_extent.height;
            ci.layers = 1;
            vk_check(vkCreateFramebuffer(gfx.device, &ci, nullptr, &finalpass_framebuffer));
        }
    }

    VkRenderPass GBuffer::get_render_pass() const
    {
        return render_pass;
    }

    VkRenderPass GBuffer::get_finalpass_render_pass() const
    {
        return finalpass_renderpass;
    }

    VkImageView GBuffer::get_position_view() const
    {
        return position_view;
    }
    VkImageView GBuffer::get_normal_view() const
    {
        return normal_view;
    }
    VkImageView GBuffer::get_albedo_view() const
    {
        return albedo_view;
    }

    VkImage GBuffer::get_final_color() const
    {
        return finalpass_image;
    }

}