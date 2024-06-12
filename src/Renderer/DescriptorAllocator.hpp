#pragma once

class DescriptorAllocator
{
public:
    struct PoolSizeRatio
    {
        vk::DescriptorType type;
        float ratio;
    };

    void init(vk::Device device, uint32_t maxSet, std::span<PoolSizeRatio> poolRatios);
    void clear(vk::Device device);
    void cleanup(vk::Device device);

    std::optional<vk::DescriptorSet> allocate(vk::Device device, vk::DescriptorSetLayout layout);
private:
    vk::DescriptorPool mPool;
};