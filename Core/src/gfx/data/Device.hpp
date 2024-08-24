#pragma once

#include <cstdint>
#include <Core/ThirdParty/VkBootstrap.h>

namespace gage::gfx::data
{
    class Instance;
    class Device
    {
    public:
        Device(const Instance& instance);
        ~Device();

    public:
        vkb::Device vkb_device{};
        VkDevice device{};
        vkb::PhysicalDevice vkb_physical_device{};
        VkPhysicalDevice physical_device{};

        uint32_t queue_family{};
        VkQueue queue{};
    };
}