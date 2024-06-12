#pragma once


class DescriptorLayoutBuilder
{
public:
    void addBinding(uint32_t binding, vk::DescriptorType type);
    void clear();
    std::optional<vk::DescriptorSetLayout> build(vk::Device device, vk::ShaderStageFlags shaderStages, void* pNext = nullptr, vk::DescriptorSetLayoutCreateFlags flags = (vk::DescriptorSetLayoutCreateFlags)0);
private:
    std::vector<vk::DescriptorSetLayoutBinding> mBindings;
};