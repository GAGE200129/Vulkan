#pragma once
#include "VulkanBuffer.hpp"

#include "StaticModelPipeline.hpp"
#include "AnimatedModelPipeline.hpp"

struct VulkanUniformBufferObject
{
  glm::mat4 view;
  glm::mat4 proj;
};

class VulkanCamera
{
public:
  glm::mat4 getProjection(const vk::Extent2D &extent)
  {
    glm::mat4 proj = glm::perspectiveRH_ZO(glm::radians(fov), (float)extent.width / (float)extent.height,
                            nearPlane, farPlane);
    return proj;
  }

  glm::mat4 getView()
  {
    glm::mat4 result;

    result = glm::rotate(glm::mat4(1.0f), glm::radians(-pitch), {1, 0, 0});
    result = glm::rotate(result, glm::radians(-yaw), {0, 1, 0});
    result = glm::translate(result, -position);

    return result;
  }

public:
  glm::vec3 position;
  float pitch, yaw;
  float nearPlane, farPlane, fov;
};

struct GLFWwindow;
class VulkanTexture;
class VulkanEngine
{
  friend class VulkanBuffer;
  friend class VulkanTexture;
  friend class ModelComponent;
  friend class AnimatedModelComponent;
  friend class StaticModelPipeline;
  friend class AnimatedModelPipeline;
public:
  static void init(GLFWwindow *window)
  {
    mWindow = window;
    initVulkan();
    initSurface();
    initDevice();
    initSwapExtent();
    initSwapchain();
    initSwapchainImageViews();
    initRenderPass();
    initDescriptorPool();
    mStaticModelPipeline.init();
    mAnimatedModelPipeline.init();
    initDepthBuffer();
    initSwapchainFramebuffers();
    initCommandPool();
    initCommandBuffer();
    initSyncObjects();
  }

  static void joint();
  static void render();
  static void cleanup() noexcept;
  static void onWindowResize(int width, int height) noexcept;
  static std::vector<char> readfile(const std::string &file);
  inline static VulkanCamera &getCamera() { return mCamera; }

private:
  static void initSwapExtent();
  static void initSwapchain();
  static void initSwapchainImageViews();
  static void initSurface();
  static void initDevice();
  static void initVulkan();
  static vk::ShaderModule initShaderModule(const std::vector<char> &code);
  static void initRenderPass();
  static void initSwapchainFramebuffers();
  static void initCommandPool();
  static void initCommandBuffer();
  static void initSyncObjects();
  static void initDescriptorPool();
  static void recreateSwapchain();
  static void cleanupSwapchain();
  static void initDepthBuffer();
  static uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags props);

private:
  static GLFWwindow *mWindow;
  static bool mWindowResized;
  static vk::Instance mInstance;
  static std::optional<vk::PhysicalDevice> mPhysicalDevice;
  static vk::Device mDevice;
  static std::optional<uint32_t> mGraphicsQueueFamily;
  static std::optional<uint32_t> mPresentQueueFamily;
  static std::optional<uint32_t> mTransferQueueFamily;
  static vk::Queue mGraphicQueue, mPresentQueue, mTransferQueue;
  static vk::SurfaceKHR mSurface;
  static vk::SwapchainKHR mSwapchain;
  static std::vector<vk::Image> mSwapchainImages;
  static std::vector<vk::ImageView> mSwapchainImageViews;
  static std::vector<vk::Framebuffer> mSwapchainFramebuffers;
  static vk::DebugUtilsMessengerEXT mDebugMessenger;
  static vk::DispatchLoaderDynamic mDynamicDispatcher;
  static vk::SurfaceCapabilitiesKHR mSurfaceCapabilities;
  static vk::SurfaceFormatKHR mSurfaceFormat;
  static vk::PresentModeKHR mPresentMode;
  static vk::Extent2D mSwapExtent;
  static vk::RenderPass mRenderPass;
  static vk::DescriptorPool mDescriptorPool;
  static vk::DescriptorSetLayout mGlobalDescriptorLayout;
  static vk::DescriptorSet mGlobalDescriptorSet;
  static VulkanBuffer mGlobalUniformBuffer;
  static void *mGlobalUniformBufferMap;

  static StaticModelPipeline mStaticModelPipeline;
  static AnimatedModelPipeline mAnimatedModelPipeline;
  static vk::CommandPool mCommandPool;
  static vk::CommandBuffer mCommandBuffer;

  static vk::Semaphore mImageAvalidableGSignal, mRenderFinishedGSignal;
  static vk::Fence mInFlightLocker;

  static vk::Image mDepthImage;
  static vk::DeviceMemory mDepthMemory;
  static vk::ImageView mDepthView;
  static uint32_t mCurrentSwapChainImageIndex;
  static VulkanCamera mCamera;
};