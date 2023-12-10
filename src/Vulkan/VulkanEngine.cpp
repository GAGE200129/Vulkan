#include "pch.hpp"
#include "VulkanEngine.hpp"


#include "ECS/Components.hpp"

#include "VulkanTexture.hpp"

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
vk::DescriptorSetLayout VulkanEngine::mGlobalDescriptorLayout, VulkanEngine::mImageDescriptorLayout;
vk::DescriptorPool VulkanEngine::mDescriptorPool;
vk::DescriptorSet VulkanEngine::mGlobalDescriptorSet;
vk::PipelineLayout VulkanEngine::mPipelineLayout;
vk::Pipeline VulkanEngine::mGraphicsPipeline;
vk::CommandPool VulkanEngine::mCommandPool;
vk::CommandBuffer VulkanEngine::mCommandBuffer;
vk::Semaphore VulkanEngine::mImageAvalidableGSignal, VulkanEngine::mRenderFinishedGSignal;
vk::Fence VulkanEngine::mInFlightLocker;
VulkanBuffer VulkanEngine::mUniformBuffer;
void *VulkanEngine::mUniformBufferMap;
vk::Image VulkanEngine::mDepthImage;
vk::DeviceMemory VulkanEngine::mDepthMemory;
vk::ImageView VulkanEngine::mDepthView;
uint32_t VulkanEngine::mCurrentSwapChainImageIndex = 0;
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

void VulkanEngine::initGraphicsPipeline()
{
  auto vertexCode = readfile("res/shaders/shader.vert.spv");
  auto fragmentCode = readfile("res/shaders/shader.frag.spv");

  vk::ShaderModule vertexModule = initShaderModule(vertexCode);
  vk::ShaderModule fragmentModule = initShaderModule(fragmentCode);
  vk::PipelineShaderStageCreateInfo vertStageCI, fragStageCI;
  vertStageCI.setStage(vk::ShaderStageFlagBits::eVertex)
      .setPName("main")
      .setModule(vertexModule);
  fragStageCI.setStage(vk::ShaderStageFlagBits::eFragment)
      .setPName("main")
      .setModule(fragmentModule);

  std::vector<vk::PipelineShaderStageCreateInfo> shaderStages = {vertStageCI, fragStageCI};
  std::vector<vk::DynamicState> dynamicStates = {vk::DynamicState::eViewport, vk::DynamicState::eScissor};
  vk::PipelineDynamicStateCreateInfo dynamicStateCI;
  dynamicStateCI.setDynamicStates(dynamicStates);
  vk::PipelineVertexInputStateCreateInfo vertexInputCI;

  // Position, normal, uv
  std::array<vk::VertexInputBindingDescription, 3> vertexInputBindingDescriptions;
  vertexInputBindingDescriptions[0].setBinding(0).setStride(sizeof(glm::vec3)).setInputRate(vk::VertexInputRate::eVertex);
  vertexInputBindingDescriptions[1].setBinding(1).setStride(sizeof(glm::vec3)).setInputRate(vk::VertexInputRate::eVertex);
  vertexInputBindingDescriptions[2].setBinding(2).setStride(sizeof(glm::vec2)).setInputRate(vk::VertexInputRate::eVertex);

  std::array<vk::VertexInputAttributeDescription, 3> vertexInputAttributeDescription;
  vertexInputAttributeDescription[0].setBinding(0).setLocation(0).setFormat(vk::Format::eR32G32B32Sfloat);
  vertexInputAttributeDescription[1].setBinding(1).setLocation(1).setFormat(vk::Format::eR32G32B32Sfloat);
  vertexInputAttributeDescription[2].setBinding(2).setLocation(2).setFormat(vk::Format::eR32G32Sfloat);

  vertexInputCI.setVertexBindingDescriptions(vertexInputBindingDescriptions)
      .setVertexAttributeDescriptions(vertexInputAttributeDescription);
  vk::PipelineInputAssemblyStateCreateInfo inputAssemblyCI;
  inputAssemblyCI.setTopology(vk::PrimitiveTopology::eTriangleList)
      .setPrimitiveRestartEnable(false);
  vk::Viewport viewport;
  viewport.setX(0)
      .setY(0)
      .setWidth(mSwapExtent.width)
      .setHeight(mSwapExtent.height)
      .setMinDepth(0.0f)
      .setMaxDepth(1.0f);
  vk::Rect2D scissor;
  scissor.setOffset({0, 0})
      .setExtent(mSwapExtent);
  vk::PipelineViewportStateCreateInfo viewportStateCI;
  viewportStateCI.setViewports(viewport)
      .setScissors(scissor);

  vk::PipelineRasterizationStateCreateInfo rasterizationStateCI;
  rasterizationStateCI.setDepthClampEnable(false)
      .setRasterizerDiscardEnable(false)
      .setPolygonMode(vk::PolygonMode::eFill)
      .setLineWidth(1.0f)
      .setCullMode(vk::CullModeFlagBits::eBack)
      .setFrontFace(vk::FrontFace::eCounterClockwise)
      .setDepthBiasEnable(false)
      .setDepthBiasConstantFactor(0.0f)
      .setDepthBiasSlopeFactor(0.0f)
      .setDepthBiasClamp(0.0f);

  vk::PipelineMultisampleStateCreateInfo multisampleStateCI;
  multisampleStateCI.setSampleShadingEnable(false)
      .setRasterizationSamples(vk::SampleCountFlagBits::e1)
      .setMinSampleShading(1.0f)
      .setPSampleMask(nullptr)
      .setAlphaToCoverageEnable(false)
      .setAlphaToOneEnable(false);

  vk::PipelineColorBlendAttachmentState blendAttachmentState;
  blendAttachmentState.setColorWriteMask(vk::ColorComponentFlagBits::eA |
                                         vk::ColorComponentFlagBits::eR |
                                         vk::ColorComponentFlagBits::eG |
                                         vk::ColorComponentFlagBits::eB)
      .setBlendEnable(false)
      .setSrcColorBlendFactor(vk::BlendFactor::eOne)
      .setDstColorBlendFactor(vk::BlendFactor::eZero)
      .setColorBlendOp(vk::BlendOp::eAdd)
      .setSrcAlphaBlendFactor(vk::BlendFactor::eOne)
      .setDstAlphaBlendFactor(vk::BlendFactor::eZero)
      .setAlphaBlendOp(vk::BlendOp::eAdd);
  vk::PipelineColorBlendStateCreateInfo colorBlendingCI;
  colorBlendingCI.setLogicOp(vk::LogicOp::eCopy)
      .setLogicOpEnable(false)
      .setAttachments(blendAttachmentState)
      .setBlendConstants({0.0f, 0.0f, 0.0f, 0.0f});

  vk::PipelineDepthStencilStateCreateInfo depthStencil;
  depthStencil
      .setDepthTestEnable(true)
      .setDepthWriteEnable(true)
      .setDepthCompareOp(vk::CompareOp::eLess)
      .setDepthBoundsTestEnable(false)
      .setMinDepthBounds(0.0f)
      .setMaxDepthBounds(1.0f)
      .setStencilTestEnable(false);

  vk::PushConstantRange ps;
  ps.setOffset(0).setSize(sizeof(glm::mat4x4)).setStageFlags(vk::ShaderStageFlagBits::eVertex);
  vk::PipelineLayoutCreateInfo pipelineLayoutCI;

  auto layouts = std::array{mGlobalDescriptorLayout, mImageDescriptorLayout};
  pipelineLayoutCI.setSetLayouts(layouts)
      .setPushConstantRanges(ps);

  mPipelineLayout = mDevice.createPipelineLayout(pipelineLayoutCI);
  vk::GraphicsPipelineCreateInfo graphicsPipelineCI;
  graphicsPipelineCI.setStages(shaderStages)
      .setPVertexInputState(&vertexInputCI)
      .setPInputAssemblyState(&inputAssemblyCI)
      .setPViewportState(&viewportStateCI)
      .setPRasterizationState(&rasterizationStateCI)
      .setPMultisampleState(&multisampleStateCI)
      .setPDepthStencilState(&depthStencil)
      .setPColorBlendState(&colorBlendingCI)
      .setPDynamicState(&dynamicStateCI)
      .setLayout(mPipelineLayout)
      .setRenderPass(mRenderPass)
      .setSubpass(0)
      .setBasePipelineHandle(nullptr)
      .setBasePipelineIndex(-1);

  auto graphicsPipelineResult = mDevice.createGraphicsPipeline(nullptr, graphicsPipelineCI);
  if (graphicsPipelineResult.result != vk::Result::eSuccess)
  {
    throw std::runtime_error("Failed to create graphics pipeline !");
  }
  mGraphicsPipeline = graphicsPipelineResult.value;

  mDevice.destroyShaderModule(vertexModule);
  mDevice.destroyShaderModule(fragmentModule);
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
      if (mode == vk::PresentModeKHR::eFifo)
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

bool VulkanEngine::prepare()
{
  constexpr uint64_t UINT64_T_MAX = std::numeric_limits<uint64_t>::max();
  if (mDevice.waitForFences(mInFlightLocker, true, UINT64_MAX) != vk::Result::eSuccess)
    throw std::runtime_error("vkWaitForFences error !");
  auto imageResult = mDevice.acquireNextImageKHR(mSwapchain, UINT64_MAX, mImageAvalidableGSignal, nullptr);
  if (imageResult.result == vk::Result::eErrorOutOfDateKHR)
  {
    recreateSwapchain();
    return false;
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

  updateUniformBuffer();

  cmdBuffer.beginRenderPass(renderpassBeginInfo, vk::SubpassContents::eInline);
  cmdBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, mGraphicsPipeline);

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
  return true;
}
void VulkanEngine::submit()
{
  vk::CommandBuffer &cmdBuffer = mCommandBuffer;
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

void VulkanEngine::updateUniformBuffer()
{
  VulkanUniformBufferObject ubo;
  ubo.proj = mCamera.getProjection(mSwapExtent);
  ubo.view = mCamera.getView();
  ubo.proj[1][1] *= -1;

  std::memcpy(mUniformBufferMap, &ubo, sizeof(ubo));
}

void VulkanEngine::initDescriptor()
{
  std::array<vk::DescriptorPoolSize, 2> dps;
  dps[0].setDescriptorCount(128).setType(vk::DescriptorType::eUniformBuffer);
  dps[1].setDescriptorCount(128).setType(vk::DescriptorType::eCombinedImageSampler);

  vk::DescriptorPoolCreateInfo descriptorPoolCI;
  descriptorPoolCI.setPoolSizes(dps)
      .setMaxSets(128 * dps.size());

  mDescriptorPool = mDevice.createDescriptorPool(descriptorPoolCI);

  vk::DescriptorSetAllocateInfo dsAI;
  dsAI.setDescriptorPool(mDescriptorPool)
      .setSetLayouts(mGlobalDescriptorLayout);
  mGlobalDescriptorSet = mDevice.allocateDescriptorSets(dsAI)[0];
  vk::DescriptorBufferInfo bufferInfo;
  bufferInfo.setBuffer(mUniformBuffer.getBuffer())
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
}
void VulkanEngine::cleanup() noexcept
{
  mDevice.destroySemaphore(mImageAvalidableGSignal);
  mDevice.destroySemaphore(mRenderFinishedGSignal);
  mDevice.destroyFence(mInFlightLocker);

  mDevice.destroyCommandPool(mCommandPool);

  mUniformBuffer.cleanup();

  mDevice.destroyPipeline(mGraphicsPipeline);
  mDevice.destroyDescriptorSetLayout(mGlobalDescriptorLayout);
  mDevice.destroyDescriptorSetLayout(mImageDescriptorLayout);
  mDevice.destroyDescriptorPool(mDescriptorPool);
  mDevice.destroyPipelineLayout(mPipelineLayout);
  mDevice.destroyRenderPass(mRenderPass);

  cleanupSwapchain();
  mDevice.destroy();

  mInstance.destroyDebugUtilsMessengerEXT(mDebugMessenger, nullptr, mDynamicDispatcher);
  mInstance.destroy();
}

void VulkanEngine::initUniformBuffers()
{
  vk::DeviceSize size = sizeof(VulkanUniformBufferObject);
  mUniformBuffer.init(size, vk::BufferUsageFlagBits::eUniformBuffer,
                      vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
  mUniformBufferMap = mUniformBuffer.getMapped(0, size);
}

void VulkanEngine::initDescriptorLayout()
{
  vk::DescriptorSetLayoutBinding layoutBinding;
  layoutBinding.setBinding(0)
      .setDescriptorType(vk::DescriptorType::eUniformBuffer)
      .setDescriptorCount(1)
      .setStageFlags(vk::ShaderStageFlagBits::eVertex);

  vk::DescriptorSetLayoutCreateInfo layoutCI;
  layoutCI.setBindings(layoutBinding);
  mGlobalDescriptorLayout = mDevice.createDescriptorSetLayout(layoutCI);

  layoutBinding.setBinding(0)
      .setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
      .setDescriptorCount(1)
      .setStageFlags(vk::ShaderStageFlagBits::eFragment);

  layoutCI.setBindings(layoutBinding);
  mImageDescriptorLayout = mDevice.createDescriptorSetLayout(layoutCI);
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