#pragma once

#include "VulkanEngine.hpp"

class VulkanTexture
{
public:
  VulkanTexture(VulkanEngine& engine) : mEngine(engine) {}

  void loadFromFile(const std::string& filePath);
private:
  void init(uint32_t width, uint32_t height, vk::Format format, vk::ImageTiling tilting,
    vk::ImageUsageFlags usage, vk::MemoryPropertyFlags);
  void transitionLayout(vk::Format format, vk::ImageLayout oldLayout, vk::ImageLayout newLayout);
private:
  VulkanEngine mEngine;
  vk::Image mHandle;
  vk::DeviceMemory mMemory;
};