#include "pch.hpp"
#include "Renderer.hpp"

#include "EngineConstants.hpp"

RendererData Renderer::gData = {};
std::shared_ptr<spdlog::logger> Renderer::gLogger;

static VKAPI_ATTR VkBool32 VKAPI_CALL gDebugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    [[maybe_unused]] VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
    [[maybe_unused]] void *pUserData)
{
    switch (messageSeverity)
    {
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
        Renderer::gLogger->info("{}", pCallbackData->pMessage);
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
        Renderer::gLogger->warn("{}", pCallbackData->pMessage);
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
        Renderer::gLogger->error("{}", pCallbackData->pMessage);
        break;
    default:

        break;
    }

    return VK_FALSE;
}

bool Renderer::init()
{
    gLogger = spdlog::stdout_color_mt("Renderer");
    gLogger->set_pattern("[%n | %^%l%$]: %v");
    gLogger->info("Initializing renderer !");
    if (!displayInit())
        return false;

    gLogger->info("Initializing vulkan !");
    vkb::InstanceBuilder builder;

    // make the vulkan instance, with basic debug features
    auto instanceResult = builder.set_app_name("EnGAGE")
                              .request_validation_layers(EngineConstants::DEBUG_ENABLED)
                              .set_debug_callback(gDebugCallback)
                              .require_api_version(1, 3, 0)
                              .build();

    if (!instanceResult)
    {
        gLogger->critical("Failed to create instance: {}",
                          vk::to_string(vk::Result(instanceResult.vk_result())));
        return false;
    }

    gData.dynamicDispatcher.init(vkGetInstanceProcAddr);
    vkb::Instance vkbInstance = instanceResult.value();
    gData.instance = vkbInstance.instance;
    gData.dynamicDispatcher.init(gData.instance);
    gData.debugMessenger = vkbInstance.debug_messenger;

    gLogger->info("Initializing surface !");
    unsigned int count;
    const char **extensions = glfwGetRequiredInstanceExtensions(&count);
    for (unsigned int i = 0; i < count; i++)
        gLogger->info("Glfw required extension: {}", extensions[i]);

    VkSurfaceKHR surface;
    VkResult surfaceResult = glfwCreateWindowSurface(gData.instance, gData.window, nullptr, &surface);
    if (surfaceResult != VK_SUCCESS)
    {
        gLogger->critical("Failed to create surface: {}",
                          vk::to_string(vk::Result(surfaceResult)));
        return false;
    }
    gData.surface = surface;

    gLogger->info("Initializing device !");
    // vulkan 1.3 features
    vk::PhysicalDeviceVulkan13Features features = {};
    features.dynamicRendering = true;
    features.synchronization2 = true;

    // vulkan 1.2 features
    vk::PhysicalDeviceVulkan12Features features12 = {};
    features12.bufferDeviceAddress = true;
    features12.descriptorIndexing = true;

    vkb::PhysicalDeviceSelector selector{vkbInstance};
    auto physicalDeviceResult = selector
                                    .set_minimum_version(1, 3)
                                    .set_required_features_13(features)
                                    .set_required_features_12(features12)
                                    .set_surface(surface)
                                    .select();
    if (!physicalDeviceResult)
    {
        gLogger->critical("Failed to select physical device: {}",
                          vk::to_string(vk::Result(physicalDeviceResult.vk_result())));
        return false;
    }
    vkb::PhysicalDevice physicalDevice = physicalDeviceResult.value();

    vkb::DeviceBuilder deviceBuilder{physicalDevice};

    auto deviceResult = deviceBuilder.build();
    if (!deviceResult)
    {
        gLogger->critical("Failed to create device: {}",
                          vk::to_string(vk::Result(deviceResult.vk_result())));
        return false;
    }

    // Get the VkDevice handle used in the rest of a vulkan application
    vkb::Device device = deviceResult.value();
    gData.device = device.device;
     // use vkbootstrap to get a Graphics queue
    gData.graphicsQueue = device.get_queue(vkb::QueueType::graphics).value();
    gData.graphicsQueueFamily = device.get_queue_index(vkb::QueueType::graphics).value();
    gData.dynamicDispatcher.init(gData.device);
    gData.physicalDevice = physicalDevice.physical_device;

     //Initilize memory allocator
    gLogger->info("Creating VMA allocator");
    VmaAllocatorCreateInfo allocatorInfo = {};
    allocatorInfo.physicalDevice = gData.physicalDevice;
    allocatorInfo.device = gData.device;
    allocatorInfo.instance = gData.instance;
    allocatorInfo.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
    vmaCreateAllocator(&allocatorInfo, &gData.allocator);

    gData.mainDeletionQueue.push_back([]()
    {
        vmaDestroyAllocator(gData.allocator);
    });


    int width, height;
    glfwGetFramebufferSize(gData.window, &width, &height);
    if(!swapchainInit(width, height))
        return false;

    if (!commandInit())
        return false;

    if (!syncInit())
        return false;

    if(!descriptorsInit())
        return false;

    if(!pipelinesInit())
        return false;

   
    return true;
}

void Renderer::render()
{
    RendererData::Frame &currentFrame = gData.frames[gData.currentFrameIndex % RENDERER_FRAMES_IN_FLIGHT];
    gData.device.waitForFences(currentFrame.renderFence, true, 1000000000);

    deletionQueueFlush(currentFrame.deletionQueue);

    gData.device.resetFences(currentFrame.renderFence);

    uint32_t swapchainImageIndex = gData.device.acquireNextImageKHR(gData.swapchain,
                                                                    1000000000, currentFrame.swapchainSemaphore, {}).value;

    vk::CommandBuffer &cmd = currentFrame.mainCommandBuffer;
    cmd.reset();

    vk::CommandBufferBeginInfo cmdBeginInfo = {};
    cmdBeginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;

    gData.drawExtent.width = gData.drawImage.imageExtent.width;
    gData.drawExtent.height = gData.drawImage.imageExtent.height;
    cmd.begin(cmdBeginInfo);

    imageTransition(cmd, gData.drawImage.image,
                    vk::ImageLayout::eUndefined, vk::ImageLayout::eGeneral);
    
    cmd.bindPipeline(vk::PipelineBindPoint::eCompute, gData.gradientPipeline);
    cmd.bindDescriptorSets(vk::PipelineBindPoint::eCompute, gData.gradientPipelineLayout, 0, {gData.drawImageDescriptor}, {});
    cmd.dispatch(std::ceil(gData.drawExtent.width / 16.0), std::ceil(gData.drawExtent.height / 16.0), 1);



    imageTransition(cmd, gData.drawImage.image,
                    vk::ImageLayout::eGeneral, vk::ImageLayout::eTransferSrcOptimal);
    imageTransition(cmd, gData.swapchainImages[swapchainImageIndex],
                    vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);
    imageCopy(cmd, gData.drawImage.image, gData.swapchainImages[swapchainImageIndex], gData.drawExtent, gData.swapchainExtent);

    imageTransition(cmd, gData.swapchainImages[swapchainImageIndex],
                    vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::ePresentSrcKHR);
    cmd.end();

    vk::CommandBufferSubmitInfo cmdInfo = {};
    cmdInfo.commandBuffer = cmd;
    vk::SemaphoreSubmitInfo waitInfo = {};
    waitInfo.stageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput;
    waitInfo.semaphore = currentFrame.swapchainSemaphore;

    vk::SemaphoreSubmitInfo signalInfo = {};
    signalInfo.stageMask = vk::PipelineStageFlagBits2::eAllGraphics;
    signalInfo.semaphore = currentFrame.renderSemaphore;

    vk::SubmitInfo2 submit = {};
    submit.waitSemaphoreInfoCount = 1;
    submit.pWaitSemaphoreInfos = &waitInfo;

    submit.signalSemaphoreInfoCount = 1;
    submit.pSignalSemaphoreInfos = &signalInfo;

    submit.commandBufferInfoCount = 1;
    submit.pCommandBufferInfos = &cmdInfo;

    gData.graphicsQueue.submit2({submit}, currentFrame.renderFence);

    vk::PresentInfoKHR presentInfo = {};
    presentInfo.pSwapchains = &gData.swapchain;
    presentInfo.swapchainCount = 1;
    presentInfo.pWaitSemaphores = &currentFrame.renderSemaphore;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pImageIndices = &swapchainImageIndex;

    gData.graphicsQueue.presentKHR(presentInfo);

    // increase the number of frames drawn
    gData.currentFrameIndex++;
}

void Renderer::drawBackground(vk::CommandBuffer cmd)
{
    vk::ClearColorValue clearValue;
    float flash = std::abs(std::sin(gData.currentFrameIndex / 120.f));
    clearValue = vk::ClearColorValue{std::array<float, 4>{0.0f, 0.0f, flash, 1.0f}};

    vk::ImageSubresourceRange range = {};
    range.aspectMask = vk::ImageAspectFlagBits::eColor;
    range.baseMipLevel = 0;
    range.levelCount = vk::RemainingMipLevels;
    range.baseArrayLayer = 0;
    range.layerCount = vk::RemainingArrayLayers;

    cmd.clearColorImage(gData.drawImage.image,
                        vk::ImageLayout::eGeneral, clearValue, {range});
}

void Renderer::cleanup()
{
    gData.device.waitIdle();
    deletionQueueFlush(gData.mainDeletionQueue);
    for (size_t i = 0; i < RENDERER_FRAMES_IN_FLIGHT; i++)
    {
        gData.device.destroyCommandPool(gData.frames[i].commandPool);
        // destroy sync objects
        gData.device.destroyFence(gData.frames[i].renderFence);
        gData.device.destroySemaphore(gData.frames[i].renderSemaphore);
        gData.device.destroySemaphore(gData.frames[i].swapchainSemaphore);
    }
    gData.device.destroySwapchainKHR(gData.swapchain);
    for (size_t i = 0; i < gData.swapchainImageViews.size(); i++)
    {
        gData.device.destroyImageView(gData.swapchainImageViews.at(i));
    }
    gData.swapchainImageViews.clear();
    gData.swapchainImages.clear();

    gData.instance.destroySurfaceKHR(gData.surface);
    gData.device.destroy();

    gData.instance.destroyDebugUtilsMessengerEXT(gData.debugMessenger, nullptr, gData.dynamicDispatcher);
    gData.instance.destroy();

    glfwDestroyWindow(gData.window);
    glfwTerminate();
}


