#include "pch.hpp"
#include "VulkanEngine.hpp"


void VulkanEngine::cleanupSwapchain()
{

    gData.device.destroyImage(gData.depthImage);
    gData.device.destroyImageView(gData.depthView);
    gData.device.freeMemory(gData.depthMemory);
    for (const vk::Framebuffer &framebuffer : gData.swapchainFramebuffers)
    {
        gData.device.destroyFramebuffer(framebuffer);
    }
    for (const vk::ImageView &imageView : gData.swapchainImageViews)
    {
        gData.device.destroyImageView(imageView);
    }
    gData.device.destroySwapchainKHR(gData.swapchain);
    gData.instance.destroySurfaceKHR(gData.surface);
}

bool VulkanEngine::initSwapchain()
{
    uint32_t imageCount = gData.surfaceCapabilities.minImageCount + 1;
    if (gData.surfaceCapabilities.maxImageCount > 0 && imageCount > gData.surfaceCapabilities.maxImageCount)
    {
        imageCount = gData.surfaceCapabilities.maxImageCount;
    }
    gData.swapchainImageCount = imageCount;

    vk::SwapchainCreateInfoKHR createInfo;
    createInfo.setSurface(gData.surface)
        .setMinImageCount(imageCount)
        .setImageFormat(gData.surfaceFormat.format)
        .setImageColorSpace(gData.surfaceFormat.colorSpace)
        .setImageExtent(gData.swapExtent)
        .setImageArrayLayers(1)
        .setImageUsage(vk::ImageUsageFlagBits::eColorAttachment)
        .setPreTransform(gData.surfaceCapabilities.currentTransform)
        .setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque)
        .setPresentMode(gData.presentMode)
        .setClipped(true)
        .setOldSwapchain(nullptr);
    if (gData.graphicsQueueFamily.value() != gData.presentQueueFamily.value())
    {
        std::array<uint32_t, 2> queueFamilyIndices = {gData.graphicsQueueFamily.value(), gData.presentQueueFamily.value()};
        createInfo.setImageSharingMode(vk::SharingMode::eConcurrent)
            .setQueueFamilyIndices(queueFamilyIndices);
    }
    else
    {
        createInfo.setImageSharingMode(vk::SharingMode::eExclusive);
    }

    auto [swapChainResult, swapChain] = gData.device.createSwapchainKHR(createInfo);
    auto [swapChainImagesResult, swapChainImages] = gData.device.getSwapchainImagesKHR(swapChain);
    if (swapChainResult != vk::Result::eSuccess ||
        swapChainImagesResult != vk::Result::eSuccess)
    {
        spdlog::critical("Failed to create swap chain: {} {}",
                         vk::to_string(swapChainResult),
                         vk::to_string(swapChainImagesResult));
        return false;
    }

    gData.swapchain = swapChain;
    gData.swapchainImages = swapChainImages;
    return true;
}

bool VulkanEngine::initSwapchainImageViews()
{
    gData.swapchainImageViews.resize(gData.swapchainImages.size());
    for (size_t i = 0; i < gData.swapchainImages.size(); i++)
    {
        vk::ImageViewCreateInfo ci;
        vk::ImageSubresourceRange crr;
        crr.setAspectMask(vk::ImageAspectFlagBits::eColor)
            .setBaseMipLevel(0)
            .setLevelCount(1)
            .setBaseArrayLayer(0)
            .setLayerCount(1);
        ci.setImage(gData.swapchainImages[i])
            .setViewType(vk::ImageViewType::e2D)
            .setFormat(gData.surfaceFormat.format)
            .setComponents(vk::ComponentMapping())
            .setSubresourceRange(crr);

        auto [imageViewResult, imageView] = gData.device.createImageView(ci);
        if (imageViewResult != vk::Result::eSuccess)
        {
            spdlog::critical("Failed to create image view: {}", vk::to_string(imageViewResult));
            return false;
        }
        gData.swapchainImageViews[i] = imageView;
    }

    return true;
}

bool VulkanEngine::initSwapchainFramebuffers()
{
    gData.swapchainFramebuffers.resize(gData.swapchainImageViews.size());
    for (size_t i = 0; i < gData.swapchainImageViews.size(); i++)
    {

        auto attachments = std::array{gData.swapchainImageViews[i], gData.depthView};
        vk::FramebufferCreateInfo framebufferCI;
        framebufferCI.setRenderPass(gData.renderPass)
            .setAttachments(attachments)
            .setWidth(gData.swapExtent.width)
            .setHeight(gData.swapExtent.height)
            .setLayers(1);

        auto [swapchainFrameBufferResult, swapChainFrameBuffer] = gData.device.createFramebuffer(framebufferCI);
        if (swapchainFrameBufferResult != vk::Result::eSuccess)
        {
            spdlog::critical("Failed to create swapchain framebuffer: {}", vk::to_string(swapchainFrameBufferResult));
            return false;
        }
        gData.swapchainFramebuffers[i] = swapChainFrameBuffer;
    }

    return true;
}

bool VulkanEngine::recreateSwapchain()
{
    if(gData.device.waitIdle() != vk::Result::eSuccess)
    {
        spdlog::critical("Wait idle failed !");
        return false;
    }
    cleanupSwapchain();
    if(!initSurface())
        return false;
    gData.surfaceCapabilities = gData.physicalDevice.value().getSurfaceCapabilitiesKHR(gData.surface).value;
    if(!initSwapExtent())
        return false;
    if(!initSwapchain())
        return false;
    if(!initSwapchainImageViews())
        return false;
    if(!initDepthBuffer())
        return false;
    if(!initSwapchainFramebuffers())
        return false;

    return true;
}

