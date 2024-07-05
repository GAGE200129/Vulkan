// #include "Texture.hpp"

// #include <Core/src/utils/VulkanHelper.hpp>

// #include <cstring>

// namespace gage::gfx::bind
// {
//     Texture::Texture(Graphics &gfx, const utils::Image &in_image) :
//         IBindable(gfx)
//     {
//         VkImageCreateInfo img_ci = {};
//         img_ci.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
//         img_ci.imageType = VK_IMAGE_TYPE_2D;
//         img_ci.extent.width = in_image.width;
//         img_ci.extent.height = in_image.height;
//         img_ci.extent.depth = 1;
//         img_ci.mipLevels = 1;
//         img_ci.arrayLayers = 1;
//         img_ci.format = image_format;
//         img_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
//         img_ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
//         img_ci.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
//         img_ci.samples = VK_SAMPLE_COUNT_1_BIT;

//         VmaAllocationCreateInfo alloc_ci = {};
//         alloc_ci.usage = VMA_MEMORY_USAGE_AUTO;
//         alloc_ci.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;

//         vk_check(vmaCreateImage(get_allocator(gfx), &img_ci, &alloc_ci, &image, &allocation, nullptr));

//         // Staging
//         VkBufferCreateInfo staging_buffer_info = {};
//         staging_buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
//         staging_buffer_info.size = in_image.data.size();
//         staging_buffer_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
//         VmaAllocationCreateInfo staging_alloc_info = {};
//         staging_alloc_info.usage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST;
//         staging_alloc_info.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
        

//         // Copy staging
//         VkBuffer staging_buffer{};
//         VmaAllocation staging_allocation{};
//         vk_check(vmaCreateBuffer(get_allocator(gfx), &staging_buffer_info, &staging_alloc_info, &staging_buffer, &staging_allocation, nullptr));

//         void *data;
//         vmaMapMemory(get_allocator(gfx), staging_allocation, &data);
//         std::memcpy(data, in_image.data.data(), in_image.data.size());
//         vmaUnmapMemory(get_allocator(gfx), staging_allocation);

//         // Copy to image
//         VkCommandBufferBeginInfo transfer_begin_info{};
//         transfer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
//         transfer_begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
//         VkCommandBuffer cmd = get_transfer_cmd(gfx);
//         vkBeginCommandBuffer(cmd, &transfer_begin_info);

//         VkImageMemoryBarrier barrier = {};
//         barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
//         barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
//         barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

//         barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
//         barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

//         barrier.image = image;
//         barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
//         barrier.subresourceRange.baseMipLevel = 0;
//         barrier.subresourceRange.levelCount = 1;
//         barrier.subresourceRange.baseArrayLayer = 0;
//         barrier.subresourceRange.layerCount = 1;

//         vkCmdPipelineBarrier(
//             cmd,
//             VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
//             VK_PIPELINE_STAGE_TRANSFER_BIT,
//             0,
//             0, nullptr,
//             0, nullptr,
//             1, &barrier);

//         VkBufferImageCopy copy_region{};
//         copy_region.bufferOffset = 0;
//         copy_region.bufferRowLength = 0;
//         copy_region.bufferImageHeight = 0;

//         copy_region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
//         copy_region.imageSubresource.mipLevel = 0;
//         copy_region.imageSubresource.baseArrayLayer = 0;
//         copy_region.imageSubresource.layerCount = 1;

//         copy_region.imageOffset = {0, 0, 0};
//         copy_region.imageExtent = {
//             (unsigned int)in_image.width,
//             (unsigned int)in_image.height,
//             1};

//         vkCmdCopyBufferToImage(cmd, staging_buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy_region);

//         barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
//         barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
//         vkCmdPipelineBarrier(
//             cmd,
//             VK_PIPELINE_STAGE_TRANSFER_BIT,
//             VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
//             0,
//             0, nullptr,
//             0, nullptr,
//             1, &barrier);

//         vkEndCommandBuffer(cmd);

//         VkSubmitInfo submit_info{};
//         submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
//         submit_info.commandBufferCount = 1;
//         submit_info.pCommandBuffers = &cmd;

//         vkQueueSubmit(get_queue(gfx), 1, &submit_info, nullptr);
//         vkQueueWaitIdle(get_queue(gfx));

//         vmaDestroyBuffer(get_allocator(gfx), staging_buffer, staging_allocation);

//         // Create image view

//         VkImageViewCreateInfo view_info{};
//         view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
//         view_info.image = image;
//         view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
//         view_info.format = image_format;
//         view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
//         view_info.subresourceRange.baseMipLevel = 0;
//         view_info.subresourceRange.levelCount = 1;
//         view_info.subresourceRange.baseArrayLayer = 0;
//         view_info.subresourceRange.layerCount = 1;

//         vk_check(vkCreateImageView(get_device(gfx), &view_info, nullptr, &image_view));

//         //Sampler
//         VkSamplerCreateInfo sampler_info{};
//         sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
//         sampler_info.magFilter = VK_FILTER_NEAREST;
//         sampler_info.minFilter = VK_FILTER_NEAREST;
//         sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
//         sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
//         sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
//         sampler_info.anisotropyEnable = VK_FALSE;
//         sampler_info.maxAnisotropy = 0;
//         sampler_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
//         sampler_info.unnormalizedCoordinates = VK_FALSE;
//         sampler_info.compareEnable = VK_FALSE;
//         sampler_info.compareOp = VK_COMPARE_OP_ALWAYS;
//         sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
//         sampler_info.mipLodBias = 0.0f;
//         sampler_info.minLod = 0.0f;
//         sampler_info.maxLod = 0.0f;

//         vk_check(vkCreateSampler(get_device(gfx), &sampler_info, nullptr, &sampler));
//     }

//     void Texture::bind(Graphics &)
//     {
//     }
//     Texture::~Texture()
//     {
//         vkDestroySampler(get_device(gfx), sampler, nullptr);
//         vmaDestroyImage(get_allocator(gfx), image, allocation);
//         vkDestroyImageView(get_device(gfx), image_view, nullptr);
//     }

//     VkImage Texture::get_image() const
//     {
//         return image;
//     }

//     VkImageView Texture::get_image_view() const
//     {
//         return image_view;
//     }
//     VkSampler Texture::get_sampler() const
//     {
//         return sampler;
//     }
// }