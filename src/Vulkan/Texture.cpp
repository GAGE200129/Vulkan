#include "pch.hpp"
#include "VulkanEngine.hpp"

#include <stb/stb_image.h>

bool VulkanEngine::textureLoadFromFile(const std::string &filePath, vk::DescriptorSetLayout layout, VulkanTexture &outTexture)
{
    // Load to memory using stbi
    int texWidth, texHeight, texChannels;
    stbi_uc *pixels = stbi_load(filePath.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    VkDeviceSize imageSize = texWidth * texHeight * 4;

    if (!pixels)
    {
        spdlog::critical("Failed to load texture image!");
        return false;
    }
    // Create a staging buffer and copy to it
    VulkanBuffer staging;
    bufferInit(imageSize, vk::BufferUsageFlagBits::eTransferSrc,
                 vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, staging);
    bufferCopy(pixels, imageSize, staging);
    stbi_image_free(pixels);

    // Create image handle
    textureInit(texWidth, texHeight,
                vk::Format::eR8G8B8A8Srgb,
                vk::ImageTiling::eOptimal,
                vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
                vk::MemoryPropertyFlagBits::eDeviceLocal, layout, outTexture);

    // Upload image to GPU
    textureTransitionLayout(outTexture.handle, vk::Format::eR8G8B8Srgb,
                            vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);

    vk::CommandBufferAllocateInfo allocInfo;
    allocInfo.setLevel(vk::CommandBufferLevel::ePrimary)
        .setCommandPool(VulkanEngine::gData.commandPool)
        .setCommandBufferCount(1);

    auto commandBuffers = VulkanEngine::gData.device.allocateCommandBuffers(allocInfo).value;
    vk::CommandBuffer commandBuffer = commandBuffers[0];
    vk::CommandBufferBeginInfo beginInfo;
    beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

    commandBuffer.begin(beginInfo);

    vk::BufferImageCopy region;
    region.imageSubresource.setAspectMask(vk::ImageAspectFlagBits::eColor)
        .setLayerCount(1);
    region.setImageOffset(vk::Offset3D(0, 0, 0))
        .setImageExtent(vk::Extent3D(texWidth, texHeight, 1));

    commandBuffer.copyBufferToImage(staging.buffer, outTexture.handle, vk::ImageLayout::eTransferDstOptimal, {region});
    commandBuffer.end();

    vk::SubmitInfo submitInfo;
    submitInfo.setCommandBuffers(commandBuffer);
    VulkanEngine::gData.graphicQueue.submit(submitInfo);
    VulkanEngine::gData.graphicQueue.waitIdle();
    VulkanEngine::gData.device.freeCommandBuffers(VulkanEngine::gData.commandPool, commandBuffer);

    textureTransitionLayout(outTexture.handle, vk::Format::eR8G8B8Srgb,
                            vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);

    bufferCleanup(staging);

    return true;
}

void VulkanEngine::textureTransitionLayout(vk::Image handle, vk::Format format, vk::ImageLayout oldLayout, vk::ImageLayout newLayout)
{
    vk::CommandBufferAllocateInfo allocInfo;
    allocInfo.setLevel(vk::CommandBufferLevel::ePrimary)
        .setCommandPool(VulkanEngine::gData.commandPool)
        .setCommandBufferCount(1);

    auto commandBuffers = VulkanEngine::gData.device.allocateCommandBuffers(allocInfo).value;
    vk::CommandBuffer commandBuffer = commandBuffers[0];
    vk::CommandBufferBeginInfo beginInfo;
    beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

    commandBuffer.begin(beginInfo);

    vk::ImageMemoryBarrier barrier;
    barrier.setOldLayout(oldLayout)
        .setNewLayout(newLayout)
        .setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
        .setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
        .setImage(handle)
        .setSrcAccessMask(vk::AccessFlagBits::eNone)
        .setDstAccessMask(vk::AccessFlagBits::eNone)
        .subresourceRange
        .setAspectMask(vk::ImageAspectFlagBits::eColor)
        .setBaseMipLevel(0)
        .setLevelCount(1)
        .setBaseArrayLayer(0)
        .setLayerCount(1);

    vk::PipelineStageFlags sourceStage;
    vk::PipelineStageFlags destinationStage;

    if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eTransferDstOptimal)
    {
        barrier.setSrcAccessMask(vk::AccessFlagBits::eNone)
            .setDstAccessMask(vk::AccessFlagBits::eTransferWrite);

        sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
        destinationStage = vk::PipelineStageFlagBits::eTransfer;
    }
    else if (oldLayout == vk::ImageLayout::eTransferDstOptimal && newLayout == vk::ImageLayout::eShaderReadOnlyOptimal)
    {
        barrier.setSrcAccessMask(vk::AccessFlagBits::eTransferWrite)
            .setDstAccessMask(vk::AccessFlagBits::eShaderRead);

        sourceStage = vk::PipelineStageFlagBits::eTransfer;
        destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
    }
    else
    {
        throw std::invalid_argument("unsupported layout transition!");
    }

    commandBuffer.pipelineBarrier(sourceStage, destinationStage,
                        (vk::DependencyFlags)0,
                        {},
                        {},
                        {barrier});

    commandBuffer.end();

    vk::SubmitInfo submitInfo;
    submitInfo.setCommandBuffers(commandBuffer);
    VulkanEngine::gData.graphicQueue.submit(submitInfo);
    VulkanEngine::gData.graphicQueue.waitIdle();
    VulkanEngine::gData.device.freeCommandBuffers(VulkanEngine::gData.commandPool, commandBuffer);
}

void VulkanEngine::textureInit(uint32_t width, uint32_t height, vk::Format format, vk::ImageTiling tiling,
                               vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties, vk::DescriptorSetLayout layout, VulkanTexture &outTexture)
{
    spdlog::info("Creating vulkan texture: width: {}, height: {}, format: {}, tiling: {}, usage: {}, property: {}",
                 width, height, vk::to_string(format),
                 vk::to_string(tiling),
                 vk::to_string(usage),
                 vk::to_string(properties));
    vk::Device &device = VulkanEngine::gData.device;
    vk::ImageCreateInfo imageCI;
    imageCI.setImageType(vk::ImageType::e2D)
        .setExtent(vk::Extent3D(width, height, 1))
        .setMipLevels(1)
        .setArrayLayers(1)
        .setFormat(format)
        .setTiling(tiling)
        .setInitialLayout(vk::ImageLayout::eUndefined)
        .setUsage(usage)
        .setSamples(vk::SampleCountFlagBits::e1)
        .setSharingMode(vk::SharingMode::eExclusive);
    outTexture.handle = device.createImage(imageCI).value;

    vk::MemoryRequirements memRequirements = device.getImageMemoryRequirements(outTexture.handle);
    vk::MemoryAllocateInfo memAI;
    memAI.setAllocationSize(memRequirements.size)
        .setMemoryTypeIndex(VulkanEngine::findMemoryType(memRequirements.memoryTypeBits, properties));

    outTexture.memory = device.allocateMemory(memAI).value;
    device.bindImageMemory(outTexture.handle, outTexture.memory, {0});

    // Create Image view
    vk::ImageViewCreateInfo viewInfo;
    viewInfo
        .setImage(outTexture.handle)
        .setViewType(vk::ImageViewType::e2D)
        .setFormat(format)
        .subresourceRange
        .setAspectMask(vk::ImageAspectFlagBits::eColor)
        .setBaseMipLevel(0)
        .setLevelCount(1)
        .setBaseArrayLayer(0)
        .setLayerCount(1);

    outTexture.imageView = device.createImageView(viewInfo).value;

    // Create sampler
    vk::SamplerCreateInfo samplerCI;
    samplerCI
        .setMagFilter(vk::Filter::eNearest)
        .setMinFilter(vk::Filter::eNearest)
        .setAddressModeU(vk::SamplerAddressMode::eRepeat)
        .setAddressModeV(vk::SamplerAddressMode::eRepeat)
        .setAddressModeW(vk::SamplerAddressMode::eRepeat)
        .setAnisotropyEnable(false)
        .setMaxAnisotropy(gData.physicalDevice->getProperties().limits.maxSamplerAnisotropy)
        .setBorderColor(vk::BorderColor::eIntOpaqueBlack)
        .setUnnormalizedCoordinates(false)
        .setCompareEnable(false)
        .setCompareOp(vk::CompareOp::eAlways)
        .setMipmapMode(vk::SamplerMipmapMode::eNearest);

    outTexture.sampler = device.createSampler(samplerCI).value;

    vk::DescriptorSetAllocateInfo dsAI;
    dsAI.setDescriptorPool(gData.descriptorPool)
        .setSetLayouts(layout);
    outTexture.descriptorSet = device.allocateDescriptorSets(dsAI).value[0];
    vk::DescriptorImageInfo imageInfo;
    imageInfo.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
        .setImageView(outTexture.imageView)
        .setSampler(outTexture.sampler);
    vk::WriteDescriptorSet writeDescriptor;
    writeDescriptor.setDstSet(outTexture.descriptorSet)
        .setDstBinding(0)
        .setDstArrayElement(0)
        .setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
        .setDescriptorCount(1)
        .setImageInfo(imageInfo);
    device.updateDescriptorSets(writeDescriptor, {});
}

void VulkanEngine::textureCleanup(VulkanTexture& texture)
{
    gData.device.freeDescriptorSets(gData.descriptorPool, texture.descriptorSet);
    gData.device.destroyImage(texture.handle);
    gData.device.destroyImageView(texture.imageView);
    gData.device.freeMemory(texture.memory);
    gData.device.destroySampler(texture.sampler);
}