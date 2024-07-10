#pragma once

#include <stack>
#include <vk_mem_alloc.h>
#include <vector>
#include <functional>
#include <optional>



#include "Exception.hpp"
#include "data/Swapchain.hpp"

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
    class DefaultPipeline;
}

struct GLFWwindow;
namespace gage::gfx
{
    class Graphics
    {
    public: 
        //Constants
        static constexpr int FRAMES_IN_FLIGHT = 3;

        const std::vector<const char*> ENABLED_INSTANCE_EXTENSIONS
        {
            
        };
        const std::vector<const char*> ENABLED_DEVICE_EXTENSIONS 
        {
            
        };
    private:
        class GlobalUniform;
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
        void clear(const data::Camera &camera);
        void end_frame();


        const std::string &get_app_name() const noexcept;


        void resize(int width, int height);
        void set_resolution_scale(float scale);

        const glm::mat4& get_projection() const;
        const glm::mat4& get_view() const;
        const data::Swapchain& get_swapchain() const;
        const data::DefaultPipeline& get_default_pipeline() const;
        data::DefaultPipeline& get_default_pipeline();
        GlobalUniform& get_global_uniform();

        //void set_exclusive_mode(bool enabled);
        VkExtent2D get_scaled_draw_extent();
    private:
        std::mutex uploading_mutex{};
        std::string app_name{};
        std::stack<std::function<void()>> delete_stack{};

        
        VkInstance instance{};
        VkDebugUtilsMessengerEXT debug_messenger{};
        VkSurfaceKHR surface{};
        VkDevice device{};
        VkPhysicalDevice physical_device{};


        VkExtent2D draw_extent{};
        VkExtent2D draw_extent_temp{};
        float draw_extent_scale{1.0f};
        bool resize_requested{};

        uint32_t swapchain_image_index{};

        std::optional<data::Swapchain> swapchain{};
        
        VkDescriptorPool desc_pool{};
        VkDescriptorSetLayout global_set_layout{};

        VkCommandPool cmd_pool{};
        
        VkQueue queue{};
        uint32_t queue_family{};

        
        struct FrameData
        {
            VkSemaphore present_semaphore{};
            VkSemaphore render_semaphore{};
            VkFence render_fence{};
            VkCommandBuffer cmd{};
            VkDescriptorSet global_set{};
            VkBuffer global_buffer{};
            VmaAllocation global_alloc{};
            VmaAllocationInfo global_alloc_info{};
        } frame_datas[FRAMES_IN_FLIGHT] {};

        struct DirectionalLight
        {
            glm::vec3 direction{0, -1, 0}; float _padding;
            glm::vec3 color{1, 1, 1}; float _padding2;
        };
        struct GlobalUniform
        {
            glm::mat4x4 projection{};
            glm::mat4x4 view{};
            glm::vec3 camera_position{}; float _padding{};
            DirectionalLight directional_light{};
        } global_uniform;
        uint32_t frame_index{};

        VmaAllocator allocator{};

        //Pipelines
        std::unique_ptr<data::DefaultPipeline> default_pipeline{};
    };
}