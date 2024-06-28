#pragma once

#include <string>
#include <vector>
#include <vk_mem_alloc.h>
#include <functional>
#include <stack>

#include "Exception.hpp"

#include <glm/mat4x4.hpp>

namespace vkb
{
    template <typename T>
    class Result;
}

namespace gage::gfx::bind
{
    class IBindable;
}

struct GLFWwindow;
namespace gage::gfx
{
    class Graphics
    {
        friend class bind::IBindable;
    public:
        Graphics(GLFWwindow *window, std::string app_name);
        Graphics(const Graphics &) = delete;
        void operator=(const Graphics &) = delete;
        ~Graphics();

        void wait();
        void clear();
        // void draw_test_triangle();
        void draw_indexed(uint32_t vertex_count);
        void end_frame();
        const std::string &get_app_name() const noexcept;

        void set_perspective(int width, int height, float fov_vertical, float near, float far);
        void set_view(const glm::mat4x4& view);

        const glm::mat4& get_projection() const;
        const glm::mat4& get_view() const;

        //void set_exclusive_mode(bool enabled);
    private:
        void create_swapchain();
        void destroy_swapchain();

        void register_window_callbacks(GLFWwindow* window);
    private:
        std::string app_name{};
        std::stack<std::function<void()>> delete_stack{};
        VkInstance instance{};
        VkDebugUtilsMessengerEXT debug_messenger{};
        VkSurfaceKHR surface{};
        VkDevice device{};
        VkPhysicalDevice physical_device{};

        VkExtent2D draw_extent{};

        bool swapchain_resize_requested{};
        uint32_t swapchain_image_index{};
        VkSwapchainKHR swapchain{};
        VkFormat swapchain_image_format{VK_FORMAT_B8G8R8A8_SRGB};
        VkFormat swapchain_depth_format{VK_FORMAT_D32_SFLOAT};
        std::vector<VkImage> swapchain_images{};
        std::vector<VkImageView> swapchain_image_views{};
        VmaAllocation swapchain_depth_image_allocation{};
        VkImage swapchain_depth_image{};
        VkImageView swapchain_depth_image_view{};

        VkDescriptorPool desc_pool{};

        VkCommandPool cmd_pool{};
        VkCommandBuffer transfer_cmd{}; //Uses trasnfer queue
        

        VkQueue graphics_queue{};
        uint32_t graphics_queue_family{};
        
        static constexpr int FRAMES_IN_FLIGHT = 2;
        struct FrameData
        {
            VkSemaphore present_semaphore{};
            VkSemaphore render_semaphore{};
            VkFence render_fence{};
            VkCommandBuffer cmd{};
        } frame_datas[FRAMES_IN_FLIGHT] {};
        uint32_t frame_index{};

        VmaAllocator allocator{};

        glm::mat4 projection{};
        glm::mat4 view{};
    };
}