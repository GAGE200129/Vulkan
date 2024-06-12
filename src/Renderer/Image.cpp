#include "pch.hpp"
#include "Renderer.hpp"

void Renderer::imageTransition(vk::CommandBuffer cmd, vk::Image image, vk::ImageLayout currentLayout, vk::ImageLayout newLayout)
{
    vk::ImageMemoryBarrier2 imageBarrier = {};

    imageBarrier.srcStageMask = vk::PipelineStageFlagBits2KHR::eAllCommands;
    imageBarrier.srcAccessMask = vk::AccessFlagBits2::eMemoryWrite;
    imageBarrier.dstStageMask = vk::PipelineStageFlagBits2KHR::eAllCommands;
    imageBarrier.dstAccessMask = vk::AccessFlagBits2::eMemoryWrite | vk::AccessFlagBits2::eMemoryRead;

    imageBarrier.oldLayout = currentLayout;
    imageBarrier.newLayout = newLayout;

    vk::ImageAspectFlags aspectMask = (newLayout == vk::ImageLayout::eDepthAttachmentOptimal) ? vk::ImageAspectFlagBits::eDepth : vk::ImageAspectFlagBits::eColor;
    vk::ImageSubresourceRange range = {};
    range.aspectMask = aspectMask;
    range.baseMipLevel = 0;
    range.levelCount = vk::RemainingMipLevels;
    range.baseArrayLayer = 0;
    range.layerCount = vk::RemainingArrayLayers;

    imageBarrier.subresourceRange = range;
    imageBarrier.image = image;

    vk::DependencyInfo depInfo = {};
    depInfo.imageMemoryBarrierCount = 1;
    depInfo.pImageMemoryBarriers = &imageBarrier;

    cmd.pipelineBarrier2(depInfo);
}

void Renderer::imageCopy(vk::CommandBuffer cmd, vk::Image src, vk::Image dst, vk::Extent2D srcSize, vk::Extent2D dstSize)
{
    vk::ImageBlit2 blitRegion = {};

	blitRegion.srcOffsets[1].x = srcSize.width;
	blitRegion.srcOffsets[1].y = srcSize.height;
	blitRegion.srcOffsets[1].z = 1;

	blitRegion.dstOffsets[1].x = dstSize.width;
	blitRegion.dstOffsets[1].y = dstSize.height;
	blitRegion.dstOffsets[1].z = 1;

    blitRegion.srcSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
	blitRegion.srcSubresource.baseArrayLayer = 0;
	blitRegion.srcSubresource.layerCount = 1;
	blitRegion.srcSubresource.mipLevel = 0;

	blitRegion.dstSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
	blitRegion.dstSubresource.baseArrayLayer = 0;
	blitRegion.dstSubresource.layerCount = 1;
	blitRegion.dstSubresource.mipLevel = 0;

    vk::BlitImageInfo2 blitInfo = {};
	blitInfo.dstImage = dst;
	blitInfo.dstImageLayout = vk::ImageLayout::eTransferDstOptimal;
	blitInfo.srcImage = src;
	blitInfo.srcImageLayout = vk::ImageLayout::eTransferSrcOptimal;
	blitInfo.filter = vk::Filter::eLinear;
	blitInfo.regionCount = 1;
	blitInfo.pRegions = &blitRegion;

    cmd.blitImage2(blitInfo);
}