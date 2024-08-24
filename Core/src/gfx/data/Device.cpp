#include <pch.hpp>
#include "Device.hpp"

#include "Instance.hpp"
#include "../Graphics.hpp"

namespace gage::gfx::data
{
    Device::Device(const Instance& instance)
    {
         // vulkan 1.2 features
        // VkPhysicalDeviceFeatures features = {};
        // features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;
        // features.ver
        // features12.ver

        // vulkan 1.0 features
        VkPhysicalDeviceFeatures features{};
        features.geometryShader = true;

        // VkPhysicalDeviceVulkan13Features features13 = {};
        // features13.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
        // features13.dynamicRendering = false;
        // features13.synchronization2 = false;

        // use vkbootstrap to select a gpu.
        vkb::PhysicalDeviceSelector selector{instance.vkb_instance};
        auto physical_device_result = selector
                                          .set_minimum_version(1, 3)
                                          .set_required_features(features)
                                          .add_required_extensions(Graphics::ENABLED_DEVICE_EXTENSIONS.size(), Graphics::ENABLED_DEVICE_EXTENSIONS.data())
                                          .set_surface(instance.surface)
                                          .select();

        vkb_check(physical_device_result, "Failed to select physical device: ");

        vkb_physical_device = physical_device_result.value();
        log().info("Selected physical device: " + vkb_physical_device.name);

        vkb::DeviceBuilder deviceBuilder{vkb_physical_device};
        auto vkb_device_result = deviceBuilder.build();
        vkb_check(vkb_device_result, "Failed to create device: ");

        vkb_device = vkb_device_result.value();
        device = vkb_device.device;
        physical_device = vkb_physical_device.physical_device;

        // use vkbootstrap to get a Graphics queue family
        auto queue_family_result = vkb_device.get_queue_index(vkb::QueueType::graphics);
        vkb_check(queue_family_result);
        queue_family = queue_family_result.value();
        // Get queue
        vkGetDeviceQueue(device, queue_family, 0, &queue);
    }

    Device::~Device()
    {
        vkDestroyDevice(device, nullptr);
    }
}