#include "pch.hpp"
#include "VulkanBuffer.hpp"

#include "VulkanEngine.hpp"

void VulkanBuffer::init(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags props)
{
  vk::BufferCreateInfo bufferCI;
  bufferCI.setSize(size)
      .setUsage(usage)
      .setSharingMode(vk::SharingMode::eExclusive);

  mBuffer = VulkanEngine::mDevice.createBuffer(bufferCI);

  vk::MemoryRequirements memRequirements = VulkanEngine::mDevice.getBufferMemoryRequirements(mBuffer);
  vk::MemoryAllocateInfo allocInfo;
  allocInfo.setAllocationSize(memRequirements.size)
      .setMemoryTypeIndex(VulkanEngine::findMemoryType(memRequirements.memoryTypeBits, props));

  mBufferMemory = VulkanEngine::mDevice.allocateMemory(allocInfo);
  VulkanEngine::mDevice.bindBufferMemory(mBuffer, mBufferMemory, 0);
}

void VulkanBuffer::initAndTransferToLocalDevice(const void *data, vk::DeviceSize size, vk::BufferUsageFlagBits usage)
{

  VulkanBuffer stagingBuffer;

  stagingBuffer.init(size, vk::BufferUsageFlagBits::eTransferSrc,
                     vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
  stagingBuffer.copy(data, size);

  this->init(size, vk::BufferUsageFlagBits::eTransferDst | usage,
             vk::MemoryPropertyFlagBits::eDeviceLocal);

  this->copy(stagingBuffer, size);
  stagingBuffer.cleanup();
}

void VulkanBuffer::copy(const VulkanBuffer &other, vk::DeviceSize size)
{
  // Copy
  vk::CommandBufferAllocateInfo cmdAI;
  cmdAI.setLevel(vk::CommandBufferLevel::ePrimary)
      .setCommandPool(VulkanEngine::mCommandPool)
      .setCommandBufferCount(1);
  vk::CommandBuffer cmdBuf = VulkanEngine::mDevice.allocateCommandBuffers(cmdAI)[0];

  vk::CommandBufferBeginInfo cmdBeginI;
  cmdBeginI.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
  cmdBuf.begin(cmdBeginI);

  vk::BufferCopy region;
  region.setSize(size);
  cmdBuf.copyBuffer(other.getBuffer(), mBuffer, region);
  cmdBuf.end();

  vk::SubmitInfo submitI;
  submitI.setCommandBuffers(cmdBuf);
  VulkanEngine::mTransferQueue.submit(submitI);
  VulkanEngine::mTransferQueue.waitIdle();

  VulkanEngine::mDevice.freeCommandBuffers(VulkanEngine::mCommandPool, cmdBuf);
}

void VulkanBuffer::copy(const void *data, vk::DeviceSize size)
{
  void *mappedData = VulkanEngine::mDevice.mapMemory(mBufferMemory, 0, size);
  memcpy(mappedData, data, size);
  VulkanEngine::mDevice.unmapMemory(mBufferMemory);
}

void VulkanBuffer::cleanup() noexcept
{
  VulkanEngine::mDevice.destroyBuffer(mBuffer);
  VulkanEngine::mDevice.freeMemory(mBufferMemory);
}

void *VulkanBuffer::getMapped(vk::DeviceSize offset, vk::DeviceSize size)
{
  return VulkanEngine::mDevice.mapMemory(mBufferMemory, offset, size);
}
