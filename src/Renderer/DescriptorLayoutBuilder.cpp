#include "pch.hpp"
#include "DescriptorLayoutBuilder.hpp"

void DescriptorLayoutBuilder::addBinding(uint32_t binding, vk::DescriptorType type)
{
    vk::DescriptorSetLayoutBinding newbind = {};
    newbind.binding = binding;
    newbind.descriptorCount = 1;
    newbind.descriptorType = type;

    mBindings.push_back(newbind);
}

void DescriptorLayoutBuilder::clear()
{
    mBindings.clear();
}

std::optional<vk::DescriptorSetLayout> DescriptorLayoutBuilder::build(vk::Device device, vk::ShaderStageFlags shaderStages, void* pNext, vk::DescriptorSetLayoutCreateFlags flags)
{
    for (auto& b : mBindings) {
        b.stageFlags |= shaderStages;
    }

    vk::DescriptorSetLayoutCreateInfo info = {};
    info.pNext = pNext;

    info.pBindings = mBindings.data();
    info.bindingCount = (uint32_t)mBindings.size();
    info.flags = flags;

    
    auto result = device.createDescriptorSetLayout(info);

    return result.result == vk::Result::eSuccess ? result.value : std::optional<vk::DescriptorSetLayout>{};
}