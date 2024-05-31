#include "pch.hpp"
#include "VulkanEngine.hpp"
#include "EngineConstants.hpp"

bool VulkanEngine::initDevice()
{
    // Select physical device
    gData.physicalDevice.reset();
    const auto [devicesResult, devices] = gData.instance.enumeratePhysicalDevices();
    if (devicesResult != vk::Result::eSuccess)
    {
        spdlog::critical("Failed to enumerate physical devices: {}", vk::to_string(devicesResult));
        return false;
    }

    for (const auto &device : devices)
    {
        const vk::PhysicalDeviceProperties properties = device.getProperties();
        const vk::PhysicalDeviceFeatures features = device.getFeatures();
        const auto [surfaceCapabilitiesResult, surfaceCapabilities] = device.getSurfaceCapabilitiesKHR(gData.surface);
        const auto [surfaceFormatResult, surfaceFormats] = device.getSurfaceFormatsKHR(gData.surface);
        const auto [presentModesResult, presentModes] = device.getSurfacePresentModesKHR(gData.surface);
        if (surfaceCapabilitiesResult != vk::Result::eSuccess ||
            surfaceFormatResult != vk::Result::eSuccess ||
            presentModesResult != vk::Result::eSuccess)
        {
            spdlog::critical("Failed to get device and surface features: {} {} {}",
                             vk::to_string(surfaceCapabilitiesResult),
                             vk::to_string(surfaceFormatResult),
                             vk::to_string(presentModesResult));
            return false;
        }

        gData.surfaceCapabilities = surfaceCapabilities;

        bool isIntergrated = properties.deviceType == vk::PhysicalDeviceType::eIntegratedGpu;
        bool isDiscrete = properties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu;
        bool validSurfaceFormat = false;
        for (const auto &format : surfaceFormats)
        {
            if (format.format == EngineConstants::VULKAN_SURFACE_FORMAT && format.colorSpace == EngineConstants::VULKAN_COLOR_SPACE)
            {
                validSurfaceFormat = true;
                gData.surfaceFormat = format;
                break;
            }
        }

        bool validPresentMode = false;
        for (const auto &mode : presentModes)
        {
            if (mode == EngineConstants::VULKAN_PRESENT_MODE)
            {
                validPresentMode = true;
                gData.presentMode = mode;
                break;
            }
        }

        // Select swap extends
        if (isIntergrated || isDiscrete && validSurfaceFormat && validPresentMode)
        {
            gData.physicalDevice = device;
            break;
        }
    }
    if (!gData.physicalDevice.has_value())
    {
        spdlog::critical("Failed to find suitable physical device !");
        return false;
    }

    // Find queue family
    std::vector<vk::QueueFamilyProperties> queueFamilies = gData.physicalDevice.value().getQueueFamilyProperties();
    gData.graphicsQueueFamily.reset();
    gData.presentQueueFamily.reset();
    gData.transferQueueFamily.reset();
    uint32_t i = 0;
    for (const auto &queueFamily : queueFamilies)
    {

        if (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics)
        {
            gData.graphicsQueueFamily = i;
        }
        if (gData.physicalDevice.value().getSurfaceSupportKHR(i, gData.surface).value)
        {
            gData.presentQueueFamily = i;
        }

        if (queueFamily.queueFlags & vk::QueueFlagBits::eTransfer)
        {
            gData.transferQueueFamily = i;
        }

        i++;
    }

    if (!gData.graphicsQueueFamily.has_value() || !gData.presentQueueFamily.has_value() || !gData.transferQueueFamily.has_value())
    {
        spdlog::critical("Failed to find family queue !");
        return false;
    }

    // Create device
    std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = {gData.graphicsQueueFamily.value(), gData.presentQueueFamily.value(), gData.transferQueueFamily.value()};

    std::vector<float> queuePriorities = {1.0f};
    for (uint32_t queueFamily : uniqueQueueFamilies)
    {
        vk::DeviceQueueCreateInfo queueCreateInfo;
        queueCreateInfo.setQueueFamilyIndex(queueFamily)
            .setQueuePriorities(queuePriorities);
        queueCreateInfos.push_back(queueCreateInfo);
    }

    vk::DeviceCreateInfo deviceCreateInfo;
    vk::PhysicalDeviceFeatures features = gData.physicalDevice.value().getFeatures();
    deviceCreateInfo.setQueueCreateInfos(queueCreateInfos)
        .setPEnabledFeatures(&features)
        .setPEnabledExtensionNames(EngineConstants::VULKAN_DEVICE_EXTENSIONS);
    auto [deviceResult, device] = gData.physicalDevice->createDevice(deviceCreateInfo);
    if (deviceResult != vk::Result::eSuccess)
    {
        spdlog::critical("Failed to create device: {}", vk::to_string(deviceResult));
        return false;
    }
    gData.device = device;
    gData.dynamicDispatcher.init(device);
    gData.graphicQueue = device.getQueue(gData.graphicsQueueFamily.value(), 0);
    gData.presentQueue = device.getQueue(gData.presentQueueFamily.value(), 0);
    gData.transferQueue = device.getQueue(gData.transferQueueFamily.value(), 0);

    return true;
}