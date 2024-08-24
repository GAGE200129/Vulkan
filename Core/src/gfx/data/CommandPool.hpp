#pragma once

#include <vulkan/vulkan.h>

namespace gage::gfx::data
{
    class Device;
    class CommandPool
    {
    public:
        CommandPool(const Device& device);
        ~CommandPool();
    private:
        const Device& device;
    public:
        VkCommandPool pool{};
    };
}