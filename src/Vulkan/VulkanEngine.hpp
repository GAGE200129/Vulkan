#pragma once
#include "Camera.hpp"

struct VulkanUniformBufferObject
{
    glm::mat4 view;
    glm::mat4 proj;
};

struct VulkanTexture
{
    vk::Image handle;
    vk::DeviceMemory memory;
    vk::ImageView imageView;
    vk::Sampler sampler;
    vk::DescriptorSet descriptorSet;
};

struct VulkanBuffer
{
    vk::Buffer buffer;
    vk::DeviceMemory bufferMemory;
};

struct VulkanStaticModelPipeline
{
    vk::PipelineLayout layout;
    vk::Pipeline pipeline;
    vk::DescriptorSetLayout imageDescriptorLayout;
};

struct VulkanData
{
    GLFWwindow *window = nullptr;
    bool windowResized = false;
    vk::Instance instance;
    std::optional<vk::PhysicalDevice> physicalDevice;
    vk::Device device;
    std::optional<uint32_t> graphicsQueueFamily;
    std::optional<uint32_t> presentQueueFamily;
    std::optional<uint32_t> transferQueueFamily;
    vk::Queue graphicQueue, presentQueue, transferQueue;
    vk::SurfaceKHR surface;
    vk::SwapchainKHR swapchain;
    std::vector<vk::Image> swapchainImages;
    std::vector<vk::ImageView> swapchainImageViews;
    std::vector<vk::Framebuffer> swapchainFramebuffers;
    vk::DebugUtilsMessengerEXT debugMessenger;
    vk::DispatchLoaderDynamic dynamicDispatcher = vk::DispatchLoaderDynamic(vkGetInstanceProcAddr);
    vk::SurfaceCapabilitiesKHR surfaceCapabilities;
    vk::SurfaceFormatKHR surfaceFormat;
    vk::PresentModeKHR presentMode;
    vk::Extent2D swapExtent;
    vk::RenderPass renderPass;
    vk::DescriptorPool descriptorPool;
    VulkanStaticModelPipeline staticModelPipeline;
    vk::CommandPool commandPool;
    vk::CommandBuffer commandBuffer;
    vk::Semaphore imageAvalidableGSignal, renderFinishedGSignal;
    vk::Fence inFlightLocker;
    vk::Image depthImage;
    vk::DeviceMemory depthMemory;
    vk::ImageView depthView;
    uint32_t currentSwapchainImageIndex = 0;
    vk::DescriptorSetLayout globalDescriptorLayout;
    vk::DescriptorSet globalDescriptorSet;
    VulkanBuffer globalUniformBuffer;
    void *globalUniformBufferMap;

};


namespace VulkanEngine
{
    bool init();
    void joint();
    void render(const Camera& camera);
    void cleanup();
    bool initSwapExtent();
    bool initSwapchain();
    bool initSwapchainImageViews();
    bool initSurface();
    bool initDevice();
    bool initVulkan();
    bool initShaderModule(const std::vector<char> &code, vk::ShaderModule& module);
    bool initRenderPass();
    bool initSwapchainFramebuffers();
    bool initCommandPool();
    bool initCommandBuffer();
    bool initSyncObjects();
    bool initDescriptorPool();
    bool recreateSwapchain();
    void cleanupSwapchain();
    bool initDepthBuffer();
    uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags props);

    //Static model pipeline
    bool staticModelPipelineInit();
    void staticModelPipelineCleanup();

    //Textures
    bool textureLoadFromFile(const std::string &filePath, vk::DescriptorSetLayout layout, VulkanTexture& outTexture);
    void textureInit(uint32_t width, uint32_t height, vk::Format format, vk::ImageTiling tilting,
              vk::ImageUsageFlags usage, vk::MemoryPropertyFlags, vk::DescriptorSetLayout layout, VulkanTexture& outTexture);
    void textureTransitionLayout(vk::Image handle, vk::Format format, vk::ImageLayout oldLayout, vk::ImageLayout newLayout);
    void textureCleanup(VulkanTexture& texture);

    //Buffer
    bool bufferInit(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags props, VulkanBuffer& outBuffer);
    void bufferInitAndTransferToLocalDevice(const void *data, vk::DeviceSize size, vk::BufferUsageFlagBits usage, VulkanBuffer& outBuffer);
    void bufferCopy(const void *data, vk::DeviceSize size, VulkanBuffer& outBuffer);
    void bufferCopy(const VulkanBuffer &src, VulkanBuffer &dst, const vk::DeviceSize size);
    void bufferCleanup(VulkanBuffer& buffer);
    void* bufferGetMapped(VulkanBuffer& buffer, vk::DeviceSize offset, vk::DeviceSize size);


    extern VulkanData gData;
};
