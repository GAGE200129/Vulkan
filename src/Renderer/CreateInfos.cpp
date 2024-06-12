#include "pch.hpp"
#include "Renderer.hpp"

vk::ImageCreateInfo Renderer::imageCreateInfo(vk::Format format,
                                              vk::ImageUsageFlags usage,
                                              vk::Extent3D extent)
{
    vk::ImageCreateInfo info = {};
    info.format = format;
    info.extent = extent;
    info.mipLevels = 1;
    info.arrayLayers = 1;
    info.imageType = vk::ImageType::e2D;

    info.samples = vk::SampleCountFlagBits::e1;
    info.tiling = vk::ImageTiling::eOptimal;
    info.usage = usage;
    return info;
}
vk::ImageViewCreateInfo Renderer::imageViewCreateInfo(vk::Format format,
                                                      vk::Image image,
                                                      vk::ImageAspectFlags aspectFlags)
{
    // build a image-view for the depth image to use for rendering
    vk::ImageViewCreateInfo info = {};
    info.viewType = vk::ImageViewType::e2D;
    info.image = image;
    info.format = format;
    info.subresourceRange.baseMipLevel = 0;
    info.subresourceRange.levelCount = 1;
    info.subresourceRange.baseArrayLayer = 0;
    info.subresourceRange.layerCount = 1;
    info.subresourceRange.aspectMask = aspectFlags;

    return info;
}