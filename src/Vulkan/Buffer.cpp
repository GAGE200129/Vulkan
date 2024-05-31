#include "pch.hpp"

#include "VulkanEngine.hpp"

bool VulkanEngine::bufferInit(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags props, VulkanBuffer &outBuffer)
{
    spdlog::info("Creating vulkan buffer: size: {}, usage: {}, property: {}", size, vk::to_string(usage), vk::to_string(props));
    vk::BufferCreateInfo bufferCI;
    bufferCI.setSize(size)
        .setUsage(usage)
        .setSharingMode(vk::SharingMode::eExclusive);

    outBuffer.buffer = VulkanEngine::gData.device.createBuffer(bufferCI).value;

    vk::MemoryRequirements memRequirements = VulkanEngine::gData.device.getBufferMemoryRequirements(outBuffer.buffer);
    vk::MemoryAllocateInfo allocInfo;
    allocInfo.setAllocationSize(memRequirements.size)
        .setMemoryTypeIndex(VulkanEngine::findMemoryType(memRequirements.memoryTypeBits, props));

    outBuffer.bufferMemory = VulkanEngine::gData.device.allocateMemory(allocInfo).value;
    VulkanEngine::gData.device.bindBufferMemory(outBuffer.buffer, outBuffer.bufferMemory, 0);

    return true;
}

void VulkanEngine::bufferCopy(const void *data, vk::DeviceSize size, VulkanBuffer &outBuffer)
{
    void *mappedData = VulkanEngine::gData.device.mapMemory(outBuffer.bufferMemory, 0, size).value;
    memcpy(mappedData, data, size);
    VulkanEngine::gData.device.unmapMemory(outBuffer.bufferMemory);
}

void VulkanEngine::bufferInitAndTransferToLocalDevice(const void *data, vk::DeviceSize size, vk::BufferUsageFlagBits usage, VulkanBuffer &outBuffer)
{

    VulkanBuffer stagingBuffer;

    bufferInit(size, vk::BufferUsageFlagBits::eTransferSrc,
               vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, stagingBuffer);
    bufferCopy(data, size, stagingBuffer);

    VulkanEngine::bufferInit(size, vk::BufferUsageFlagBits::eTransferDst | usage,
                             vk::MemoryPropertyFlagBits::eDeviceLocal, outBuffer);

    bufferCopy(stagingBuffer, outBuffer, size);
    bufferCleanup(stagingBuffer);
}

void VulkanEngine::bufferCopy(const VulkanBuffer &src, VulkanBuffer &dst,const  vk::DeviceSize size)
{
    vk::CommandBufferAllocateInfo cmdAI;
    cmdAI.setLevel(vk::CommandBufferLevel::ePrimary)
        .setCommandPool(VulkanEngine::gData.commandPool)
        .setCommandBufferCount(1);
    vk::CommandBuffer cmdBuf = VulkanEngine::gData.device.allocateCommandBuffers(cmdAI).value[0];

    vk::CommandBufferBeginInfo cmdBeginI;
    cmdBeginI.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
    cmdBuf.begin(cmdBeginI);

    vk::BufferCopy region;
    region.setSize(size);
    cmdBuf.copyBuffer(src.buffer, dst.buffer, region);
    cmdBuf.end();

    vk::SubmitInfo submitI;
    submitI.setCommandBuffers(cmdBuf);
    gData.transferQueue.submit(submitI);
    gData.transferQueue.waitIdle();

    gData.device.freeCommandBuffers(gData.commandPool, cmdBuf);
}

void VulkanEngine::bufferCleanup(VulkanBuffer& buffer)
{
    VulkanEngine::gData.device.destroyBuffer(buffer.buffer);
    VulkanEngine::gData.device.freeMemory(buffer.bufferMemory);
}

void* VulkanEngine::bufferGetMapped(VulkanBuffer& buffer, vk::DeviceSize offset, vk::DeviceSize size)
{
    return VulkanEngine::gData.device.mapMemory(buffer.bufferMemory, offset, size).value;
}
