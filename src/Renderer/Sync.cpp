#include "pch.hpp"
#include "Renderer.hpp"

bool Renderer::syncInit()
{
    vk::FenceCreateInfo fenceCreateInfo = {};
    fenceCreateInfo.flags = vk::FenceCreateFlagBits::eSignaled;
    
	vk::SemaphoreCreateInfo semaphoreCreateInfo = {};
    for (size_t i = 0; i < RENDERER_FRAMES_IN_FLIGHT; i++) {
        gData.frames[i].renderFence = gData.device.createFence(fenceCreateInfo).value;


		gData.frames[i].renderSemaphore = gData.device.createSemaphore(semaphoreCreateInfo).value;
        gData.frames[i].swapchainSemaphore = gData.device.createSemaphore(semaphoreCreateInfo).value;
	}


    return true;
}
