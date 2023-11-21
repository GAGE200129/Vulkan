#include "VulkanTexture.hpp"

#include <stb/stb_image.h>

void VulkanTexture::loadFromFile(const std::string &filePath)
{
  int texWidth, texHeight, texChannels;
  stbi_uc *pixels = stbi_load(filePath.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
  VkDeviceSize imageSize = texWidth * texHeight * 4;

  if (!pixels)
  {
    throw std::runtime_error("failed to load texture image!");
  }

  VulkanBuffer staging(mEngine);
  staging.init(imageSize, vk::BufferUsageFlagBits::eTransferSrc,
               vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
  staging.copy(pixels, imageSize);
  stbi_image_free(pixels);

  init(texWidth, texHeight, vk::Format::eR8G8B8A8Srgb, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled, vk::MemoryPropertyFlagBits::eDeviceLocal);

  staging.cleanup();
}

void VulkanTexture::transitionLayout(vk::Format format, vk::ImageLayout oldLayout, vk::ImageLayout newLayout)
{
  vk::CommandBufferAllocateInfo allocInfo;
  allocInfo.setLevel(vk::CommandBufferLevel::ePrimary)
      .setCommandPool(mEngine.mCommandPool)
      .setCommandBufferCount(1);

  vk::CommandBuffer commandBuffer = mEngine.mDevice.allocateCommandBuffers(allocInfo)[0];
  vk::CommandBufferBeginInfo beginInfo;
  beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

  commandBuffer.begin(beginInfo);
  
  commandBuffer.end();

  vk::SubmitInfo submitInfo;
  submitInfo.setCommandBuffers(commandBuffer);
  mEngine.mGraphicQueue.submit(submitInfo);
  mEngine.mGraphicQueue.waitIdle();
  mEngine.mDevice.freeCommandBuffers(mEngine.mCommandPool, commandBuffer);
}

void VulkanTexture::init(uint32_t width, uint32_t height, vk::Format format, vk::ImageTiling tiling,
                         vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties)
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

  mHandle = mEngine.mDevice.createImage(imageCI);

  vk::MemoryRequirements memRequirements;
  mEngine.mDevice.getImageMemoryRequirements(mHandle);

  vk::MemoryAllocateInfo memAI;
  memAI.setAllocationSize(memRequirements.size)
      .setMemoryTypeIndex(mEngine.findMemoryType(memRequirements.memoryTypeBits, properties));

  mMemory = mEngine.mDevice.allocateMemory(memAI);

  mEngine.mDevice.bindImageMemory(mHandle, mMemory, {0});
}