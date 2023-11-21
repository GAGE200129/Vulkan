#pragma once

#include <vulkan/vulkan.hpp>

class VulkanEngine;
class VulkanBuffer
{

public:
  VulkanBuffer(VulkanEngine& engine) : mEngine(engine) {}
  void init(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags props);

  void initAndTransferToLocalDevice(const void *data, vk::DeviceSize size, vk::BufferUsageFlagBits usage);
  void copy(const void* data, vk::DeviceSize size);
  void copy(const VulkanBuffer& other, vk::DeviceSize size);
  void cleanup() noexcept;

  void* getMapped(vk::DeviceSize offset, vk::DeviceSize size);
  inline const vk::Buffer& getBuffer() const noexcept { return mBuffer; }
private:
  VulkanEngine& mEngine;
  vk::Buffer mBuffer;
  vk::DeviceMemory mBufferMemory;
};