#include "pch.hpp"
#include "VulkanTexture.hpp"

#include <stb/stb_image.h>

void VulkanTexture::loadFromFile(const std::string &filePath, vk::DescriptorSetLayout outputLayout)
{
  // Load to memory using stbi
  int texWidth, texHeight, texChannels;
  stbi_uc *pixels = stbi_load(filePath.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
  VkDeviceSize imageSize = texWidth * texHeight * 4;

  if (!pixels)
  {
    throw std::runtime_error("failed to load texture image!");
  }
  // Create a staging buffer and copy to it
  VulkanBuffer staging;
  staging.init(imageSize, vk::BufferUsageFlagBits::eTransferSrc,
               vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
  staging.copy(pixels, imageSize);
  stbi_image_free(pixels);

  // Create image handle
  init(texWidth, texHeight,
       vk::Format::eR8G8B8A8Srgb,
       vk::ImageTiling::eOptimal,
       vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
       vk::MemoryPropertyFlagBits::eDeviceLocal, outputLayout);

  // Upload image to GPU
  transitionLayout(vk::Format::eR8G8B8Srgb,
                   vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);
  copyBufferToImage(staging, texWidth, texHeight);
  transitionLayout(vk::Format::eR8G8B8Srgb,
                   vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);

  staging.cleanup();

  
}


void VulkanTexture::copyBufferToImage(const VulkanBuffer &buffer, uint32_t width, uint32_t height)
{
  auto cmd = beginSingleTimeCmd();
  vk::BufferImageCopy region;

  region.imageSubresource.setAspectMask(vk::ImageAspectFlagBits::eColor)
      .setLayerCount(1);

  region.setImageOffset(vk::Offset3D(0, 0, 0))
      .setImageExtent(vk::Extent3D(width, height, 1));

  cmd.copyBufferToImage(buffer.getBuffer(), mHandle, vk::ImageLayout::eTransferDstOptimal, {region});

  endSingleTimeCmd(cmd);
}

void VulkanTexture::transitionLayout(vk::Format format, vk::ImageLayout oldLayout, vk::ImageLayout newLayout)
{
  auto cmd = beginSingleTimeCmd();
  vk::ImageMemoryBarrier barrier;
  barrier.setOldLayout(oldLayout)
      .setNewLayout(newLayout)
      .setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
      .setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
      .setImage(mHandle)
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

  cmd.pipelineBarrier(sourceStage, destinationStage,
                      (vk::DependencyFlags)0,
                      {},
                      {},
                      {barrier});

  endSingleTimeCmd(cmd);
}

void VulkanTexture::init(uint32_t width, uint32_t height, vk::Format format, vk::ImageTiling tiling,
                         vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties, vk::DescriptorSetLayout outputLayout)
{
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

  mHandle = VulkanEngine::mDevice.createImage(imageCI);

  vk::MemoryRequirements memRequirements = VulkanEngine::mDevice.getImageMemoryRequirements(mHandle);
  vk::MemoryAllocateInfo memAI;
  memAI.setAllocationSize(memRequirements.size)
      .setMemoryTypeIndex(VulkanEngine::findMemoryType(memRequirements.memoryTypeBits, properties));

  mMemory = VulkanEngine::mDevice.allocateMemory(memAI);

  VulkanEngine::mDevice.bindImageMemory(mHandle, mMemory, {0});

  // Create Image view
  vk::ImageViewCreateInfo viewInfo;
  viewInfo
      .setImage(mHandle)
      .setViewType(vk::ImageViewType::e2D)
      .setFormat(vk::Format::eR8G8B8A8Srgb)
      .subresourceRange
      .setAspectMask(vk::ImageAspectFlagBits::eColor)
      .setBaseMipLevel(0)
      .setLevelCount(1)
      .setBaseArrayLayer(0)
      .setLayerCount(1);

  mImageView = VulkanEngine::mDevice.createImageView(viewInfo);

  // Create sampler
  vk::SamplerCreateInfo samplerCI;
  samplerCI
      .setMagFilter(vk::Filter::eNearest)
      .setMinFilter(vk::Filter::eNearest)
      .setAddressModeU(vk::SamplerAddressMode::eRepeat)
      .setAddressModeV(vk::SamplerAddressMode::eRepeat)
      .setAddressModeW(vk::SamplerAddressMode::eRepeat)
      .setAnisotropyEnable(false)
      .setMaxAnisotropy(VulkanEngine::mPhysicalDevice->getProperties().limits.maxSamplerAnisotropy)
      .setBorderColor(vk::BorderColor::eIntOpaqueBlack)
      .setUnnormalizedCoordinates(false)
      .setCompareEnable(false)
      .setCompareOp(vk::CompareOp::eAlways)
      .setMipmapMode(vk::SamplerMipmapMode::eNearest);

  mSampler = VulkanEngine::mDevice.createSampler(samplerCI);

  vk::DescriptorSetAllocateInfo dsAI;
  dsAI.setDescriptorPool(VulkanEngine::mDescriptorPool)
      .setSetLayouts(outputLayout);
  mDescriptorSet = VulkanEngine::mDevice.allocateDescriptorSets(dsAI)[0];
  vk::DescriptorImageInfo imageInfo;
  imageInfo.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
    .setImageView(mImageView)
    .setSampler(mSampler);
  vk::WriteDescriptorSet writeDescriptor;
  writeDescriptor.setDstSet(mDescriptorSet)
      .setDstBinding(0)
      .setDstArrayElement(0)
      .setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
      .setDescriptorCount(1)
      .setImageInfo(imageInfo);
  VulkanEngine::mDevice.updateDescriptorSets(writeDescriptor, {});
}

vk::CommandBuffer VulkanTexture::beginSingleTimeCmd()
{
  vk::CommandBufferAllocateInfo allocInfo;
  allocInfo.setLevel(vk::CommandBufferLevel::ePrimary)
      .setCommandPool(VulkanEngine::mCommandPool)
      .setCommandBufferCount(1);

  vk::CommandBuffer commandBuffer = VulkanEngine::mDevice.allocateCommandBuffers(allocInfo)[0];
  vk::CommandBufferBeginInfo beginInfo;
  beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

  commandBuffer.begin(beginInfo);

  return commandBuffer;
}
void VulkanTexture::endSingleTimeCmd(vk::CommandBuffer cmd)
{
  cmd.end();

  vk::SubmitInfo submitInfo;
  submitInfo.setCommandBuffers(cmd);
  VulkanEngine::mGraphicQueue.submit(submitInfo);
  VulkanEngine::mGraphicQueue.waitIdle();
  VulkanEngine::mDevice.freeCommandBuffers(VulkanEngine::mCommandPool, cmd);
}