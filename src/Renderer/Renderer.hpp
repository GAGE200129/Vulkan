#pragma once

#include "VkBootstrap.hpp"
#include "vulkan/vulkan.hpp"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "vk_mem_alloc.h"
#include "DescriptorAllocator.hpp"
#include "DescriptorLayoutBuilder.hpp"

#include <deque>
#include <functional>

constexpr size_t RENDERER_FRAMES_IN_FLIGHT = 2;

using DeletionQueue = std::deque<std::function<void()>>;

struct AllocatedImage
{
    vk::Image image;
    vk::ImageView imageView;
    VmaAllocation allocation;
    vk::Extent3D imageExtent;
    vk::Format imageFormat;
};

struct GLFWwindow;
struct RendererData
{
    //Main
    GLFWwindow* window;
    vk::DispatchLoaderDynamic dynamicDispatcher;
    vk::Instance instance;
    vk::DebugUtilsMessengerEXT debugMessenger;
    vk::PhysicalDevice physicalDevice;
    vk::Device device;
    vk::SurfaceKHR surface;

     vk::Queue graphicsQueue;
	uint32_t graphicsQueueFamily;

    //Swapchains
    vk::SwapchainKHR swapchain;
	vk::Format swapchainImageFormat;
	std::vector<vk::Image> swapchainImages;
	std::vector<vk::ImageView> swapchainImageViews;
	vk::Extent2D swapchainExtent;
    
    //Syncs & frame
    size_t currentFrameIndex;
    struct Frame {
        vk::CommandPool commandPool;
        vk::CommandBuffer mainCommandBuffer;
        vk::Semaphore swapchainSemaphore, renderSemaphore;
	    vk::Fence renderFence;
        DeletionQueue deletionQueue;
    } frames[RENDERER_FRAMES_IN_FLIGHT];

    AllocatedImage drawImage;
    vk::Extent2D drawExtent;
    vk::DescriptorSet drawImageDescriptor;
    vk::DescriptorSetLayout drawImageDescriptorLayout;


    VmaAllocator allocator;
    DescriptorAllocator globalDescriptorAllocator;
    DeletionQueue mainDeletionQueue;

    //Pipelines
    vk::Pipeline gradientPipeline;
    vk::PipelineLayout gradientPipelineLayout;
};

namespace Renderer
{

    bool init();
    void cleanup();
    void render();

    void drawBackground(vk::CommandBuffer cmd);
    //Display
    bool displayInit();

    //Command
    bool commandInit();
    
    //Sync objects
    bool syncInit();

    //Swapchain
    bool swapchainInit(int width, int height);

    //Pipeline
    bool pipelinesInit();
    bool backgroundPipelineInit();


    //Image
    void imageTransition(vk::CommandBuffer cmd, vk::Image image, vk::ImageLayout currentLayout, vk::ImageLayout newLayout);
    void imageCopy(vk::CommandBuffer cmd, vk::Image src, vk::Image dst, vk::Extent2D srcSize, vk::Extent2D dstSize);

    //Utils
    void deletionQueueFlush(DeletionQueue& queue);
    std::optional<vk::ShaderModule> loadShaderModule(const std::string& filePath);


    //Descriptors
    bool descriptorsInit();


    //Create infos

    vk::ImageCreateInfo imageCreateInfo(vk::Format format, vk::ImageUsageFlags usage, vk::Extent3D extent);
    vk::ImageViewCreateInfo imageViewCreateInfo(vk::Format format, vk::Image image, vk::ImageAspectFlags aspectFlags);

    extern RendererData gData;
    extern std::shared_ptr<spdlog::logger> gLogger;
}