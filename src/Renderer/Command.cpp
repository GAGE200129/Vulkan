#include "pch.hpp"
#include "Renderer.hpp"

bool Renderer::commandInit()
{
    vk::CommandPoolCreateInfo commandPoolInfo =  {};
	commandPoolInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
	commandPoolInfo.queueFamilyIndex = gData.graphicsQueueFamily;

    for (size_t i = 0; i < RENDERER_FRAMES_IN_FLIGHT; i++) 
    {

        auto poolResult = gData.device.createCommandPool(commandPoolInfo);
        if(poolResult.result != vk::Result::eSuccess)
        {
            gLogger->critical("Failed to create command pool: {}", vk::to_string(poolResult.result));
            return false;
        }
        gData.frames[i].commandPool = poolResult.value;

		// allocate the default command buffer that we will use for rendering
		vk::CommandBufferAllocateInfo cmdAllocInfo = {};
		cmdAllocInfo.commandPool = gData.frames[i].commandPool;
		cmdAllocInfo.commandBufferCount = 1;
		cmdAllocInfo.level = vk::CommandBufferLevel::ePrimary;

        auto cmdAllocResult = gData.device.allocateCommandBuffers(cmdAllocInfo);
        if(cmdAllocResult.result != vk::Result::eSuccess)
        {
            gLogger->critical("Failed to allocate command buffer: {}", vk::to_string(cmdAllocResult.result));
            return false;
        }
        gData.frames[i].mainCommandBuffer = cmdAllocResult.value.at(0);
	}
    return true;
}
