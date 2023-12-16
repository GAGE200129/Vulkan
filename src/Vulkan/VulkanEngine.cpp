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

void VulkanEngine::initSwapchain()
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

  mSwapchain = mDevice.createSwapchainKHR(createInfo);
  mSwapchainImages = mDevice.getSwapchainImagesKHR(mSwapchain);
}

vk::ShaderModule VulkanEngine::initShaderModule(const std::vector<char> &code)
{
  vk::ShaderModuleCreateInfo ci;
  ci.setPCode((const uint32_t *)code.data()).setCodeSize(code.size());
  return mDevice.createShaderModule(ci);
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

void VulkanEngine::recreateSwapchain()
{
  mDevice.waitIdle();
  cleanupSwapchain();
  initSurface();
  mSurfaceCapabilities = mPhysicalDevice.value().getSurfaceCapabilitiesKHR(mSurface);
  initSwapExtent();
  initSwapchain();
  initSwapchainImageViews();
  initDepthBuffer();
  initSwapchainFramebuffers();
}

void VulkanEngine::initSurface()
{
  VkSurfaceKHR surface;
  if (glfwCreateWindowSurface(mInstance, mWindow, nullptr, &surface) != VK_SUCCESS)
  {
    throw std::runtime_error("Failed to create window surface !");
  }
  mSurface = surface;
}

void VulkanEngine::initSwapchainImageViews()
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
    mSwapchainImageViews[i] = mDevice.createImageView(ci);
  }
}

void VulkanEngine::initDevice()
{
  // Select physical device
  mPhysicalDevice.reset();
  std::vector<vk::PhysicalDevice> devices = mInstance.enumeratePhysicalDevices();
  if (devices.size() == 0)
    throw std::runtime_error("Cannot find physical device !");

  for (const auto &device : devices)
  {
    vk::PhysicalDeviceProperties properties = device.getProperties();
    vk::PhysicalDeviceFeatures features = device.getFeatures();
    mSurfaceCapabilities = device.getSurfaceCapabilitiesKHR(mSurface);
    std::vector<vk::SurfaceFormatKHR> surfaceFormats = device.getSurfaceFormatsKHR(mSurface);
    std::vector<vk::PresentModeKHR> presentModes = device.getSurfacePresentModesKHR(mSurface);

    bool is_intergrated = properties.deviceType == vk::PhysicalDeviceType::eIntegratedGpu;
    bool is_discrete = properties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu;
    bool has_geometry = features.geometryShader;
    bool valid_surface_format = false;
    for (const auto &format : surfaceFormats)
    {
      if (format.format == vk::Format::eB8G8R8A8Srgb && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
      {
        valid_surface_format = true;
        mSurfaceFormat = format;
        break;
      }
    }

    bool valid_present_mode = false;
    for (const auto &mode : presentModes)
    {
      if (mode == vk::PresentModeKHR::eImmediate)
      {
        valid_present_mode = true;
        mPresentMode = mode;
        break;
      }
    }

    // Select swap extends

    if (is_intergrated || is_discrete && has_geometry && valid_surface_format && valid_present_mode)
    {
      mPhysicalDevice = device;
      break;
    }
  }
  if (!mPhysicalDevice.has_value())
    throw std::runtime_error("Cannot select physical device !");

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
    if (mPhysicalDevice.value().getSurfaceSupportKHR(i, mSurface))
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
    throw std::runtime_error("Failed to find family queue !");

  // Select extensions
  std::vector<std::string> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
  std::vector<const char *> deviceExtensionStrings;
  for (const auto &d : deviceExtensions)
  {
    deviceExtensionStrings.push_back(d.c_str());
  }

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
      .setPEnabledExtensionNames(deviceExtensionStrings);
  mDevice = mPhysicalDevice->createDevice(deviceCreateInfo);
  mDynamicDispatcher.init(mDevice);
  mGraphicQueue = mDevice.getQueue(mGraphicsQueueFamily.value(), 0);
  mPresentQueue = mDevice.getQueue(mPresentQueueFamily.value(), 0);
  mTransferQueue = mDevice.getQueue(mTransferQueueFamily.value(), 0);
}

void VulkanEngine::initSwapExtent()
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
}

void VulkanEngine::initVulkan()
{
  vk::ApplicationInfo info;
  info.setApiVersion(VK_MAKE_API_VERSION(0, 1, 0, 0))
      .setApplicationVersion(VK_MAKE_API_VERSION(0, 1, 0, 0))
      .setEngineVersion(VK_MAKE_API_VERSION(0, 1, 0, 0))
      .setPEngineName("EnGAGE");
  vk::InstanceCreateInfo createInfo;
  createInfo.setPApplicationInfo(&info);

  std::vector<std::string> extensions, layers;
  std::vector<const char *> extensionStringPtrs, layerStringPtrs;
  {
    uint32_t glfwExtensionCount = 0;
    const char **glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    for (size_t i = 0; i < glfwExtensionCount; i++)
    {
      extensions.push_back(glfwExtensions[i]);
    }
    extensions.push_back("VK_EXT_debug_utils");
  }
  {
    layers.push_back("VK_LAYER_KHRONOS_validation");
  }

  for (const auto &s : extensions)
  {
    extensionStringPtrs.push_back(s.c_str());
  }

  for (const auto &l : layers)
  {
    layerStringPtrs.push_back(l.c_str());
  }

  createInfo.setPEnabledExtensionNames(extensionStringPtrs)
      .setPEnabledLayerNames(layerStringPtrs);

  mInstance = vk::createInstance(createInfo);
  mDynamicDispatcher.init(mInstance);

  // Create debug messenger
  vk::DebugUtilsMessengerCreateInfoEXT debugCreateInfo;
  debugCreateInfo.setMessageSeverity(vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError)
      .setMessageType(vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance)
      .setPfnUserCallback(debugCallback)
      .setPUserData(nullptr);

  mDebugMessenger = mInstance.createDebugUtilsMessengerEXT(debugCreateInfo, nullptr, mDynamicDispatcher);
}

void VulkanEngine::registerLuaScript(lua_State* L)
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

void VulkanEngine::initRenderPass()
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

  mRenderPass = mDevice.createRenderPass(renderPassCI);
}

void VulkanEngine::initCommandPool()
{
  vk::CommandPoolCreateInfo commandPoolCI;
  commandPoolCI.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer)
      .setQueueFamilyIndex(mGraphicsQueueFamily.value());

  mCommandPool = mDevice.createCommandPool(commandPoolCI);
}

void VulkanEngine::initCommandBuffer()
{
  vk::CommandBufferAllocateInfo commandBufferAllocateCI;
  commandBufferAllocateCI.setCommandPool(mCommandPool)
      .setLevel(vk::CommandBufferLevel::ePrimary)
      .setCommandBufferCount(1);

  mCommandBuffer = mDevice.allocateCommandBuffers(commandBufferAllocateCI)[0];
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


  //Update ubo
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

  cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, mStaticModelPipeline.mLayout, 0, {mGlobalDescriptorSet}, {});
  cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, mAnimatedModelPipeline.mLayout, 0, {mGlobalDescriptorSet}, {});

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

void VulkanEngine::initDepthBuffer()
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

  mDepthImage = mDevice.createImage(imageCI);

  vk::MemoryRequirements memRequirements = mDevice.getImageMemoryRequirements(mDepthImage);
  vk::MemoryAllocateInfo memAI;
  memAI.setAllocationSize(memRequirements.size)
      .setMemoryTypeIndex(findMemoryType(memRequirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal));

  mDepthMemory = mDevice.allocateMemory(memAI);

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

  mDepthView = mDevice.createImageView(viewInfo);
}

void VulkanEngine::initSyncObjects()
{
  vk::SemaphoreCreateInfo semaphoreCI;
  vk::FenceCreateInfo fenceCI;
  fenceCI.setFlags(vk::FenceCreateFlagBits::eSignaled);

  mImageAvalidableGSignal = mDevice.createSemaphore(semaphoreCI);
  mRenderFinishedGSignal = mDevice.createSemaphore(semaphoreCI);
  mInFlightLocker = mDevice.createFence(fenceCI);
}

void VulkanEngine::initDescriptorPool()
{
  std::array<vk::DescriptorPoolSize, 2> dps;
  dps[0].setDescriptorCount(512).setType(vk::DescriptorType::eUniformBuffer);
  dps[1].setDescriptorCount(512).setType(vk::DescriptorType::eCombinedImageSampler);

  vk::DescriptorPoolCreateInfo descriptorPoolCI;
  descriptorPoolCI.setPoolSizes(dps)
      .setMaxSets(512);

  mDescriptorPool = mDevice.createDescriptorPool(descriptorPoolCI);

  vk::DescriptorSetLayoutBinding layoutBinding;
  layoutBinding.setBinding(0)
      .setDescriptorType(vk::DescriptorType::eUniformBuffer)
      .setDescriptorCount(1)
      .setStageFlags(vk::ShaderStageFlagBits::eVertex);

  vk::DescriptorSetLayoutCreateInfo layoutCI;
  layoutCI.setBindings(layoutBinding);
  mGlobalDescriptorLayout = VulkanEngine::mDevice.createDescriptorSetLayout(layoutCI);

  // Uniform buffer
  vk::DeviceSize size = sizeof(VulkanUniformBufferObject);
  mGlobalUniformBuffer.init(size, vk::BufferUsageFlagBits::eUniformBuffer,
                            vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
  mGlobalUniformBufferMap = mGlobalUniformBuffer.getMapped(0, size);

  // Descriptor
  vk::DescriptorSetAllocateInfo dsAI;
  dsAI.setDescriptorPool(VulkanEngine::mDescriptorPool)
      .setSetLayouts(mGlobalDescriptorLayout);
  mGlobalDescriptorSet = VulkanEngine::mDevice.allocateDescriptorSets(dsAI)[0];
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
  VulkanEngine::mDevice.updateDescriptorSets(writeDescriptor, {});
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

void VulkanEngine::initSwapchainFramebuffers()
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

    mSwapchainFramebuffers[i] = mDevice.createFramebuffer(framebufferCI);
  }
}

std::vector<char> VulkanEngine::readfile(const std::string &file)
{
  std::ifstream f(file, std::ios::ate | std::ios::binary);

  if (!f.is_open())
  {
    throw std::runtime_error("failed to open file!");
  }

  size_t fileSize = (size_t)f.tellg();
  std::vector<char> buffer(fileSize);
  f.seekg(0);
  f.read(buffer.data(), fileSize);
  f.close();

  return buffer;
}