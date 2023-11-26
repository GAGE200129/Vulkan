#pragma once

#include "VulkanEngine.hpp"

class VulkanTexture
{
  friend class VulkanEngine;
public:
  VulkanTexture(VulkanEngine& engine) : mEngine(engine) {}

  void loadFromFile(const std::string& filePath);

  void cleanup()
  {
    mEngine.mDevice.destroySampler(mSampler);
    mEngine.mDevice.destroyImageView(mImageView);
    mEngine.mDevice.destroyImage(mHandle);
    mEngine.mDevice.freeMemory(mMemory);
  }
private:
  void init(uint32_t width, uint32_t height, vk::Format format, vk::ImageTiling tilting,
    vk::ImageUsageFlags usage, vk::MemoryPropertyFlags);
  void transitionLayout(vk::Format format, vk::ImageLayout oldLayout, vk::ImageLayout newLayout);
  void copyBufferToImage(const VulkanBuffer& buffer, uint32_t width, uint32_t height);
  

  vk::CommandBuffer beginSingleTimeCmd();
  void endSingleTimeCmd(vk::CommandBuffer buffer);
private:
  VulkanEngine& mEngine;
  vk::Image mHandle;
  vk::DeviceMemory mMemory;
  vk::ImageView mImageView;
  vk::Sampler mSampler;
  vk::DescriptorSet mDescriptorSet;
};