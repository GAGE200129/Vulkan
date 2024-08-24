#include <pch.hpp>
#include "ShadowPass.hpp"

#include "../../Graphics.hpp"

#include <Core/src/utils/VulkanHelper.hpp>

namespace gage::gfx::data::g_buffer
{
    ShadowPass::ShadowPass(Graphics& gfx) :
        gfx(gfx)
    {
        create_image();
        create_render_pass();
        create_framebuffer();
    }
    ShadowPass::~ShadowPass()
    {
        destroy_image();
        destroy_render_pass();
        destroy_framebuffer();
    }
    void ShadowPass::reset()
    {
        destroy_image();
        destroy_framebuffer();
        create_image();
        create_framebuffer();
    }
    void ShadowPass::create_image()
    {
        VkImageCreateInfo image_ci = {};
        image_ci.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        image_ci.imageType = VK_IMAGE_TYPE_2D;
        image_ci.extent.width = gfx.directional_light_shadow_map_resolution;
        image_ci.extent.height = gfx.directional_light_shadow_map_resolution;
        image_ci.extent.depth = 1;
        image_ci.mipLevels = 1;
        image_ci.arrayLayers = gfx.CASCADE_COUNT; // Layers
        image_ci.format = SHADOW_FORMAT;
        image_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
        image_ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        image_ci.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        image_ci.samples = VK_SAMPLE_COUNT_1_BIT;

        vk_check(vkCreateImage(gfx.device.device, &image_ci, nullptr, &shadowpass_image));

        VkMemoryRequirements mem_reqs{};
        vkGetImageMemoryRequirements(gfx.device.device, shadowpass_image, &mem_reqs);

        VkMemoryAllocateInfo mem_alloc_info{};
        mem_alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        mem_alloc_info.allocationSize = mem_reqs.size;
        mem_alloc_info.memoryTypeIndex = utils::find_memory_type(gfx.device.physical_device, mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        vk_check(vkAllocateMemory(gfx.device.device, &mem_alloc_info, nullptr, &shadowpass_image_memory));
        vk_check(vkBindImageMemory(gfx.device.device, shadowpass_image, shadowpass_image_memory, 0));

        VkImageViewCreateInfo image_view_ci = {};
        image_view_ci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        image_view_ci.image = shadowpass_image;
        image_view_ci.format = SHADOW_FORMAT;
        image_view_ci.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
        image_view_ci.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        image_view_ci.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        image_view_ci.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        image_view_ci.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        image_view_ci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        image_view_ci.subresourceRange.baseMipLevel = 0;
        image_view_ci.subresourceRange.baseArrayLayer = 0;
        image_view_ci.subresourceRange.layerCount = gfx.CASCADE_COUNT; // Layers
        image_view_ci.subresourceRange.levelCount = 1;

        vk_check(vkCreateImageView(gfx.device.device, &image_view_ci, nullptr, &shadowpass_image_view));

        // Link to global set
        for (uint32_t i = 0; i < Graphics::FRAMES_IN_FLIGHT; i++)
        {
            VkDescriptorImageInfo image_info{};
            image_info.sampler = gfx.defaults.sampler;
            image_info.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
            image_info.imageView = shadowpass_image_view;

            VkWriteDescriptorSet descriptor_write{};
            descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptor_write.descriptorCount = 1;
            descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptor_write.dstBinding = 1;
            descriptor_write.dstSet = gfx.frame_datas[i].global_set;
            descriptor_write.pImageInfo = &image_info;
            vkUpdateDescriptorSets(gfx.device.device, 1, &descriptor_write, 0, nullptr);
        }
    }
    void ShadowPass::create_render_pass()
    {
         VkAttachmentDescription depth_attachment{};
        depth_attachment.format = SHADOW_FORMAT;
        depth_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
        depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        depth_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depth_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depth_attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

        // Subpass

        VkAttachmentReference depth_attachment_ref{};
        depth_attachment_ref.attachment = 0;
        depth_attachment_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 0;
        subpass.pDepthStencilAttachment = &depth_attachment_ref;

        std::array<VkSubpassDependency, 2> dependencies;

        dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
        dependencies[0].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        dependencies[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
        dependencies[0].dstSubpass = 0;
        dependencies[0].dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependencies[0].dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        dependencies[1].srcSubpass = 0;
        dependencies[1].srcStageMask = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
        dependencies[1].srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
        dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        VkRenderPassCreateInfo ci{};
        ci.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        ci.attachmentCount = 1;
        ci.pAttachments = &depth_attachment;
        ci.subpassCount = 1;
        ci.pSubpasses = &subpass;
        ci.dependencyCount = dependencies.size();
        ci.pDependencies = dependencies.data();

        vk_check(vkCreateRenderPass(gfx.device.device, &ci, nullptr, &shadowpass_renderpass));
    }
    void ShadowPass::create_framebuffer()
    {
        VkFramebufferCreateInfo ci{};
        ci.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        ci.renderPass = shadowpass_renderpass;
        ci.attachmentCount = 1;
        ci.pAttachments = &shadowpass_image_view;
        ci.width = gfx.directional_light_shadow_map_resolution;
        ci.height = gfx.directional_light_shadow_map_resolution;
        ci.layers = gfx.CASCADE_COUNT;
        vk_check(vkCreateFramebuffer(gfx.device.device, &ci, nullptr, &shadowpass_framebuffer));
    }
    void ShadowPass::destroy_image()
    {
        vkDestroyImage(gfx.device.device, shadowpass_image, nullptr);
        vkDestroyImageView(gfx.device.device, shadowpass_image_view, nullptr);
        vkFreeMemory(gfx.device.device, shadowpass_image_memory, nullptr);
    }
    void ShadowPass::destroy_render_pass()
    {
        vkDestroyRenderPass(gfx.device.device, shadowpass_renderpass, nullptr);
    }
    void ShadowPass::destroy_framebuffer()
    {
         vkDestroyFramebuffer(gfx.device.device, shadowpass_framebuffer, nullptr);
    }
}