#pragma once

#include <string>
#include <vector>
#include <vulkan/vulkan.h>
#include <functional>
#include <stack>

#include "Exception.hpp"
#include "GraphicsPipeline.hpp"

namespace vkb
{
    template<typename T>
    class Result;
}

struct GLFWwindow;
namespace gage::gfx
{
    class Graphics
    {
    public:
        Graphics(GLFWwindow *window, std::string app_name);
        Graphics(const Graphics&) = delete;
        void operator=(const Graphics&) = delete;
        ~Graphics();

        void clear();
        void draw_test_triangle();
        void end_frame();
        const std::string& get_app_name() const noexcept ;
    private:
        void create_swapchain();
        void destroy_swapchain();

    private:
        std::string app_name{};
        std::stack<std::function<void()>> delete_stack{};
        VkInstance instance{};
        VkDebugUtilsMessengerEXT debug_messenger{};
        VkSurfaceKHR surface{};
        VkDevice device{};
        VkPhysicalDevice physical_device{};

        VkExtent2D draw_extent{};

        uint32_t swapchain_image_index{};
        VkSwapchainKHR swapchain{};
        VkFormat swapchain_image_format{VK_FORMAT_B8G8R8A8_SRGB};
        std::vector<VkImage> swapchain_images{};
	    std::vector<VkImageView> swapchain_image_views{};

        VkCommandPool cmd_pool {};
        VkCommandBuffer cmd{};

        VkQueue graphics_queue{};
	    uint32_t graphics_queue_family{};


        VkSemaphore present_semaphore{};
        VkSemaphore render_semaphore{};
	    VkFence render_fence{};

        std::unique_ptr<GraphicsPipeline> graphics_pipeline{};
    };
}