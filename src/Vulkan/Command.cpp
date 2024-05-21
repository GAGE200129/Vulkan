#include "pch.hpp"
#include "VulkanEngine.hpp"

bool VulkanEngine::initCommandPool()
{
    spdlog::info("Creating command pool !");
    vk::CommandPoolCreateInfo commandPoolCI;
    commandPoolCI.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer)
        .setQueueFamilyIndex(gData.graphicsQueueFamily.value());

    auto [commandPoolResult, commandPool] = gData.device.createCommandPool(commandPoolCI);
    if (commandPoolResult != vk::Result::eSuccess)
    {
        spdlog::critical("Failed to create command pool: {}", vk::to_string(commandPoolResult));
        return false;
    }

    gData.commandPool = commandPool;
    return true;
}

bool VulkanEngine::initCommandBuffer()
{
    vk::CommandBufferAllocateInfo commandBufferAllocateCI;
    commandBufferAllocateCI.setCommandPool(gData.commandPool)
        .setLevel(vk::CommandBufferLevel::ePrimary)
        .setCommandBufferCount(1);

    auto [commandBufferResult, commandBuffers] = gData.device.allocateCommandBuffers(commandBufferAllocateCI);
    if (commandBufferResult != vk::Result::eSuccess)
    {
        spdlog::critical("Failed to create command buffer: {}", vk::to_string(commandBufferResult));
        return false;
    }

    gData.commandBuffer = commandBuffers[0];

    return true;
}