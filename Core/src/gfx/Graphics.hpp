#pragma once

#include <string>
#include <vector>
#include <vk_mem_alloc.h>
#include <functional>
#include <stack>
#include <optional>
#include <mutex>
#include <glm/mat4x4.hpp>

#include "Exception.hpp"
#include "data/Swapchain.hpp"
#include "data/DefaultPipeline.hpp"

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
    class Camera;
    class GPUBuffer;
    class CPUBuffer;
    class DescriptorSet;
    class Image;
}

struct GLFWwindow;
namespace gage::gfx
{
    class Graphics
    {
        friend class bind::IBindable;
        friend class data::Swapchain;
        friend class data::DefaultPipeline;
        friend class data::GPUBuffer;
        friend class data::CPUBuffer;
        friend class data::DescriptorSet;
        friend class data::Image;
    public:
        Graphics(GLFWwindow *window, std::string app_name);
        Graphics(const Graphics &) = delete;
        void operator=(const Graphics &) = delete;
        ~Graphics();

        void wait();
        VkCommandBuffer clear(const data::Camera& camera);
        void draw_indexed(uint32_t vertex_count);
        void end_frame();


        const std::string &get_app_name() const noexcept;


        void set_resize(int width, int height);
        void set_resolution_scale(float scale);

        const glm::mat4& get_projection() const;
        const glm::mat4& get_view() const;
        const data::Swapchain& get_swapchain() const;
        const data::DefaultPipeline& get_default_pipeline() const;
        data::DefaultPipeline& get_default_pipeline();
       

        //void set_exclusive_mode(bool enabled);
        VkExtent2D get_scaled_draw_extent();
    private:
        std::mutex uploading_mutex{};
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

        std::optional<data::Swapchain> swapchain{};
        
        VkDescriptorPool desc_pool{};

        VkCommandPool cmd_pool{};
        
        VkQueue queue{};
        uint32_t queue_family{};

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

        //Pipelines
        std::optional<data::DefaultPipeline> default_pipeline{};
    };
}