#pragma once

#include "VulkanEngine.hpp"

class VulkanTexture
{
    friend class VulkanEngine;
    friend class ModelComponent;
    friend class AnimatedModelComponent;

public:
    VulkanTexture() {}

    void loadFromFile(const std::string &filePath, vk::DescriptorSetLayout outputLayout);

    void cleanup()
    {
        VulkanEngine::mDevice.destroySampler(mSampler);
        VulkanEngine::mDevice.destroyImageView(mImageView);
        VulkanEngine::mDevice.destroyImage(mHandle);
        VulkanEngine::mDevice.freeMemory(mMemory);
    }

private:
    void init(uint32_t width, uint32_t height, vk::Format format, vk::ImageTiling tilting,
              vk::ImageUsageFlags usage, vk::MemoryPropertyFlags, vk::DescriptorSetLayout outputLayout);
    void transitionLayout(vk::Format format, vk::ImageLayout oldLayout, vk::ImageLayout newLayout);
    void copyBufferToImage(const VulkanBuffer &buffer, uint32_t width, uint32_t height);

    vk::CommandBuffer beginSingleTimeCmd();
    void endSingleTimeCmd(vk::CommandBuffer buffer);

private:
    vk::Image mHandle;
    vk::DeviceMemory mMemory;
    vk::ImageView mImageView;
    vk::Sampler mSampler;
    vk::DescriptorSet mDescriptorSet;
};