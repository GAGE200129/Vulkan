#include "pch.hpp"
#include "Renderer.hpp"

#include <vulkan/vk_enum_string_helper.h>

bool Renderer::swapchainInit(int width, int height)
{
    gLogger->info("Initializing swapchain !");
    vkb::SwapchainBuilder swapchainBuilder{gData.physicalDevice, gData.device, gData.surface};

    gData.swapchainImageFormat = vk::Format::eR8G8B8A8Unorm;

    vkb::Swapchain vkbSwapchain = swapchainBuilder
        //.use_default_format_selection()
        .set_desired_format(VkSurfaceFormatKHR{.format = (VkFormat)gData.swapchainImageFormat, .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR})
        // use vsync present mode
        .set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
        .set_desired_extent(width, height)
        .add_image_usage_flags(VK_IMAGE_USAGE_TRANSFER_DST_BIT)
        .build()
        .value();

    gData.swapchainExtent = vkbSwapchain.extent;
    // store swapchain and its related images
    gData.swapchain = vkbSwapchain.swapchain;
    auto swapChainImages = vkbSwapchain.get_images().value();
    auto swapChainImageViews = vkbSwapchain.get_image_views().value();

    gData.swapchainImages.clear();
    gData.swapchainImageViews.clear();
    for (const auto &image : swapChainImages)
    {
        gData.swapchainImages.push_back(image);
    }
    for (const auto &imageView : swapChainImageViews)
    {
        gData.swapchainImageViews.push_back(imageView);
    }
    vk::Extent3D drawImageExtent = {
        width,
        height,
        1
    };
    gData.drawImage.imageFormat = vk::Format::eR16G16B16A16Sfloat;
    gData.drawImage.imageExtent = drawImageExtent;

    vk::ImageUsageFlags drawImageUsages = {};
	drawImageUsages |= vk::ImageUsageFlagBits::eTransferSrc;
    drawImageUsages |= vk::ImageUsageFlagBits::eTransferDst;
    drawImageUsages |= vk::ImageUsageFlagBits::eStorage;
    drawImageUsages |= vk::ImageUsageFlagBits::eColorAttachment;

    const VkImageCreateInfo renderImageInfo = imageCreateInfo(gData.drawImage.imageFormat, 
        drawImageUsages, 
        drawImageExtent);

    VmaAllocationCreateInfo renderImageAllocInfo = {};
    renderImageAllocInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
    renderImageAllocInfo.requiredFlags = VkMemoryPropertyFlags(vk::MemoryPropertyFlagBits::eDeviceLocal);
    VkImage image;
    VkResult result;
    if((result = vmaCreateImage(gData.allocator,
        &renderImageInfo,
        &renderImageAllocInfo, 
        &image, 
        &gData.drawImage.allocation, nullptr)) != VK_SUCCESS)
    {
        gLogger->critical("VMA Create image, failed to create image: {}", string_VkResult(result));
        return false;
    }
    gData.drawImage.image = image;
    
    vk::ImageViewCreateInfo renderViewInfo = imageViewCreateInfo(gData.drawImage.imageFormat,
        gData.drawImage.image,
        vk::ImageAspectFlagBits::eColor);
    
    gData.drawImage.imageView = gData.device.createImageView(renderViewInfo).value;

    gData.mainDeletionQueue.push_back([]() {
        gData.device.destroyImageView(gData.drawImage.imageView);
		vmaDestroyImage(gData.allocator, gData.drawImage.image, gData.drawImage.allocation);
	});

    return true;
}