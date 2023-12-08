#pragma once
#include "VulkanBuffer.hpp"


struct VulkanUniformBufferObject
{
  glm::mat4 view;
  glm::mat4 proj;
};

class VulkanCamera
{
public:

  glm::mat4 getProjection(const vk::Extent2D& extent)
  {
    return glm::perspective(glm::radians(fov), (float)extent.width / (float)extent.height,
                              nearPlane, farPlane);
  }

  glm::mat4 getView()
  {
    glm::mat4 result;

    result = glm::translate(glm::mat4(1.0f), -position);

    result = glm::rotate(result, glm::radians(pitch), {1, 0, 0});
    result = glm::rotate(result, glm::radians(yaw), {0, 1, 0});


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
public:

  static void init(GLFWwindow* window)
  {
    mWindow = window;
    initVulkan();
    initSurface();
    initDevice();
    initSwapExtent();
    initSwapchain();
    initSwapchainImageViews();
    initRenderPass();
    initDescriptorLayout();
    initGraphicsPipeline();
    initDepthBuffer();
    initSwapchainFramebuffers();
    initCommandPool();
    initCommandBuffer();
    initUniformBuffers();
    initDescriptor();
    initSyncObjects();
  }

  static void joint();
  static bool prepare();
  static void submit();
  static void render();
  static void cleanup() noexcept;
  static void onWindowResize(int width, int height) noexcept;
  static std::vector<char> readfile(const std::string &file);

private:
  static void initUniformBuffers();
  static void updateUniformBuffer();
  static void initDescriptorLayout();
  static void initSwapExtent();
  static void initSwapchain();
  static void initSwapchainImageViews();
  static void initSurface();
  static void initDevice();
  static void initVulkan();
  static void initGraphicsPipeline();
  static vk::ShaderModule initShaderModule(const std::vector<char> &code);
  static void initRenderPass();
  static void initSwapchainFramebuffers();
  static void initCommandPool();
  static void initCommandBuffer();
  static void initSyncObjects();
  static void recreateSwapchain();
  static void cleanupSwapchain();
  static void initDescriptor();
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
  static vk::DescriptorSetLayout mGlobalDescriptorLayout, mImageDescriptorLayout;
  static vk::DescriptorPool mDescriptorPool;
  static vk::DescriptorSet mGlobalDescriptorSet;
   
  static vk::PipelineLayout mPipelineLayout;
  static vk::Pipeline mGraphicsPipeline;
  static vk::CommandPool mCommandPool;
  static vk::CommandBuffer mCommandBuffer;
 
  static vk::Semaphore mImageAvalidableGSignal, mRenderFinishedGSignal;
  static vk::Fence mInFlightLocker;
 
  static VulkanBuffer mUniformBuffer;
  static void* mUniformBufferMap;
 
  static vk::Image mDepthImage;
  static vk::DeviceMemory mDepthMemory;
  static vk::ImageView mDepthView;
  static uint32_t mCurrentSwapChainImageIndex;
  static VulkanCamera mCamera;
}; 