#include "pch.hpp"
#include "VulkanEngine.hpp"

bool VulkanEngine::initSyncObjects()
{
    vk::SemaphoreCreateInfo semaphoreCI;
    vk::FenceCreateInfo fenceCI;
    fenceCI.setFlags(vk::FenceCreateFlagBits::eSignaled);

    auto [result1, imageAvalidableGSignal] = gData.device.createSemaphore(semaphoreCI);
    auto [result2, renderFinishedGSignal] = gData.device.createSemaphore(semaphoreCI);
    auto [result3, inFlightLocker] = gData.device.createFence(fenceCI);

    if (result1 != vk::Result::eSuccess ||
        result2 != vk::Result::eSuccess ||
        result3 != vk::Result::eSuccess)
    {
        spdlog::critical("Failed to create sync objects: {} {} {}", vk::to_string(result1),
                         vk::to_string(result2),
                         vk::to_string(result3));

        return false;
    }

    gData.imageAvalidableGSignal = imageAvalidableGSignal;
    gData.renderFinishedGSignal = renderFinishedGSignal;
    gData.inFlightLocker = inFlightLocker;

    return true;
}