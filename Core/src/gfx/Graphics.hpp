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

namespace gage::gfx::data
{
    class IData;
    class GUBO;
    class Camera;
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
        void clear(const data::Camera& camera);
        // void draw_test_triangle();
        void draw_indexed(uint32_t vertex_count);
        void end_frame();
        const std::string &get_app_name() const noexcept;


        void set_resize(int width, int height);
        void set_resolution_scale(float scale);

        const glm::mat4& get_projection() const;
        const glm::mat4& get_view() const;
        const data::GUBO& get_global_uniform_buffer() const;
        data::GUBO& get_global_uniform_buffer();


        std::tuple<uint32_t, uint32_t> get_color_image() const;
        std::tuple<uint32_t, uint32_t> get_depth_image() const;

        //void set_exclusive_mode(bool enabled);
        VkExtent2D get_scaled_draw_extent();
    private:
        void create_swapchain();
        void destroy_swapchain();
    private:
        std::string app_name{};
        std::stack<std::function<void()>> delete_stack{};

        static constexpr const char* ENABLED_INSTANCE_EXTENSIONS[] = 
        {
            "VK_KHR_external_memory_capabilities"
        };
        static constexpr const char* ENABLED_DEVICE_EXTENSIONS[] = 
        {
            "VK_EXT_extended_dynamic_state3",
            "VK_KHR_external_memory",
            "VK_KHR_external_memory_fd"
        };
        VkInstance instance{};
        VkDebugUtilsMessengerEXT debug_messenger{};
        VkSurfaceKHR surface{};
        VkDevice device{};
        VkPhysicalDevice physical_device{};


        VkExtent2D draw_extent{};
        VkExtent2D draw_extent_temp{};
        float draw_extent_scale{1.0f};

        bool swapchain_resize_requested{};
        uint32_t swapchain_image_index{};
        VkSwapchainKHR swapchain{};
        VkFormat swapchain_image_format{VK_FORMAT_B8G8R8A8_UNORM};
        VkColorSpaceKHR swapchain_image_color_space{VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
        VkFormat swapchain_depth_format{VK_FORMAT_D32_SFLOAT};
        VkPresentModeKHR swapchain_present_mode{VK_PRESENT_MODE_IMMEDIATE_KHR};
        std::vector<VkImage> swapchain_images{};
        std::vector<VkImageView> swapchain_image_views{};

        VkDeviceMemory swapchain_depth_image_memory{};
        VkDeviceSize swapchain_depth_image_memory_size{};
        VkImage swapchain_depth_image{};
        VkImageView swapchain_depth_image_view{};

        VkDeviceMemory swapchain_color_image_memory{};
        VkDeviceSize swapchain_color_image_memory_size{};
        VkImage swapchain_color_image{};
        VkImageView swapchain_color_image_view{};

        VkDescriptorPool desc_pool{};
        std::unique_ptr<data::GUBO> global_uniform_buffer{};

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
    };
}