#include "pch.hpp"
#include "VulkanEngine.hpp"

bool VulkanEngine::initDescriptorPool()
{
    std::array<vk::DescriptorPoolSize, 2> dps;
    dps[0].setDescriptorCount(512).setType(vk::DescriptorType::eUniformBuffer);
    dps[1].setDescriptorCount(512).setType(vk::DescriptorType::eCombinedImageSampler);

    vk::DescriptorPoolCreateInfo descriptorPoolCI;
    descriptorPoolCI.setPoolSizes(dps)
        .setMaxSets(512);

    auto [descriptorPoolResult, descriptorPool] = gData.device.createDescriptorPool(descriptorPoolCI);
    if (descriptorPoolResult != vk::Result::eSuccess)
    {
        spdlog::critical("Failed to create descriptor pool: {}", vk::to_string(descriptorPoolResult));
        return false;
    }

    gData.descriptorPool = descriptorPool;

    vk::DescriptorSetLayoutBinding layoutBinding;
    layoutBinding.setBinding(0)
        .setDescriptorType(vk::DescriptorType::eUniformBuffer)
        .setDescriptorCount(1)
        .setStageFlags(vk::ShaderStageFlagBits::eVertex);

    vk::DescriptorSetLayoutCreateInfo layoutCI;
    layoutCI.setBindings(layoutBinding);

    auto [globalDescriptorLayoutResult, globalDescriptorLayout] = gData.device.createDescriptorSetLayout(layoutCI);
    if (globalDescriptorLayoutResult != vk::Result::eSuccess)
    {
        spdlog::critical("Failed to create global descriptor layout: {}", vk::to_string(globalDescriptorLayoutResult));
        return false;
    }
    gData.globalDescriptorLayout = globalDescriptorLayout;

    // Uniform buffer
    vk::DeviceSize size = sizeof(VulkanUniformBufferObject);
    VulkanEngine::bufferInit(size, vk::BufferUsageFlagBits::eUniformBuffer,
                              vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
                              gData.globalUniformBuffer);
    gData.globalUniformBufferMap = VulkanEngine::bufferGetMapped(gData.globalUniformBuffer, 0, size);

    // Descriptor
    vk::DescriptorSetAllocateInfo dsAI;
    dsAI.setDescriptorPool(gData.descriptorPool)
        .setSetLayouts(gData.globalDescriptorLayout);

    auto [globalDescriptorSetResult, globalDescriptorSet] = gData.device.allocateDescriptorSets(dsAI);
    if (globalDescriptorSetResult != vk::Result::eSuccess)
    {
        spdlog::critical("Failed to create global descriptor set: {}", vk::to_string(globalDescriptorSetResult));
        return false;
    }
    gData.globalDescriptorSet = globalDescriptorSet[0];

    vk::DescriptorBufferInfo bufferInfo;
    bufferInfo.setBuffer(gData.globalUniformBuffer.buffer)
        .setOffset(0)
        .setRange(sizeof(VulkanUniformBufferObject));
    vk::WriteDescriptorSet writeDescriptor;
    writeDescriptor.setDstSet(gData.globalDescriptorSet)
        .setDstBinding(0)
        .setDstArrayElement(0)
        .setDescriptorType(vk::DescriptorType::eUniformBuffer)
        .setDescriptorCount(1)
        .setBufferInfo(bufferInfo);
    gData.device.updateDescriptorSets(writeDescriptor, {});

    return true;
}