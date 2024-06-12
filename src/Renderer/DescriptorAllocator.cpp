#include "pch.hpp"
#include "DescriptorAllocator.hpp"

void DescriptorAllocator::init(vk::Device device, uint32_t maxSets, std::span<PoolSizeRatio> poolRatios)
{
    std::vector<vk::DescriptorPoolSize> poolSizes;
    for (PoolSizeRatio ratio : poolRatios) {

        vk::DescriptorPoolSize size;
        size.type = ratio.type;
        size.descriptorCount = uint32_t(ratio.ratio * maxSets);
        poolSizes.push_back(size);
    }

	vk::DescriptorPoolCreateInfo poolInfo = {};
	poolInfo.maxSets = maxSets;
	poolInfo.poolSizeCount = (uint32_t)poolSizes.size();
	poolInfo.pPoolSizes = poolSizes.data();

    auto result = device.createDescriptorPool(poolInfo);
    if(result.result != vk::Result::eSuccess)
        spdlog::critical("Failed to create descriptor pool: {}", vk::to_string(result.result));
    
    mPool = result.value;
}

void DescriptorAllocator::clear(vk::Device device)
{
    device.resetDescriptorPool(mPool);
}

void DescriptorAllocator::cleanup(vk::Device device)
{
    device.destroyDescriptorPool(mPool);
}


std::optional<vk::DescriptorSet> DescriptorAllocator::allocate(vk::Device device, vk::DescriptorSetLayout layout)
{
    vk::DescriptorSetAllocateInfo allocInfo = {};
    allocInfo.descriptorPool = mPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &layout;


    auto result = device.allocateDescriptorSets(allocInfo);
    
    std::optional<vk::DescriptorSet> res;
    

    return result.result == vk::Result::eSuccess ? result.value[0] : std::optional<vk::DescriptorSet>{};
}
