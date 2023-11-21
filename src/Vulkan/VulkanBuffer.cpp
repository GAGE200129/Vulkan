#include "VulkanBuffer.hpp"

#include "VulkanEngine.hpp"

void VulkanBuffer::init(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags props)
{
  vk::BufferCreateInfo bufferCI;
  bufferCI.setSize(size)
      .setUsage(usage)
      .setSharingMode(vk::SharingMode::eExclusive);

  mBuffer = mEngine.mDevice.createBuffer(bufferCI);

  vk::MemoryRequirements memRequirements = mEngine.mDevice.getBufferMemoryRequirements(mBuffer);
  vk::MemoryAllocateInfo allocInfo;
  allocInfo.setAllocationSize(memRequirements.size)
      .setMemoryTypeIndex(mEngine.findMemoryType(memRequirements.memoryTypeBits, props));

  mBufferMemory = mEngine.mDevice.allocateMemory(allocInfo);
  mEngine.mDevice.bindBufferMemory(mBuffer, mBufferMemory, 0);
}

void VulkanBuffer::initAndTransferToLocalDevice(const void *data, vk::DeviceSize size, vk::BufferUsageFlagBits usage)
{

  VulkanBuffer stagingBuffer(mEngine);

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
      .setCommandPool(mEngine.mCommandPool)
      .setCommandBufferCount(1);
  vk::CommandBuffer cmdBuf = mEngine.mDevice.allocateCommandBuffers(cmdAI)[0];

  vk::CommandBufferBeginInfo cmdBeginI;
  cmdBeginI.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
  cmdBuf.begin(cmdBeginI);

  vk::BufferCopy region;
  region.setSize(size);
  cmdBuf.copyBuffer(other.getBuffer(), mBuffer, region);
  cmdBuf.end();

  vk::SubmitInfo submitI;
  submitI.setCommandBuffers(cmdBuf);
  mEngine.mTransferQueue.submit(submitI);
  mEngine.mTransferQueue.waitIdle();

  mEngine.mDevice.freeCommandBuffers(mEngine.mCommandPool, cmdBuf);
}

void VulkanBuffer::copy(const void *data, vk::DeviceSize size)
{
  void *mappedData = mEngine.mDevice.mapMemory(mBufferMemory, 0, size);
  memcpy(mappedData, data, size);
  mEngine.mDevice.unmapMemory(mBufferMemory);
}

void VulkanBuffer::cleanup() noexcept
{
  mEngine.mDevice.destroyBuffer(mBuffer);
  mEngine.mDevice.freeMemory(mBufferMemory);
}

void *VulkanBuffer::getMapped(vk::DeviceSize offset, vk::DeviceSize size)
{
  return mEngine.mDevice.mapMemory(mBufferMemory, offset, size);
}
