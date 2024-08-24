#pragma once

#include <vulkan/vulkan.h>

namespace gage::gfx::data
{
    class Device;
    class DescriptorPool
    {
    public:
        DescriptorPool(const Device& device);
        ~DescriptorPool();
    private:
        const Device& device;
    public:
        VkDescriptorPool pool{};
    };
}