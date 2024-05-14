#include "pch.hpp"
#include "VulkanEngine.hpp"

#include "ECS/Components.hpp"

#include "VulkanTexture.hpp"
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

GLFWwindow *VulkanEngine::mWindow = nullptr;
bool VulkanEngine::mWindowResized = false;
vk::Instance VulkanEngine::mInstance;
std::optional<vk::PhysicalDevice> VulkanEngine::mPhysicalDevice;
vk::Device VulkanEngine::mDevice;
std::optional<uint32_t> VulkanEngine::mGraphicsQueueFamily;
std::optional<uint32_t> VulkanEngine::mPresentQueueFamily;
std::optional<uint32_t> VulkanEngine::mTransferQueueFamily;
vk::Queue VulkanEngine::mGraphicQueue, VulkanEngine::mPresentQueue, VulkanEngine::mTransferQueue;
vk::SurfaceKHR VulkanEngine::mSurface;
vk::SwapchainKHR VulkanEngine::mSwapchain;
std::vector<vk::Image> VulkanEngine::mSwapchainImages;
std::vector<vk::ImageView> VulkanEngine::mSwapchainImageViews;
std::vector<vk::Framebuffer> VulkanEngine::mSwapchainFramebuffers;
vk::DebugUtilsMessengerEXT VulkanEngine::mDebugMessenger;
vk::DispatchLoaderDynamic VulkanEngine::mDynamicDispatcher = vk::DispatchLoaderDynamic(vkGetInstanceProcAddr);
vk::SurfaceCapabilitiesKHR VulkanEngine::mSurfaceCapabilities;
vk::SurfaceFormatKHR VulkanEngine::mSurfaceFormat;
vk::PresentModeKHR VulkanEngine::mPresentMode;
vk::Extent2D VulkanEngine::mSwapExtent;
vk::RenderPass VulkanEngine::mRenderPass;
vk::DescriptorPool VulkanEngine::mDescriptorPool;
StaticModelPipeline VulkanEngine::mStaticModelPipeline;
AnimatedModelPipeline VulkanEngine::mAnimatedModelPipeline;
vk::CommandPool VulkanEngine::mCommandPool;
vk::CommandBuffer VulkanEngine::mCommandBuffer;
vk::Semaphore VulkanEngine::mImageAvalidableGSignal, VulkanEngine::mRenderFinishedGSignal;
vk::Fence VulkanEngine::mInFlightLocker;
vk::Image VulkanEngine::mDepthImage;
vk::DeviceMemory VulkanEngine::mDepthMemory;
vk::ImageView VulkanEngine::mDepthView;
uint32_t VulkanEngine::mCurrentSwapChainImageIndex = 0;
vk::DescriptorSetLayout VulkanEngine::mGlobalDescriptorLayout;
vk::DescriptorSet VulkanEngine::mGlobalDescriptorSet;
VulkanBuffer VulkanEngine::mGlobalUniformBuffer;
void *VulkanEngine::mGlobalUniformBufferMap;

VulkanCamera VulkanEngine::mCamera = {{0, 0, 0}, 0, 0, 0.1, 100.0f, 70.0f};

void VulkanEngine::cleanupSwapchain()
{

    mDevice.destroyImage(mDepthImage);
    mDevice.destroyImageView(mDepthView);
    mDevice.freeMemory(mDepthMemory);
    for (const vk::Framebuffer &framebuffer : mSwapchainFramebuffers)
    {
        mDevice.destroyFramebuffer(framebuffer);
    }
    for (const vk::ImageView &imageView : mSwapchainImageViews)
    {
        mDevice.destroyImageView(imageView);
    }
    mDevice.destroySwapchainKHR(mSwapchain);
    mInstance.destroySurfaceKHR(mSurface);
}

bool VulkanEngine::initSwapchain()
{
    uint32_t imageCount = mSurfaceCapabilities.minImageCount + 1;
    if (mSurfaceCapabilities.maxImageCount > 0 && imageCount > mSurfaceCapabilities.maxImageCount)
    {
        imageCount = mSurfaceCapabilities.maxImageCount;
    }

    vk::SwapchainCreateInfoKHR createInfo;
    createInfo.setSurface(mSurface)
        .setMinImageCount(imageCount)
        .setImageFormat(mSurfaceFormat.format)
        .setImageColorSpace(mSurfaceFormat.colorSpace)
        .setImageExtent(mSwapExtent)
        .setImageArrayLayers(1)
        .setImageUsage(vk::ImageUsageFlagBits::eColorAttachment)
        .setPreTransform(mSurfaceCapabilities.currentTransform)
        .setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque)
        .setPresentMode(mPresentMode)
        .setClipped(true)
        .setOldSwapchain(nullptr);
    if (mGraphicsQueueFamily.value() != mPresentQueueFamily.value())
    {
        std::array<uint32_t, 2> queueFamilyIndices = {mGraphicsQueueFamily.value(), mPresentQueueFamily.value()};
        createInfo.setImageSharingMode(vk::SharingMode::eConcurrent)
            .setQueueFamilyIndices(queueFamilyIndices);
    }
    else
    {
        createInfo.setImageSharingMode(vk::SharingMode::eExclusive);
    }

    auto [swapChainResult, swapChain] = mDevice.createSwapchainKHR(createInfo);
    auto [swapChainImagesResult, swapChainImages] = mDevice.getSwapchainImagesKHR(swapChain);
    if (swapChainResult != vk::Result::eSuccess ||
        swapChainImagesResult != vk::Result::eSuccess)
    {
        spdlog::critical("Failed to create swap chain: {} {}",
                         vk::to_string(swapChainResult),
                         vk::to_string(swapChainImagesResult));
        return false;
    }

    mSwapchain = swapChain;
    mSwapchainImages = swapChainImages;
    return true;
}

bool VulkanEngine::initShaderModule(const std::vector<char> &code, vk::ShaderModule& module)
{
    vk::ShaderModuleCreateInfo ci;
    ci.setPCode((const uint32_t *)code.data()).setCodeSize(code.size());

    auto[result, shaderModule] = mDevice.createShaderModule(ci);
    if(result != vk::Result::eSuccess)
    {
        spdlog::critical("Failed to create shader module: {}", vk::to_string(result));
        return false;   
    }
    module = shaderModule;
    return true;
}

uint32_t VulkanEngine::findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags props)
{
    vk::PhysicalDeviceMemoryProperties physicalProps = mPhysicalDevice.value().getMemoryProperties();
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

bool VulkanEngine::recreateSwapchain()
{
    if(mDevice.waitIdle() != vk::Result::eSuccess)
    {
        spdlog::critical("Wait idle failed !");
        return false;
    }
    cleanupSwapchain();
    if(!initSurface())
        return false;
    mSurfaceCapabilities = mPhysicalDevice.value().getSurfaceCapabilitiesKHR(mSurface).value;
    if(!initSwapExtent())
        return false;
    if(!initSwapchain())
        return false;
    if(!initSwapchainImageViews())
        return false;
    if(!initDepthBuffer())
        return false;
    if(!initSwapchainFramebuffers())
        return false;

    return true;
}

bool VulkanEngine::initSurface()
{
    VkSurfaceKHR surface;
    if (glfwCreateWindowSurface(mInstance, mWindow, nullptr, &surface) != VK_SUCCESS)
    {
        spdlog::critical("Failed to create window surface !");
        return false;
    }
    mSurface = surface;

    return true;
}

bool VulkanEngine::initSwapchainImageViews()
{
    mSwapchainImageViews.resize(mSwapchainImages.size());
    for (size_t i = 0; i < mSwapchainImages.size(); i++)
    {
        vk::ImageViewCreateInfo ci;
        vk::ImageSubresourceRange crr;
        crr.setAspectMask(vk::ImageAspectFlagBits::eColor)
            .setBaseMipLevel(0)
            .setLevelCount(1)
            .setBaseArrayLayer(0)
            .setLayerCount(1);
        ci.setImage(mSwapchainImages[i])
            .setViewType(vk::ImageViewType::e2D)
            .setFormat(mSurfaceFormat.format)
            .setComponents(vk::ComponentMapping())
            .setSubresourceRange(crr);

        auto [imageViewResult, imageView] = mDevice.createImageView(ci);
        if (imageViewResult != vk::Result::eSuccess)
        {
            spdlog::critical("Failed to create image view: {}", vk::to_string(imageViewResult));
            return false;
        }
        mSwapchainImageViews[i] = imageView;
    }

    return true;
}

bool VulkanEngine::initDevice()
{
    // Select physical device
    mPhysicalDevice.reset();
    const auto [devicesResult, devices] = mInstance.enumeratePhysicalDevices();
    if (devicesResult != vk::Result::eSuccess)
    {
        spdlog::critical("Failed to enumerate physical devices: {}", vk::to_string(devicesResult));
        return false;
    }

    for (const auto &device : devices)
    {
        const vk::PhysicalDeviceProperties properties = device.getProperties();
        const vk::PhysicalDeviceFeatures features = device.getFeatures();
        const auto [surfaceCapabilitiesResult, surfaceCapabilities] = device.getSurfaceCapabilitiesKHR(mSurface);
        const auto [surfaceFormatResult, surfaceFormats] = device.getSurfaceFormatsKHR(mSurface);
        const auto [presentModesResult, presentModes] = device.getSurfacePresentModesKHR(mSurface);
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

        mSurfaceCapabilities = surfaceCapabilities;

        bool isIntergrated = properties.deviceType == vk::PhysicalDeviceType::eIntegratedGpu;
        bool isDiscrete = properties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu;
        bool validSurfaceFormat = false;
        for (const auto &format : surfaceFormats)
        {
            if (format.format == vk::Format::eB8G8R8A8Srgb && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
            {
                validSurfaceFormat = true;
                mSurfaceFormat = format;
                break;
            }
        }

        bool validPresentMode = false;
        for (const auto &mode : presentModes)
        {
            if (mode == vk::PresentModeKHR::eFifo)
            {
                validPresentMode = true;
                mPresentMode = mode;
                break;
            }
        }

        // Select swap extends
        if (isIntergrated || isDiscrete && validSurfaceFormat && validPresentMode)
        {
            mPhysicalDevice = device;
            break;
        }
    }
    if (!mPhysicalDevice.has_value())
    {
        spdlog::critical("Failed to find suitable physical device !");
        return false;
    }

    // Find queue family
    std::vector<vk::QueueFamilyProperties> queueFamilies = mPhysicalDevice.value().getQueueFamilyProperties();
    mGraphicsQueueFamily.reset();
    mPresentQueueFamily.reset();
    mTransferQueueFamily.reset();
    uint32_t i = 0;
    for (const auto &queueFamily : queueFamilies)
    {

        if (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics)
        {
            mGraphicsQueueFamily = i;
        }
        if (mPhysicalDevice.value().getSurfaceSupportKHR(i, mSurface).value)
        {
            mPresentQueueFamily = i;
        }

        if (queueFamily.queueFlags & vk::QueueFlagBits::eTransfer)
        {
            mTransferQueueFamily = i;
        }

        i++;
    }

    if (!mGraphicsQueueFamily.has_value() || !mPresentQueueFamily.has_value() || !mTransferQueueFamily.has_value())
    {
        spdlog::critical("Failed to find family queue !");
        return false;
    }

    // Select extensions
    std::vector<const char *> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    // Create device
    std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = {mGraphicsQueueFamily.value(), mPresentQueueFamily.value(), mTransferQueueFamily.value()};

    std::vector<float> queuePriorities = {1.0f};
    for (uint32_t queueFamily : uniqueQueueFamilies)
    {
        vk::DeviceQueueCreateInfo queueCreateInfo;
        queueCreateInfo.setQueueFamilyIndex(queueFamily)
            .setQueuePriorities(queuePriorities);
        queueCreateInfos.push_back(queueCreateInfo);
    }

    vk::DeviceCreateInfo deviceCreateInfo;
    vk::PhysicalDeviceFeatures features = mPhysicalDevice.value().getFeatures();
    deviceCreateInfo.setQueueCreateInfos(queueCreateInfos)
        .setPEnabledFeatures(&features)
        .setPEnabledExtensionNames(deviceExtensions);
    auto [deviceResult, device] = mPhysicalDevice->createDevice(deviceCreateInfo);
    if (deviceResult != vk::Result::eSuccess)
    {
        spdlog::critical("Failed to create device: {}", vk::to_string(deviceResult));
        return false;
    }
    mDevice = device;
    mDynamicDispatcher.init(mDevice);
    mGraphicQueue = mDevice.getQueue(mGraphicsQueueFamily.value(), 0);
    mPresentQueue = mDevice.getQueue(mPresentQueueFamily.value(), 0);
    mTransferQueue = mDevice.getQueue(mTransferQueueFamily.value(), 0);

    return true;
}

bool VulkanEngine::initSwapExtent()
{
    if (mSurfaceCapabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
    {
        mSwapExtent = mSurfaceCapabilities.currentExtent;
    }
    else
    {
        int width, height;
        glfwGetFramebufferSize(mWindow, &width, &height);

        VkExtent2D actualExtent = {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)};

        actualExtent.width = std::clamp(actualExtent.width, mSurfaceCapabilities.minImageExtent.width, mSurfaceCapabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, mSurfaceCapabilities.minImageExtent.height, mSurfaceCapabilities.maxImageExtent.height);
        mSwapExtent = actualExtent;
    }

    return true;
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

    std::vector<const char *> extensions, layers;
    uint32_t glfwExtensionCount = 0;
    const char **glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    for (size_t i = 0; i < glfwExtensionCount; i++)
    {
        extensions.push_back(glfwExtensions[i]);
    }
    extensions.push_back("VK_EXT_debug_utils");
    layers.push_back("VK_LAYER_KHRONOS_validation");

    createInfo.setPEnabledExtensionNames(extensions)
        .setPEnabledLayerNames(layers);

    const auto [instanceResult, instance] = vk::createInstance(createInfo);
    if (instanceResult != vk::Result::eSuccess)
    {
        spdlog::critical("Failed to create instance: {}", vk::to_string(instanceResult));
        return false;
    }
    mInstance = instance;
    mDynamicDispatcher.init(mInstance);

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
        mInstance.createDebugUtilsMessengerEXT(debugCreateInfo, nullptr, mDynamicDispatcher);
    if (debugMessengerResult != vk::Result::eSuccess)
    {
        spdlog::critical("Failed to create debug utils: {}", vk::to_string(debugMessengerResult));
        return false;
    }
    mDebugMessenger = debugMessenger;

    return true;
}

void VulkanEngine::registerLuaScript(lua_State *L)
{
    auto luaUpdateCameraParams = [](lua_State *L) -> int
    {
        auto &camera = VulkanEngine::getCamera();
        camera.position.x = lua_tonumber(L, 1);
        camera.position.y = lua_tonumber(L, 2);
        camera.position.z = lua_tonumber(L, 3);

        camera.pitch = lua_tonumber(L, 4);
        camera.yaw = lua_tonumber(L, 5);

        camera.fov = lua_tonumber(L, 6);
        camera.nearPlane = lua_tonumber(L, 7);
        camera.farPlane = lua_tonumber(L, 8);

        return 0;
    };

    lua_register(L, "vk_camera_update_params", luaUpdateCameraParams);
}

bool VulkanEngine::initRenderPass()
{
    vk::AttachmentDescription colorAttachment;
    colorAttachment.setFormat(mSurfaceFormat.format)
        .setSamples(vk::SampleCountFlagBits::e1)
        .setLoadOp(vk::AttachmentLoadOp::eClear)
        .setStoreOp(vk::AttachmentStoreOp::eStore)
        .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
        .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
        .setInitialLayout(vk::ImageLayout::eUndefined)
        .setFinalLayout(vk::ImageLayout::ePresentSrcKHR);

    vk::AttachmentReference colorAttachmentRef;
    colorAttachmentRef.setAttachment(0)
        .setLayout(vk::ImageLayout::eColorAttachmentOptimal);

    vk::AttachmentDescription depthAttachment;
    depthAttachment.setFormat(vk::Format::eD32Sfloat)
        .setSamples(vk::SampleCountFlagBits::e1)
        .setLoadOp(vk::AttachmentLoadOp::eClear)
        .setStoreOp(vk::AttachmentStoreOp::eDontCare)
        .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
        .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
        .setInitialLayout(vk::ImageLayout::eUndefined)
        .setFinalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);

    vk::AttachmentReference depthAttachmentRef;
    depthAttachmentRef.setAttachment(1)
        .setLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);

    vk::SubpassDescription subpass;
    subpass.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
        .setColorAttachments(colorAttachmentRef)
        .setPDepthStencilAttachment(&depthAttachmentRef);

    vk::SubpassDependency subpassDependency;
    subpassDependency.setSrcSubpass(VK_SUBPASS_EXTERNAL)
        .setDstSubpass(0)
        .setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests)
        .setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests)
        .setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentWrite);

    vk::RenderPassCreateInfo renderPassCI;
    auto attachments = std::array{colorAttachment, depthAttachment};
    renderPassCI.setAttachments(attachments)
        .setSubpasses(subpass)
        .setDependencies(subpassDependency);

    auto [renderPassResult, renderPass] = mDevice.createRenderPass(renderPassCI);
    if (renderPassResult != vk::Result::eSuccess)
    {
        spdlog::critical("Failed to create render pass: {}", vk::to_string(renderPassResult));
        return false;
    }
    mRenderPass = renderPass;
    return true;
}

bool VulkanEngine::initCommandPool()
{
    spdlog::info("Creating command pool !");
    vk::CommandPoolCreateInfo commandPoolCI;
    commandPoolCI.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer)
        .setQueueFamilyIndex(mGraphicsQueueFamily.value());

    auto [commandPoolResult, commandPool] = mDevice.createCommandPool(commandPoolCI);
    if (commandPoolResult != vk::Result::eSuccess)
    {
        spdlog::critical("Failed to create command pool: {}", vk::to_string(commandPoolResult));
        return false;
    }

    mCommandPool = commandPool;
    return true;
}

bool VulkanEngine::initCommandBuffer()
{
    vk::CommandBufferAllocateInfo commandBufferAllocateCI;
    commandBufferAllocateCI.setCommandPool(mCommandPool)
        .setLevel(vk::CommandBufferLevel::ePrimary)
        .setCommandBufferCount(1);

    auto [commandBufferResult, commandBuffers] = mDevice.allocateCommandBuffers(commandBufferAllocateCI);
    if (commandBufferResult != vk::Result::eSuccess)
    {
        spdlog::critical("Failed to create command buffer: {}", vk::to_string(commandBufferResult));
        return false;
    }

    mCommandBuffer = commandBuffers[0];

    return true;
}

void VulkanEngine::joint()
{
    mDevice.waitIdle();
}

void VulkanEngine::render()
{
    constexpr uint64_t UINT64_T_MAX = std::numeric_limits<uint64_t>::max();
    if (mDevice.waitForFences(mInFlightLocker, true, UINT64_MAX) != vk::Result::eSuccess)
        throw std::runtime_error("vkWaitForFences error !");
    auto imageResult = mDevice.acquireNextImageKHR(mSwapchain, UINT64_MAX, mImageAvalidableGSignal, nullptr);
    if (imageResult.result == vk::Result::eErrorOutOfDateKHR)
    {
        recreateSwapchain();
        return;
    }
    else if (imageResult.result != vk::Result::eSuccess && imageResult.result != vk::Result::eSuboptimalKHR)
    {
        throw std::runtime_error("Failed to acquire next swapchain image !");
    }
    mDevice.resetFences(mInFlightLocker);

    mCurrentSwapChainImageIndex = imageResult.value;
    vk::CommandBuffer &cmdBuffer = mCommandBuffer;
    cmdBuffer.reset();

    vk::CommandBufferBeginInfo cmdBufferBeginInfo;
    if (cmdBuffer.begin(&cmdBufferBeginInfo) != vk::Result::eSuccess)
        throw std::runtime_error("Failed to record cmd buffer !");

    std::array<vk::ClearValue, 2> clearValues;

    clearValues[0].setColor(std::array{0.1f, 0.1f, 0.1f, 1.0f});
    clearValues[1].setDepthStencil({1.0f, 0});
    vk::RenderPassBeginInfo renderpassBeginInfo;
    renderpassBeginInfo.setRenderPass(mRenderPass)
        .setFramebuffer(mSwapchainFramebuffers[mCurrentSwapChainImageIndex])
        .setRenderArea({{0, 0}, mSwapExtent})
        .setClearValues(clearValues);

    // Update ubo
    VulkanUniformBufferObject ubo;
    ubo.proj = VulkanEngine::mCamera.getProjection(VulkanEngine::mSwapExtent);
    ubo.view = VulkanEngine::mCamera.getView();
    ubo.proj[1][1] *= -1;
    std::memcpy(mGlobalUniformBufferMap, &ubo, sizeof(ubo));

    cmdBuffer.beginRenderPass(renderpassBeginInfo, vk::SubpassContents::eInline);

    vk::Viewport viewport;
    viewport.setX(0)
        .setY(0)
        .setWidth(mSwapExtent.width)
        .setHeight(mSwapExtent.height)
        .setMinDepth(0.0f)
        .setMaxDepth(1.0f);
    cmdBuffer.setViewport(0, viewport);
    vk::Rect2D scissor;
    scissor.setOffset({0, 0})
        .setExtent(mSwapExtent);
    cmdBuffer.setScissor(0, scissor);

    GameObject::globalRender();

    cmdBuffer.endRenderPass();
    cmdBuffer.end();

    vk::SubmitInfo submitInfo;
    std::vector<vk::PipelineStageFlags> waitStages = {vk::PipelineStageFlagBits::eColorAttachmentOutput};
    submitInfo.setWaitSemaphores(mImageAvalidableGSignal)
        .setWaitDstStageMask(waitStages)
        .setCommandBuffers(mCommandBuffer)
        .setSignalSemaphores(mRenderFinishedGSignal);

    mGraphicQueue.submit(submitInfo, mInFlightLocker);

    vk::PresentInfoKHR presentInfo;
    presentInfo.setWaitSemaphores(mRenderFinishedGSignal)
        .setSwapchains(mSwapchain)
        .setImageIndices(mCurrentSwapChainImageIndex);

    vk::Result presentResult = mPresentQueue.presentKHR(presentInfo);

    if (presentResult == vk::Result::eErrorOutOfDateKHR || presentResult == vk::Result::eSuboptimalKHR || mWindowResized)
    {
        mWindowResized = false;
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
        .setExtent(vk::Extent3D(mSwapExtent.width, mSwapExtent.height, 1))
        .setMipLevels(1)
        .setArrayLayers(1)
        .setFormat(vk::Format::eD32Sfloat)
        .setTiling(vk::ImageTiling::eOptimal)
        .setInitialLayout(vk::ImageLayout::eUndefined)
        .setUsage(vk::ImageUsageFlagBits::eDepthStencilAttachment)
        .setSamples(vk::SampleCountFlagBits::e1)
        .setSharingMode(vk::SharingMode::eExclusive);
    auto [depthImageResult, depthImage] = mDevice.createImage(imageCI);
    if (depthImageResult != vk::Result::eSuccess)
    {
        spdlog::critical("Failed to create depth image result: {}", vk::to_string(depthImageResult));
        return false;
    }
    mDepthImage = depthImage;

    vk::MemoryRequirements memRequirements = mDevice.getImageMemoryRequirements(mDepthImage);
    vk::MemoryAllocateInfo memAI;
    memAI.setAllocationSize(memRequirements.size)
        .setMemoryTypeIndex(findMemoryType(memRequirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal));

    auto [depthMemoryResult, depthMemory] = mDevice.allocateMemory(memAI);

    if (depthMemoryResult != vk::Result::eSuccess)
    {
        spdlog::critical("Failed to create depth memory result: {}", vk::to_string(depthMemoryResult));
        return false;
    }
    mDepthMemory = depthMemory;

    mDevice.bindImageMemory(mDepthImage, mDepthMemory, {0});

    // Create Image view
    vk::ImageViewCreateInfo viewInfo;
    viewInfo
        .setImage(mDepthImage)
        .setViewType(vk::ImageViewType::e2D)
        .setFormat(vk::Format::eD32Sfloat)
        .subresourceRange
        .setAspectMask(vk::ImageAspectFlagBits::eDepth)
        .setBaseMipLevel(0)
        .setLevelCount(1)
        .setBaseArrayLayer(0)
        .setLayerCount(1);

    auto [depthViewResult, depthView] = mDevice.createImageView(viewInfo);
    if (depthViewResult != vk::Result::eSuccess)
    {
        spdlog::critical("Failed to create depth view: {}", vk::to_string(depthViewResult));
        return false;
    }
    mDepthView = depthView;
    return true;
}

bool VulkanEngine::initSyncObjects()
{
    vk::SemaphoreCreateInfo semaphoreCI;
    vk::FenceCreateInfo fenceCI;
    fenceCI.setFlags(vk::FenceCreateFlagBits::eSignaled);

    auto [result1, imageAvalidableGSignal] = mDevice.createSemaphore(semaphoreCI);
    auto [result2, renderFinishedGSignal] = mDevice.createSemaphore(semaphoreCI);
    auto [result3, inFlightLocker] = mDevice.createFence(fenceCI);

    if (result1 != vk::Result::eSuccess ||
        result2 != vk::Result::eSuccess ||
        result3 != vk::Result::eSuccess)
    {
        spdlog::critical("Failed to create sync objects: {} {} {}", vk::to_string(result1),
                         vk::to_string(result2),
                         vk::to_string(result3));

        return false;
    }

    mImageAvalidableGSignal = imageAvalidableGSignal;
    mRenderFinishedGSignal = renderFinishedGSignal;
    mInFlightLocker = inFlightLocker;

    return true;
}

bool VulkanEngine::initDescriptorPool()
{
    std::array<vk::DescriptorPoolSize, 2> dps;
    dps[0].setDescriptorCount(512).setType(vk::DescriptorType::eUniformBuffer);
    dps[1].setDescriptorCount(512).setType(vk::DescriptorType::eCombinedImageSampler);

    vk::DescriptorPoolCreateInfo descriptorPoolCI;
    descriptorPoolCI.setPoolSizes(dps)
        .setMaxSets(512);

    auto [descriptorPoolResult, descriptorPool] = mDevice.createDescriptorPool(descriptorPoolCI);
    if (descriptorPoolResult != vk::Result::eSuccess)
    {
        spdlog::critical("Failed to create descriptor pool: {}", vk::to_string(descriptorPoolResult));
        return false;
    }

    mDescriptorPool = descriptorPool;

    vk::DescriptorSetLayoutBinding layoutBinding;
    layoutBinding.setBinding(0)
        .setDescriptorType(vk::DescriptorType::eUniformBuffer)
        .setDescriptorCount(1)
        .setStageFlags(vk::ShaderStageFlagBits::eVertex);

    vk::DescriptorSetLayoutCreateInfo layoutCI;
    layoutCI.setBindings(layoutBinding);

    auto [globalDescriptorLayoutResult, globalDescriptorLayout] = mDevice.createDescriptorSetLayout(layoutCI);
    if (globalDescriptorLayoutResult != vk::Result::eSuccess)
    {
        spdlog::critical("Failed to create global descriptor layout: {}", vk::to_string(globalDescriptorLayoutResult));
        return false;
    }
    mGlobalDescriptorLayout = globalDescriptorLayout;

    // Uniform buffer
    vk::DeviceSize size = sizeof(VulkanUniformBufferObject);
    mGlobalUniformBuffer.init(size, vk::BufferUsageFlagBits::eUniformBuffer,
                              vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
    mGlobalUniformBufferMap = mGlobalUniformBuffer.getMapped(0, size);

    // Descriptor
    vk::DescriptorSetAllocateInfo dsAI;
    dsAI.setDescriptorPool(mDescriptorPool)
        .setSetLayouts(mGlobalDescriptorLayout);

    auto [globalDescriptorSetResult, globalDescriptorSet] = mDevice.allocateDescriptorSets(dsAI);
    if (globalDescriptorSetResult != vk::Result::eSuccess)
    {
        spdlog::critical("Failed to create global descriptor set: {}", vk::to_string(globalDescriptorSetResult));
        return false;
    }
    mGlobalDescriptorSet = globalDescriptorSet[0];

    vk::DescriptorBufferInfo bufferInfo;
    bufferInfo.setBuffer(mGlobalUniformBuffer.getBuffer())
        .setOffset(0)
        .setRange(sizeof(VulkanUniformBufferObject));
    vk::WriteDescriptorSet writeDescriptor;
    writeDescriptor.setDstSet(mGlobalDescriptorSet)
        .setDstBinding(0)
        .setDstArrayElement(0)
        .setDescriptorType(vk::DescriptorType::eUniformBuffer)
        .setDescriptorCount(1)
        .setBufferInfo(bufferInfo);
    mDevice.updateDescriptorSets(writeDescriptor, {});

    return true;
}
void VulkanEngine::cleanup() noexcept
{
    mDevice.destroySemaphore(mImageAvalidableGSignal);
    mDevice.destroySemaphore(mRenderFinishedGSignal);
    mDevice.destroyFence(mInFlightLocker);

    mDevice.destroyCommandPool(mCommandPool);

    mStaticModelPipeline.cleanup();
    mAnimatedModelPipeline.cleanup();
    mGlobalUniformBuffer.cleanup();
    mDevice.destroyDescriptorSetLayout(mGlobalDescriptorLayout);
    mDevice.destroyDescriptorPool(mDescriptorPool);
    mDevice.destroyRenderPass(mRenderPass);

    cleanupSwapchain();
    mDevice.destroy();

    mInstance.destroyDebugUtilsMessengerEXT(mDebugMessenger, nullptr, mDynamicDispatcher);
    mInstance.destroy();
}

void VulkanEngine::onWindowResize(int width, int height) noexcept
{
    mWindowResized = true;
}

bool VulkanEngine::initSwapchainFramebuffers()
{
    mSwapchainFramebuffers.resize(mSwapchainImageViews.size());
    for (size_t i = 0; i < mSwapchainImageViews.size(); i++)
    {

        auto attachments = std::array{mSwapchainImageViews[i], mDepthView};
        vk::FramebufferCreateInfo framebufferCI;
        framebufferCI.setRenderPass(mRenderPass)
            .setAttachments(attachments)
            .setWidth(mSwapExtent.width)
            .setHeight(mSwapExtent.height)
            .setLayers(1);

        auto [swapchainFrameBufferResult, swapChainFrameBuffer] = mDevice.createFramebuffer(framebufferCI);
        if (swapchainFrameBufferResult != vk::Result::eSuccess)
        {
            spdlog::critical("Failed to create swapchain framebuffer: {}", vk::to_string(swapchainFrameBufferResult));
            return false;
        }
        mSwapchainFramebuffers[i] = swapChainFrameBuffer;
    }

    return true;
}

std::vector<char> VulkanEngine::readfile(const std::string &file)
{
    std::ifstream f(file, std::ios::ate | std::ios::binary);

    if (!f.is_open())
    {
        throw std::runtime_error("failed to open file: " + file);
    }

    size_t fileSize = (size_t)f.tellg();
    std::vector<char> buffer(fileSize);
    f.seekg(0);
    f.read(buffer.data(), fileSize);
    f.close();

    return buffer;
}