#pragma once

#include <vulkan/vulkan.h>

namespace gage::gfx::data
{
    class Device;
    class DescriptorPool;
    class GlobalDescriptorSetLayout
    {
    public:
        GlobalDescriptorSetLayout(const Device& device);
        ~GlobalDescriptorSetLayout();
    private:
        const Device& device;
    public:
        VkDescriptorSetLayout layout{};
    };
}