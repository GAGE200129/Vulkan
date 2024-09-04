#include <pch.hpp>
#include "SSAOPass.hpp"

#include "../../Graphics.hpp"

#include <Core/src/utils/VulkanHelper.hpp>

namespace gage::gfx::data::g_buffer
{
    SSAOPass::SSAOPass(const Graphics &gfx) : 
        gfx(gfx)
    {
        create_image();
        create_render_pass();
        create_framebuffer();
    }
    SSAOPass::~SSAOPass()
    {
        destroy_image();
        destroy_render_pass();
        destroy_framebuffer();
    }

    void SSAOPass::reset()
    {
        destroy_image();
        destroy_framebuffer();
        create_image();
        create_framebuffer();
    }

    void SSAOPass::create_image()
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
        image_ci.format = FORMAT;
        vk_check(vkCreateImage(gfx.device.device, &image_ci, nullptr, &image));

        VkMemoryRequirements mem_reqs{};
        vkGetImageMemoryRequirements(gfx.device.device, image, &mem_reqs);
        VkMemoryAllocateInfo mem_alloc_info{};
        mem_alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        mem_alloc_info.allocationSize = mem_reqs.size;
        mem_alloc_info.memoryTypeIndex = utils::find_memory_type(gfx.device.physical_device, mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        vk_check(vkAllocateMemory(gfx.device.device, &mem_alloc_info, nullptr, &image_memory));
        vk_check(vkBindImageMemory(gfx.device.device, image, image_memory, 0));

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
        image_view_ci.format = FORMAT;
        image_view_ci.image = image;
        vk_check(vkCreateImageView(gfx.device.device, &image_view_ci, nullptr, &image_view));
    }

    void SSAOPass::create_render_pass()
    {
        std::vector<VkAttachmentDescription> attachments = {
                {
                    .flags = 0,
                    .format = FORMAT,
                    .samples = VK_SAMPLE_COUNT_1_BIT,
                    .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                    .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                    .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                    .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                    .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                    .finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                },
            };

            std::vector<VkAttachmentReference> attachment_refs = {
                {.attachment = 0, .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
            };
            VkSubpassDescription subpass{};
            subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
            subpass.colorAttachmentCount = attachment_refs.size();
            subpass.pColorAttachments = attachment_refs.data();

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
            ci.attachmentCount = attachments.size();
            ci.pAttachments = attachments.data();
            ci.subpassCount = 1;
            ci.pSubpasses = &subpass;
            ci.dependencyCount = dependencies.size();
            ci.pDependencies = dependencies.data();
            vk_check(vkCreateRenderPass(gfx.device.device, &ci, nullptr, &render_pass));
    }

    void SSAOPass::create_framebuffer()
    {
        std::vector<VkImageView> attachments = {
            image_view
        };
        VkFramebufferCreateInfo ci{};
        ci.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        ci.renderPass = render_pass;
        ci.attachmentCount = attachments.size();
        ci.pAttachments = attachments.data();
        ci.width = gfx.draw_extent.width;
        ci.height = gfx.draw_extent.height;
        ci.layers = 1;
        vk_check(vkCreateFramebuffer(gfx.device.device, &ci, nullptr, &framebuffer));
    }

    void SSAOPass::destroy_image()
    {
        vkDestroyImage(gfx.device.device, image, nullptr);
        vkDestroyImageView(gfx.device.device, image_view, nullptr);
        vkFreeMemory(gfx.device.device, image_memory, nullptr);

    }
    void SSAOPass::destroy_render_pass()
    {
        vkDestroyRenderPass(gfx.device.device, render_pass, nullptr);
    }
    void SSAOPass::destroy_framebuffer()
    {
        vkDestroyFramebuffer(gfx.device.device, framebuffer, nullptr);
    }

}