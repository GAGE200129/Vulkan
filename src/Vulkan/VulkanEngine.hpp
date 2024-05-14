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
    friend class MapPipeline;
    friend class Map;

public:
    static bool init(GLFWwindow *window)
    {
        spdlog::info("Creating a vulkan context !");
        mWindow = window;
        if(!initVulkan())
            return false;
        if(!initSurface())
            return false;
        if(!initDevice())
            return false;
        if(!initSwapExtent())
            return false;

        if(!initSwapchain())
            return false;

        if(!initSwapchainImageViews())
            return false;

        if(!initRenderPass())
            return false;
        
        if(!initDescriptorPool())
            return false;

        if(!mStaticModelPipeline.init())
            return false;
        if(!mAnimatedModelPipeline.init())
            return false;
        if(!initDepthBuffer())
            return false;
        if(!initSwapchainFramebuffers())
            return false;
        if(!initCommandPool())
            return false;
        if(!initCommandBuffer())
            return false;
        if(!initSyncObjects())
            return false;

        return true;
    }
    static void registerLuaScript(lua_State *L);
    static void joint();
    static void render();
    static void cleanup() noexcept;
    static void onWindowResize(int width, int height) noexcept;
    static std::vector<char> readfile(const std::string &file);
    inline static VulkanCamera &getCamera() { return mCamera; }

private:
    static bool initSwapExtent();
    static bool initSwapchain();
    static bool initSwapchainImageViews();
    static bool initSurface();
    static bool initDevice();
    static bool initVulkan();
    static bool initShaderModule(const std::vector<char> &code, vk::ShaderModule& module);
    static bool initRenderPass();
    static bool initSwapchainFramebuffers();
    static bool initCommandPool();
    static bool initCommandBuffer();
    static bool initSyncObjects();
    static bool initDescriptorPool();
    static bool recreateSwapchain();
    static void cleanupSwapchain();
    static bool initDepthBuffer();
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