#include <pch.hpp>
#include "Image.hpp"

#include "../Graphics.hpp"

namespace gage::gfx::data
{
    Image::Image(const Graphics &gfx, ImageCreateInfo ci) : gfx(gfx)
    {
        // size_t size_in_bytes = width * height * 4;
        log().trace("Allocating image: width: {}, height: {}, size_in_bytes: {}", ci.width, ci.height, ci.size_in_bytes);


        {
            VkImageCreateInfo img_ci = {};
            img_ci.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            img_ci.imageType = VK_IMAGE_TYPE_2D;
            img_ci.extent.width = ci.width;
            img_ci.extent.height = ci.height;
            img_ci.extent.depth = 1;
            img_ci.mipLevels = ci.mip_levels;
            img_ci.arrayLayers = 1;
            img_ci.format = ci.format;
            img_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
            img_ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            img_ci.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
            img_ci.samples = VK_SAMPLE_COUNT_1_BIT;

            VmaAllocationCreateInfo alloc_ci = {};
            alloc_ci.usage = VMA_MEMORY_USAGE_AUTO;
            alloc_ci.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;

            vk_check(vmaCreateImage(gfx.allocator.allocator, &img_ci, &alloc_ci, &image, &allocation, nullptr));
        }

        // Copy to gpu
        {
            VkBufferCreateInfo staging_buffer_info = {};
            staging_buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            staging_buffer_info.size = ci.size_in_bytes;
            staging_buffer_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
            VmaAllocationCreateInfo staging_alloc_info = {};
            staging_alloc_info.usage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST;
            staging_alloc_info.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;

            // Copy staging
            VkBuffer staging_buffer{};
            VmaAllocation staging_allocation{};
            VmaAllocationInfo staging_info{};
            vk_check(vmaCreateBuffer(gfx.allocator.allocator, &staging_buffer_info, &staging_alloc_info, &staging_buffer, &staging_allocation, &staging_info));
            std::memcpy(staging_info.pMappedData, ci.image_data, ci.size_in_bytes);

            // Allocate cmd buffer
            VkCommandBufferAllocateInfo cmd_alloc_info{};
            cmd_alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            cmd_alloc_info.commandPool = gfx.cmd_pool.pool;
            cmd_alloc_info.commandBufferCount = 1;
            cmd_alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

            VkCommandBuffer cmd{};
            vk_check(vkAllocateCommandBuffers(gfx.device.device, &cmd_alloc_info, &cmd));

            VkCommandBufferBeginInfo transfer_begin_info{};
            transfer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            transfer_begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
            vkBeginCommandBuffer(cmd, &transfer_begin_info);

            // Transition to transfer dst
            {
                VkImageMemoryBarrier barrier = {};
                barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
                barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

                barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

                barrier.image = image;
                barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                barrier.subresourceRange.baseMipLevel = 0;
                barrier.subresourceRange.levelCount = ci.mip_levels;
                barrier.subresourceRange.baseArrayLayer = 0;
                barrier.subresourceRange.layerCount = 1;

                vkCmdPipelineBarrier(
                    cmd,
                    VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                    VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                    0,
                    0, nullptr,
                    0, nullptr,
                    1, &barrier);
            }

            // Copy
            {
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
                    (uint32_t)ci.width,
                    (uint32_t)ci.height,
                    1};

                vkCmdCopyBufferToImage(cmd, staging_buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy_region);
            }

            if (ci.mip_levels > 1)
            {
                generate_mip_maps(cmd, ci.mip_levels, ci.width, ci.height);
            }

            {
                VkImageMemoryBarrier barrier = {};
                barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
                barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
                barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

                barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

                barrier.image = image;
                barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                barrier.subresourceRange.baseMipLevel = ci.mip_levels - 1;
                barrier.subresourceRange.levelCount = 1;
                barrier.subresourceRange.baseArrayLayer = 0;
                barrier.subresourceRange.layerCount = 1;

                vkCmdPipelineBarrier(
                    cmd,
                    VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                    VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                    0,
                    0, nullptr,
                    0, nullptr,
                    1, &barrier);
            }

            vkEndCommandBuffer(cmd);

            VkSubmitInfo submit_info{};
            submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            submit_info.commandBufferCount = 1;
            submit_info.pCommandBuffers = &cmd;

            vkQueueSubmit(gfx.device.queue, 1, &submit_info, nullptr);
            vkQueueWaitIdle(gfx.device.queue);

            // Cleanup resources
            vmaDestroyBuffer(gfx.allocator.allocator, staging_buffer, staging_allocation);
            vkFreeCommandBuffers(gfx.device.device, gfx.cmd_pool.pool, 1, &cmd);
        }

        // Create image view
        {
            VkImageViewCreateInfo view_info{};
            view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            view_info.image = image;
            view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
            view_info.format = ci.format;
            view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            view_info.subresourceRange.baseMipLevel = 0;
            view_info.subresourceRange.levelCount = ci.mip_levels;
            view_info.subresourceRange.baseArrayLayer = 0;
            view_info.subresourceRange.layerCount = 1;

            vk_check(vkCreateImageView(gfx.device.device, &view_info, nullptr, &image_view));
        }

        // Sampler
        {
            VkSamplerCreateInfo sampler_info{};
            sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
            sampler_info.magFilter = ci.mag_filter;
            sampler_info.minFilter = ci.min_filter;
            sampler_info.addressModeU = ci.address_node;
            sampler_info.addressModeV = ci.address_node;
            sampler_info.addressModeW = ci.address_node;
            sampler_info.anisotropyEnable = VK_FALSE;
            sampler_info.maxAnisotropy = 0;
            sampler_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
            sampler_info.unnormalizedCoordinates = VK_FALSE;
            sampler_info.compareEnable = VK_FALSE;
            sampler_info.compareOp = VK_COMPARE_OP_ALWAYS;
            sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
            sampler_info.mipLodBias = 0.0f;
            sampler_info.minLod = 0.0f;
            sampler_info.maxLod = ci.mip_levels;

            vk_check(vkCreateSampler(gfx.device.device, &sampler_info, nullptr, &sampler));
        }
    }
    Image::~Image()
    {
        log().trace("Deallocating image");
        vkDestroySampler(gfx.device.device, sampler, nullptr);
        vmaDestroyImage(gfx.allocator.allocator, image, allocation);
        vkDestroyImageView(gfx.device.device, image_view, nullptr);
    }
    void Image::generate_mip_maps(VkCommandBuffer cmd, uint32_t mip_levels, uint32_t width, uint32_t height)
    {
        VkImageMemoryBarrier barrier = {};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;

        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

        barrier.image = image;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;

        int32_t mip_width = width;
        int32_t mip_height = height;
        for (uint32_t i = 1; i < mip_levels; i++)
        {
            barrier.subresourceRange.baseMipLevel = i - 1;
            barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

            vkCmdPipelineBarrier(cmd,
                                 VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
                                 0, nullptr,
                                 0, nullptr,
                                 1, &barrier);
            VkImageBlit blit{};
            blit.srcOffsets[0] = {0, 0, 0};
            blit.srcOffsets[1] = {mip_width, mip_height, 1};
            blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            blit.srcSubresource.mipLevel = i - 1;
            blit.srcSubresource.baseArrayLayer = 0;
            blit.srcSubresource.layerCount = 1;
            blit.dstOffsets[0] = {0, 0, 0};
            blit.dstOffsets[1] = {mip_width > 1 ? mip_width / 2 : 1, mip_height > 1 ? mip_height / 2 : 1, 1};
            blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            blit.dstSubresource.mipLevel = i;
            blit.dstSubresource.baseArrayLayer = 0;
            blit.dstSubresource.layerCount = 1;

            vkCmdBlitImage(cmd,
                           image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                           image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                           1, &blit,
                           VK_FILTER_LINEAR);

            barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            vkCmdPipelineBarrier(cmd,
                                 VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
                                 0, nullptr,
                                 0, nullptr,
                                 1, &barrier);
            if (mip_width > 1)
                mip_width /= 2;
            if (mip_height > 1)
                mip_height /= 2;
        }
    }

    VkImage Image::get_image() const
    {
        return image;
    }

    VkImageView Image::get_image_view() const
    {
        return image_view;
    }
    VkSampler Image::get_sampler() const
    {
        return sampler;
    }
}