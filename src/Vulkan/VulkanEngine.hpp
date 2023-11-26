#pragma once

#include <vulkan/vulkan.hpp>

#include <optional>
#include <vector>
#include <glm/glm.hpp>

#include <entt/entt.hpp>

#include "VulkanBuffer.hpp"

struct VulkanUniformBufferObject
{
  glm::mat4 view;
  glm::mat4 proj;
};

struct GLFWwindow;
class VulkanTexture;
class VulkanEngine
{
  friend class VulkanBuffer;
  friend class VulkanTexture;
public:
  VulkanEngine(GLFWwindow *&window, entt::registry& entt) noexcept
      : mWindow(window), mDynamicDispatcher(vkGetInstanceProcAddr),
      mUniformBuffer(*this), 
      mEntt(entt)
  {
  }
  void init()
  {
    initVulkan();
    initSurface();
    initDevice();
    initSwapExtent();
    initSwapchain();
    initSwapchainImageViews();
    initRenderPass();
    initDescriptorLayout();
    initGraphicsPipeline();
    initSwapchainFramebuffers();
    initCommandPool();
    initCommandBuffer();
    initUniformBuffers();
    initDescriptor();
    initSyncObjects();
  }

  void joint();
  void render(VulkanTexture& texture);
  void cleanup() noexcept;
  void onWindowResize(int width, int height) noexcept;

  static std::vector<char> readfile(const std::string &file);

private:
  void initUniformBuffers();
  void updateUniformBuffer();
  void initDescriptorLayout();
  void initSwapExtent();
  void initSwapchain();
  void initSwapchainImageViews();
  void initSurface();
  void initDevice();
  void initVulkan();
  void initGraphicsPipeline();
  vk::ShaderModule initShaderModule(const std::vector<char> &code);
  void initRenderPass();
  void initSwapchainFramebuffers();
  void initCommandPool();
  void initCommandBuffer();
  void initSyncObjects();
  void recreateSwapchain();
  void cleanupSwapchain();
  void initDescriptor();
  uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags props);

private:
  GLFWwindow *&mWindow;
  bool mWindowResized = false;
  vk::Instance mInstance;
  std::optional<vk::PhysicalDevice> mPhysicalDevice;
  vk::Device mDevice;
  std::optional<uint32_t> mGraphicsQueueFamily;
  std::optional<uint32_t> mPresentQueueFamily;
  std::optional<uint32_t> mTransferQueueFamily;
  vk::Queue mGraphicQueue, mPresentQueue, mTransferQueue;
  vk::SurfaceKHR mSurface;
  vk::SwapchainKHR mSwapchain;
  std::vector<vk::Image> mSwapchainImages;
  std::vector<vk::ImageView> mSwapchainImageViews;
  std::vector<vk::Framebuffer> mSwapchainFramebuffers;
  vk::DebugUtilsMessengerEXT mDebugMessenger;
  vk::DispatchLoaderDynamic mDynamicDispatcher;
  vk::SurfaceCapabilitiesKHR mSurfaceCapabilities;
  vk::SurfaceFormatKHR mSurfaceFormat;
  vk::PresentModeKHR mPresentMode;
  vk::Extent2D mSwapExtent;
  vk::RenderPass mRenderPass;
  vk::DescriptorSetLayout mGlobalDescriptorLayout, mImageDescriptorLayout;
  vk::DescriptorPool mDescriptorPool;
  vk::DescriptorSet mGlobalDescriptorSet;
  
  vk::PipelineLayout mPipelineLayout;
  vk::Pipeline mGraphicsPipeline;
  vk::CommandPool mCommandPool;
  vk::CommandBuffer mCommandBuffer;

  vk::Semaphore mImageAvalidableGSignal, mRenderFinishedGSignal;
  vk::Fence mInFlightLocker;

  VulkanBuffer mUniformBuffer;
  void* mUniformBufferMap;
  
  entt::registry& mEntt;
};