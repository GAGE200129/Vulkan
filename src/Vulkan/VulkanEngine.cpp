#include "pch.hpp"
#include "VulkanEngine.hpp"

#include "ECS/Components.hpp"

#include "ECS/GameObject.hpp"

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
    void *pUserData)
{

    switch (messageSeverity)
    {
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
        spdlog::info("{}", pCallbackData->pMessage);
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
        spdlog::warn("{}", pCallbackData->pMessage);
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
        spdlog::error("{}", pCallbackData->pMessage);
        break;
    }

    return VK_FALSE;
}

VulkanData VulkanEngine::gData = {};


bool VulkanEngine::init()
{
    spdlog::info("Creating a vulkan window !");
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    gData.window = glfwCreateWindow(EngineConstants::DISPLAY_WIDTH, EngineConstants::DISPLAY_HEIGHT,
         EngineConstants::DISPLAY_TITLE, nullptr, nullptr);
    glfwSetFramebufferSizeCallback(gData.window, [](GLFWwindow *pWindow, int width, int height)
    {
        gData.windowResized = true;
    });


    spdlog::info("Creating a vulkan context !");
    if (!initVulkan())
        return false;
    if (!initSurface())
        return false;
    if (!initDevice())
        return false;
    if (!initSwapExtent())
        return false;

    if (!initSwapchain())
        return false;

    if (!initSwapchainImageViews())
        return false;

    if (!initRenderPass())
        return false;

    if (!initDescriptorPool())
        return false;

    if (!staticModelPipelineInit())
        return false;

    if (!initDepthBuffer())
        return false;
    if (!initSwapchainFramebuffers())
        return false;
    if (!initCommandPool())
        return false;
    if (!initCommandBuffer())
        return false;
    if (!initSyncObjects())
        return false;

    return true;
}

uint32_t VulkanEngine::findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags props)
{
    vk::PhysicalDeviceMemoryProperties physicalProps = gData.physicalDevice.value().getMemoryProperties();
    for (uint32_t i = 0; i < physicalProps.memoryTypeCount; i++)
    {
        if (typeFilter & (1 << i) &&
            (physicalProps.memoryTypes[i].propertyFlags & props) ==
                props)
        {
            return i;
        }
    }
    throw std::runtime_error("Failed to find suitable memory type !");
}

bool VulkanEngine::initVulkan()
{
    vk::ApplicationInfo info;
    info.setApiVersion(VK_MAKE_API_VERSION(0, 1, 0, 0))
        .setApplicationVersion(VK_MAKE_API_VERSION(0, 1, 0, 0))
        .setEngineVersion(VK_MAKE_API_VERSION(0, 1, 0, 0))
        .setPEngineName("EnGAGE");
    vk::InstanceCreateInfo createInfo;
    createInfo.setPApplicationInfo(&info);

    createInfo.setPEnabledExtensionNames(EngineConstants::VULKAN_EXTENSIONS)
        .setPEnabledLayerNames(EngineConstants::VULKAN_LAYERS);

    const auto [instanceResult, instance] = vk::createInstance(createInfo);
    if (instanceResult != vk::Result::eSuccess)
    {
        spdlog::critical("Failed to create instance: {}", vk::to_string(instanceResult));
        return false;
    }
    gData.instance = instance;
    gData.dynamicDispatcher.init(gData.instance);

    // Create debug messenger
    vk::DebugUtilsMessengerCreateInfoEXT debugCreateInfo;
    debugCreateInfo.setMessageSeverity(vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
                                       vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
                                       vk::DebugUtilsMessageSeverityFlagBitsEXT::eError)
        .setMessageType(vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
                        vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
                        vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance)
        .setPfnUserCallback(debugCallback)
        .setPUserData(nullptr);

    const auto [debugMessengerResult, debugMessenger] =
        gData.instance.createDebugUtilsMessengerEXT(debugCreateInfo, nullptr, gData.dynamicDispatcher);
    if (debugMessengerResult != vk::Result::eSuccess)
    {
        spdlog::critical("Failed to create debug utils: {}", vk::to_string(debugMessengerResult));
        return false;
    }
    gData.debugMessenger = debugMessenger;

    return true;
}

void VulkanEngine::joint()
{
    gData.device.waitIdle();
}

void VulkanEngine::render(const Camera &camera)
{
    constexpr uint64_t UINT64_T_MAX = std::numeric_limits<uint64_t>::max();
    if (gData.device.waitForFences(gData.inFlightLocker, true, UINT64_MAX) != vk::Result::eSuccess)
        throw std::runtime_error("vkWaitForFences error !");
    auto imageResult = gData.device.acquireNextImageKHR(gData.swapchain, UINT64_MAX, gData.imageAvalidableGSignal, nullptr);
    if (imageResult.result == vk::Result::eErrorOutOfDateKHR)
    {
        recreateSwapchain();
        return;
    }
    else if (imageResult.result != vk::Result::eSuccess && imageResult.result != vk::Result::eSuboptimalKHR)
    {
        throw std::runtime_error("Failed to acquire next swapchain image !");
    }
    gData.device.resetFences(gData.inFlightLocker);

    gData.currentSwapchainImageIndex = imageResult.value;
    vk::CommandBuffer &cmdBuffer = gData.commandBuffer;
    cmdBuffer.reset();

    vk::CommandBufferBeginInfo cmdBufferBeginInfo;
    if (cmdBuffer.begin(&cmdBufferBeginInfo) != vk::Result::eSuccess)
        throw std::runtime_error("Failed to record cmd buffer !");

    std::array<vk::ClearValue, 2> clearValues;

    clearValues[0].setColor(std::array{0.1f, 0.1f, 0.1f, 1.0f});
    clearValues[1].setDepthStencil({1.0f, 0});
    vk::RenderPassBeginInfo renderpassBeginInfo;
    renderpassBeginInfo.setRenderPass(gData.renderPass)
        .setFramebuffer(gData.swapchainFramebuffers[gData.currentSwapchainImageIndex])
        .setRenderArea({{0, 0}, gData.swapExtent})
        .setClearValues(clearValues);

    // Update ubo
    VulkanUniformBufferObject ubo;
    ubo.proj = camera.getProjection(gData.swapExtent);
    ubo.view = camera.getView();
    ubo.proj[1][1] *= -1;
    std::memcpy(gData.globalUniformBufferMap, &ubo, sizeof(ubo));

    cmdBuffer.beginRenderPass(renderpassBeginInfo, vk::SubpassContents::eInline);

    vk::Viewport viewport;
    viewport.setX(0)
        .setY(0)
        .setWidth(gData.swapExtent.width)
        .setHeight(gData.swapExtent.height)
        .setMinDepth(0.0f)
        .setMaxDepth(1.0f);
    cmdBuffer.setViewport(0, viewport);
    vk::Rect2D scissor;
    scissor.setOffset({0, 0})
        .setExtent(gData.swapExtent);
    cmdBuffer.setScissor(0, scissor);

    //Render
    

    cmdBuffer.endRenderPass();
    cmdBuffer.end();

    vk::SubmitInfo submitInfo;
    std::vector<vk::PipelineStageFlags> waitStages = {vk::PipelineStageFlagBits::eColorAttachmentOutput};
    submitInfo.setWaitSemaphores(gData.imageAvalidableGSignal)
        .setWaitDstStageMask(waitStages)
        .setCommandBuffers(gData.commandBuffer)
        .setSignalSemaphores(gData.renderFinishedGSignal);

    gData.graphicQueue.submit(submitInfo, gData.inFlightLocker);

    vk::PresentInfoKHR presentInfo;
    presentInfo.setWaitSemaphores(gData.renderFinishedGSignal)
        .setSwapchains(gData.swapchain)
        .setImageIndices(gData.currentSwapchainImageIndex);

    vk::Result presentResult = gData.presentQueue.presentKHR(presentInfo);

    if (presentResult == vk::Result::eErrorOutOfDateKHR || presentResult == vk::Result::eSuboptimalKHR || gData.windowResized)
    {
        gData.windowResized = false;
        recreateSwapchain();
    }
    else if (presentResult != vk::Result::eSuccess)
    {
        throw std::runtime_error("Failed to present !");
    }
}

bool VulkanEngine::initDepthBuffer()
{
    vk::ImageCreateInfo imageCI;
    imageCI.setImageType(vk::ImageType::e2D)
        .setExtent(vk::Extent3D(gData.swapExtent.width, gData.swapExtent.height, 1))
        .setMipLevels(1)
        .setArrayLayers(1)
        .setFormat(vk::Format::eD32Sfloat)
        .setTiling(vk::ImageTiling::eOptimal)
        .setInitialLayout(vk::ImageLayout::eUndefined)
        .setUsage(vk::ImageUsageFlagBits::eDepthStencilAttachment)
        .setSamples(vk::SampleCountFlagBits::e1)
        .setSharingMode(vk::SharingMode::eExclusive);
    auto [depthImageResult, depthImage] = gData.device.createImage(imageCI);
    if (depthImageResult != vk::Result::eSuccess)
    {
        spdlog::critical("Failed to create depth image result: {}", vk::to_string(depthImageResult));
        return false;
    }
    gData.depthImage = depthImage;

    vk::MemoryRequirements memRequirements = gData.device.getImageMemoryRequirements(gData.depthImage);
    vk::MemoryAllocateInfo memAI;
    memAI.setAllocationSize(memRequirements.size)
        .setMemoryTypeIndex(findMemoryType(memRequirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal));

    auto [depthMemoryResult, depthMemory] = gData.device.allocateMemory(memAI);

    if (depthMemoryResult != vk::Result::eSuccess)
    {
        spdlog::critical("Failed to create depth memory result: {}", vk::to_string(depthMemoryResult));
        return false;
    }
    gData.depthMemory = depthMemory;

    gData.device.bindImageMemory(gData.depthImage, gData.depthMemory, {0});

    // Create Image view
    vk::ImageViewCreateInfo viewInfo;
    viewInfo
        .setImage(gData.depthImage)
        .setViewType(vk::ImageViewType::e2D)
        .setFormat(vk::Format::eD32Sfloat)
        .subresourceRange
        .setAspectMask(vk::ImageAspectFlagBits::eDepth)
        .setBaseMipLevel(0)
        .setLevelCount(1)
        .setBaseArrayLayer(0)
        .setLayerCount(1);

    auto [depthViewResult, depthView] = gData.device.createImageView(viewInfo);
    if (depthViewResult != vk::Result::eSuccess)
    {
        spdlog::critical("Failed to create depth view: {}", vk::to_string(depthViewResult));
        return false;
    }
    gData.depthView = depthView;
    return true;
}

void VulkanEngine::cleanup()
{
    gData.device.destroySemaphore(gData.imageAvalidableGSignal);
    gData.device.destroySemaphore(gData.renderFinishedGSignal);
    gData.device.destroyFence(gData.inFlightLocker);

    gData.device.destroyCommandPool(gData.commandPool);

    staticModelPipelineCleanup();
    bufferCleanup(gData.globalUniformBuffer);
    gData.device.destroyDescriptorSetLayout(gData.globalDescriptorLayout);
    gData.device.destroyDescriptorPool(gData.descriptorPool);
    gData.device.destroyRenderPass(gData.renderPass);

    cleanupSwapchain();
    gData.device.destroy();

    gData.instance.destroyDebugUtilsMessengerEXT(gData.debugMessenger, nullptr, gData.dynamicDispatcher);
    gData.instance.destroy();
    glfwDestroyWindow(VulkanEngine::gData.window);
    glfwTerminate();
}
